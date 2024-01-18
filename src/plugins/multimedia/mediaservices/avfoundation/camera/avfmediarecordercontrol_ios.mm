/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

#include "avfmediarecordercontrol_ios.h"
#include "avfcamerarenderercontrol.h"
#include "avfcamerasession.h"
#include "avfcameracontrol.h"
#include "avfcameraservice.h"
#include "avfcameradebug.h"
#include "avfaudioencodersettingscontrol.h"
#include "avfvideoencodersettingscontrol.h"
#include "avfmediacontainercontrol.h"
#include "avfcamerautility.h"
#include <qdebug.h>

namespace {

bool qt_is_writable_file_URL(NSURL *fileURL)
{
    Q_ASSERT(fileURL);

    if (![fileURL isFileURL])
        return false;

    if (NSString *path = [[fileURL path] stringByExpandingTildeInPath]) {
        return [[NSFileManager defaultManager]
                isWritableFileAtPath:[path stringByDeletingLastPathComponent]];
    }

    return false;
}

bool qt_file_exists(NSURL *fileURL)
{
    Q_ASSERT(fileURL);

    if (NSString *path = [[fileURL path] stringByExpandingTildeInPath])
        return [[NSFileManager defaultManager] fileExistsAtPath:path];

    return false;
}

}

AVFMediaRecorderControlIOS::AVFMediaRecorderControlIOS(AVFCameraService *service, QObject *parent)
    : QMediaRecorderControl(parent)
    , m_service(service)
    , m_state(QMediaRecorder::StoppedState)
    , m_lastStatus(QMediaRecorder::UnloadedStatus)
    , m_audioSettings(nil)
    , m_videoSettings(nil)
{
    Q_ASSERT(service);

    m_writer.reset([[AVFMediaAssetWriter alloc] initWithDelegate:this]);
    if (!m_writer) {
        qDebugCamera() << Q_FUNC_INFO << "failed to create an asset writer";
        return;
    }

    AVFCameraControl *cameraControl = m_service->cameraControl();
    if (!cameraControl) {
        qDebugCamera() << Q_FUNC_INFO << "camera control is nil";
        return;
    }

    connect(cameraControl, SIGNAL(captureModeChanged(QCamera::CaptureModes)),
                           SLOT(captureModeChanged(QCamera::CaptureModes)));
    connect(cameraControl, SIGNAL(statusChanged(QCamera::Status)),
                           SLOT(cameraStatusChanged(QCamera::Status)));
}

AVFMediaRecorderControlIOS::~AVFMediaRecorderControlIOS()
{
    [m_writer abort];

    if (m_audioSettings)
        [m_audioSettings release];
    if (m_videoSettings)
        [m_videoSettings release];
}

QUrl AVFMediaRecorderControlIOS::outputLocation() const
{
    return m_outputLocation;
}

bool AVFMediaRecorderControlIOS::setOutputLocation(const QUrl &location)
{
    m_outputLocation = location;
    return location.scheme() == QLatin1String("file") || location.scheme().isEmpty();
}

QMediaRecorder::State AVFMediaRecorderControlIOS::state() const
{
    return m_state;
}

QMediaRecorder::Status AVFMediaRecorderControlIOS::status() const
{
    return m_lastStatus;
}

qint64 AVFMediaRecorderControlIOS::duration() const
{
    return m_writer.data()->m_durationInMs.load();
}

bool AVFMediaRecorderControlIOS::isMuted() const
{
    return false;
}

qreal AVFMediaRecorderControlIOS::volume() const
{
    return 1.;
}

void AVFMediaRecorderControlIOS::applySettings()
{
    AVFCameraSession *session = m_service->session();
    if (!session)
        return;

    if (m_state != QMediaRecorder::StoppedState
            || (session->state() != QCamera::ActiveState && session->state() != QCamera::LoadedState)
            || !m_service->cameraControl()->captureMode().testFlag(QCamera::CaptureVideo)) {
        return;
    }

    // audio settings
    m_audioSettings = m_service->audioEncoderSettingsControl()->applySettings();
    if (m_audioSettings)
        [m_audioSettings retain];

    // video settings
    AVCaptureConnection *conn = [m_service->videoOutput()->videoDataOutput() connectionWithMediaType:AVMediaTypeVideo];
    m_videoSettings = m_service->videoEncoderSettingsControl()->applySettings(conn);
    if (m_videoSettings)
        [m_videoSettings retain];
}

void AVFMediaRecorderControlIOS::unapplySettings()
{
    m_service->audioEncoderSettingsControl()->unapplySettings();

    AVCaptureConnection *conn = [m_service->videoOutput()->videoDataOutput() connectionWithMediaType:AVMediaTypeVideo];
    m_service->videoEncoderSettingsControl()->unapplySettings(conn);

    if (m_audioSettings) {
        [m_audioSettings release];
        m_audioSettings = nil;
    }
    if (m_videoSettings) {
        [m_videoSettings release];
        m_videoSettings = nil;
    }
}

void AVFMediaRecorderControlIOS::setState(QMediaRecorder::State state)
{
    Q_ASSERT(m_service->session()
             && m_service->session()->captureSession());

    if (!m_writer) {
        qDebugCamera() << Q_FUNC_INFO << "Invalid recorder";
        return;
    }

    if (state == m_state)
        return;

    switch (state) {
    case QMediaRecorder::RecordingState:
    {
        AVFCameraControl *cameraControl = m_service->cameraControl();
        Q_ASSERT(cameraControl);

        if (!(cameraControl->captureMode() & QCamera::CaptureVideo)) {
            qDebugCamera() << Q_FUNC_INFO << "wrong capture mode, CaptureVideo expected";
            Q_EMIT error(QMediaRecorder::ResourceError, tr("Failed to start recording"));
            return;
        }

        if (cameraControl->status() != QCamera::ActiveStatus) {
            qDebugCamera() << Q_FUNC_INFO << "can not start record while camera is not active";
            Q_EMIT error(QMediaRecorder::ResourceError, tr("Failed to start recording"));
            return;
        }

        const QString path(m_outputLocation.scheme() == QLatin1String("file") ?
                           m_outputLocation.path() : m_outputLocation.toString());
        const QUrl fileURL(QUrl::fromLocalFile(m_storageLocation.generateFileName(path, QCamera::CaptureVideo,
                           QLatin1String("clip_"),
                           m_service->mediaContainerControl()->containerFormat())));

        NSURL *nsFileURL = fileURL.toNSURL();
        if (!nsFileURL) {
            qWarning() << Q_FUNC_INFO << "invalid output URL:" << fileURL;
            Q_EMIT error(QMediaRecorder::ResourceError, tr("Invalid output file URL"));
            return;
        }
        if (!qt_is_writable_file_URL(nsFileURL)) {
            qWarning() << Q_FUNC_INFO << "invalid output URL:" << fileURL
                       << "(the location is not writable)";
            Q_EMIT error(QMediaRecorder::ResourceError, tr("Non-writeable file location"));
            return;
        }
        if (qt_file_exists(nsFileURL)) {
            // We test for/handle this error here since AWAssetWriter will raise an
            // Objective-C exception, which is not good at all.
            qWarning() << Q_FUNC_INFO << "invalid output URL:" << fileURL
                       << "(file already exists)";
            Q_EMIT error(QMediaRecorder::ResourceError, tr("File already exists"));
            return;
        }

        AVCaptureSession *session = m_service->session()->captureSession();
        // We stop session now so that no more frames for renderer's queue
        // generated, will restart in assetWriterStarted.
        [session stopRunning];

        applySettings();

        // Make sure the video is recorded in device orientation.
        // The top of the video will match the side of the device which is on top
        // when recording starts (regardless of the UI orientation).
        AVFCameraInfo cameraInfo = m_service->session()->activeCameraInfo();
        int screenOrientation = 360 - m_orientationHandler.currentOrientation();
        float rotation = 0;
        if (cameraInfo.position == QCamera::FrontFace)
            rotation = (screenOrientation + cameraInfo.orientation) % 360;
        else
            rotation = (screenOrientation + (360 - cameraInfo.orientation)) % 360;

        // convert to radians
        rotation *= M_PI / 180.f;

        if ([m_writer setupWithFileURL:nsFileURL
                      cameraService:m_service
                      audioSettings:m_audioSettings
                      videoSettings:m_videoSettings
                      transform:CGAffineTransformMakeRotation(rotation)]) {

            m_state = QMediaRecorder::RecordingState;
            m_lastStatus = QMediaRecorder::StartingStatus;

            Q_EMIT actualLocationChanged(fileURL);
            Q_EMIT stateChanged(m_state);
            Q_EMIT statusChanged(m_lastStatus);

            // Apple recommends to call startRunning and do all
            // setup on a special queue, and that's what we had
            // initially (dispatch_async to writerQueue). Unfortunately,
            // writer's queue is not the only queue/thread that can
            // access/modify the session, and as a result we have
            // all possible data/race-conditions with Obj-C exceptions
            // at best and something worse in general.
            // Now we try to only modify session on the same thread.
            [m_writer start];
        } else {
            [session startRunning];
            Q_EMIT error(QMediaRecorder::FormatError, tr("Failed to start recording"));
        }
    } break;
    case QMediaRecorder::PausedState:
    {
        Q_EMIT error(QMediaRecorder::FormatError, tr("Recording pause not supported"));
        return;
    } break;
    case QMediaRecorder::StoppedState:
    {
        // Do not check the camera status, we can stop if we started.
        stopWriter();
    }
    }
}

void AVFMediaRecorderControlIOS::setMuted(bool muted)
{
    (void) muted;
    qDebugCamera() << Q_FUNC_INFO << "not implemented";
}

void AVFMediaRecorderControlIOS::setVolume(qreal volume)
{
    (void) volume;
    qDebugCamera() << Q_FUNC_INFO << "not implemented";
}

void AVFMediaRecorderControlIOS::assetWriterStarted()
{
    m_lastStatus = QMediaRecorder::RecordingStatus;
    Q_EMIT statusChanged(QMediaRecorder::RecordingStatus);
}

void AVFMediaRecorderControlIOS::assetWriterFinished()
{
    AVFCameraControl *cameraControl = m_service->cameraControl();
    Q_ASSERT(cameraControl);

    const QMediaRecorder::Status lastStatus = m_lastStatus;

    if (cameraControl->captureMode() & QCamera::CaptureVideo)
        m_lastStatus = QMediaRecorder::LoadedStatus;
    else
        m_lastStatus = QMediaRecorder::UnloadedStatus;

    unapplySettings();

    m_service->videoOutput()->resetCaptureDelegate();
    [m_service->session()->captureSession() startRunning];

    if (m_lastStatus != lastStatus)
        Q_EMIT statusChanged(m_lastStatus);
}

void AVFMediaRecorderControlIOS::captureModeChanged(QCamera::CaptureModes newMode)
{
    AVFCameraControl *cameraControl = m_service->cameraControl();
    Q_ASSERT(cameraControl);

    const QMediaRecorder::Status lastStatus = m_lastStatus;

    if (newMode & QCamera::CaptureVideo) {
        if (cameraControl->status() == QCamera::ActiveStatus)
            m_lastStatus = QMediaRecorder::LoadedStatus;
    } else {
        if (m_lastStatus == QMediaRecorder::RecordingStatus)
           return stopWriter();
        else
            m_lastStatus = QMediaRecorder::UnloadedStatus;
    }

    if (m_lastStatus != lastStatus)
        Q_EMIT statusChanged(m_lastStatus);
}

void AVFMediaRecorderControlIOS::cameraStatusChanged(QCamera::Status newStatus)
{
    AVFCameraControl *cameraControl = m_service->cameraControl();
    Q_ASSERT(cameraControl);

    const QMediaRecorder::Status lastStatus = m_lastStatus;
    const bool isCapture = cameraControl->captureMode() & QCamera::CaptureVideo;
    if (newStatus == QCamera::StartingStatus) {
        if (isCapture && m_lastStatus == QMediaRecorder::UnloadedStatus)
            m_lastStatus = QMediaRecorder::LoadingStatus;
    } else if (newStatus == QCamera::ActiveStatus) {
        if (isCapture && m_lastStatus == QMediaRecorder::LoadingStatus)
            m_lastStatus = QMediaRecorder::LoadedStatus;
    } else {
        if (m_lastStatus == QMediaRecorder::RecordingStatus)
            return stopWriter();
        if (newStatus == QCamera::UnloadedStatus)
            m_lastStatus = QMediaRecorder::UnloadedStatus;
    }

    if (lastStatus != m_lastStatus)
        Q_EMIT statusChanged(m_lastStatus);
}

void AVFMediaRecorderControlIOS::stopWriter()
{
    if (m_lastStatus == QMediaRecorder::RecordingStatus) {
        m_state = QMediaRecorder::StoppedState;
        m_lastStatus = QMediaRecorder::FinalizingStatus;

        Q_EMIT stateChanged(m_state);
        Q_EMIT statusChanged(m_lastStatus);

        [m_writer stop];
    }
}

