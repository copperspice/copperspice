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

#include <qdeclarativepixmapcache_p.h>
#include <qdeclarativenetworkaccessmanagerfactory.h>
#include <qdeclarativeimageprovider.h>
#include <qdeclarativeengine.h>
#include <qdeclarativeglobal_p.h>
#include <qdeclarativeengine_p.h>

#include <QCoreApplication>
#include <QImageReader>
#include <QHash>
#include <QNetworkReply>
#include <QPixmapCache>
#include <QFile>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QBuffer>
#include <QWaitCondition>
#include <QtCore/qdebug.h>
#include <QSslError>

#define IMAGEREQUEST_MAX_REQUEST_COUNT       8
#define IMAGEREQUEST_MAX_REDIRECT_RECURSION 16
#define CACHE_EXPIRE_TIME 30
#define CACHE_REMOVAL_FRACTION 4

QT_BEGIN_NAMESPACE

// The cache limit describes the maximum "junk" in the cache.
// These are the same defaults as QPixmapCache
#if defined(Q_WS_QWS)
static int cache_limit = 2048 * 1024; // 2048 KB cache limit for embedded
#else
static int cache_limit = 10240 * 1024; // 10 MB cache limit for desktop
#endif

class QDeclarativePixmapReader;
class QDeclarativePixmapData;

class QDeclarativePixmapReply : public QObject
{
   DECL_CS_OBJECT(QDeclarativePixmapReply)

 public:
   enum ReadError { NoError, Loading, Decoding };

   QDeclarativePixmapReply(QDeclarativePixmapData *);
   ~QDeclarativePixmapReply();

   QDeclarativePixmapData *data;
   QDeclarativeEngine *engineForReader; // always access reader inside readerMutex.
   QSize requestSize;
   QUrl url;

   bool loading;
   int redirectCount;

   class Event : public QEvent
   {
    public:
      Event(ReadError, const QString &, const QSize &, const QImage &);

      ReadError error;
      QString errorString;
      QSize implicitSize;
      QImage image;
   };
   void postReply(ReadError, const QString &, const QSize &, const QImage &);

   DECL_CS_SIGNAL_1(Public, void finished())
   DECL_CS_SIGNAL_2(finished)

   DECL_CS_SIGNAL_1(Public, void downloadProgress(qint64 un_named_arg1, qint64 un_named_arg2))
   DECL_CS_SIGNAL_2(downloadProgress, un_named_arg1, un_named_arg2)

 protected:
   bool event(QEvent *event);

 private:
   Q_DISABLE_COPY(QDeclarativePixmapReply)

 public:
   static int finishedIndex;
   static int downloadProgressIndex;
};

class QDeclarativePixmapReaderThreadObject : public QObject
{
   DECL_CS_OBJECT(QDeclarativePixmapReaderThreadObject)

 public:
   QDeclarativePixmapReaderThreadObject(QDeclarativePixmapReader *);
   void processJobs();
   virtual bool event(QEvent *e);

 private:
   DECL_CS_SLOT_1(Private, void networkRequestDone())
   DECL_CS_SLOT_2(networkRequestDone)

   QDeclarativePixmapReader *reader;
};

class QDeclarativePixmapData;

class QDeclarativePixmapReader : public QThread
{
   DECL_CS_OBJECT(QDeclarativePixmapReader)

 public:
   QDeclarativePixmapReader(QDeclarativeEngine *eng);
   ~QDeclarativePixmapReader();

   QDeclarativePixmapReply *getImage(QDeclarativePixmapData *);
   void cancel(QDeclarativePixmapReply *rep);

   static QDeclarativePixmapReader *instance(QDeclarativeEngine *engine);
   static QDeclarativePixmapReader *existingInstance(QDeclarativeEngine *engine);

 protected:
   void run();

 private:
   friend class QDeclarativePixmapReaderThreadObject;
   void processJobs();
   void processJob(QDeclarativePixmapReply *, const QUrl &, const QSize &);
   void networkRequestDone(QNetworkReply *);

   QList<QDeclarativePixmapReply *> jobs;
   QList<QDeclarativePixmapReply *> cancelled;
   QDeclarativeEngine *engine;
   QObject *eventLoopQuitHack;

   QMutex mutex;
   QDeclarativePixmapReaderThreadObject *threadObject;
   QWaitCondition waitCondition;

   QNetworkAccessManager *networkAccessManager();
   QNetworkAccessManager *accessManager;

   QHash<QNetworkReply *, QDeclarativePixmapReply *> replies;

   static int replyDownloadProgress;
   static int replyFinished;
   static int downloadProgress;
   static int threadNetworkRequestDone;
   static QHash<QDeclarativeEngine *, QDeclarativePixmapReader *> readers;
 public:
   static QMutex readerMutex;
};

class QDeclarativePixmapData
{
 public:
   QDeclarativePixmapData(const QUrl &u, const QSize &s, const QString &e)
      : refCount(1), inCache(false), pixmapStatus(QDeclarativePixmap::Error),
        url(u), errorString(e), requestSize(s), reply(0), prevUnreferenced(0),
        prevUnreferencedPtr(0), nextUnreferenced(0) {
   }

   QDeclarativePixmapData(const QUrl &u, const QSize &r)
      : refCount(1), inCache(false), pixmapStatus(QDeclarativePixmap::Loading),
        url(u), requestSize(r), reply(0), prevUnreferenced(0), prevUnreferencedPtr(0),
        nextUnreferenced(0) {
   }

   QDeclarativePixmapData(const QUrl &u, const QPixmap &p, const QSize &s, const QSize &r)
      : refCount(1), inCache(false), privatePixmap(false), pixmapStatus(QDeclarativePixmap::Ready),
        url(u), pixmap(p), implicitSize(s), requestSize(r), reply(0), prevUnreferenced(0),
        prevUnreferencedPtr(0), nextUnreferenced(0) {
   }

   QDeclarativePixmapData(const QPixmap &p)
      : refCount(1), inCache(false), privatePixmap(true), pixmapStatus(QDeclarativePixmap::Ready),
        pixmap(p), implicitSize(p.size()), requestSize(p.size()), reply(0), prevUnreferenced(0),
        prevUnreferencedPtr(0), nextUnreferenced(0) {
   }

   int cost() const;
   void addref();
   void release();
   void addToCache();
   void removeFromCache();

   uint refCount;

   bool inCache: 1;
   bool privatePixmap: 1;

   QDeclarativePixmap::Status pixmapStatus;
   QUrl url;
   QString errorString;
   QPixmap pixmap;
   QSize implicitSize;
   QSize requestSize;

   QDeclarativePixmapReply *reply;

   QDeclarativePixmapData *prevUnreferenced;
   QDeclarativePixmapData **prevUnreferencedPtr;
   QDeclarativePixmapData *nextUnreferenced;
};

int QDeclarativePixmapReply::finishedIndex = -1;
int QDeclarativePixmapReply::downloadProgressIndex = -1;

// XXX
QHash<QDeclarativeEngine *, QDeclarativePixmapReader *> QDeclarativePixmapReader::readers;
QMutex QDeclarativePixmapReader::readerMutex;

int QDeclarativePixmapReader::replyDownloadProgress = -1;
int QDeclarativePixmapReader::replyFinished = -1;
int QDeclarativePixmapReader::downloadProgress = -1;
int QDeclarativePixmapReader::threadNetworkRequestDone = -1;


void QDeclarativePixmapReply::postReply(ReadError error, const QString &errorString,
                                        const QSize &implicitSize, const QImage &image)
{
   loading = false;
   QCoreApplication::postEvent(this, new Event(error, errorString, implicitSize, image));
}

QDeclarativePixmapReply::Event::Event(ReadError e, const QString &s, const QSize &iSize, const QImage &i)
   : QEvent(QEvent::User), error(e), errorString(s), implicitSize(iSize), image(i)
{
}

QNetworkAccessManager *QDeclarativePixmapReader::networkAccessManager()
{
   if (!accessManager) {
      Q_ASSERT(threadObject);
      accessManager = QDeclarativeEnginePrivate::get(engine)->createNetworkAccessManager(threadObject);
   }
   return accessManager;
}

static bool readImage(const QUrl &url, QIODevice *dev, QImage *image, QString *errorString, QSize *impsize,
                      const QSize &requestSize)
{
   QImageReader imgio(dev);

   bool force_scale = false;
   if (url.path().endsWith(QLatin1String(".svg"), Qt::CaseInsensitive)) {
      imgio.setFormat("svg"); // QSvgPlugin::capabilities bug QTBUG-9053
      force_scale = true;
   }

   bool scaled = false;
   if (requestSize.width() > 0 || requestSize.height() > 0) {
      QSize s = imgio.size();
      if (requestSize.width() && (force_scale || requestSize.width() < s.width())) {
         if (requestSize.height() <= 0) {
            s.setHeight(s.height()*requestSize.width() / s.width());
         }
         s.setWidth(requestSize.width());
         scaled = true;
      }
      if (requestSize.height() && (force_scale || requestSize.height() < s.height())) {
         if (requestSize.width() <= 0) {
            s.setWidth(s.width()*requestSize.height() / s.height());
         }
         s.setHeight(requestSize.height());
         scaled = true;
      }
      if (scaled) {
         imgio.setScaledSize(s);
      }
   }

   if (impsize) {
      *impsize = imgio.size();
   }

   if (imgio.read(image)) {
      if (impsize && impsize->width() < 0) {
         *impsize = image->size();
      }
      return true;
   } else {
      if (errorString)
         *errorString = QDeclarativePixmap::tr("Error decoding: %1: %2").arg(url.toString())
                        .arg(imgio.errorString());
      return false;
   }
}

QDeclarativePixmapReader::QDeclarativePixmapReader(QDeclarativeEngine *eng)
   : QThread(eng), engine(eng), threadObject(0), accessManager(0)
{
   eventLoopQuitHack = new QObject;
   eventLoopQuitHack->moveToThread(this);
   connect(eventLoopQuitHack, SIGNAL(destroyed(QObject *)), SLOT(quit()), Qt::DirectConnection);
   start(QThread::IdlePriority);
}

QDeclarativePixmapReader::~QDeclarativePixmapReader()
{
   readerMutex.lock();
   readers.remove(engine);
   readerMutex.unlock();

   mutex.lock();
   // manually cancel all outstanding jobs.
   foreach (QDeclarativePixmapReply * reply, jobs) {
      delete reply;
   }
   jobs.clear();
   QList<QDeclarativePixmapReply *> activeJobs = replies.values();
   foreach (QDeclarativePixmapReply * reply, activeJobs) {
      if (reply->loading) {
         cancelled.append(reply);
         reply->data = 0;
      }
   }
   if (threadObject) {
      threadObject->processJobs();
   }
   mutex.unlock();

   eventLoopQuitHack->deleteLater();
   wait();
}

void QDeclarativePixmapReader::networkRequestDone(QNetworkReply *reply)
{
   QDeclarativePixmapReply *job = replies.take(reply);

   if (job) {
      job->redirectCount++;
      if (job->redirectCount < IMAGEREQUEST_MAX_REDIRECT_RECURSION) {
         QVariant redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
         if (redirect.isValid()) {
            QUrl url = reply->url().resolved(redirect.toUrl());
            QNetworkRequest req(url);
            req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);

            reply->deleteLater();
            reply = networkAccessManager()->get(req);

            QMetaObject::connect(reply, replyDownloadProgress, job, downloadProgress);
            QMetaObject::connect(reply, replyFinished, threadObject, threadNetworkRequestDone);

            replies.insert(reply, job);
            return;
         }
      }

      QImage image;
      QDeclarativePixmapReply::ReadError error = QDeclarativePixmapReply::NoError;
      QString errorString;
      QSize readSize;
      if (reply->error()) {
         error = QDeclarativePixmapReply::Loading;
         errorString = reply->errorString();
      } else {
         QByteArray all = reply->readAll();
         QBuffer buff(&all);
         buff.open(QIODevice::ReadOnly);
         if (!readImage(reply->url(), &buff, &image, &errorString, &readSize, job->requestSize)) {
            error = QDeclarativePixmapReply::Decoding;
         }
      }
      // send completion event to the QDeclarativePixmapReply
      mutex.lock();
      if (!cancelled.contains(job)) {
         job->postReply(error, errorString, readSize, image);
      }
      mutex.unlock();
   }
   reply->deleteLater();

   // kick off event loop again incase we have dropped below max request count
   threadObject->processJobs();
}

QDeclarativePixmapReaderThreadObject::QDeclarativePixmapReaderThreadObject(QDeclarativePixmapReader *i)
   : reader(i)
{
}

void QDeclarativePixmapReaderThreadObject::processJobs()
{
   QCoreApplication::postEvent(this, new QEvent(QEvent::User));
}

bool QDeclarativePixmapReaderThreadObject::event(QEvent *e)
{
   if (e->type() == QEvent::User) {
      reader->processJobs();
      return true;
   } else {
      return QObject::event(e);
   }
}

void QDeclarativePixmapReaderThreadObject::networkRequestDone()
{
   QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
   reader->networkRequestDone(reply);
}

void QDeclarativePixmapReader::processJobs()
{
   QMutexLocker locker(&mutex);

   while (true) {
      if (cancelled.isEmpty() && (jobs.isEmpty() || replies.count() >= IMAGEREQUEST_MAX_REQUEST_COUNT)) {
         return;   // Nothing else to do
      }

      // Clean cancelled jobs
      if (cancelled.count()) {
         for (int i = 0; i < cancelled.count(); ++i) {
            QDeclarativePixmapReply *job = cancelled.at(i);
            QNetworkReply *reply = replies.key(job, 0);
            if (reply && reply->isRunning()) {
               // cancel any jobs already started
               replies.remove(reply);
               reply->close();
            }
            // deleteLater, since not owned by this thread
            job->deleteLater();
         }
         cancelled.clear();
      }

      if (!jobs.isEmpty() && replies.count() < IMAGEREQUEST_MAX_REQUEST_COUNT) {
         QDeclarativePixmapReply *runningJob = jobs.takeLast();
         runningJob->loading = true;
         QUrl url = runningJob->url;
         QSize requestSize = runningJob->requestSize;
         locker.unlock();
         processJob(runningJob, url, requestSize);
         locker.relock();
      }
   }
}

void QDeclarativePixmapReader::processJob(QDeclarativePixmapReply *runningJob, const QUrl &url,
      const QSize &requestSize)
{
   // fetch
   if (url.scheme() == QLatin1String("image")) {
      // Use QmlImageProvider
      QSize readSize;
      QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(engine);
      QImage image = ep->getImageFromProvider(url, &readSize, requestSize);

      QDeclarativePixmapReply::ReadError errorCode = QDeclarativePixmapReply::NoError;
      QString errorStr;
      if (image.isNull()) {
         errorCode = QDeclarativePixmapReply::Loading;
         errorStr = QDeclarativePixmap::tr("Failed to get image from provider: %1").arg(url.toString());
      }
      mutex.lock();
      if (!cancelled.contains(runningJob)) {
         runningJob->postReply(errorCode, errorStr, readSize, image);
      }
      mutex.unlock();
   } else {
      QString lf = QDeclarativeEnginePrivate::urlToLocalFileOrQrc(url);
      if (!lf.isEmpty()) {
         // Image is local - load/decode immediately
         QImage image;
         QDeclarativePixmapReply::ReadError errorCode = QDeclarativePixmapReply::NoError;
         QString errorStr;
         QFile f(lf);
         QSize readSize;
         if (f.open(QIODevice::ReadOnly)) {
            if (!readImage(url, &f, &image, &errorStr, &readSize, requestSize)) {
               errorCode = QDeclarativePixmapReply::Loading;
            }
         } else {
            errorStr = QDeclarativePixmap::tr("Cannot open: %1").arg(url.toString());
            errorCode = QDeclarativePixmapReply::Loading;
         }
         mutex.lock();
         if (!cancelled.contains(runningJob)) {
            runningJob->postReply(errorCode, errorStr, readSize, image);
         }
         mutex.unlock();
      } else {
         // Network resource
         QNetworkRequest req(url);
         req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
         QNetworkReply *reply = networkAccessManager()->get(req);
         QMetaObject::connect(reply, replyDownloadProgress, runningJob, downloadProgress);
         QMetaObject::connect(reply, replyFinished, threadObject, threadNetworkRequestDone);
         replies.insert(reply, runningJob);
      }
   }
}

QDeclarativePixmapReader *QDeclarativePixmapReader::instance(QDeclarativeEngine *engine)
{
   // XXX NOTE: must be called within readerMutex locking.
   QDeclarativePixmapReader *reader = readers.value(engine);
   if (!reader) {
      reader = new QDeclarativePixmapReader(engine);
      readers.insert(engine, reader);
   }

   return reader;
}

QDeclarativePixmapReader *QDeclarativePixmapReader::existingInstance(QDeclarativeEngine *engine)
{
   // XXX NOTE: must be called within readerMutex locking.
   return readers.value(engine, 0);
}

QDeclarativePixmapReply *QDeclarativePixmapReader::getImage(QDeclarativePixmapData *data)
{
   mutex.lock();
   QDeclarativePixmapReply *reply = new QDeclarativePixmapReply(data);
   reply->engineForReader = engine;
   jobs.append(reply);
   // XXX
   if (threadObject) {
      threadObject->processJobs();
   }
   mutex.unlock();
   return reply;
}

void QDeclarativePixmapReader::cancel(QDeclarativePixmapReply *reply)
{
   mutex.lock();
   if (reply->loading) {
      cancelled.append(reply);
      reply->data = 0;
      // XXX
      if (threadObject) {
         threadObject->processJobs();
      }
   } else {
      jobs.removeAll(reply);
      delete reply;
   }
   mutex.unlock();
}

void QDeclarativePixmapReader::run()
{
   if (replyDownloadProgress == -1) {
      const QMetaObject *nr = &QNetworkReply::staticMetaObject;
      const QMetaObject *pr = &QDeclarativePixmapReply::staticMetaObject;
      const QMetaObject *ir = &QDeclarativePixmapReaderThreadObject::staticMetaObject;
      replyDownloadProgress = nr->indexOfSignal("downloadProgress(qint64,qint64)");
      replyFinished = nr->indexOfSignal("finished()");
      downloadProgress = pr->indexOfSignal("downloadProgress(qint64,qint64)");
      threadNetworkRequestDone = ir->indexOfSlot("networkRequestDone()");
   }

   mutex.lock();
   threadObject = new QDeclarativePixmapReaderThreadObject(this);
   mutex.unlock();

   processJobs();
   exec();

   delete threadObject;
   threadObject = 0;
}

class QDeclarativePixmapKey
{
 public:
   const QUrl *url;
   const QSize *size;
};

inline bool operator==(const QDeclarativePixmapKey &lhs, const QDeclarativePixmapKey &rhs)
{
   return *lhs.size == *rhs.size && *lhs.url == *rhs.url;
}

inline uint qHash(const QDeclarativePixmapKey &key)
{
   return qHash(*key.url) ^ key.size->width() ^ key.size->height();
}

class QDeclarativePixmapStore : public QObject
{
   DECL_CS_OBJECT(QDeclarativePixmapStore)

 public:
   QDeclarativePixmapStore();

   void unreferencePixmap(QDeclarativePixmapData *);
   void referencePixmap(QDeclarativePixmapData *);
   void flushCache();

 protected:
   virtual void timerEvent(QTimerEvent *);

 public:
   QHash<QDeclarativePixmapKey, QDeclarativePixmapData *> m_cache;

 private:
   void shrinkCache(int remove);

   QDeclarativePixmapData *m_unreferencedPixmaps;
   QDeclarativePixmapData *m_lastUnreferencedPixmap;

   int m_unreferencedCost;
   int m_timerId;
};
Q_GLOBAL_STATIC(QDeclarativePixmapStore, pixmapStore);

QDeclarativePixmapStore::QDeclarativePixmapStore()
   : m_unreferencedPixmaps(0), m_lastUnreferencedPixmap(0), m_unreferencedCost(0), m_timerId(-1)
{
}

void QDeclarativePixmapStore::unreferencePixmap(QDeclarativePixmapData *data)
{
   Q_ASSERT(data->prevUnreferenced == 0);
   Q_ASSERT(data->prevUnreferencedPtr == 0);
   Q_ASSERT(data->nextUnreferenced == 0);

   data->nextUnreferenced = m_unreferencedPixmaps;
   data->prevUnreferencedPtr = &m_unreferencedPixmaps;

   m_unreferencedPixmaps = data;
   if (m_unreferencedPixmaps->nextUnreferenced) {
      m_unreferencedPixmaps->nextUnreferenced->prevUnreferenced = m_unreferencedPixmaps;
      m_unreferencedPixmaps->nextUnreferenced->prevUnreferencedPtr = &m_unreferencedPixmaps->nextUnreferenced;
   }

   if (!m_lastUnreferencedPixmap) {
      m_lastUnreferencedPixmap = data;
   }

   m_unreferencedCost += data->cost();

   shrinkCache(-1); // Shrink the cache incase it has become larger than cache_limit

   if (m_timerId == -1 && m_unreferencedPixmaps) {
      m_timerId = startTimer(CACHE_EXPIRE_TIME * 1000);
   }
}

void QDeclarativePixmapStore::referencePixmap(QDeclarativePixmapData *data)
{
   Q_ASSERT(data->prevUnreferencedPtr);

   *data->prevUnreferencedPtr = data->nextUnreferenced;
   if (data->nextUnreferenced) {
      data->nextUnreferenced->prevUnreferencedPtr = data->prevUnreferencedPtr;
      data->nextUnreferenced->prevUnreferenced = data->prevUnreferenced;
   }
   if (m_lastUnreferencedPixmap == data) {
      m_lastUnreferencedPixmap = data->prevUnreferenced;
   }

   data->nextUnreferenced = 0;
   data->prevUnreferencedPtr = 0;
   data->prevUnreferenced = 0;

   m_unreferencedCost -= data->cost();
}

void QDeclarativePixmapStore::shrinkCache(int remove)
{
   while ((remove > 0 || m_unreferencedCost > cache_limit) && m_lastUnreferencedPixmap) {
      QDeclarativePixmapData *data = m_lastUnreferencedPixmap;
      Q_ASSERT(data->nextUnreferenced == 0);

      *data->prevUnreferencedPtr = 0;
      m_lastUnreferencedPixmap = data->prevUnreferenced;
      data->prevUnreferencedPtr = 0;
      data->prevUnreferenced = 0;

      remove -= data->cost();
      m_unreferencedCost -= data->cost();
      data->removeFromCache();
      delete data;
   }
}

void QDeclarativePixmapStore::timerEvent(QTimerEvent *)
{
   int removalCost = m_unreferencedCost / CACHE_REMOVAL_FRACTION;

   shrinkCache(removalCost);

   if (m_unreferencedPixmaps == 0) {
      killTimer(m_timerId);
      m_timerId = -1;
   }
}

/*
    Remove all unreferenced pixmaps from the cache.
*/
void QDeclarativePixmapStore::flushCache()
{
   shrinkCache(m_unreferencedCost);
}

QDeclarativePixmapReply::QDeclarativePixmapReply(QDeclarativePixmapData *d)
   : data(d), engineForReader(0), requestSize(d->requestSize), url(d->url), loading(false), redirectCount(0)
{
   if (finishedIndex == -1) {
      finishedIndex = QDeclarativePixmapReply::staticMetaObject.indexOfSignal("finished()");
      downloadProgressIndex = QDeclarativePixmapReply::staticMetaObject.indexOfSignal("downloadProgress(qint64,qint64)");
   }
}

QDeclarativePixmapReply::~QDeclarativePixmapReply()
{
}

bool QDeclarativePixmapReply::event(QEvent *event)
{
   if (event->type() == QEvent::User) {

      if (data) {
         Event *de = static_cast<Event *>(event);
         data->pixmapStatus = (de->error == NoError) ? QDeclarativePixmap::Ready : QDeclarativePixmap::Error;

         if (data->pixmapStatus == QDeclarativePixmap::Ready) {
            data->pixmap = QPixmap::fromImage(de->image);
            data->implicitSize = de->implicitSize;
         } else {
            data->errorString = de->errorString;
            data->removeFromCache(); // We don't continue to cache error'd pixmaps
         }

         data->reply = 0;
         emit finished();
      }

      delete this;
      return true;
   } else {
      return QObject::event(event);
   }
}

int QDeclarativePixmapData::cost() const
{
   return (pixmap.width() * pixmap.height() * pixmap.depth()) / 8;
}

void QDeclarativePixmapData::addref()
{
   ++refCount;
   if (prevUnreferencedPtr) {
      pixmapStore()->referencePixmap(this);
   }
}

void QDeclarativePixmapData::release()
{
   Q_ASSERT(refCount > 0);
   --refCount;

   if (refCount == 0) {
      if (reply) {
         QDeclarativePixmapReply *cancelReply = reply;
         reply->data = 0;
         reply = 0;
         QDeclarativePixmapReader::readerMutex.lock();
         QDeclarativePixmapReader *reader = QDeclarativePixmapReader::existingInstance(cancelReply->engineForReader);
         if (reader) {
            reader->cancel(cancelReply);
         }
         QDeclarativePixmapReader::readerMutex.unlock();
      }

      if (pixmapStatus == QDeclarativePixmap::Ready) {
         pixmapStore()->unreferencePixmap(this);
      } else {
         removeFromCache();
         delete this;
      }
   }
}

void QDeclarativePixmapData::addToCache()
{
   if (!inCache) {
      QDeclarativePixmapKey key = { &url, &requestSize };
      pixmapStore()->m_cache.insert(key, this);
      inCache = true;
   }
}

void QDeclarativePixmapData::removeFromCache()
{
   if (inCache) {
      QDeclarativePixmapKey key = { &url, &requestSize };
      pixmapStore()->m_cache.remove(key);
      inCache = false;
   }
}

static QDeclarativePixmapData *createPixmapDataSync(QDeclarativeEngine *engine, const QUrl &url,
      const QSize &requestSize, bool *ok)
{
   if (url.scheme() == QLatin1String("image")) {
      QSize readSize;
      QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(engine);
      QDeclarativeImageProvider::ImageType imageType = ep->getImageProviderType(url);

      switch (imageType) {
         case QDeclarativeImageProvider::Image: {
            QImage image = ep->getImageFromProvider(url, &readSize, requestSize);
            if (!image.isNull()) {
               *ok = true;
               return new QDeclarativePixmapData(url, QPixmap::fromImage(image), readSize, requestSize);
            }
         }
         case QDeclarativeImageProvider::Pixmap: {
            QPixmap pixmap = ep->getPixmapFromProvider(url, &readSize, requestSize);
            if (!pixmap.isNull()) {
               *ok = true;
               return new QDeclarativePixmapData(url, pixmap, readSize, requestSize);
            }
         }
      }

      // no matching provider, or provider has bad image type, or provider returned null image
      return new QDeclarativePixmapData(url, requestSize,
                                        QDeclarativePixmap::tr("Failed to get image from provider: %1").arg(url.toString()));
   }

   QString localFile = QDeclarativeEnginePrivate::urlToLocalFileOrQrc(url);
   if (localFile.isEmpty()) {
      return 0;
   }

   QFile f(localFile);
   QSize readSize;
   QString errorString;

   if (f.open(QIODevice::ReadOnly)) {
      QImage image;
      if (readImage(url, &f, &image, &errorString, &readSize, requestSize)) {
         *ok = true;
         return new QDeclarativePixmapData(url, QPixmap::fromImage(image), readSize, requestSize);
      }
   } else {
      errorString = QDeclarativePixmap::tr("Cannot open: %1").arg(url.toString());
   }
   return new QDeclarativePixmapData(url, requestSize, errorString);
}


struct QDeclarativePixmapNull {
   QUrl url;
   QPixmap pixmap;
   QSize size;
};
Q_GLOBAL_STATIC(QDeclarativePixmapNull, nullPixmap);

QDeclarativePixmap::QDeclarativePixmap()
   : d(0)
{
}

QDeclarativePixmap::QDeclarativePixmap(QDeclarativeEngine *engine, const QUrl &url)
   : d(0)
{
   load(engine, url);
}

QDeclarativePixmap::QDeclarativePixmap(QDeclarativeEngine *engine, const QUrl &url, const QSize &size)
   : d(0)
{
   load(engine, url, size);
}

QDeclarativePixmap::~QDeclarativePixmap()
{
   if (d) {
      d->release();
      d = 0;
   }
}

bool QDeclarativePixmap::isNull() const
{
   return d == 0;
}

bool QDeclarativePixmap::isReady() const
{
   return status() == Ready;
}

bool QDeclarativePixmap::isError() const
{
   return status() == Error;
}

bool QDeclarativePixmap::isLoading() const
{
   return status() == Loading;
}

QString QDeclarativePixmap::error() const
{
   if (d) {
      return d->errorString;
   } else {
      return QString();
   }
}

QDeclarativePixmap::Status QDeclarativePixmap::status() const
{
   if (d) {
      return d->pixmapStatus;
   } else {
      return Null;
   }
}

const QUrl &QDeclarativePixmap::url() const
{
   if (d) {
      return d->url;
   } else {
      return nullPixmap()->url;
   }
}

const QSize &QDeclarativePixmap::implicitSize() const
{
   if (d) {
      return d->implicitSize;
   } else {
      return nullPixmap()->size;
   }
}

const QSize &QDeclarativePixmap::requestSize() const
{
   if (d) {
      return d->requestSize;
   } else {
      return nullPixmap()->size;
   }
}

const QPixmap &QDeclarativePixmap::pixmap() const
{
   if (d) {
      return d->pixmap;
   } else {
      return nullPixmap()->pixmap;
   }
}

void QDeclarativePixmap::setPixmap(const QPixmap &p)
{
   clear();

   if (!p.isNull()) {
      d = new QDeclarativePixmapData(p);
   }
}

int QDeclarativePixmap::width() const
{
   if (d) {
      return d->pixmap.width();
   } else {
      return 0;
   }
}

int QDeclarativePixmap::height() const
{
   if (d) {
      return d->pixmap.height();
   } else {
      return 0;
   }
}

QRect QDeclarativePixmap::rect() const
{
   if (d) {
      return d->pixmap.rect();
   } else {
      return QRect();
   }
}

void QDeclarativePixmap::load(QDeclarativeEngine *engine, const QUrl &url)
{
   load(engine, url, QSize(), QDeclarativePixmap::Cache);
}

void QDeclarativePixmap::load(QDeclarativeEngine *engine, const QUrl &url, QDeclarativePixmap::Options options)
{
   load(engine, url, QSize(), options);
}

void QDeclarativePixmap::load(QDeclarativeEngine *engine, const QUrl &url, const QSize &size)
{
   load(engine, url, size, QDeclarativePixmap::Cache);
}

void QDeclarativePixmap::load(QDeclarativeEngine *engine, const QUrl &url, const QSize &requestSize,
                              QDeclarativePixmap::Options options)
{
   if (d) {
      d->release();
      d = 0;
   }

   QDeclarativePixmapKey key = { &url, &requestSize };
   QDeclarativePixmapStore *store = pixmapStore();

   QHash<QDeclarativePixmapKey, QDeclarativePixmapData *>::Iterator iter = store->m_cache.find(key);

   if (iter == store->m_cache.end()) {
      if (options & QDeclarativePixmap::Asynchronous) {
         // pixmaps can only be loaded synchronously
         if (url.scheme() == QLatin1String("image")
               && QDeclarativeEnginePrivate::get(engine)->getImageProviderType(url) == QDeclarativeImageProvider::Pixmap) {
            options &= ~QDeclarativePixmap::Asynchronous;
         }
      }

      if (!(options & QDeclarativePixmap::Asynchronous)) {
         bool ok = false;
         d = createPixmapDataSync(engine, url, requestSize, &ok);
         if (ok) {
            if (options & QDeclarativePixmap::Cache) {
               d->addToCache();
            }
            return;
         }
         if (d) { // loadable, but encountered error while loading
            return;
         }
      }

      if (!engine) {
         return;
      }

      d = new QDeclarativePixmapData(url, requestSize);
      if (options & QDeclarativePixmap::Cache) {
         d->addToCache();
      }
      QDeclarativePixmapReader::readerMutex.lock();
      d->reply = QDeclarativePixmapReader::instance(engine)->getImage(d);
      QDeclarativePixmapReader::readerMutex.unlock();
   } else {
      d = *iter;
      d->addref();
   }
}

void QDeclarativePixmap::clear()
{
   if (d) {
      d->release();
      d = 0;
   }
}

void QDeclarativePixmap::clear(QObject *obj)
{
   if (d) {
      if (d->reply) {
         QObject::disconnect(d->reply, 0, obj, 0);
      }
      d->release();
      d = 0;
   }
}

bool QDeclarativePixmap::connectFinished(QObject *object, const char *method)
{
   if (!d || !d->reply) {
      qWarning("QDeclarativePixmap: connectFinished() called when not loading.");
      return false;
   }

   return QObject::connect(d->reply, SIGNAL(finished()), object, method);
}

bool QDeclarativePixmap::connectFinished(QObject *object, int method)
{
   if (!d || !d->reply) {
      qWarning("QDeclarativePixmap: connectFinished() called when not loading.");
      return false;
   }

   return QMetaObject::connect(d->reply, QDeclarativePixmapReply::finishedIndex, object, method);
}

bool QDeclarativePixmap::connectDownloadProgress(QObject *object, const char *method)
{
   if (!d || !d->reply) {
      qWarning("QDeclarativePixmap: connectDownloadProgress() called when not loading.");
      return false;
   }

   return QObject::connect(d->reply, SIGNAL(downloadProgress(qint64, qint64)), object, method);
}

bool QDeclarativePixmap::connectDownloadProgress(QObject *object, int method)
{
   if (!d || !d->reply) {
      qWarning("QDeclarativePixmap: connectDownloadProgress() called when not loading.");
      return false;
   }

   return QMetaObject::connect(d->reply, QDeclarativePixmapReply::downloadProgressIndex, object, method);
}

void QDeclarativePixmap::flushCache()
{
   pixmapStore()->flushCache();
}

QT_END_NAMESPACE
