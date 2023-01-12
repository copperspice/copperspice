/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#ifndef DIRECTSHOWPLAYERCONTROL_H
#define DIRECTSHOWPLAYERCONTROL_H

#include <dshow.h>

#include <qcoreevent.h>
#include <qmediacontent.h>
#include <qmediaplayercontrol.h>
#include <directshowplayerservice.h>

class DirectShowPlayerControl : public QMediaPlayerControl
{
   CS_OBJECT(DirectShowPlayerControl)

 public:
   DirectShowPlayerControl(DirectShowPlayerService *service, QObject *parent = nullptr);
   ~DirectShowPlayerControl();

   QMediaPlayer::State state() const;

   QMediaPlayer::MediaStatus mediaStatus() const;

   qint64 duration() const;

   qint64 position() const;
   void setPosition(qint64 position);

   int volume() const;
   void setVolume(int volume);

   bool isMuted() const;
   void setMuted(bool muted);

   int bufferStatus() const;

   bool isAudioAvailable() const;
   bool isVideoAvailable() const;

   bool isSeekable() const;

   QMediaTimeRange availablePlaybackRanges() const;

   qreal playbackRate() const;
   void setPlaybackRate(qreal rate);

   QMediaContent media() const;
   const QIODevice *mediaStream() const;
   void setMedia(const QMediaContent &media, QIODevice *stream);

   void play();
   void pause();
   void stop();

   void updateState(QMediaPlayer::State state);
   void updateStatus(QMediaPlayer::MediaStatus status);
   void updateMediaInfo(qint64 duration, int streamTypes, bool seekable);
   void updatePlaybackRate(qreal rate);
   void updateAudioOutput(IBaseFilter *filter);
   void updateError(QMediaPlayer::Error error, const QString &errorString);
   void updatePosition(qint64 position);

 protected:
   void customEvent(QEvent *event);

 private:
   enum Properties {
      StateProperty        = 0x01,
      StatusProperty       = 0x02,
      StreamTypesProperty  = 0x04,
      DurationProperty     = 0x08,
      PlaybackRateProperty = 0x10,
      SeekableProperty     = 0x20,
      ErrorProperty        = 0x40,
      PositionProperty     = 0x80
   };

   enum Event {
      PropertiesChanged = QEvent::User
   };

   void playOrPause(QMediaPlayer::State state);

   void scheduleUpdate(int properties);
   void emitPropertyChanges();
   void setVolumeHelper(int volume);

   DirectShowPlayerService *m_service;
   IBasicAudio *m_audio;
   QIODevice *m_stream;
   int m_updateProperties;
   QMediaPlayer::State m_state;
   QMediaPlayer::MediaStatus m_status;
   QMediaPlayer::Error m_error;
   int m_streamTypes;
   int m_volume;
   bool m_muted;
   qint64 m_emitPosition;
   qint64 m_pendingPosition;
   qint64 m_duration;
   qreal m_playbackRate;
   bool m_seekable;
   QMediaContent m_media;
   QString m_errorString;

};

#endif
