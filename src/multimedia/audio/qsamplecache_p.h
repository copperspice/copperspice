/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QSAMPLECACHE_P_H
#define QSAMPLECACHE_P_H

#include <QtCore/qobject.h>
#include <QtCore/qthread.h>
#include <QtCore/qurl.h>
#include <QtCore/qmutex.h>
#include <QtCore/qmap.h>
#include <QtCore/qset.h>
#include <qaudioformat.h>

class QIODevice;
class QNetworkAccessManager;
class QSampleCache;
class QWaveDecoder;

// Lives in application thread
class Q_MULTIMEDIA_EXPORT QSample : public QObject
{
   MULTI_CS_OBJECT(QSample)

 public:
   friend class QSampleCache;
   enum State {
      Creating,
      Loading,
      Error,
      Ready,
   };

   State state() const;
   // These are not (currently) locked because they are only meant to be called after these
   // variables are updated to their final states
   const QByteArray &data() const {
      Q_ASSERT(state() == Ready);
      return m_soundData;
   }
   const QAudioFormat &format() const {
      Q_ASSERT(state() == Ready);
      return m_audioFormat;
   }
   void release();

   MULTI_CS_SIGNAL_1(Public, void error())
   MULTI_CS_SIGNAL_2(error)

   MULTI_CS_SIGNAL_1(Public, void ready())
   MULTI_CS_SIGNAL_2(ready)

 protected:
   QSample(const QUrl &url, QSampleCache *parent);

 private:
   void onReady();
   void cleanup();
   void addRef();
   void loadIfNecessary();
   QSample();
   ~QSample();

   mutable QMutex m_mutex;
   QSampleCache *m_parent;
   QByteArray   m_soundData;
   QAudioFormat m_audioFormat;
   QIODevice    *m_stream;
   QWaveDecoder *m_waveDecoder;
   QUrl         m_url;
   qint64       m_sampleReadLength;
   State        m_state;
   int          m_ref;

   MULTI_CS_SLOT_1(Private, void load())
   MULTI_CS_SLOT_2(load)

   MULTI_CS_SLOT_1(Private, void decoderError())
   MULTI_CS_SLOT_2(decoderError)

   MULTI_CS_SLOT_1(Private, void readSample())
   MULTI_CS_SLOT_2(readSample)

   MULTI_CS_SLOT_1(Private, void decoderReady())
   MULTI_CS_SLOT_2(decoderReady)

};

class Q_MULTIMEDIA_EXPORT QSampleCache : public QObject
{
   MULTI_CS_OBJECT(QSampleCache)

 public:
   friend class QSample;

   QSampleCache(QObject *parent = 0);
   ~QSampleCache();

   QSample *requestSample(const QUrl &url);
   void setCapacity(qint64 capacity);

   bool isLoading() const;
   bool isCached(const QUrl &url) const;

   MULTI_CS_SIGNAL_1(Public, void isLoadingChanged())
   MULTI_CS_SIGNAL_2(isLoadingChanged)

 private:
   QMap<QUrl, QSample *> m_samples;
   QSet<QSample *> m_staleSamples;
   QNetworkAccessManager *m_networkAccessManager;
   mutable QMutex m_mutex;
   qint64 m_capacity;
   qint64 m_usage;
   QThread m_loadingThread;

   QNetworkAccessManager &networkAccessManager();
   void refresh(qint64 usageChange);
   bool notifyUnreferencedSample(QSample *sample);
   void removeUnreferencedSample(QSample *sample);
   void unloadSample(QSample *sample);

   void loadingRelease();
   int m_loadingRefCount;
   QMutex m_loadingMutex;
};

#endif
