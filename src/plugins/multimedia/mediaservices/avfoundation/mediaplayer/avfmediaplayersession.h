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

#ifndef AVFMEDIAPLAYERSESSION_H
#define AVFMEDIAPLAYERSESSION_H

#include <QObject>
#include <QByteArray>
#include <QSet>
#include <QResource>

#include <QMediaPlayerControl>
#include <QMediaPlayer>

class AVFMediaPlayerService;
class AVFVideoOutput;

class AVFMediaPlayerSession : public QObject
{
   CS_OBJECT(AVFMediaPlayerSession)

 public:
   AVFMediaPlayerSession(AVFMediaPlayerService *service, QObject *parent = nullptr);
   virtual ~AVFMediaPlayerSession();

   void setVideoOutput(AVFVideoOutput *output);
   void *currentAssetHandle();

   QMediaPlayer::State state() const;
   QMediaPlayer::MediaStatus mediaStatus() const;

   QMediaContent media() const;
   const QIODevice *mediaStream() const;
   void setMedia(const QMediaContent &content, QIODevice *stream);

   qint64 position() const;
   qint64 duration() const;

   int bufferStatus() const;

   int volume() const;
   bool isMuted() const;

   bool isAudioAvailable() const;
   bool isVideoAvailable() const;

   bool isSeekable() const;
   QMediaTimeRange availablePlaybackRanges() const;

   qreal playbackRate() const;

   inline bool isVolumeSupported() const {
      return m_volumeSupported;
   }

   CS_SLOT_1(Public, void setPlaybackRate(qreal rate))
   CS_SLOT_2(setPlaybackRate)

   CS_SLOT_1(Public, void setPosition(qint64 pos))
   CS_SLOT_2(setPosition)

   CS_SLOT_1(Public, void play())
   CS_SLOT_2(play)
   CS_SLOT_1(Public, void pause())
   CS_SLOT_2(pause)

   CS_SLOT_1(Public, void stop())
   CS_SLOT_2(stop)

   CS_SLOT_1(Public, void setVolume(int volume))
   CS_SLOT_2(setVolume)

   CS_SLOT_1(Public, void setMuted(bool muted))
   CS_SLOT_2(setMuted)

   CS_SLOT_1(Public, void processEOS())
   CS_SLOT_2(processEOS)

   CS_SLOT_1(Public, void processLoadStateChange())
   CS_SLOT_2(processLoadStateChange)

   CS_SLOT_1(Public, void processPositionChange())
   CS_SLOT_2(processPositionChange)

   CS_SLOT_1(Public, void processMediaLoadError())
   CS_SLOT_2(processMediaLoadError)

   CS_SIGNAL_1(Public, void positionChanged(qint64 position))
   CS_SIGNAL_2(positionChanged, position)

   CS_SIGNAL_1(Public, void durationChanged(qint64 duration))
   CS_SIGNAL_2(durationChanged, duration)

   CS_SIGNAL_1(Public, void stateChanged(QMediaPlayer::State newState))
   CS_SIGNAL_2(stateChanged, newState)

   CS_SIGNAL_1(Public, void mediaStatusChanged(QMediaPlayer::MediaStatus status))
   CS_SIGNAL_2(mediaStatusChanged, status)

   CS_SIGNAL_1(Public, void volumeChanged(int volume))
   CS_SIGNAL_2(volumeChanged, volume)

   CS_SIGNAL_1(Public, void mutedChanged(bool muted))
   CS_SIGNAL_2(mutedChanged, muted)

   CS_SIGNAL_1(Public, void audioAvailableChanged(bool audioAvailable))
   CS_SIGNAL_2(audioAvailableChanged, audioAvailable)

   CS_SIGNAL_1(Public, void videoAvailableChanged(bool videoAvailable))
   CS_SIGNAL_2(videoAvailableChanged, videoAvailable)

   CS_SIGNAL_1(Public, void playbackRateChanged(qreal rate))
   CS_SIGNAL_2(playbackRateChanged, rate)

   CS_SIGNAL_1(Public, void seekableChanged(bool seekable))
   CS_SIGNAL_2(seekableChanged, seekable)

   CS_SIGNAL_1(Public, void error(int error, const QString &errorString))
   CS_SIGNAL_2(error, error, errorString)

 private:
   class ResourceHandler
   {
    public:
      ResourceHandler()
         : resource(nullptr)
      {
      }

      ~ResourceHandler() {
         clear();
      }

      void setResourceFile(const QString &file) {
         if (resource) {
            if (resource->fileName() == file) {
               return;
            }
            delete resource;
            rawData.clear();
         }
         resource = new QResource(file);
      }

      bool isValid() const {
         return resource && resource->isValid() && resource->data() != nullptr;
      }

      const uchar *data() {
         if (!isValid()) {
            return nullptr;
         }
         if (resource->isCompressed()) {
            if (rawData.size() == 0) {
               rawData = qUncompress(resource->data(), resource->size());
            }
            return (const uchar *)rawData.constData();
         }
         return resource->data();
      }

      qint64 size() {
         if (data() == nullptr) {
            return 0;
         }
         return resource->isCompressed() ? rawData.size() : resource->size();
      }

      void clear() {
         delete resource;
         rawData.clear();
      }

      QResource *resource;
      QByteArray rawData;
   };

   void setAudioAvailable(bool available);
   void setVideoAvailable(bool available);
   void setSeekable(bool seekable);

   AVFMediaPlayerService *m_service;
   AVFVideoOutput *m_videoOutput;

   QMediaPlayer::State m_state;
   QMediaPlayer::MediaStatus m_mediaStatus;
   QIODevice *m_mediaStream;
   QMediaContent m_resources;
   ResourceHandler m_resourceHandler;

   const bool m_volumeSupported;
   bool m_muted;
   bool m_tryingAsync;
   int m_volume;
   qreal m_rate;
   qint64 m_requestedPosition;

   qint64 m_duration;
   bool m_videoAvailable;
   bool m_audioAvailable;
   bool m_seekable;

   void *m_observer;
};

#endif
