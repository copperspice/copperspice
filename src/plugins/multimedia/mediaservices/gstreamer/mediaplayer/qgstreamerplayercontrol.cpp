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

#include <qgstreamerplayercontrol.h>

#include <qdir.h>
#include <qsocketnotifier.h>
#include <qurl.h>
#include <qdebug.h>
#include <qgstreamerplayersession.h>

#include <qmediaplaylistnavigator_p.h>
#include <qmediaresourcepolicy_p.h>
#include <qmediaresourceset_p.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// #define DEBUG_PLAYBIN

QGstreamerPlayerControl::QGstreamerPlayerControl(QGstreamerPlayerSession *session, QObject *parent)
   : QMediaPlayerControl(parent), m_session(session), m_userRequestedState(QMediaPlayer::StoppedState)
   , m_currentState(QMediaPlayer::StoppedState), m_mediaStatus(QMediaPlayer::NoMedia), m_bufferProgress(-1)
   , m_pendingSeekPosition(-1), m_setMediaPending(false), m_stream(nullptr)
{
   m_resources = QMediaResourcePolicy::createResourceSet<QMediaPlayerResourceSetInterface>();
   Q_ASSERT(m_resources);

   connect(m_session, &QGstreamerPlayerSession::positionChanged,          this, &QGstreamerPlayerControl::positionChanged);
   connect(m_session, &QGstreamerPlayerSession::durationChanged,          this, &QGstreamerPlayerControl::durationChanged);
   connect(m_session, &QGstreamerPlayerSession::mutedStateChanged,        this, &QGstreamerPlayerControl::mutedChanged);
   connect(m_session, &QGstreamerPlayerSession::volumeChanged,            this, &QGstreamerPlayerControl::volumeChanged);

   connect(m_session, &QGstreamerPlayerSession::stateChanged,             this, &QGstreamerPlayerControl::updateSessionState);
   connect(m_session, &QGstreamerPlayerSession::bufferingProgressChanged, this, &QGstreamerPlayerControl::setBufferProgress);
   connect(m_session, &QGstreamerPlayerSession::playbackFinished,         this, &QGstreamerPlayerControl::processEOS);
   connect(m_session, &QGstreamerPlayerSession::audioAvailableChanged,    this, &QGstreamerPlayerControl::audioAvailableChanged);
   connect(m_session, &QGstreamerPlayerSession::videoAvailableChanged,    this, &QGstreamerPlayerControl::videoAvailableChanged);
   connect(m_session, &QGstreamerPlayerSession::seekableChanged,          this, &QGstreamerPlayerControl::seekableChanged);
   connect(m_session, &QGstreamerPlayerSession::error,                    this, &QGstreamerPlayerControl::error);
   connect(m_session, &QGstreamerPlayerSession::invalidMedia,             this, &QGstreamerPlayerControl::handleInvalidMedia);
   connect(m_session, &QGstreamerPlayerSession::playbackRateChanged,      this, &QGstreamerPlayerControl::playbackRateChanged);

   connect(m_resources, &QMediaPlayerResourceSetInterface::resourcesGranted, this, &QGstreamerPlayerControl::handleResourcesGranted);

   // denied signal should be queued to have correct state update process,
   // since in playOrPause, when acquire is call on resource set, it may trigger a resourcesDenied signal immediately,
   // so handleResourcesDenied should be processed later, otherwise it will be overwritten by state update later in playOrPause.

   connect(m_resources, &QMediaPlayerResourceSetInterface::resourcesDenied,  this, &QGstreamerPlayerControl::handleResourcesDenied, Qt::QueuedConnection);
   connect(m_resources, &QMediaPlayerResourceSetInterface::resourcesLost,    this, &QGstreamerPlayerControl::handleResourcesLost);
}

QGstreamerPlayerControl::~QGstreamerPlayerControl()
{
   QMediaResourcePolicy::destroyResourceSet(m_resources);
}

QMediaPlayerResourceSetInterface *QGstreamerPlayerControl::resources() const
{
   return m_resources;
}

qint64 QGstreamerPlayerControl::position() const
{
   return m_pendingSeekPosition != -1 ? m_pendingSeekPosition : m_session->position();
}

qint64 QGstreamerPlayerControl::duration() const
{
   return m_session->duration();
}

QMediaPlayer::State QGstreamerPlayerControl::state() const
{
   return m_currentState;
}

QMediaPlayer::MediaStatus QGstreamerPlayerControl::mediaStatus() const
{
   return m_mediaStatus;
}

int QGstreamerPlayerControl::bufferStatus() const
{
   if (m_bufferProgress == -1) {
      return m_session->state() == QMediaPlayer::StoppedState ? 0 : 100;
   } else {
      return m_bufferProgress;
   }
}

int QGstreamerPlayerControl::volume() const
{
   return m_session->volume();
}

bool QGstreamerPlayerControl::isMuted() const
{
   return m_session->isMuted();
}

bool QGstreamerPlayerControl::isSeekable() const
{
   return m_session->isSeekable();
}

QMediaTimeRange QGstreamerPlayerControl::availablePlaybackRanges() const
{
   return m_session->availablePlaybackRanges();
}

qreal QGstreamerPlayerControl::playbackRate() const
{
   return m_session->playbackRate();
}

void QGstreamerPlayerControl::setPlaybackRate(qreal rate)
{
   m_session->setPlaybackRate(rate);
}

void QGstreamerPlayerControl::setPosition(qint64 pos)
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO << pos / 1000.0;
#endif

   pushState();

   if (m_mediaStatus == QMediaPlayer::EndOfMedia) {
      m_mediaStatus = QMediaPlayer::LoadedMedia;
   }

   if (m_currentState == QMediaPlayer::StoppedState) {
      m_pendingSeekPosition = pos;
      emit positionChanged(m_pendingSeekPosition);

   } else if (m_session->isSeekable()) {
      m_session->showPrerollFrames(true);
      m_session->seek(pos);
      m_pendingSeekPosition = -1;

   } else if (m_session->state() == QMediaPlayer::StoppedState) {
      m_pendingSeekPosition = pos;
      emit positionChanged(m_pendingSeekPosition);

   } else if (m_pendingSeekPosition != -1) {
      m_pendingSeekPosition = -1;
      emit positionChanged(m_pendingSeekPosition);

   }

   popAndNotifyState();
}

void QGstreamerPlayerControl::play()
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO;
#endif

   // m_userRequestedState is needed to know that we need to resume playback when resource-policy
   // regranted the resources after lost, since m_currentState will become paused when resources are lost

   m_userRequestedState = QMediaPlayer::PlayingState;
   playOrPause(QMediaPlayer::PlayingState);
}

void QGstreamerPlayerControl::pause()
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO;
#endif
   m_userRequestedState = QMediaPlayer::PausedState;

   playOrPause(QMediaPlayer::PausedState);
}

void QGstreamerPlayerControl::playOrPause(QMediaPlayer::State newState)
{
   if (m_mediaStatus == QMediaPlayer::NoMedia) {
      return;
   }

   pushState();

   if (m_setMediaPending) {
      m_mediaStatus = QMediaPlayer::LoadingMedia;
      setMedia(m_currentResource, m_stream);
   }

   if (m_mediaStatus == QMediaPlayer::EndOfMedia && m_pendingSeekPosition == -1) {
      m_pendingSeekPosition = 0;
   }

   if (! m_resources->isGranted()) {
      m_resources->acquire();
   }

   if (m_resources->isGranted()) {
      // show prerolled frame if switching from stopped state
      if (m_pendingSeekPosition == -1) {
         m_session->showPrerollFrames(true);

      } else if (m_session->state() == QMediaPlayer::StoppedState) {
         // Do not evaluate the next two conditions

      } else if (m_session->isSeekable()) {
         m_session->pause();
         m_session->showPrerollFrames(true);
         m_session->seek(m_pendingSeekPosition);
         m_pendingSeekPosition = -1;

      } else {
         m_pendingSeekPosition = -1;

      }

      bool ok = false;

      // To prevent displaying the first video frame when playback is resumed the
      // pipeline is paused instead of playing, seeked to requested position, and after
      // seeking is finished (position updated) playback is restarted with show-preroll-frame enabled.

      if (newState == QMediaPlayer::PlayingState && m_pendingSeekPosition == -1) {
         ok = m_session->play();

      } else {
         ok = m_session->pause();
      }

      if (! ok) {
         newState = QMediaPlayer::StoppedState;
      }
   }

   if (m_mediaStatus == QMediaPlayer::InvalidMedia) {
      m_mediaStatus = QMediaPlayer::LoadingMedia;
   }

   m_currentState = newState;

   if (m_mediaStatus == QMediaPlayer::EndOfMedia || m_mediaStatus == QMediaPlayer::LoadedMedia) {
      if (m_bufferProgress == -1 || m_bufferProgress == 100) {
         m_mediaStatus = QMediaPlayer::BufferedMedia;
      } else {
         m_mediaStatus = QMediaPlayer::BufferingMedia;
      }
   }

   popAndNotifyState();

   emit positionChanged(position());
}

void QGstreamerPlayerControl::stop()
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO;
#endif

   m_userRequestedState = QMediaPlayer::StoppedState;

   pushState();

   if (m_currentState != QMediaPlayer::StoppedState) {
      m_currentState = QMediaPlayer::StoppedState;
      m_session->showPrerollFrames(false); // stop showing prerolled frames in stop state
      if (m_resources->isGranted()) {
         m_session->pause();
      }

      if (m_mediaStatus != QMediaPlayer::EndOfMedia) {
         m_pendingSeekPosition = 0;
         emit positionChanged(position());
      }
   }

   popAndNotifyState();
}

void QGstreamerPlayerControl::setVolume(int volume)
{
   m_session->setVolume(volume);
}

void QGstreamerPlayerControl::setMuted(bool muted)
{
   m_session->setMuted(muted);
}

QMediaContent QGstreamerPlayerControl::media() const
{
   return m_currentResource;
}

const QIODevice *QGstreamerPlayerControl::mediaStream() const
{
   return m_stream;
}

void QGstreamerPlayerControl::setMedia(const QMediaContent &content, QIODevice *stream)
{
#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO;
#endif

   pushState();

   m_currentState = QMediaPlayer::StoppedState;
   QMediaContent oldMedia = m_currentResource;
   m_pendingSeekPosition  = 0;
   m_session->showPrerollFrames(false); // do not show prerolled frames until pause() or play() explicitly called
   m_setMediaPending = false;

   if (! content.isNull() || stream) {
      if (!m_resources->isGranted()) {
         m_resources->acquire();
      }

   } else {
      m_resources->release();
   }

   m_session->stop();

   bool userStreamValid = false;

   if (m_bufferProgress != -1) {
      m_bufferProgress = -1;
      emit bufferStatusChanged(0);
   }

   m_currentResource = content;
   m_stream = stream;

   QNetworkRequest request;

   if (m_stream) {
      userStreamValid = stream->isOpen() && m_stream->isReadable();
      request = content.canonicalRequest();
   } else if (!content.isNull()) {
      request = content.canonicalRequest();
   }

#if defined(HAVE_GST_APPSRC)
   if (m_stream) {
      if (userStreamValid) {
         m_session->loadFromStream(request, m_stream);

      } else {
         m_mediaStatus = QMediaPlayer::InvalidMedia;
         emit error(QMediaPlayer::FormatError, tr("Attempting to play invalid user stream"));

         if (m_currentState != QMediaPlayer::PlayingState) {
            m_resources->release();
         }

         popAndNotifyState();
         return;
      }
   } else {
      m_session->loadFromUri(request);
   }

#else
   (void) userStreamValid;
   m_session->loadFromUri(request);
#endif

#if defined(HAVE_GST_APPSRC)
   if (!request.url().isEmpty() || userStreamValid) {
#else
   if (!request.url().isEmpty()) {
#endif

      m_mediaStatus = QMediaPlayer::LoadingMedia;
      m_session->pause();

   } else {
      m_mediaStatus = QMediaPlayer::NoMedia;
      setBufferProgress(0);
   }

   if (m_currentResource != oldMedia) {
      emit mediaChanged(m_currentResource);
   }

   emit positionChanged(position());

   if (content.isNull() && !stream) {
      m_resources->release();
   }

   popAndNotifyState();
}

void QGstreamerPlayerControl::setVideoOutput(QObject *output)
{
   m_session->setVideoRenderer(output);
}

bool QGstreamerPlayerControl::isAudioAvailable() const
{
   return m_session->isAudioAvailable();
}

bool QGstreamerPlayerControl::isVideoAvailable() const
{
   return m_session->isVideoAvailable();
}

void QGstreamerPlayerControl::updateSessionState(QMediaPlayer::State state)
{
   pushState();

   if (state == QMediaPlayer::StoppedState) {
      m_session->showPrerollFrames(false);
      m_currentState = QMediaPlayer::StoppedState;
   }

   if (state == QMediaPlayer::PausedState && m_currentState != QMediaPlayer::StoppedState) {
      if (m_pendingSeekPosition != -1 && m_session->isSeekable()) {
         m_session->showPrerollFrames(true);
         m_session->seek(m_pendingSeekPosition);
      }
      m_pendingSeekPosition = -1;

      if (m_currentState == QMediaPlayer::PlayingState) {
         m_session->play();
      }
   }

   updateMediaStatus();

   popAndNotifyState();
}

void QGstreamerPlayerControl::updateMediaStatus()
{
   pushState();
   QMediaPlayer::MediaStatus oldStatus = m_mediaStatus;

   switch (m_session->state()) {
      case QMediaPlayer::StoppedState:
         if (m_currentResource.isNull()) {
            m_mediaStatus = QMediaPlayer::NoMedia;

         } else if (oldStatus != QMediaPlayer::InvalidMedia) {
            m_mediaStatus = QMediaPlayer::LoadingMedia;
         }
         break;

      case QMediaPlayer::PlayingState:
      case QMediaPlayer::PausedState:
         if (m_currentState == QMediaPlayer::StoppedState) {
            m_mediaStatus = QMediaPlayer::LoadedMedia;
         } else {
            if (m_bufferProgress == -1 || m_bufferProgress == 100) {
               m_mediaStatus = QMediaPlayer::BufferedMedia;
            } else {
               m_mediaStatus = QMediaPlayer::StalledMedia;
            }
         }
         break;
   }

   if (m_currentState == QMediaPlayer::PlayingState && !m_resources->isGranted()) {
      m_mediaStatus = QMediaPlayer::StalledMedia;
   }

   //EndOfMedia status should be kept, until reset by pause, play or setMedia
   if (oldStatus == QMediaPlayer::EndOfMedia) {
      m_mediaStatus = QMediaPlayer::EndOfMedia;
   }

   popAndNotifyState();
}

void QGstreamerPlayerControl::processEOS()
{
   pushState();
   m_mediaStatus = QMediaPlayer::EndOfMedia;
   emit positionChanged(position());
   m_session->endOfMediaReset();

   if (m_currentState != QMediaPlayer::StoppedState) {
      m_currentState = QMediaPlayer::StoppedState;
      m_session->showPrerollFrames(false); // stop showing prerolled frames in stop state
   }

   popAndNotifyState();
}

void QGstreamerPlayerControl::setBufferProgress(int progress)
{
   if (m_bufferProgress == progress || m_mediaStatus == QMediaPlayer::NoMedia) {
      return;
   }

#ifdef DEBUG_PLAYBIN
   qDebug() << Q_FUNC_INFO << progress;
#endif

   m_bufferProgress = progress;

   if (m_resources->isGranted()) {
      if (m_currentState == QMediaPlayer::PlayingState &&
         m_bufferProgress == 100 &&
         m_session->state() != QMediaPlayer::PlayingState) {
         m_session->play();
      }

      if (!m_session->isLiveSource() && m_bufferProgress < 100 &&
         (m_session->state() == QMediaPlayer::PlayingState ||
            m_session->pendingState() == QMediaPlayer::PlayingState)) {
         m_session->pause();
      }
   }

   updateMediaStatus();

   emit bufferStatusChanged(m_bufferProgress);
}

void QGstreamerPlayerControl::handleInvalidMedia()
{
   pushState();
   m_mediaStatus  = QMediaPlayer::InvalidMedia;
   m_currentState = QMediaPlayer::StoppedState;
   m_setMediaPending = true;
   popAndNotifyState();
}

void QGstreamerPlayerControl::handleResourcesGranted()
{
   pushState();

   //This may be triggered when there is an auto resume
   //from resource-policy, we need to take action according to m_userRequestedState
   //rather than m_currentState
   m_currentState = m_userRequestedState;
   if (m_currentState != QMediaPlayer::StoppedState) {
      playOrPause(m_currentState);
   } else {
      updateMediaStatus();
   }

   popAndNotifyState();
}

void QGstreamerPlayerControl::handleResourcesLost()
{
   //on resource lost the pipeline should be paused
   //player status is changed to paused
   pushState();
   QMediaPlayer::State oldState = m_currentState;

   m_session->pause();

   if (oldState != QMediaPlayer::StoppedState ) {
      m_currentState = QMediaPlayer::PausedState;
   }

   popAndNotifyState();
}

void QGstreamerPlayerControl::handleResourcesDenied()
{
   //on resource denied the pipeline should stay paused
   //player status is changed to paused
   pushState();

   if (m_currentState != QMediaPlayer::StoppedState ) {
      m_currentState = QMediaPlayer::PausedState;
   }

   popAndNotifyState();
}

void QGstreamerPlayerControl::pushState()
{
   m_stateStack.push(m_currentState);
   m_mediaStatusStack.push(m_mediaStatus);
}

void QGstreamerPlayerControl::popAndNotifyState()
{
   Q_ASSERT(!m_stateStack.isEmpty());

   QMediaPlayer::State oldState = m_stateStack.pop();
   QMediaPlayer::MediaStatus oldMediaStatus = m_mediaStatusStack.pop();

   if (m_stateStack.isEmpty()) {

      if (m_mediaStatus != oldMediaStatus) {

#ifdef DEBUG_PLAYBIN
         qDebug() << "Media status changed:" << m_mediaStatus;
#endif

         emit mediaStatusChanged(m_mediaStatus);
      }

      if (m_currentState != oldState) {
#ifdef DEBUG_PLAYBIN
         qDebug() << "State changed:" << m_currentState;
#endif

         emit stateChanged(m_currentState);
      }
   }
}

