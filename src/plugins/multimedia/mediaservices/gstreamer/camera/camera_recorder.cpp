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

#include <camera_recorder.h>
#include <camera_control.h>
#include <camera_resourcepolicy.h>
#include <camera_audioencoder.h>
#include <camera_videoencoder.h>
#include <camera_container.h>
#include <qdebug.h>

CameraBinRecorder::CameraBinRecorder(CameraBinSession *session)
   : QMediaRecorderControl(session), m_session(session),
     m_state(QMediaRecorder::StoppedState), m_status(QMediaRecorder::UnloadedStatus)
{
   connect(m_session, &CameraBinSession::statusChanged,       this, &CameraBinRecorder::updateStatus);
   connect(m_session, &CameraBinSession::pendingStateChanged, this, &CameraBinRecorder::updateStatus);
   connect(m_session, &CameraBinSession::busyChanged,         this, &CameraBinRecorder::updateStatus);
   connect(m_session, &CameraBinSession::durationChanged,     this, &CameraBinRecorder::durationChanged);
   connect(m_session, &CameraBinSession::mutedChanged,        this, &CameraBinRecorder::mutedChanged);

   connect(m_session->cameraControl()->resourcePolicy(), &CamerabinResourcePolicy::canCaptureChanged,
           this, &CameraBinRecorder::updateStatus);
}

CameraBinRecorder::~CameraBinRecorder()
{
}

QUrl CameraBinRecorder::outputLocation() const
{
   return m_session->outputLocation();
}

bool CameraBinRecorder::setOutputLocation(const QUrl &sink)
{
   m_session->setOutputLocation(sink);
   return true;
}

QMediaRecorder::State CameraBinRecorder::state() const
{
   return m_state;
}

QMediaRecorder::Status CameraBinRecorder::status() const
{
   return m_status;
}

void CameraBinRecorder::updateStatus()
{
   QCamera::Status sessionStatus = m_session->status();

   QMediaRecorder::State oldState = m_state;
   QMediaRecorder::Status oldStatus = m_status;

   if (sessionStatus == QCamera::ActiveStatus &&
         m_session->captureMode().testFlag(QCamera::CaptureVideo)) {

      if (!m_session->cameraControl()->resourcePolicy()->canCapture()) {
         m_status = QMediaRecorder::UnavailableStatus;
         m_state  = QMediaRecorder::StoppedState;
         m_session->stopVideoRecording();

      } else  if (m_state == QMediaRecorder::RecordingState) {
         m_status = QMediaRecorder::RecordingStatus;

      } else {
         m_status = m_session->isBusy() ? QMediaRecorder::FinalizingStatus : QMediaRecorder::LoadedStatus;
      }

   } else {
      if (m_state == QMediaRecorder::RecordingState) {
         m_state = QMediaRecorder::StoppedState;
         m_session->stopVideoRecording();
      }
      m_status = m_session->pendingState() == QCamera::ActiveState
                 && m_session->captureMode().testFlag(QCamera::CaptureVideo)
                 ? QMediaRecorder::LoadingStatus : QMediaRecorder::UnloadedStatus;
   }

   if (m_state != oldState) {
      emit stateChanged(m_state);
   }

   if (m_status != oldStatus) {
      emit statusChanged(m_status);
   }
}

qint64 CameraBinRecorder::duration() const
{
   return m_session->duration();
}


void CameraBinRecorder::applySettings()
{
#ifdef HAVE_GST_ENCODING_PROFILES
   CameraBinContainer *containerControl = m_session->mediaContainerControl();
   CameraBinAudioEncoder *audioEncoderControl = m_session->audioEncodeControl();
   CameraBinVideoEncoder *videoEncoderControl = m_session->videoEncodeControl();

   containerControl->resetActualContainerFormat();
   audioEncoderControl->resetActualSettings();
   videoEncoderControl->resetActualSettings();

   //encodebin doesn't like the encoding profile with ANY caps,
   //if container and codecs are not specified,
   //try to find a commonly used supported combination
   if (containerControl->containerFormat().isEmpty() &&
         audioEncoderControl->audioSettings().codec().isEmpty() &&
         videoEncoderControl->videoSettings().codec().isEmpty()) {

      QList<QStringList> candidates;
      candidates.append(QStringList() << "video/x-matroska" << "video/x-h264" << "audio/mpeg, mpegversion=(int)4");
      candidates.append(QStringList() << "video/webm" << "video/x-vp8" << "audio/x-vorbis");
      candidates.append(QStringList() << "application/ogg" << "video/x-theora" << "audio/x-vorbis");
      candidates.append(QStringList() << "video/quicktime" << "video/x-h264" << "audio/mpeg, mpegversion=(int)4");
      candidates.append(QStringList() << "video/quicktime" << "video/x-h264" << "audio/mpeg");
      candidates.append(QStringList() << "video/x-msvideo" << "video/x-divx" << "audio/mpeg");

      for (const QStringList &candidate : candidates) {
         if (containerControl->supportedContainers().contains(candidate[0]) &&
               videoEncoderControl->supportedVideoCodecs().contains(candidate[1]) &&
               audioEncoderControl->supportedAudioCodecs().contains(candidate[2])) {
            containerControl->setActualContainerFormat(candidate[0]);

            QVideoEncoderSettings videoSettings = videoEncoderControl->videoSettings();
            videoSettings.setCodec(candidate[1]);
            videoEncoderControl->setActualVideoSettings(videoSettings);

            QAudioEncoderSettings audioSettings = audioEncoderControl->audioSettings();
            audioSettings.setCodec(candidate[2]);
            audioEncoderControl->setActualAudioSettings(audioSettings);

            break;
         }
      }
   }
#endif
}

#ifdef HAVE_GST_ENCODING_PROFILES

GstEncodingContainerProfile *CameraBinRecorder::videoProfile()
{
   GstEncodingContainerProfile *containerProfile = m_session->mediaContainerControl()->createProfile();

   if (containerProfile) {
      GstEncodingProfile *audioProfile = m_session->audioEncodeControl()->createProfile();
      GstEncodingProfile *videoProfile = m_session->videoEncodeControl()->createProfile();

      if (audioProfile) {
         if (!gst_encoding_container_profile_add_profile(containerProfile, audioProfile)) {
            gst_encoding_profile_unref(audioProfile);
         }
      }
      if (videoProfile) {
         if (!gst_encoding_container_profile_add_profile(containerProfile, videoProfile)) {
            gst_encoding_profile_unref(videoProfile);
         }
      }
   }

   return containerProfile;
}

#endif

void CameraBinRecorder::setState(QMediaRecorder::State state)
{
   if (m_state == state) {
      return;
   }

   QMediaRecorder::State oldState = m_state;
   QMediaRecorder::Status oldStatus = m_status;

   switch (state) {
      case QMediaRecorder::StoppedState:
         m_state = state;
         m_status = QMediaRecorder::FinalizingStatus;
         m_session->stopVideoRecording();
         break;

      case QMediaRecorder::PausedState:
         emit error(QMediaRecorder::ResourceError, tr("QMediaRecorder::pause() is not supported by camerabin2."));
         break;

      case QMediaRecorder::RecordingState:

         if (m_session->status() != QCamera::ActiveStatus) {
            emit error(QMediaRecorder::ResourceError, tr("Service has not been started"));

         } else if (!m_session->cameraControl()->resourcePolicy()->canCapture()) {
            emit error(QMediaRecorder::ResourceError, tr("Recording permissions are not available"));

         } else {
            m_session->recordVideo();
            m_state = state;
            m_status = QMediaRecorder::RecordingStatus;
            emit actualLocationChanged(m_session->outputLocation());
         }
   }

   if (m_state != oldState) {
      emit stateChanged(m_state);
   }

   if (m_status != oldStatus) {
      emit statusChanged(m_status);
   }
}

bool CameraBinRecorder::isMuted() const
{
   return m_session->isMuted();
}

qreal CameraBinRecorder::volume() const
{
   return 1.0;
}

void CameraBinRecorder::setMuted(bool muted)
{
   m_session->setMuted(muted);
}

void CameraBinRecorder::setVolume(qreal volume)
{
   if (!qFuzzyCompare(volume, qreal(1.0))) {
      qWarning() << "Media service does not support recorder audio gain.";
   }
}
