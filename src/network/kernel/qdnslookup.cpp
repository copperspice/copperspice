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

#include <qdnslookup.h>
#include <qdnslookup_p.h>

#include <qcoreapplication.h>
#include <qdatetime.h>
#include <qthreadstorage.h>
#include <qurl.h>

#include <algorithm>

QDnsLookupThreadPool *cs_DnsLookupThreadPool()
{
   static QDnsLookupThreadPool retval;
   return &retval;
}

QThreadStorage<bool *> *cs_DnsLookupSeedStorage()
{
   static QThreadStorage<bool *> retval;
   return &retval;
}

static bool qt_qdnsmailexchangerecord_less_than(const QDnsMailExchangeRecord &r1,
      const QDnsMailExchangeRecord &r2)
{
   // Lower numbers are more preferred than higher ones.
   return r1.preference() < r2.preference();
}

static void qt_qdnsmailexchangerecord_sort(QList<QDnsMailExchangeRecord> &records)
{
   // If we have no more than one result, we are done.
   if (records.size() <= 1) {
      return;
   }

   // Order the records by preference.
   std::sort(records.begin(), records.end(), qt_qdnsmailexchangerecord_less_than);

   int i = 0;
   while (i < records.size()) {

      // Determine the slice of records with the current preference.
      QList<QDnsMailExchangeRecord> slice;
      const quint16 slicePreference = records[i].preference();
      for (int j = i; j < records.size(); ++j) {
         if (records[j].preference() != slicePreference) {
            break;
         }
         slice << records[j];
      }

      // Randomize the slice of records.
      while (!slice.isEmpty()) {
         const unsigned int pos = qrand() % slice.size();
         records[i++] = slice.takeAt(pos);
      }
   }
}

static bool qt_qdnsservicerecord_less_than(const QDnsServiceRecord &r1, const QDnsServiceRecord &r2)
{
   // Order by priority, or if the priorities are equal,
   // put zero weight records first.
   return r1.priority() < r2.priority()
          || (r1.priority() == r2.priority()
              && r1.weight() == 0 && r2.weight() > 0);
}

static void qt_qdnsservicerecord_sort(QList<QDnsServiceRecord> &records)
{
   // If we have no more than one result, we are done.
   if (records.size() <= 1) {
      return;
   }

   // Order the records by priority, and for records with an equal
   // priority, put records with a zero weight first.
   std::sort(records.begin(), records.end(), qt_qdnsservicerecord_less_than);

   int i = 0;
   while (i < records.size()) {

      // Determine the slice of records with the current priority.
      QList<QDnsServiceRecord> slice;
      const quint16 slicePriority = records[i].priority();
      unsigned int sliceWeight = 0;

      for (int j = i; j < records.size(); ++j) {
         if (records[j].priority() != slicePriority) {
            break;
         }
         sliceWeight += records[j].weight();
         slice << records[j];
      }

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("qt_qdnsservicerecord_sort() Priority %i (size: %lli, total weight: %i)",
             slicePriority, slice.size(), sliceWeight);
#endif

      // Order the slice of records
      while (! slice.isEmpty()) {
         const unsigned int weightThreshold = qrand() % (sliceWeight + 1);
         unsigned int summedWeight = 0;

         for (int j = 0; j < slice.size(); ++j) {
            summedWeight += slice[j].weight();

            if (summedWeight >= weightThreshold) {

#if defined(CS_SHOW_DEBUG_NETWORK)
               qDebug("qt_qdnsservicerecord_sort() Adding %s %i (weight: %i)",
                     csPrintable(slice[j].target()), slice[j].port(), slice[j].weight());
#endif

               // Adjust the slice weight and take the current record.
               sliceWeight -= slice[j].weight();
               records[i++] = slice.takeAt(j);
               break;
            }
         }
      }
   }
}

const char *QDnsLookupPrivate::msgNoIpV6NameServerAdresses =
   cs_mark_tr("QDnsLookupRunnable", "IPv6 addresses for nameservers are currently not supported");

QDnsLookup::QDnsLookup(QObject *parent)
   : QObject(parent), d_ptr(new QDnsLookupPrivate)
{
   Q_D(QDnsLookup);

   d->q_ptr = this;
}

QDnsLookup::QDnsLookup(Type type, const QString &name, QObject *parent)
   : QObject(parent), d_ptr(new QDnsLookupPrivate)
{
   Q_D(QDnsLookup);

   d->q_ptr = this;
   d->name  = name;
   d->type  = type;
}

QDnsLookup::QDnsLookup(Type type, const QString &name, const QHostAddress &nameserver, QObject *parent)
   : QObject(parent), d_ptr(new QDnsLookupPrivate)
{
   Q_D(QDnsLookup);

   d->q_ptr = this;
   d->name  = name;
   d->type  = type;
   d->nameserver = nameserver;
}

QDnsLookup::~QDnsLookup()
{
}

QDnsLookup::Error QDnsLookup::error() const
{
   return d_func()->reply.error;
}

QString QDnsLookup::errorString() const
{
   return d_func()->reply.errorString;
}

bool QDnsLookup::isFinished() const
{
   return d_func()->isFinished;
}

QString QDnsLookup::name() const
{
   return d_func()->name;
}

void QDnsLookup::setName(const QString &name)
{
   Q_D(QDnsLookup);

   if (name != d->name) {
      d->name = name;
      emit nameChanged(name);
   }
}

QDnsLookup::Type QDnsLookup::type() const
{
   return d_func()->type;
}

void QDnsLookup::setType(Type type)
{
   Q_D(QDnsLookup);
   if (type != d->type) {
      d->type = type;
      emit typeChanged(type);
   }
}

QHostAddress QDnsLookup::nameserver() const
{
   return d_func()->nameserver;
}

void QDnsLookup::setNameserver(const QHostAddress &nameserver)
{
   Q_D(QDnsLookup);
   if (nameserver != d->nameserver) {
      d->nameserver = nameserver;
      emit nameserverChanged(nameserver);
   }
}

QList<QDnsDomainNameRecord> QDnsLookup::canonicalNameRecords() const
{
   return d_func()->reply.canonicalNameRecords;
}

QList<QDnsHostAddressRecord> QDnsLookup::hostAddressRecords() const
{
   return d_func()->reply.hostAddressRecords;
}

QList<QDnsMailExchangeRecord> QDnsLookup::mailExchangeRecords() const
{
   return d_func()->reply.mailExchangeRecords;
}

QList<QDnsDomainNameRecord> QDnsLookup::nameServerRecords() const
{
   return d_func()->reply.nameServerRecords;
}

QList<QDnsDomainNameRecord> QDnsLookup::pointerRecords() const
{
   return d_func()->reply.pointerRecords;
}

QList<QDnsServiceRecord> QDnsLookup::serviceRecords() const
{
   return d_func()->reply.serviceRecords;
}

QList<QDnsTextRecord> QDnsLookup::textRecords() const
{
   return d_func()->reply.textRecords;
}

void QDnsLookup::abort()
{
   Q_D(QDnsLookup);
   if (d->runnable) {
      d->runnable = nullptr;
      d->reply = QDnsLookupReply();
      d->reply.error = QDnsLookup::OperationCancelledError;
      d->reply.errorString = tr("Operation cancelled");
      d->isFinished = true;
      emit finished();
   }
}

void QDnsLookup::lookup()
{
   Q_D(QDnsLookup);
   d->isFinished = false;
   d->reply = QDnsLookupReply();
   d->runnable = new QDnsLookupRunnable(d->type, QUrl::toAce(d->name), d->nameserver);

   connect(d->runnable, &QDnsLookupRunnable::finished,
         this, &QDnsLookup::_q_lookupFinished, Qt::BlockingQueuedConnection);

   cs_DnsLookupThreadPool()->start(d->runnable);
}

QDnsDomainNameRecord::QDnsDomainNameRecord()
   : d(new QDnsDomainNameRecordPrivate)
{
}

QDnsDomainNameRecord::QDnsDomainNameRecord(const QDnsDomainNameRecord &other)
   : d(other.d)
{
}

QDnsDomainNameRecord::~QDnsDomainNameRecord()
{
}

QString QDnsDomainNameRecord::name() const
{
   return d->name;
}

quint32 QDnsDomainNameRecord::timeToLive() const
{
   return d->timeToLive;
}

QString QDnsDomainNameRecord::value() const
{
   return d->value;
}

QDnsDomainNameRecord &QDnsDomainNameRecord::operator=(const QDnsDomainNameRecord &other)
{
   d = other.d;
   return *this;
}

QDnsHostAddressRecord::QDnsHostAddressRecord()
   : d(new QDnsHostAddressRecordPrivate)
{
}

QDnsHostAddressRecord::QDnsHostAddressRecord(const QDnsHostAddressRecord &other)
   : d(other.d)
{
}

QDnsHostAddressRecord::~QDnsHostAddressRecord()
{
}

QString QDnsHostAddressRecord::name() const
{
   return d->name;
}

quint32 QDnsHostAddressRecord::timeToLive() const
{
   return d->timeToLive;
}

QHostAddress QDnsHostAddressRecord::value() const
{
   return d->value;
}

QDnsHostAddressRecord &QDnsHostAddressRecord::operator=(const QDnsHostAddressRecord &other)
{
   d = other.d;
   return *this;
}


QDnsMailExchangeRecord::QDnsMailExchangeRecord()
   : d(new QDnsMailExchangeRecordPrivate)
{
}

QDnsMailExchangeRecord::QDnsMailExchangeRecord(const QDnsMailExchangeRecord &other)
   : d(other.d)
{
}

QDnsMailExchangeRecord::~QDnsMailExchangeRecord()
{
}

QString QDnsMailExchangeRecord::exchange() const
{
   return d->exchange;
}

QString QDnsMailExchangeRecord::name() const
{
   return d->name;
}

quint16 QDnsMailExchangeRecord::preference() const
{
   return d->preference;
}

quint32 QDnsMailExchangeRecord::timeToLive() const
{
   return d->timeToLive;
}

QDnsMailExchangeRecord &QDnsMailExchangeRecord::operator=(const QDnsMailExchangeRecord &other)
{
   d = other.d;
   return *this;
}

QDnsServiceRecord::QDnsServiceRecord()
   : d(new QDnsServiceRecordPrivate)
{
}

QDnsServiceRecord::QDnsServiceRecord(const QDnsServiceRecord &other)
   : d(other.d)
{
}

QDnsServiceRecord::~QDnsServiceRecord()
{
}

QString QDnsServiceRecord::name() const
{
   return d->name;
}

quint16 QDnsServiceRecord::port() const
{
   return d->port;
}

quint16 QDnsServiceRecord::priority() const
{
   return d->priority;
}

QString QDnsServiceRecord::target() const
{
   return d->target;
}

quint32 QDnsServiceRecord::timeToLive() const
{
   return d->timeToLive;
}

quint16 QDnsServiceRecord::weight() const
{
   return d->weight;
}

QDnsServiceRecord &QDnsServiceRecord::operator=(const QDnsServiceRecord &other)
{
   d = other.d;
   return *this;
}

QDnsTextRecord::QDnsTextRecord()
   : d(new QDnsTextRecordPrivate)
{
}

QDnsTextRecord::QDnsTextRecord(const QDnsTextRecord &other)
   : d(other.d)
{
}

QDnsTextRecord::~QDnsTextRecord()
{
}

QString QDnsTextRecord::name() const
{
   return d->name;
}

quint32 QDnsTextRecord::timeToLive() const
{
   return d->timeToLive;
}

QList<QByteArray> QDnsTextRecord::values() const
{
   return d->values;
}

QDnsTextRecord &QDnsTextRecord::operator=(const QDnsTextRecord &other)
{
   d = other.d;
   return *this;
}

void QDnsLookupPrivate::_q_lookupFinished(const QDnsLookupReply &_reply)
{
   Q_Q(QDnsLookup);

   if (runnable == q->sender()) {

#if defined(CS_SHOW_DEBUG_NETWORK)
      qDebug("DNS reply for %s: %i (%s)", csPrintable(name), _reply.error, csPrintable(_reply.errorString));
#endif

      reply = _reply;
      runnable = nullptr;
      isFinished = true;
      emit q->finished();
   }
}

void QDnsLookupRunnable::run()
{
   QDnsLookupReply reply;

   // Validate input.
   if (requestName.isEmpty()) {
      reply.error = QDnsLookup::InvalidRequestError;
      reply.errorString = tr("Invalid domain name");
      emit finished(reply);
      return;
   }

   // Perform request.
   query(requestType, requestName, nameserver, &reply);

   // Sort results.
   if (! cs_DnsLookupSeedStorage()->hasLocalData()) {
      qsrand(QTime(0, 0, 0).msecsTo(QTime::currentTime()) ^ reinterpret_cast<quintptr>(this));
      cs_DnsLookupSeedStorage()->setLocalData(new bool(true));
   }

   qt_qdnsmailexchangerecord_sort(reply.mailExchangeRecords);
   qt_qdnsservicerecord_sort(reply.serviceRecords);

   emit finished(reply);
}

QDnsLookupThreadPool::QDnsLookupThreadPool()
   : signalsConnected(false)
{
   // Run up to 5 lookups in parallel.
   setMaxThreadCount(5);
}

void QDnsLookupThreadPool::start(QRunnable *runnable)
{
   // Ensure threads complete at application destruction.
   if (! signalsConnected) {
      QMutexLocker signalsLocker(&signalsMutex);

      if (!signalsConnected) {
         QCoreApplication *app = QCoreApplication::instance();

         if (!app) {
            qWarning("QDnsLookupThreadPool::start() QCoreApplication must be started before calling this method");
            delete runnable;
            return;
         }

         moveToThread(app->thread());
         connect(app, &QCoreApplication::destroyed, this, &QDnsLookupThreadPool::_q_applicationDestroyed, Qt::DirectConnection);
         signalsConnected = true;
      }
   }

   QThreadPool::start(runnable);
}

void QDnsLookupThreadPool::_q_applicationDestroyed()
{
   waitForDone();
   signalsConnected = false;
}

void QDnsLookup::_q_lookupFinished(const QDnsLookupReply &reply)
{
   Q_D(QDnsLookup);
   d->_q_lookupFinished(reply);
}
