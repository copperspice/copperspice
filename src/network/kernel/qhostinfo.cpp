/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qhostinfo.h>
#include <qhostinfo_p.h>
#include <qnetworksession_p.h>

#include <qscopedpointer.h>
#include <qabstracteventdispatcher.h>
#include <qcoreapplication.h>
#include <qmetaobject.h>
#include <qstringlist.h>
#include <qthread.h>
#include <qurl.h>

#ifdef Q_OS_UNIX
#  include <unistd.h>
#endif

//#define QHOSTINFO_DEBUG
Q_GLOBAL_STATIC(QHostInfoLookupManager, theHostInfoLookupManager)

static QAtomicInt theIdCounter = 1;

int QHostInfo::lookupHost(const QString &name, QObject *receiver, const char *member)
{

#if defined QHOSTINFO_DEBUG
   qDebug("QHostInfo::lookupHost(\"%s\", %p, %s)",
          name.toLatin1().constData(), receiver, member ? member + 1 : 0);
#endif

   if (! QAbstractEventDispatcher::instance(QThread::currentThread())) {
      qWarning("QHostInfo::lookupHost() called with no event dispatcher");
      return -1;
   }

   // generate unique ID
   int id = theIdCounter.fetchAndAddRelaxed(1);

   if (name.isEmpty()) {

      if (! receiver) {
         return -1;
      }

      QHostInfo hostInfo(id);
      hostInfo.setError(QHostInfo::HostNotFound);
      hostInfo.setErrorString(QCoreApplication::translate("QHostInfo", "No host name given"));

      QScopedPointer<QHostInfoResult> result(new QHostInfoResult);
      QObject::connect(result.data(), SIGNAL(resultsReady(const QHostInfo &)), receiver, member, Qt::QueuedConnection);

      result.data()->emitResultsReady(hostInfo);
      return id;
   }

   QHostInfoLookupManager *manager = theHostInfoLookupManager();

   if (manager) {
      // the application is still alive
      if (manager->cache.isEnabled()) {
         // check cache first
         bool valid = false;
         QHostInfo info = manager->cache.get(name, &valid);

         if (valid) {
            if (!receiver) {
               return -1;
            }

            info.setLookupId(id);
            QHostInfoResult result;

            QObject::connect(&result, SIGNAL(resultsReady(const QHostInfo &)), receiver, member, Qt::QueuedConnection);
            result.emitResultsReady(info);

            return id;
         }
      }

      // cache is not enabled or it was not in the cache, do normal lookup
      QHostInfoRunnable *runnable = new QHostInfoRunnable(name, id);
      if (receiver) {
         QObject::connect(&runnable->resultEmitter, SIGNAL(resultsReady(const QHostInfo &)), receiver, member, Qt::QueuedConnection);
      }

      manager->scheduleLookup(runnable);
   }

   return id;
}

void QHostInfo::abortHostLookup(int id)
{
   theHostInfoLookupManager()->abortLookup(id);
}

QHostInfo QHostInfo::fromName(const QString &name)
{
#if defined QHOSTINFO_DEBUG
   qDebug("QHostInfo::fromName(\"%s\")", name.toLatin1().constData());
#endif

   QHostInfo hostInfo = QHostInfoAgent::fromName(name);
   QAbstractHostInfoLookupManager *manager = theHostInfoLookupManager();
   manager->cache.put(name, hostInfo);
   return hostInfo;
}

#ifndef QT_NO_BEARERMANAGEMENT
QHostInfo QHostInfoPrivate::fromName(const QString &name, QSharedPointer<QNetworkSession> session)
{
#if defined QHOSTINFO_DEBUG
   qDebug("QHostInfoPrivate::fromName(\"%s\") with session %p", name.toLatin1().constData(), session.data());
#endif

   QHostInfo hostInfo = QHostInfoAgent::fromName(name, session);
   QAbstractHostInfoLookupManager *manager = theHostInfoLookupManager();
   manager->cache.put(name, hostInfo);
   return hostInfo;
}
#endif


#ifndef QT_NO_BEARERMANAGEMENT
QHostInfo QHostInfoAgent::fromName(const QString &hostName, QSharedPointer<QNetworkSession>)
{
   return QHostInfoAgent::fromName(hostName);
}
#endif


QHostInfo::QHostInfo(int id)
   : d(new QHostInfoPrivate)
{
   d->lookupId = id;
}

QHostInfo::QHostInfo(const QHostInfo &other)
   : d(new QHostInfoPrivate(*other.d.data()))
{
}

QHostInfo &QHostInfo::operator=(const QHostInfo &other)
{
   *d.data() = *other.d.data();
   return *this;
}

QHostInfo::~QHostInfo()
{
}

QList<QHostAddress> QHostInfo::addresses() const
{
   return d->addrs;
}

void QHostInfo::setAddresses(const QList<QHostAddress> &addresses)
{
   d->addrs = addresses;
}

QString QHostInfo::hostName() const
{
   return d->hostName;
}


void QHostInfo::setHostName(const QString &hostName)
{
   d->hostName = hostName;
}

QHostInfo::HostInfoError QHostInfo::error() const
{
   return d->err;
}

void QHostInfo::setError(HostInfoError error)
{
   d->err = error;
}

int QHostInfo::lookupId() const
{
   return d->lookupId;
}

void QHostInfo::setLookupId(int id)
{
   d->lookupId = id;
}

QString QHostInfo::errorString() const
{
   return d->errorStr;
}

void QHostInfo::setErrorString(const QString &str)
{
   d->errorStr = str;
}

QString QHostInfo::localHostName()
{
   return QSysInfo::machineHostName();
}

QHostInfoRunnable::QHostInfoRunnable(const QString &hn, int i) : toBeLookedUp(hn), id(i)
{
   setAutoDelete(true);
}

// the QHostInfoLookupManager will at some point call this via a QThreadPool
void QHostInfoRunnable::run()
{
   QHostInfoLookupManager *manager = theHostInfoLookupManager();
   // check aborted
   if (manager->wasAborted(id)) {
      manager->lookupFinished(this);
      return;
   }

   QHostInfo hostInfo;

   // QHostInfo::lookupHost already checks the cache. However we need to check
   // it here too because it might have been cache saved by another QHostInfoRunnable
   // in the meanwhile while this QHostInfoRunnable was scheduled but not running
   if (manager->cache.isEnabled()) {
      // check the cache first
      bool valid = false;
      hostInfo = manager->cache.get(toBeLookedUp, &valid);
      if (!valid) {
         // not in cache, we need to do the lookup and store the result in the cache
         hostInfo = QHostInfoAgent::fromName(toBeLookedUp);
         manager->cache.put(toBeLookedUp, hostInfo);
      }
   } else {
      // cache is not enabled, just do the lookup and continue
      hostInfo = QHostInfoAgent::fromName(toBeLookedUp);
   }

   // check aborted again
   if (manager->wasAborted(id)) {
      manager->lookupFinished(this);
      return;
   }

   // signal emission
   hostInfo.setLookupId(id);
   resultEmitter.emitResultsReady(hostInfo);

   // now also iterate through the postponed ones
   {
      QMutexLocker locker(&manager->mutex);
      QMutableListIterator<QHostInfoRunnable *> iterator(manager->postponedLookups);
      while (iterator.hasNext()) {
         QHostInfoRunnable *postponed = iterator.next();
         if (toBeLookedUp == postponed->toBeLookedUp) {
            // we can now emit
            iterator.remove();
            hostInfo.setLookupId(postponed->id);
            postponed->resultEmitter.emitResultsReady(hostInfo);
            delete postponed;
         }
      }
   }

   manager->lookupFinished(this);

   // thread goes back to QThreadPool
}

QHostInfoLookupManager::QHostInfoLookupManager() : mutex(QMutex::Recursive), wasDeleted(false)
{
   moveToThread(QCoreApplicationPrivate::mainThread());
   connect(QCoreApplication::instance(), SIGNAL(destroyed()), this, SLOT(waitForThreadPoolDone()), Qt::DirectConnection);
   threadPool.setMaxThreadCount(20); // do 20 DNS lookups in parallel
}

QHostInfoLookupManager::~QHostInfoLookupManager()
{
   wasDeleted = true;

   // don't qDeleteAll currentLookups, the QThreadPool has ownership
   clear();
}

void QHostInfoLookupManager::clear()
{
   {
      QMutexLocker locker(&mutex);
      qDeleteAll(postponedLookups);
      qDeleteAll(scheduledLookups);
      qDeleteAll(finishedLookups);
      postponedLookups.clear();
      scheduledLookups.clear();
      finishedLookups.clear();
   }

   threadPool.waitForDone();
   cache.clear();
}

void QHostInfoLookupManager::work()
{
   if (wasDeleted) {
      return;
   }

   // goals of this function:
   //  - launch new lookups via the thread pool
   //  - make sure only one lookup per host/IP is in progress

   QMutexLocker locker(&mutex);

   if (!finishedLookups.isEmpty()) {
      // remove ID from aborted if it is in there
      for (int i = 0; i < finishedLookups.length(); i++) {
         abortedLookups.removeAll(finishedLookups.at(i)->id);
      }

      finishedLookups.clear();
   }

   if (!postponedLookups.isEmpty()) {
      // try to start the postponed ones

      QMutableListIterator<QHostInfoRunnable *> iterator(postponedLookups);
      while (iterator.hasNext()) {
         QHostInfoRunnable *postponed = iterator.next();

         // check if none of the postponed hostnames is currently running
         bool alreadyRunning = false;
         for (int i = 0; i < currentLookups.length(); i++) {
            if (currentLookups.at(i)->toBeLookedUp == postponed->toBeLookedUp) {
               alreadyRunning = true;
               break;
            }
         }
         if (!alreadyRunning) {
            iterator.remove();
            scheduledLookups.prepend(postponed); // prepend! we want to finish it ASAP
         }
      }
   }

   if (!scheduledLookups.isEmpty()) {
      // try to start the new ones
      QMutableListIterator<QHostInfoRunnable *> iterator(scheduledLookups);
      while (iterator.hasNext()) {
         QHostInfoRunnable *scheduled = iterator.next();

         // check if a lookup for this host is already running, then postpone
         for (int i = 0; i < currentLookups.size(); i++) {
            if (currentLookups.at(i)->toBeLookedUp == scheduled->toBeLookedUp) {
               iterator.remove();
               postponedLookups.append(scheduled);
               scheduled = 0;
               break;
            }
         }

         if (scheduled && currentLookups.size() < threadPool.maxThreadCount()) {
            // runnable now running in new thread, track this in currentLookups
            threadPool.start(scheduled);
            iterator.remove();
            currentLookups.append(scheduled);
         } else {
            // was postponed, continue iterating
            continue;
         }
      };
   }
}

// called by QHostInfo
void QHostInfoLookupManager::scheduleLookup(QHostInfoRunnable *r)
{
   if (wasDeleted) {
      return;
   }

   QMutexLocker locker(&this->mutex);
   scheduledLookups.enqueue(r);
   work();
}

// called by QHostInfo
void QHostInfoLookupManager::abortLookup(int id)
{
   if (wasDeleted) {
      return;
   }

   QMutexLocker locker(&this->mutex);

   // is postponed? delete and return
   for (int i = 0; i < postponedLookups.length(); i++) {
      if (postponedLookups.at(i)->id == id) {
         delete postponedLookups.takeAt(i);
         return;
      }
   }

   // is scheduled? delete and return
   for (int i = 0; i < scheduledLookups.length(); i++) {
      if (scheduledLookups.at(i)->id == id) {
         delete scheduledLookups.takeAt(i);
         return;
      }
   }

   if (!abortedLookups.contains(id)) {
      abortedLookups.append(id);
   }
}

// called from QHostInfoRunnable
bool QHostInfoLookupManager::wasAborted(int id)
{
   if (wasDeleted) {
      return true;
   }

   QMutexLocker locker(&this->mutex);
   return abortedLookups.contains(id);
}

// called from QHostInfoRunnable
void QHostInfoLookupManager::lookupFinished(QHostInfoRunnable *r)
{
   if (wasDeleted) {
      return;
   }

   QMutexLocker locker(&this->mutex);
   currentLookups.removeOne(r);
   finishedLookups.append(r);
   work();
}

// This function returns immediately when we had a result in the cache, else it will later emit a signal
QHostInfo qt_qhostinfo_lookup(const QString &name, QObject *receiver, const char *member, bool *valid, int *id)
{
   *valid = false;
   *id = -1;

   // check cache
   QAbstractHostInfoLookupManager *manager = theHostInfoLookupManager();
   if (manager && manager->cache.isEnabled()) {
      QHostInfo info = manager->cache.get(name, valid);
      if (*valid) {
         return info;
      }
   }

   // was not in cache, trigger lookup
   *id = QHostInfo::lookupHost(name, receiver, member);

   // return empty response, valid==false
   return QHostInfo();
}

void qt_qhostinfo_clear_cache()
{
   QAbstractHostInfoLookupManager *manager = theHostInfoLookupManager();
   if (manager) {
      manager->clear();
   }
}

void qt_qhostinfo_enable_cache(bool e)
{
   QAbstractHostInfoLookupManager *manager = theHostInfoLookupManager();
   if (manager) {
      manager->cache.setEnabled(e);
   }
}

void qt_qhostinfo_cache_inject(const QString &hostname, const QHostInfo &resolution)
{
   QAbstractHostInfoLookupManager *manager = theHostInfoLookupManager();
   if (!manager || !manager->cache.isEnabled()) {
      return;
   }

   manager->cache.put(hostname, resolution);
}
// cache for 60 seconds
// cache 128 items
QHostInfoCache::QHostInfoCache() : max_age(60), enabled(true), cache(128)
{
#ifdef QT_QHOSTINFO_CACHE_DISABLED_BY_DEFAULT
   enabled = false;
#endif
}

bool QHostInfoCache::isEnabled()
{
   return enabled;
}

// this function is currently only used for the auto tests
// and not usable by public API
void QHostInfoCache::setEnabled(bool e)
{
   enabled = e;
}


QHostInfo QHostInfoCache::get(const QString &name, bool *valid)
{
   QMutexLocker locker(&this->mutex);

   *valid = false;
   QHostInfoCacheElement *element = cache.object(name);

   if (element != nullptr) {
      if (element->age.elapsed() < max_age * 1000) {
         *valid = true;
      }
      return element->info;

      // FIXME idea:
      // if too old but not expired, trigger a new lookup to freshen our cache
   }

   return QHostInfo();
}

void QHostInfoCache::put(const QString &name, const QHostInfo &info)
{
   // if the lookup failed, don't cache
   if (info.error() != QHostInfo::NoError) {
      return;
   }

   QHostInfoCacheElement *element = new QHostInfoCacheElement();
   element->info = info;
   element->age = QElapsedTimer();
   element->age.start();

   QMutexLocker locker(&this->mutex);
   cache.insert(name, element); // cache will take ownership
}

void QHostInfoCache::clear()
{
   QMutexLocker locker(&this->mutex);
   cache.clear();
}

QAbstractHostInfoLookupManager *QAbstractHostInfoLookupManager::globalInstance()
{
   return theHostInfoLookupManager();
}

