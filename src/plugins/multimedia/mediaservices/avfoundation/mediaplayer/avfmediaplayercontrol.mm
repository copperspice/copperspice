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

#include <avfmediaplayercontrol.h>
#include <avfmediaplayersession.h>

AVFMediaPlayerControl::AVFMediaPlayerControl(QObject *parent)
   : QMediaPlayerControl(parent)
{
}

AVFMediaPlayerControl::~AVFMediaPlayerControl()
{
#ifdef QT_DEBUG_AVF
   qDebug() << Q_FUNC_INFO;
#endif
}

void AVFMediaPlayerControl::setSession(AVFMediaPlayerSession *session)
{
   m_session = session;

   connect(m_session, &AVFMediaPlayerSession::positionChanged,       this, &AVFMediaPlayerControl::positionChanged);
   connect(m_session, &AVFMediaPlayerSession::durationChanged,       this, &AVFMediaPlayerControl::durationChanged);
   connect(m_session, &AVFMediaPlayerSession::stateChanged,          this, &AVFMediaPlayerControl::stateChanged);
   connect(m_session, &AVFMediaPlayerSession::mediaStatusChanged,    this, &AVFMediaPlayerControl::mediaStatusChanged);
   connect(m_session, &AVFMediaPlayerSession::volumeChanged,         this, &AVFMediaPlayerControl::volumeChanged);
   connect(m_session, &AVFMediaPlayerSession::mutedChanged,          this, &AVFMediaPlayerControl::mutedChanged);
   connect(m_session, &AVFMediaPlayerSession::audioAvailableChanged, this, &AVFMediaPlayerControl::audioAvailableChanged);
   connect(m_session, &AVFMediaPlayerSession::videoAvailableChanged, this, &AVFMediaPlayerControl::videoAvailableChanged);
   connect(m_session, &AVFMediaPlayerSession::error,                 this, &AVFMediaPlayerControl::error);
   connect(m_session, &AVFMediaPlayerSession::playbackRateChanged,   this, &AVFMediaPlayerControl::playbackRateChanged);
   connect(m_session, &AVFMediaPlayerSession::seekableChanged,       this, &AVFMediaPlayerControl::seekableChanged);
}

QMediaPlayer::State AVFMediaPlayerControl::state() const
{
   return m_session->state();
}

QMediaPlayer::MediaStatus AVFMediaPlayerControl::mediaStatus() const
{
   return m_session->mediaStatus();
}

QMediaContent AVFMediaPlayerControl::media() const
{
   return m_session->media();
}

const QIODevice *AVFMediaPlayerControl::mediaStream() const
{
   return m_session->mediaStream();
}

void AVFMediaPlayerControl::setMedia(const QMediaContent &content, QIODevice *stream)
{
   m_session->setMedia(content, stream);

   Q_EMIT mediaChanged(content);
}

qint64 AVFMediaPlayerControl::position() const
{
   return m_session->position();
}

qint64 AVFMediaPlayerControl::duration() const
{
   return m_session->duration();
}

int AVFMediaPlayerControl::bufferStatus() const
{
   return m_session->bufferStatus();
}

int AVFMediaPlayerControl::volume() const
{
   return m_session->volume();
}

bool AVFMediaPlayerControl::isMuted() const
{
   return m_session->isMuted();
}

bool AVFMediaPlayerControl::isAudioAvailable() const
{
   return m_session->isAudioAvailable();
}

bool AVFMediaPlayerControl::isVideoAvailable() const
{
   return m_session->isVideoAvailable();
}

bool AVFMediaPlayerControl::isSeekable() const
{
   return m_session->isSeekable();
}

QMediaTimeRange AVFMediaPlayerControl::availablePlaybackRanges() const
{
   return m_session->availablePlaybackRanges();
}

qreal AVFMediaPlayerControl::playbackRate() const
{
   return m_session->playbackRate();
}

void AVFMediaPlayerControl::setPlaybackRate(qreal rate)
{
   m_session->setPlaybackRate(rate);
}

void AVFMediaPlayerControl::setPosition(qint64 pos)
{
   m_session->setPosition(pos);
}

void AVFMediaPlayerControl::play()
{
   m_session->play();
}

void AVFMediaPlayerControl::pause()
{
   m_session->pause();
}

void AVFMediaPlayerControl::stop()
{
   m_session->stop();
}

void AVFMediaPlayerControl::setVolume(int volume)
{
   m_session->setVolume(volume);
}

void AVFMediaPlayerControl::setMuted(bool muted)
{
   m_session->setMuted(muted);
}
