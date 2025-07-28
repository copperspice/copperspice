/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <avfimagecapturecontrol.h>

#include <avfcameracontrol.h>
#include <avfcameraservice.h>
#include <avfcamerautility.h>
#include <qbuffer.h>
#include <qdebug.h>
#include <qfile.h>
#include <qimagereader.h>
#include <qtconcurrentrun.h>
#include <qurl.h>

#include <qvideoframe_p.h>

AVFImageCaptureControl::AVFImageCaptureControl(AVFCameraService *service, QObject *parent)
   : QCameraImageCaptureControl(parent), m_session(service->session()), m_cameraControl(service->cameraControl()),
     m_ready(false), m_lastCaptureId(0), m_videoConnection(nil)
{
    (void) service;

    m_stillImageOutput = [[AVCaptureStillImageOutput alloc] init];

    NSDictionary *outputSettings = [[NSDictionary alloc] initWithObjectsAndKeys: AVVideoCodecJPEG, AVVideoCodecKey, nil];

    [m_stillImageOutput setOutputSettings:outputSettings];
    [outputSettings release];

    connect(m_cameraControl, SIGNAL(captureModeChanged(QCamera::CaptureModes)), SLOT(updateReadyStatus()));
    connect(m_cameraControl, SIGNAL(statusChanged(QCamera::Status)), SLOT(updateReadyStatus()));

    connect(m_session, SIGNAL(readyToConfigureConnections()), SLOT(updateCaptureConnection()));
    connect(m_cameraControl, SIGNAL(captureModeChanged(QCamera::CaptureModes)), SLOT(updateCaptureConnection()));

    connect(m_session, &AVFCameraSession::newViewfinderFrame,
            this, &AVFImageCaptureControl::onNewViewfinderFrame, Qt::DirectConnection);
}

AVFImageCaptureControl::~AVFImageCaptureControl()
{
}

bool AVFImageCaptureControl::isReadyForCapture() const
{
    return m_videoConnection &&
            m_cameraControl->captureMode().testFlag(QCamera::CaptureStillImage) &&
            m_cameraControl->status() == QCamera::ActiveStatus;
}

void AVFImageCaptureControl::updateReadyStatus()
{
    if (m_ready != isReadyForCapture()) {
        m_ready = !m_ready;
#if defined(CS_SHOW_DEBUG_PLUGINS_AVF)
        qDebug() << "ReadyToCapture status changed:" << m_ready;
#endif
        emit readyForCaptureChanged(m_ready);
    }
}

int AVFImageCaptureControl::capture(const QString &fileName)
{
    ++m_lastCaptureId;

    if (! isReadyForCapture()) {
        QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                  Q_ARG(int, m_lastCaptureId),
                                  Q_ARG(int, QCameraImageCapture::NotReadyError),
                                  Q_ARG(QString, tr("Camera not ready")));
        return m_lastCaptureId;
    }

    QString actualFileName = m_storageLocation.generateFileName(fileName,
                 QCamera::CaptureStillImage, "img_", "jpg");

#if defined(CS_SHOW_DEBUG_PLUGINS_AVF)
    qDebug() << "Capture image to" << actualFileName;
#endif

    CaptureRequest request = { m_lastCaptureId, new QSemaphore };
    m_requestsMutex.lock();
    m_captureRequests.enqueue(request);
    m_requestsMutex.unlock();

    [m_stillImageOutput captureStillImageAsynchronouslyFromConnection:m_videoConnection
                        completionHandler: ^(CMSampleBufferRef imageSampleBuffer, NSError *error) {

        // Wait for the preview to be generated before saving the JPEG
        request.previewReady->acquire();
        delete request.previewReady;

        if (error) {
            QStringList messageParts;
            messageParts << QString::fromUtf8([[error localizedDescription] UTF8String]);
            messageParts << QString::fromUtf8([[error localizedFailureReason] UTF8String]);
            messageParts << QString::fromUtf8([[error localizedRecoverySuggestion] UTF8String]);

            QString errorMessage = messageParts.join(" ");

#if defined(CS_SHOW_DEBUG_PLUGINS_AVF)
            qDebug() << "Image capture failed:" << errorMessage;
#endif

            QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                      Q_ARG(int, request.captureId),
                                      Q_ARG(int, QCameraImageCapture::ResourceError),
                                      Q_ARG(QString, errorMessage));
        } else {
#if defined(CS_SHOW_DEBUG_PLUGINS_AVF)
            qDebug() << "Image capture completed:" << actualFileName;
#endif

            NSData *nsJpgData = [AVCaptureStillImageOutput jpegStillImageNSDataRepresentation:imageSampleBuffer];
            QByteArray jpgData = QByteArray::fromRawData((const char *)[nsJpgData bytes], [nsJpgData length]);

            QFile f(actualFileName);
            if (f.open(QFile::WriteOnly)) {
                if (f.write(jpgData) != -1) {
                    QMetaObject::invokeMethod(this, "imageSaved", Qt::QueuedConnection,
                                              Q_ARG(int, request.captureId),
                                              Q_ARG(QString, actualFileName));
                } else {
                    QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                              Q_ARG(int, request.captureId),
                                              Q_ARG(int, QCameraImageCapture::OutOfSpaceError),
                                              Q_ARG(QString, f.errorString()));
                }

            } else {
                QString errorMessage = tr("Could not open destination file:\n%1").formatArg(actualFileName);
                QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                          Q_ARG(int, request.captureId),
                                          Q_ARG(int, QCameraImageCapture::ResourceError),
                                          Q_ARG(QString, errorMessage));
            }
        }
    }];

    return request.captureId;
}

void AVFImageCaptureControl::onNewViewfinderFrame(const QVideoFrame &frame)
{
    QMutexLocker locker(&m_requestsMutex);

    if (m_captureRequests.isEmpty())
        return;

    CaptureRequest request = m_captureRequests.dequeue();
    Q_EMIT imageExposed(request.captureId);

    QtConcurrent::run(this, &AVFImageCaptureControl::makeCapturePreview,
                  request, frame, 0 /* rotation */);
}

void AVFImageCaptureControl::makeCapturePreview(CaptureRequest request,
      const QVideoFrame &frame, int rotation)
{
    QTransform transform;
    transform.rotate(rotation);

    Q_EMIT imageCaptured(request.captureId, qt_imageFromVideoFrame(frame).transformed(transform));

    request.previewReady->release();
}

void AVFImageCaptureControl::cancelCapture()
{
    //not supported
}

void AVFImageCaptureControl::updateCaptureConnection()
{
    if (m_cameraControl->captureMode().testFlag(QCamera::CaptureStillImage)) {
        AVCaptureSession *captureSession = m_session->captureSession();

        if (![captureSession.outputs containsObject:m_stillImageOutput]) {
            if ([captureSession canAddOutput:m_stillImageOutput]) {
                // Lock the video capture device to make sure the active format is not reset
                const AVFConfigurationLock lock(m_session->videoCaptureDevice());
                [captureSession addOutput:m_stillImageOutput];
                m_videoConnection = [m_stillImageOutput connectionWithMediaType:AVMediaTypeVideo];
                updateReadyStatus();
            }
        } else {
            m_videoConnection = [m_stillImageOutput connectionWithMediaType:AVMediaTypeVideo];
        }
    }
}
