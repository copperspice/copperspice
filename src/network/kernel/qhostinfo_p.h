/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QHOSTINFO_P_H
#define QHOSTINFO_P_H

#include <qhostinfo.h>

#include <qcache.h>
#include <qcoreapplication.h>
#include <qelapsedtimer.h>
#include <qlist.h>
#include <qmutex.h>
#include <qmutex.h>
#include <qnetworksession.h>
#include <qobject.h>
#include <qpointer.h>
#include <qqueue.h>
#include <qrunnable.h>
#include <qsharedpointer.h>
#include <qthread.h>
#include <qthreadpool.h>
#include <qwaitcondition.h>

#include <qcoreapplication_p.h>

class QHostInfoResult : public QObject
{
   NET_CS_OBJECT(QHostInfoResult)

 public :
   NET_CS_SLOT_1(Public, void emitResultsReady(const QHostInfo &info) {
      emit resultsReady(info);
   })
   NET_CS_SLOT_2(emitResultsReady)

   NET_CS_SIGNAL_1(Public, void resultsReady(const QHostInfo &info))
   NET_CS_SIGNAL_2(resultsReady, info)
};

// needs to be QObject because fromName calls tr()
class QHostInfoAgent : public QObject
{
   NET_CS_OBJECT(QHostInfoAgent)

 public:
   static QHostInfo fromName(const QString &hostName);

#ifndef QT_NO_BEARERMANAGEMENT
   static QHostInfo fromName(const QString &hostName, QSharedPointer<QNetworkSession> networkSession);
#endif

};

class QHostInfoPrivate
{
 public:
   QHostInfoPrivate()
      : err(QHostInfo::NoError), errorStr(QString::fromLatin1(cs_mark_tr("QHostInfo", "Unknown error"))), lookupId(0)
   {
   }

#ifndef QT_NO_BEARERMANAGEMENT
   //not a public API yet
   static QHostInfo fromName(const QString &hostName, QSharedPointer<QNetworkSession> networkSession);
#endif

   QHostInfo::HostInfoError err;
   QString errorStr;
   QList<QHostAddress> addrs;
   QString hostName;
   int lookupId;
};

QHostInfo Q_NETWORK_EXPORT qt_qhostinfo_lookup(const QString &name, QObject *receiver, const QString &member, bool *valid, int *id);
void qt_qhostinfo_clear_cache();
void qt_qhostinfo_enable_cache(bool e);
void qt_qhostinfo_cache_inject(const QString &hostname, const QHostInfo &resolution);

class QHostInfoCache
{
 public:
   QHostInfoCache();
   const int max_age; // seconds

   QHostInfo get(const QString &name, bool *valid);
   void put(const QString &name, const QHostInfo &info);
   void clear();

   bool isEnabled();
   void setEnabled(bool e);

 private:
   bool enabled;
   struct QHostInfoCacheElement {
      QHostInfo info;
      QElapsedTimer age;
   };
   QCache<QString, QHostInfoCacheElement> cache;
   QMutex mutex;
};

// the following classes are used for the (normal) case: We use multiple threads to lookup DNS

class QHostInfoRunnable : public QRunnable
{
 public:
   QHostInfoRunnable (const QString &hn, int i);
   void run() override;

   QString toBeLookedUp;
   int id;
   QHostInfoResult resultEmitter;
};


class QAbstractHostInfoLookupManager : public QObject
{
   NET_CS_OBJECT(QAbstractHostInfoLookupManager)

 public:
   ~QAbstractHostInfoLookupManager() {}
   virtual void clear() = 0;

   QHostInfoCache cache;

 protected:
   QAbstractHostInfoLookupManager() {}
   static QAbstractHostInfoLookupManager *globalInstance();

};


class QHostInfoLookupManager : public QAbstractHostInfoLookupManager
{
   NET_CS_OBJECT(QHostInfoLookupManager)

 public:
   QHostInfoLookupManager();
   ~QHostInfoLookupManager();

   void clear() override;
   void work();

   // called from QHostInfo
   void scheduleLookup(QHostInfoRunnable *r);
   void abortLookup(int id);

   // called from QHostInfoRunnable
   void lookupFinished(QHostInfoRunnable *r);
   bool wasAborted(int id);

   friend class QHostInfoRunnable;

 protected:
   QList<QHostInfoRunnable *> currentLookups; // in progress
   QList<QHostInfoRunnable *> postponedLookups; // postponed because in progress for same host
   QQueue<QHostInfoRunnable *> scheduledLookups; // not yet started
   QList<QHostInfoRunnable *> finishedLookups; // recently finished
   QList<int> abortedLookups; // ids of aborted lookups

   QThreadPool threadPool;

   QRecursiveMutex mutex;

   bool wasDeleted;

 private:
   NET_CS_SLOT_1(Private, void waitForThreadPoolDone() { threadPool.waitForDone(); }  )
   NET_CS_SLOT_2(waitForThreadPoolDone)
};

#endif