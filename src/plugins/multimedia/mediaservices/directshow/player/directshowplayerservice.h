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

#ifndef DIRECTSHOWPLAYERSERVICE_H
#define DIRECTSHOWPLAYERSERVICE_H

#include <qcoreevent.h>
#include <qmutex.h>
#include <qurl.h>
#include <qwaitcondition.h>
#include <qmediaplayer.h>
#include <qmediaresource.h>
#include <qmediaservice.h>
#include <qmediatimerange.h>

#include <directshoweventloop.h>
#include <dsplayer_global.h>

#include <dshow.h>

class QMediaContent;
class QVideoWindowControl;

class DirectShowAudioEndpointControl;
class DirectShowMetaDataControl;
class DirectShowPlayerControl;
class DirectShowVideoRendererControl;

class DirectShowPlayerService : public QMediaService
{
   CS_OBJECT(DirectShowPlayerService)

 public:
   enum StreamType {
      AudioStream = 0x01,
      VideoStream = 0x02
   };

   DirectShowPlayerService(QObject *parent = nullptr);
   ~DirectShowPlayerService();

   QMediaControl *requestControl(const QString &name) override;
   void releaseControl(QMediaControl *control) override;

   void load(const QMediaContent &media, QIODevice *stream);
   void play();
   void pause();
   void stop();

   qint64 position() const;
   QMediaTimeRange availablePlaybackRanges() const;

   void seek(qint64 position);
   void setRate(qreal rate);

   int bufferStatus() const;

   void setAudioOutput(IBaseFilter *filter);
   void setVideoOutput(IBaseFilter *filter);

 protected:
   void customEvent(QEvent *event) override;

 private:
   CS_SLOT_1(Private, void videoOutputChanged())
   CS_SLOT_2(videoOutputChanged)

   void releaseGraph();
   void updateStatus();

   int findStreamTypes(IBaseFilter *source) const;
   int findStreamType(IPin *pin) const;

   bool isConnected(IBaseFilter *filter, PIN_DIRECTION direction) const;
   IBaseFilter *getConnected(IBaseFilter *filter, PIN_DIRECTION direction) const;

   void run();

   void doSetUrlSource(QMutexLocker *locker);
   void doSetStreamSource(QMutexLocker *locker);
   void doRender(QMutexLocker *locker);
   void doFinalizeLoad(QMutexLocker *locker);
   void doSetRate(QMutexLocker *locker);
   void doSeek(QMutexLocker *locker);
   void doPlay(QMutexLocker *locker);
   void doPause(QMutexLocker *locker);
   void doStop(QMutexLocker *locker);
   void doReleaseAudioOutput(QMutexLocker *locker);
   void doReleaseVideoOutput(QMutexLocker *locker);
   void doReleaseGraph(QMutexLocker *locker);

   void graphEvent(QMutexLocker *locker);

   enum Task {
      Shutdown           = 0x0001,
      SetUrlSource       = 0x0002,
      SetStreamSource    = 0x0004,
      SetSource          = SetUrlSource | SetStreamSource,
      SetAudioOutput     = 0x0008,
      SetVideoOutput     = 0x0010,
      SetOutputs         = SetAudioOutput | SetVideoOutput,
      Render             = 0x0020,
      FinalizeLoad       = 0x0040,
      SetRate            = 0x0080,
      Seek               = 0x0100,
      Play               = 0x0200,
      Pause              = 0x0400,
      Stop               = 0x0800,
      ReleaseGraph       = 0x1000,
      ReleaseAudioOutput = 0x2000,
      ReleaseVideoOutput = 0x4000,
      ReleaseFilters     = ReleaseGraph | ReleaseAudioOutput | ReleaseVideoOutput
   };

   enum Event {
      FinalizedLoad = QEvent::User,
      Error,
      RateChange,
      Started,
      Paused,
      DurationChange,
      StatusChange,
      EndOfMedia,
      PositionChange
   };

   enum GraphStatus {
      NoMedia,
      Loading,
      Loaded,
      InvalidMedia
   };

   DirectShowPlayerControl *m_playerControl;
   DirectShowMetaDataControl *m_metaDataControl;
   DirectShowVideoRendererControl *m_videoRendererControl;

   QVideoWindowControl *m_videoWindowControl;
   DirectShowAudioEndpointControl *m_audioEndpointControl;

   QThread *m_taskThread;
   DirectShowEventLoop *m_loop;
   int m_pendingTasks;
   int m_executingTask;
   int m_executedTasks;
   HANDLE m_taskHandle;
   HANDLE m_eventHandle;
   GraphStatus m_graphStatus;
   QMediaPlayer::Error m_error;
   QIODevice *m_stream;
   IFilterGraph2 *m_graph;
   IBaseFilter *m_source;
   IBaseFilter *m_audioOutput;
   IBaseFilter *m_videoOutput;
   int m_streamTypes;
   qreal m_rate;
   qint64 m_position;
   qint64 m_seekPosition;
   qint64 m_duration;
   bool m_buffering;
   bool m_seekable;
   bool m_atEnd;
   bool m_dontCacheNextSeekResult;
   QMediaTimeRange m_playbackRange;
   QUrl m_url;
   QList<QMediaResource> m_resources;
   QString m_errorString;
   QMutex m_mutex;

   friend class DirectShowPlayerServiceThread;
};

#endif
