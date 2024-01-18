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

#include <avfcameradebug.h>
#include <avfmediarecordercontrol.h>
#include <avfcamerasession.h>
#include <avfcameraservice.h>
#include <avfcameracontrol.h>
#include <avfaudioinputselectorcontrol.h>
#include <avfaudioencodersettingscontrol.h>
#include <avfvideoencodersettingscontrol.h>
#include <avfmediacontainercontrol.h>
#include <qurl.h>
#include <qfileinfo.h>
#include <qcameracontrol.h>

@interface AVFMediaRecorderDelegate : NSObject <AVCaptureFileOutputRecordingDelegate>
{
@private
    AVFMediaRecorderControl *m_recorder;
}

- (AVFMediaRecorderDelegate *) initWithRecorder:(AVFMediaRecorderControl*)recorder;

- (void) captureOutput:(AVCaptureFileOutput *)captureOutput
         didStartRecordingToOutputFileAtURL:(NSURL *)fileURL
         fromConnections:(NSArray *)connections;

- (void) captureOutput:(AVCaptureFileOutput *)captureOutput
         didFinishRecordingToOutputFileAtURL:(NSURL *)fileURL
         fromConnections:(NSArray *)connections
         error:(NSError *)error;
@end

@implementation AVFMediaRecorderDelegate

- (AVFMediaRecorderDelegate *) initWithRecorder:(AVFMediaRecorderControl*)recorder
{
    if (!(self = [super init]))
        return nil;

    self->m_recorder = recorder;
    return self;
}

- (void) captureOutput:(AVCaptureFileOutput *)captureOutput
         didStartRecordingToOutputFileAtURL:(NSURL *)fileURL
         fromConnections:(NSArray *)connections
{
    (void) captureOutput;
    (void) fileURL;
    (void) connections;

    QMetaObject::invokeMethod(m_recorder, "handleRecordingStarted", Qt::QueuedConnection);
}

- (void) captureOutput:(AVCaptureFileOutput *)captureOutput
         didFinishRecordingToOutputFileAtURL:(NSURL *)fileURL
         fromConnections:(NSArray *)connections
         error:(NSError *)error
{
    (void) captureOutput;
    (void) fileURL;
    (void) connections;

    if (error) {
        QStringList messageParts;
        messageParts << QString::fromUtf8([[error localizedDescription] UTF8String]);
        messageParts << QString::fromUtf8([[error localizedFailureReason] UTF8String]);
        messageParts << QString::fromUtf8([[error localizedRecoverySuggestion] UTF8String]);

        QString errorMessage = messageParts.join(" ");

        QMetaObject::invokeMethod(m_recorder, "handleRecordingFailed", Qt::QueuedConnection,
                                  Q_ARG(QString, errorMessage));
    } else {
        QMetaObject::invokeMethod(m_recorder, "handleRecordingFinished", Qt::QueuedConnection);
    }
}

@end


AVFMediaRecorderControl::AVFMediaRecorderControl(AVFCameraService *service, QObject *parent)
   : QMediaRecorderControl(parent)
   , m_service(service)
   , m_cameraControl(service->cameraControl())
   , m_audioInputControl(service->audioInputSelectorControl())
   , m_session(service->session())
   , m_connected(false)
   , m_state(QMediaRecorder::StoppedState)
   , m_lastStatus(QMediaRecorder::UnloadedStatus)
   , m_recordingStarted(false)
   , m_recordingFinished(false)
   , m_muted(false)
   , m_volume(1.0)
   , m_audioInput(nil)
   , m_restoreFPS(-1, -1)
{
    m_movieOutput = [[AVCaptureMovieFileOutput alloc] init];
    m_recorderDelagate = [[AVFMediaRecorderDelegate alloc] initWithRecorder:this];

    connect(m_cameraControl, SIGNAL(stateChanged(QCamera::State)), SLOT(updateStatus()));
    connect(m_cameraControl, SIGNAL(statusChanged(QCamera::Status)), SLOT(updateStatus()));
    connect(m_cameraControl, SIGNAL(captureModeChanged(QCamera::CaptureModes)), SLOT(setupSessionForCapture()));
    connect(m_session, SIGNAL(readyToConfigureConnections()), SLOT(setupSessionForCapture()));
    connect(m_session, SIGNAL(stateChanged(QCamera::State)), SLOT(setupSessionForCapture()));
}

AVFMediaRecorderControl::~AVFMediaRecorderControl()
{
    if (m_movieOutput) {
        [m_session->captureSession() removeOutput:m_movieOutput];
        [m_movieOutput release];
    }

    if (m_audioInput) {
        [m_session->captureSession() removeInput:m_audioInput];
        [m_audioInput release];
    }

    [m_recorderDelagate release];
}

QUrl AVFMediaRecorderControl::outputLocation() const
{
    return m_outputLocation;
}

bool AVFMediaRecorderControl::setOutputLocation(const QUrl &location)
{
    m_outputLocation = location;
    return location.scheme() == QLatin1String("file") || location.scheme().isEmpty();
}

QMediaRecorder::State AVFMediaRecorderControl::state() const
{
    return m_state;
}

QMediaRecorder::Status AVFMediaRecorderControl::status() const
{
    QMediaRecorder::Status status = m_lastStatus;
    //bool videoEnabled = m_cameraControl->captureMode().testFlag(QCamera::CaptureVideo);

    if (m_cameraControl->status() == QCamera::ActiveStatus && m_connected) {
        if (m_state == QMediaRecorder::StoppedState) {
            if (m_recordingStarted && !m_recordingFinished)
                status = QMediaRecorder::FinalizingStatus;
            else
                status = QMediaRecorder::LoadedStatus;
        } else {
            status = m_recordingStarted ? QMediaRecorder::RecordingStatus :
                                            QMediaRecorder::StartingStatus;
        }
    } else {
        //camera not started yet
        status = m_cameraControl->state() == QCamera::ActiveState && m_connected ?
                    QMediaRecorder::LoadingStatus:
                    QMediaRecorder::UnloadedStatus;
    }

    return status;
}

void AVFMediaRecorderControl::updateStatus()
{
    QMediaRecorder::Status newStatus = status();

    if (m_lastStatus != newStatus) {
        qDebugCamera() << "Camera recorder status changed: " << m_lastStatus << " -> " << newStatus;
        m_lastStatus = newStatus;
        Q_EMIT statusChanged(m_lastStatus);
    }
}


qint64 AVFMediaRecorderControl::duration() const
{
    if (!m_movieOutput)
        return 0;

    return qint64(CMTimeGetSeconds(m_movieOutput.recordedDuration) * 1000);
}

bool AVFMediaRecorderControl::isMuted() const
{
    return m_muted;
}

qreal AVFMediaRecorderControl::volume() const
{
    return m_volume;
}

void AVFMediaRecorderControl::applySettings()
{
    if (m_state != QMediaRecorder::StoppedState
            || (m_session->state() != QCamera::ActiveState && m_session->state() != QCamera::LoadedState)
            || !m_service->cameraControl()->captureMode().testFlag(QCamera::CaptureVideo)) {
        return;
    }

    // Configure audio settings
    [m_movieOutput setOutputSettings:m_service->audioEncoderSettingsControl()->applySettings()
                   forConnection:[m_movieOutput connectionWithMediaType:AVMediaTypeAudio]];

    // Configure video settings
    AVCaptureConnection *videoConnection = [m_movieOutput connectionWithMediaType:AVMediaTypeVideo];
    NSDictionary *videoSettings = m_service->videoEncoderSettingsControl()->applySettings(videoConnection);

    const AVFConfigurationLock lock(m_session->videoCaptureDevice()); // prevents activeFormat from being overridden

    [m_movieOutput setOutputSettings:videoSettings forConnection:videoConnection];
}

void AVFMediaRecorderControl::unapplySettings()
{
    m_service->audioEncoderSettingsControl()->unapplySettings();
    m_service->videoEncoderSettingsControl()->unapplySettings([m_movieOutput connectionWithMediaType:AVMediaTypeVideo]);
}

void AVFMediaRecorderControl::setState(QMediaRecorder::State state)
{
    if (m_state == state)
        return;

    qDebugCamera() << Q_FUNC_INFO << m_state << " -> " << state;

    switch (state) {
    case QMediaRecorder::RecordingState:
    {
        if (m_connected) {
            QString outputLocationPath = m_outputLocation.scheme() == QLatin1String("file") ?
                        m_outputLocation.path() : m_outputLocation.toString();

            QString extension = m_service->mediaContainerControl()->containerFormat();

            QUrl actualLocation = QUrl::fromLocalFile(
                        m_storageLocation.generateFileName(outputLocationPath,
                                                           QCamera::CaptureVideo,
                                                           QLatin1String("clip_"),
                                                           extension));

            qDebugCamera() << "Video capture location:" << actualLocation.toString();

            applySettings();

            [m_movieOutput startRecordingToOutputFileURL:actualLocation.toNSURL()
                           recordingDelegate:m_recorderDelagate];

            m_state = QMediaRecorder::RecordingState;
            m_recordingStarted = false;
            m_recordingFinished = false;

            Q_EMIT actualLocationChanged(actualLocation);
        } else {
            Q_EMIT error(QMediaRecorder::FormatError, tr("Recorder not configured"));
        }

    } break;
    case QMediaRecorder::PausedState:
    {
        Q_EMIT error(QMediaRecorder::FormatError, tr("Recording pause not supported"));
        return;
    } break;
    case QMediaRecorder::StoppedState:
    {
        m_state = QMediaRecorder::StoppedState;
        [m_movieOutput stopRecording];
        unapplySettings();
    }
    }

    updateStatus();
    Q_EMIT stateChanged(m_state);
}

void AVFMediaRecorderControl::setMuted(bool muted)
{
    if (m_muted != muted) {
        m_muted = muted;
        Q_EMIT mutedChanged(muted);
    }
}

void AVFMediaRecorderControl::setVolume(qreal volume)
{
    if (m_volume != volume) {
        m_volume = volume;
        Q_EMIT volumeChanged(volume);
    }
}

void AVFMediaRecorderControl::handleRecordingStarted()
{
    m_recordingStarted = true;
    updateStatus();
}

void AVFMediaRecorderControl::handleRecordingFinished()
{
    m_recordingFinished = true;
    updateStatus();
}

void AVFMediaRecorderControl::handleRecordingFailed(const QString &message)
{
    m_recordingFinished = true;
    if (m_state != QMediaRecorder::StoppedState) {
        m_state = QMediaRecorder::StoppedState;
        Q_EMIT stateChanged(m_state);
    }
    updateStatus();

    Q_EMIT error(QMediaRecorder::ResourceError, message);
}

void AVFMediaRecorderControl::setupSessionForCapture()
{
    //adding movie output causes high CPU usage even when while recording is not active,
    //connect it only while video capture mode is enabled.
    // Similarly, connect the Audio input only in that mode, since it's only necessary
    // when recording anyway. Adding an Audio input will trigger the microphone permission
    // request on iOS, but it shoudn't do so until we actually try to record.
    AVCaptureSession *captureSession = m_session->captureSession();

    if (! m_connected
            && m_cameraControl->captureMode().testFlag(QCamera::CaptureVideo)
            && m_session->state() != QCamera::UnloadedState) {

        // Lock the video capture device to make sure the active format is not reset
        const AVFConfigurationLock lock(m_session->videoCaptureDevice());

        // Add audio input
        // Allow recording even if something wrong happens with the audio input initialization
        AVCaptureDevice *audioDevice = m_audioInputControl->createCaptureDevice();

        if (! audioDevice) {
            qWarning("No audio input device available");

        } else {
            NSError *error = nil;
            m_audioInput = [AVCaptureDeviceInput deviceInputWithDevice:audioDevice error:&error];

            if (!m_audioInput) {
                qWarning() << "Failed to create audio device input";
            } else if (![captureSession canAddInput:m_audioInput]) {
                qWarning() << "Unable to connect the audio input";
                m_audioInput = nullptr;
            } else {
                [m_audioInput retain];
                [captureSession addInput:m_audioInput];
            }
        }

        if ([captureSession canAddOutput:m_movieOutput]) {
            [captureSession addOutput:m_movieOutput];
            m_connected = true;
        } else {
            Q_EMIT error(QMediaRecorder::ResourceError, tr("Could not connect the video recorder"));
            qWarning() << "Unable to  connect the video recorder";
        }
    } else if (m_connected
               && (!m_cameraControl->captureMode().testFlag(QCamera::CaptureVideo)
                   || m_session->state() == QCamera::UnloadedState)) {

        // Lock the video capture device to make sure the active format is not reset
        const AVFConfigurationLock lock(m_session->videoCaptureDevice());

        [captureSession removeOutput:m_movieOutput];

        if (m_audioInput) {
            [captureSession removeInput:m_audioInput];
            [m_audioInput release];
            m_audioInput = nil;
        }

        m_connected = false;
    }

    updateStatus();
}
