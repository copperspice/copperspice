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

#include <qsamplecache_p.h>

#include <qdebug.h>
#include <qnetaccess_manager.h>
#include <qnetwork_reply.h>
#include <qnetwork_request.h>

#include <qwavedecoder_p.h>

QSampleCache::QSampleCache(QObject *parent)
   : QObject(parent), m_networkAccessManager(nullptr), m_capacity(0), m_usage(0), m_loadingRefCount(0)
{
   m_loadingThread.setObjectName("QSampleCache::LoadingThread");

   connect(&m_loadingThread, &QThread::finished, this, &QSampleCache::isLoadingChanged);
   connect(&m_loadingThread, &QThread::started,  this, &QSampleCache::isLoadingChanged);
}

QNetworkAccessManager &QSampleCache::networkAccessManager()
{
   if (! m_networkAccessManager) {
      m_networkAccessManager = new QNetworkAccessManager();
   }

   return *m_networkAccessManager;
}

QSampleCache::~QSampleCache()
{
   QRecursiveMutexLocker m(&m_mutex);

   m_loadingThread.quit();
   m_loadingThread.wait();

   // Killing the loading thread means that no samples can be
   // deleted using deleteLater.  And some samples that had deleteLater
   // already called won't have been processed (m_staleSamples)
   for (QSample *sample : m_samples) {
      delete sample;
   }

   for (QSample *sample : m_staleSamples) {
      delete sample;   // deleting a sample does affect the m_staleSamples list, but for each copies it
   }

   m_networkAccessManager->deleteLater();
}

void QSampleCache::loadingRelease()
{
   QMutexLocker locker(&m_loadingMutex);
   --m_loadingRefCount;

   if (m_loadingRefCount == 0) {
      if (m_loadingThread.isRunning()) {
         m_loadingThread.exit();
      }
   }
}

bool QSampleCache::isLoading() const
{
   return m_loadingThread.isRunning();
}

bool QSampleCache::isCached(const QUrl &url) const
{
   QRecursiveMutexLocker locker(&m_mutex);
   return m_samples.contains(url);
}

QSample *QSampleCache::requestSample(const QUrl &url)
{
   // lock and add first to make sure live loadingThread will not be killed during this function call
   m_loadingMutex.lock();
   m_loadingRefCount++;
   m_loadingMutex.unlock();

   if (!m_loadingThread.isRunning()) {
      m_loadingThread.start();
   }

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << "QSampleCache: request sample [" << url << "]";
#endif

   QRecursiveMutexLocker locker(&m_mutex);

   QMap<QUrl, QSample *>::iterator it = m_samples.find(url);
   QSample *sample;

   if (it == m_samples.end()) {
      sample = new QSample(url, this);
      m_samples.insert(url, sample);
      sample->moveToThread(&m_loadingThread);
   } else {
      sample = *it;
   }

   sample->addRef();
   locker.unlock();

   sample->loadIfNecessary();
   return sample;
}

void QSampleCache::setCapacity(qint64 capacity)
{
   QRecursiveMutexLocker locker(&m_mutex);

   if (m_capacity == capacity) {
      return;
   }

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << "QSampleCache: capacity changes from " << m_capacity << "to " << capacity;
#endif

   if (m_capacity > 0 && capacity <= 0) { //memory management strategy changed
      for (QMap<QUrl, QSample *>::iterator it = m_samples.begin(); it != m_samples.end();) {
         QSample *sample = *it;
         if (sample->m_ref == 0) {
            unloadSample(sample);
            it = m_samples.erase(it);
         } else {
            it++;
         }
      }
   }

   m_capacity = capacity;
   refresh(0);
}

// mutex must be locked before calling this method
void QSampleCache::unloadSample(QSample *sample)
{
   m_usage -= sample->m_soundData.size();
   m_staleSamples.insert(sample);
   sample->deleteLater();
}

// called from any thread
void QSampleCache::refresh(qint64 usageChange)
{
   QRecursiveMutexLocker locker(&m_mutex);
   m_usage += usageChange;

   if (m_capacity <= 0 || m_usage <= m_capacity) {
      return;
   }

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qint64 recoveredSize = 0;
#endif

   // free unused samples to keep usage under capacity limit.
   for (QMap<QUrl, QSample *>::iterator it = m_samples.begin(); it != m_samples.end();) {
      QSample *sample = *it;
      if (sample->m_ref > 0) {
         ++it;
         continue;
      }

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
      recoveredSize += sample->m_soundData.size();
#endif

      unloadSample(sample);
      it = m_samples.erase(it);
      if (m_usage <= m_capacity) {
         return;
      }
   }

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << "QSampleCache: refresh(" << usageChange
      << ") recovered size =" << recoveredSize
      << "new usage =" << m_usage;
#endif

   if (m_usage > m_capacity) {
      qWarning() << "QSampleCache: usage[" << m_usage << " out of limit[" << m_capacity << "]";
   }
}

// called from any thread
void QSampleCache::removeUnreferencedSample(QSample *sample)
{
   QRecursiveMutexLocker m(&m_mutex);
   m_staleSamples.remove(sample);
}

// called in loader thread (since this lives in that thread)
// also called from application thread after loader thread dies.
QSample::~QSample()
{
   // Remove ourselves from our parent
   m_parent->removeUnreferencedSample(this);

   QMutexLocker locker(&m_mutex);

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << "~QSample" << this << ": deleted [" << m_url << "]" << QThread::currentThread();
#endif

   cleanup();
}

// called from the main application thread
void QSample::loadIfNecessary()
{
   QMutexLocker locker(&m_mutex);
   if (m_state == QSample::Error || m_state == QSample::Creating) {
      m_state = QSample::Loading;
      QMetaObject::invokeMethod(this, "load", Qt::QueuedConnection);

   } else {
      dynamic_cast<QSampleCache *>(m_parent)->loadingRelease();

   }
}

// called from any thread
bool QSampleCache::notifyUnreferencedSample(QSample *sample)
{
   QRecursiveMutexLocker locker(&m_mutex);
   if (m_capacity > 0) {
      return false;
   }

   m_samples.remove(sample->m_url);
   unloadSample(sample);

   return true;
}

// called in the main application thread
void QSample::release()
{
   QMutexLocker locker(&m_mutex);

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << "Sample:: release" << this << QThread::currentThread() << m_ref;
#endif

   --m_ref;

   if (m_ref == 0) {
      m_parent->notifyUnreferencedSample(this);
   }
}

// called when destroying or loading a stream, mutex must be locked before calling this method
void QSample::cleanup()
{
   if (m_waveDecoder != nullptr) {
      m_waveDecoder->deleteLater();
   }

   if (m_stream != nullptr) {
      m_stream->deleteLater();
   }

   m_waveDecoder = nullptr;
   m_stream = nullptr;
}

// called from the main application thread
void QSample::addRef()
{
   ++m_ref;
}

// called from the thread which is loading the sample
void QSample::readSample()
{
   Q_ASSERT(QThread::currentThread()->objectName() == "QSampleCache::LoadingThread");

   QMutexLocker m(&m_mutex);

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << "QSample: readSample";
#endif

   qint64 read = m_waveDecoder->read(m_soundData.data() + m_sampleReadLength,
         qMin(m_waveDecoder->bytesAvailable(), qint64(m_waveDecoder->size() - m_sampleReadLength)));

   if (read > 0) {
      m_sampleReadLength += read;
   }

   if (m_sampleReadLength < m_waveDecoder->size()) {
      return;
   }

   Q_ASSERT(m_sampleReadLength == qint64(m_soundData.size()));
   onReady();
}

// called from the thread which is loading the sample
void QSample::decoderReady()
{
   Q_ASSERT(QThread::currentThread()->objectName() == QString("QSampleCache::LoadingThread"));

   QMutexLocker m(&m_mutex);

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << "QSample: decoder ready";
#endif

   m_parent->refresh(m_waveDecoder->size());

   m_soundData.resize(m_waveDecoder->size());

   m_sampleReadLength = 0;
   qint64 read = m_waveDecoder->read(m_soundData.data(), m_waveDecoder->size());

   if (read > 0) {
      m_sampleReadLength += read;
   }

   if (m_sampleReadLength >= m_waveDecoder->size()) {
      onReady();
   }
}

// called from any thread
QSample::State QSample::state() const
{
   QMutexLocker m(&m_mutex);
   return m_state;
}

// called in loading thread
// this is a named constructor, unsure if this needs a lock (?)
void QSample::load()
{
   Q_ASSERT(QThread::currentThread()->objectName() == "QSampleCache::LoadingThread");

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << "QSample: load [" << m_url << "]";
#endif

   m_stream = m_parent->networkAccessManager().get(QNetworkRequest(m_url));
   connect(m_stream, &QNetworkReply::error, this, &QSample::decoderError);

   m_waveDecoder = new QWaveDecoder(m_stream);
   connect(m_waveDecoder, &QWaveDecoder::formatKnown,  this, &QSample::decoderReady);
   connect(m_waveDecoder, &QWaveDecoder::parsingError, this, &QSample::decoderError);
   connect(m_waveDecoder, &QWaveDecoder::readyRead,    this, &QSample::readSample);
}

// Called in loading thread
void QSample::decoderError()
{
   Q_ASSERT(QThread::currentThread()->objectName() == "QSampleCache::LoadingThread");
   QMutexLocker m(&m_mutex);

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << "QSample: decoder error";
#endif

   cleanup();
   m_state = QSample::Error;
   dynamic_cast<QSampleCache *>(m_parent)->loadingRelease();
   emit error();
}

// Called in loading thread from decoder when sample is done. Locked already.
void QSample::onReady()
{
   Q_ASSERT(QThread::currentThread()->objectName() == "QSampleCache::LoadingThread");

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << "QSample: load ready";
#endif

   m_audioFormat = m_waveDecoder->audioFormat();
   cleanup();
   m_state = QSample::Ready;
   dynamic_cast<QSampleCache *>(m_parent)->loadingRelease();
   emit ready();
}

// Called in application thread, then moved to loader thread
QSample::QSample(const QUrl &url, QSampleCache *parent)
   : m_parent(parent), m_waveDecoder(nullptr), m_stream(nullptr),
     m_url(url), m_sampleReadLength(0), m_state(Creating), m_ref(0)
{
}
