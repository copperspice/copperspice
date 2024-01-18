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

#ifndef QMEDIAPLAYERCONTROL_H
#define QMEDIAPLAYERCONTROL_H

#include <qpair.h>
#include <qmediacontrol.h>
#include <qmediaplayer.h>
#include <qmediatimerange.h>

class QMediaPlaylist;

class Q_MULTIMEDIA_EXPORT QMediaPlayerControl : public QMediaControl
{
   MULTI_CS_OBJECT(QMediaPlayerControl)

 public:
   ~QMediaPlayerControl();

   virtual QMediaPlayer::State state() const = 0;

   virtual QMediaPlayer::MediaStatus mediaStatus() const = 0;

   virtual qint64 duration() const = 0;

   virtual qint64 position() const = 0;
   virtual void setPosition(qint64 position) = 0;

   virtual int volume() const = 0;
   virtual void setVolume(int volume) = 0;

   virtual bool isMuted() const = 0;
   virtual void setMuted(bool muted) = 0;

   virtual int bufferStatus() const = 0;

   virtual bool isAudioAvailable() const = 0;
   virtual bool isVideoAvailable() const = 0;

   virtual bool isSeekable() const = 0;

   virtual QMediaTimeRange availablePlaybackRanges() const = 0;

   virtual qreal playbackRate() const = 0;
   virtual void setPlaybackRate(qreal rate) = 0;

   virtual QMediaContent media() const = 0;
   virtual const QIODevice *mediaStream() const = 0;
   virtual void setMedia(const QMediaContent &media, QIODevice *stream) = 0;

   virtual void play()  = 0;
   virtual void pause() = 0;
   virtual void stop()  = 0;

   MULTI_CS_SIGNAL_1(Public, void mediaChanged(const QMediaContent &content))
   MULTI_CS_SIGNAL_2(mediaChanged, content)

   MULTI_CS_SIGNAL_1(Public, void durationChanged(qint64 duration))
   MULTI_CS_SIGNAL_2(durationChanged, duration)

   MULTI_CS_SIGNAL_1(Public, void positionChanged(qint64 position))
   MULTI_CS_SIGNAL_2(positionChanged, position)

   MULTI_CS_SIGNAL_1(Public, void stateChanged(QMediaPlayer::State state))
   MULTI_CS_SIGNAL_2(stateChanged, state)

   MULTI_CS_SIGNAL_1(Public, void mediaStatusChanged(QMediaPlayer::MediaStatus status))
   MULTI_CS_SIGNAL_2(mediaStatusChanged, status)

   MULTI_CS_SIGNAL_1(Public, void volumeChanged(int volume))
   MULTI_CS_SIGNAL_2(volumeChanged, volume)

   MULTI_CS_SIGNAL_1(Public, void mutedChanged(bool muted))
   MULTI_CS_SIGNAL_2(mutedChanged, muted)

   MULTI_CS_SIGNAL_1(Public, void audioAvailableChanged(bool audioAvailable))
   MULTI_CS_SIGNAL_2(audioAvailableChanged, audioAvailable)

   MULTI_CS_SIGNAL_1(Public, void videoAvailableChanged(bool videoAvailable))
   MULTI_CS_SIGNAL_2(videoAvailableChanged, videoAvailable)

   MULTI_CS_SIGNAL_1(Public, void bufferStatusChanged(int progress))
   MULTI_CS_SIGNAL_2(bufferStatusChanged, progress)

   MULTI_CS_SIGNAL_1(Public, void seekableChanged(bool seekable))
   MULTI_CS_SIGNAL_2(seekableChanged, seekable)

   MULTI_CS_SIGNAL_1(Public, void availablePlaybackRangesChanged(const QMediaTimeRange &ranges))
   MULTI_CS_SIGNAL_2(availablePlaybackRangesChanged, ranges)

   MULTI_CS_SIGNAL_1(Public, void playbackRateChanged(qreal rate))
   MULTI_CS_SIGNAL_2(playbackRateChanged, rate)

   MULTI_CS_SIGNAL_1(Public, void error(int error, const QString &errorString))
   MULTI_CS_SIGNAL_2(error, error, errorString)

 protected:
   explicit QMediaPlayerControl(QObject *parent = nullptr);
};

#define QMediaPlayerControl_Key "com.copperspice.CS.mediaPlayerControl/1.0"
CS_DECLARE_INTERFACE(QMediaPlayerControl, QMediaPlayerControl_Key)

#endif

