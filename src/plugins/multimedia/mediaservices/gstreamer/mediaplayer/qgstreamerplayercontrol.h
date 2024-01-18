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

#ifndef QGSTREAMERPLAYERCONTROL_H
#define QGSTREAMERPLAYERCONTROL_H

#include <qobject.h>
#include <qstack.h>

#include <qmediaplayercontrol.h>
#include <qmediaplayer.h>

#include <limits.h>

class QMediaPlayerResourceSetInterface;
class QMediaPlaylist;
class QMediaPlaylistNavigator;
class QSocketNotifier;

class QGstreamerPlayerSession;
class QGstreamerPlayerService;

class QGstreamerPlayerControl : public QMediaPlayerControl
{
   CS_OBJECT(QGstreamerPlayerControl)

 public:
   QGstreamerPlayerControl(QGstreamerPlayerSession *session, QObject *parent = nullptr);
   ~QGstreamerPlayerControl();

   QMediaPlayer::State state() const override;
   QMediaPlayer::MediaStatus mediaStatus() const override;

   qint64 position() const override;
   qint64 duration() const override;

   int bufferStatus() const override;

   int volume() const override;
   bool isMuted() const override;

   bool isAudioAvailable() const override;
   bool isVideoAvailable() const override;
   void setVideoOutput(QObject *output);

   bool isSeekable() const override;
   QMediaTimeRange availablePlaybackRanges() const override;

   qreal playbackRate() const override;
   void setPlaybackRate(qreal rate) override;

   QMediaContent media() const override;
   const QIODevice *mediaStream() const override;
   void setMedia(const QMediaContent &, QIODevice *) override;

   QMediaPlayerResourceSetInterface *resources() const;

   CS_SLOT_1(Public, void setPosition(qint64 pos) override)
   CS_SLOT_2(setPosition)

   CS_SLOT_1(Public, void play() override)
   CS_SLOT_2(play)
   CS_SLOT_1(Public, void pause() override)
   CS_SLOT_2(pause)
   CS_SLOT_1(Public, void stop() override)
   CS_SLOT_2(stop)

   CS_SLOT_1(Public, void setVolume(int volume) override)
   CS_SLOT_2(setVolume)
   CS_SLOT_1(Public, void setMuted(bool muted) override)
   CS_SLOT_2(setMuted)

 private:
   void playOrPause(QMediaPlayer::State state);

   void pushState();
   void popAndNotifyState();

   QGstreamerPlayerSession *m_session;
   QMediaPlayer::State m_userRequestedState;
   QMediaPlayer::State m_currentState;
   QMediaPlayer::MediaStatus m_mediaStatus;
   QStack<QMediaPlayer::State> m_stateStack;
   QStack<QMediaPlayer::MediaStatus> m_mediaStatusStack;

   int m_bufferProgress;
   qint64 m_pendingSeekPosition;
   bool m_setMediaPending;
   QMediaContent m_currentResource;
   QIODevice *m_stream;

   QMediaPlayerResourceSetInterface *m_resources;

   CS_SLOT_1(Private, void updateSessionState(QMediaPlayer::State state))
   CS_SLOT_2(updateSessionState)
   CS_SLOT_1(Private, void updateMediaStatus())
   CS_SLOT_2(updateMediaStatus)
   CS_SLOT_1(Private, void processEOS())
   CS_SLOT_2(processEOS)
   CS_SLOT_1(Private, void setBufferProgress(int progress))
   CS_SLOT_2(setBufferProgress)

   CS_SLOT_1(Private, void handleInvalidMedia())
   CS_SLOT_2(handleInvalidMedia)

   CS_SLOT_1(Private, void handleResourcesGranted())
   CS_SLOT_2(handleResourcesGranted)
   CS_SLOT_1(Private, void handleResourcesLost())
   CS_SLOT_2(handleResourcesLost)
   CS_SLOT_1(Private, void handleResourcesDenied())
   CS_SLOT_2(handleResourcesDenied)
};

#endif
