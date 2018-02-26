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

#include <qdnslookup.h>
#include <qdnslookup_p.h>

#include <qcoreapplication.h>
#include <qdatetime.h>
#include <qthreadstorage.h>
#include <qurl.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QDnsLookupThreadPool, theDnsLookupThreadPool);
Q_GLOBAL_STATIC(QThreadStorage<bool *>, theDnsLookupSeedStorage);

static bool qt_qdnsmailexchangerecord_less_than(const QDnsMailExchangeRecord &r1,
      const QDnsMailExchangeRecord &r2)
{
   // Lower numbers are more preferred than higher ones.
   return r1.preference() < r2.preference();
}

/*!
    Sorts a list of QDnsMailExchangeRecord objects according to RFC 5321.
*/

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

/*!
    Sorts a list of QDnsServiceRecord objects according to RFC 2782.
*/

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
#ifdef QDNSLOOKUP_DEBUG
      qDebug("qt_qdnsservicerecord_sort() : priority %i (size: %i, total weight: %i)",
             slicePriority, slice.size(), sliceWeight);
#endif

      // Order the slice of records.
      while (!slice.isEmpty()) {
         const unsigned int weightThreshold = qrand() % (sliceWeight + 1);
         unsigned int summedWeight = 0;
         for (int j = 0; j < slice.size(); ++j) {
            summedWeight += slice[j].weight();
            if (summedWeight >= weightThreshold) {
#ifdef QDNSLOOKUP_DEBUG
               qDebug("qt_qdnsservicerecord_sort() : adding %s %i (weight: %i)",
                      qPrintable(slice[j].target()), slice[j].port(),
                      slice[j].weight());
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
   QT_TRANSLATE_NOOP("QDnsLookupRunnable", "IPv6 addresses for nameservers are currently not supported");

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

/*!
    \property QDnsLookup::nameserver
    \brief the nameserver to use for DNS lookup.
*/

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

/*!
    Returns the list of canonical name records associated with this lookup.
*/

QList<QDnsDomainNameRecord> QDnsLookup::canonicalNameRecords() const
{
   return d_func()->reply.canonicalNameRecords;
}

/*!
    Returns the list of host address records associated with this lookup.
*/

QList<QDnsHostAddressRecord> QDnsLookup::hostAddressRecords() const
{
   return d_func()->reply.hostAddressRecords;
}

/*!
    Returns the list of mail exchange records associated with this lookup.

    The records are sorted according to
    \l{http://www.rfc-editor.org/rfc/rfc5321.txt}{RFC 5321}, so if you use them
    to connect to servers, you should try them in the order they are listed.
*/

QList<QDnsMailExchangeRecord> QDnsLookup::mailExchangeRecords() const
{
   return d_func()->reply.mailExchangeRecords;
}

/*!
    Returns the list of name server records associated with this lookup.
*/

QList<QDnsDomainNameRecord> QDnsLookup::nameServerRecords() const
{
   return d_func()->reply.nameServerRecords;
}

/*!
    Returns the list of pointer records associated with this lookup.
*/

QList<QDnsDomainNameRecord> QDnsLookup::pointerRecords() const
{
   return d_func()->reply.pointerRecords;
}

/*!
    Returns the list of service records associated with this lookup.

    The records are sorted according to
    \l{http://www.rfc-editor.org/rfc/rfc2782.txt}{RFC 2782}, so if you use them
    to connect to servers, you should try them in the order they are listed.
*/

QList<QDnsServiceRecord> QDnsLookup::serviceRecords() const
{
   return d_func()->reply.serviceRecords;
}

/*!
    Returns the list of text records associated with this lookup.
*/

QList<QDnsTextRecord> QDnsLookup::textRecords() const
{
   return d_func()->reply.textRecords;
}

/*!
    Aborts the DNS lookup operation.

    If the lookup is already finished, does nothing.
*/

void QDnsLookup::abort()
{
   Q_D(QDnsLookup);
   if (d->runnable) {
      d->runnable = 0;
      d->reply = QDnsLookupReply();
      d->reply.error = QDnsLookup::OperationCancelledError;
      d->reply.errorString = tr("Operation cancelled");
      d->isFinished = true;
      emit finished();
   }
}

/*!
    Performs the DNS lookup.

    The \l{QDnsLookup::finished()}{finished()} signal is emitted upon completion.
*/

void QDnsLookup::lookup()
{
   Q_D(QDnsLookup);
   d->isFinished = false;
   d->reply = QDnsLookupReply();
   d->runnable = new QDnsLookupRunnable(d->type, QUrl::toAce(d->name), d->nameserver);
   connect(d->runnable, SIGNAL(finished(QDnsLookupReply)),
           this, SLOT(_q_lookupFinished(QDnsLookupReply)),
           Qt::BlockingQueuedConnection);
   theDnsLookupThreadPool()->start(d->runnable);
}

/*!
    \class QDnsDomainNameRecord
    \brief The QDnsDomainNameRecord class stores information about a domain
    name record.

    \inmodule QtNetwork
    \ingroup network
    \ingroup shared

    When performing a name server lookup, zero or more records will be returned.
    Each record is represented by a QDnsDomainNameRecord instance.

    \sa QDnsLookup
*/

/*!
    Constructs an empty domain name record object.
*/

QDnsDomainNameRecord::QDnsDomainNameRecord()
   : d(new QDnsDomainNameRecordPrivate)
{
}

/*!
    Constructs a copy of \a other.
*/

QDnsDomainNameRecord::QDnsDomainNameRecord(const QDnsDomainNameRecord &other)
   : d(other.d)
{
}

/*!
    Destroys a domain name record.
*/

QDnsDomainNameRecord::~QDnsDomainNameRecord()
{
}

/*!
    Returns the name for this record.
*/

QString QDnsDomainNameRecord::name() const
{
   return d->name;
}

/*!
    Returns the duration in seconds for which this record is valid.
*/

quint32 QDnsDomainNameRecord::timeToLive() const
{
   return d->timeToLive;
}

/*!
    Returns the value for this domain name record.
*/

QString QDnsDomainNameRecord::value() const
{
   return d->value;
}

/*!
    Assigns the data of the \a other object to this record object,
    and returns a reference to it.
*/

QDnsDomainNameRecord &QDnsDomainNameRecord::operator=(const QDnsDomainNameRecord &other)
{
   d = other.d;
   return *this;
}
/*!
    \fn void QDnsDomainNameRecord::swap(QDnsDomainNameRecord &other)

    Swaps this domain-name record instance with \a other. This
    function is very fast and never fails.
*/

/*!
    \class QDnsHostAddressRecord
    \brief The QDnsHostAddressRecord class stores information about a host
    address record.

    \inmodule QtNetwork
    \ingroup network
    \ingroup shared

    When performing an address lookup, zero or more records will be
    returned. Each record is represented by a QDnsHostAddressRecord instance.

    \sa QDnsLookup
*/

/*!
    Constructs an empty host address record object.
*/

QDnsHostAddressRecord::QDnsHostAddressRecord()
   : d(new QDnsHostAddressRecordPrivate)
{
}

/*!
    Constructs a copy of \a other.
*/

QDnsHostAddressRecord::QDnsHostAddressRecord(const QDnsHostAddressRecord &other)
   : d(other.d)
{
}

/*!
    Destroys a host address record.
*/

QDnsHostAddressRecord::~QDnsHostAddressRecord()
{
}

/*!
    Returns the name for this record.
*/

QString QDnsHostAddressRecord::name() const
{
   return d->name;
}

/*!
    Returns the duration in seconds for which this record is valid.
*/

quint32 QDnsHostAddressRecord::timeToLive() const
{
   return d->timeToLive;
}

/*!
    Returns the value for this host address record.
*/

QHostAddress QDnsHostAddressRecord::value() const
{
   return d->value;
}

/*!
    Assigns the data of the \a other object to this record object,
    and returns a reference to it.
*/

QDnsHostAddressRecord &QDnsHostAddressRecord::operator=(const QDnsHostAddressRecord &other)
{
   d = other.d;
   return *this;
}
/*!
    \fn void QDnsHostAddressRecord::swap(QDnsHostAddressRecord &other)

    Swaps this host address record instance with \a other. This
    function is very fast and never fails.
*/

/*!
    \class QDnsMailExchangeRecord
    \brief The QDnsMailExchangeRecord class stores information about a DNS MX record.

    \inmodule QtNetwork
    \ingroup network
    \ingroup shared

    When performing a lookup on a service, zero or more records will be
    returned. Each record is represented by a QDnsMailExchangeRecord instance.

    The meaning of the fields is defined in
    \l{http://www.rfc-editor.org/rfc/rfc1035.txt}{RFC 1035}.

    \sa QDnsLookup
*/

/*!
    Constructs an empty mail exchange record object.
*/

QDnsMailExchangeRecord::QDnsMailExchangeRecord()
   : d(new QDnsMailExchangeRecordPrivate)
{
}

/*!
    Constructs a copy of \a other.
*/

QDnsMailExchangeRecord::QDnsMailExchangeRecord(const QDnsMailExchangeRecord &other)
   : d(other.d)
{
}

/*!
    Destroys a mail exchange record.
*/

QDnsMailExchangeRecord::~QDnsMailExchangeRecord()
{
}

/*!
    Returns the domain name of the mail exchange for this record.
*/

QString QDnsMailExchangeRecord::exchange() const
{
   return d->exchange;
}

/*!
    Returns the name for this record.
*/

QString QDnsMailExchangeRecord::name() const
{
   return d->name;
}

/*!
    Returns the preference for this record.
*/

quint16 QDnsMailExchangeRecord::preference() const
{
   return d->preference;
}

/*!
    Returns the duration in seconds for which this record is valid.
*/

quint32 QDnsMailExchangeRecord::timeToLive() const
{
   return d->timeToLive;
}

/*!
    Assigns the data of the \a other object to this record object,
    and returns a reference to it.
*/

QDnsMailExchangeRecord &QDnsMailExchangeRecord::operator=(const QDnsMailExchangeRecord &other)
{
   d = other.d;
   return *this;
}
/*!
    \fn void QDnsMailExchangeRecord::swap(QDnsMailExchangeRecord &other)

    Swaps this mail exchange record with \a other. This function is
    very fast and never fails.
*/

/*!
    \class QDnsServiceRecord
    \brief The QDnsServiceRecord class stores information about a DNS SRV record.

    \inmodule QtNetwork
    \ingroup network
    \ingroup shared

    When performing a lookup on a service, zero or more records will be
    returned. Each record is represented by a QDnsServiceRecord instance.

    The meaning of the fields is defined in
    \l{http://www.rfc-editor.org/rfc/rfc2782.txt}{RFC 2782}.

    \sa QDnsLookup
*/

/*!
    Constructs an empty service record object.
*/

QDnsServiceRecord::QDnsServiceRecord()
   : d(new QDnsServiceRecordPrivate)
{
}

/*!
    Constructs a copy of \a other.
*/

QDnsServiceRecord::QDnsServiceRecord(const QDnsServiceRecord &other)
   : d(other.d)
{
}

/*!
    Destroys a service record.
*/

QDnsServiceRecord::~QDnsServiceRecord()
{
}

/*!
    Returns the name for this record.
*/

QString QDnsServiceRecord::name() const
{
   return d->name;
}

/*!
    Returns the port on the target host for this service record.
*/

quint16 QDnsServiceRecord::port() const
{
   return d->port;
}

/*!
    Returns the priority for this service record.

    A client must attempt to contact the target host with the lowest-numbered
    priority.
*/

quint16 QDnsServiceRecord::priority() const
{
   return d->priority;
}

/*!
    Returns the domain name of the target host for this service record.
*/

QString QDnsServiceRecord::target() const
{
   return d->target;
}

/*!
    Returns the duration in seconds for which this record is valid.
*/

quint32 QDnsServiceRecord::timeToLive() const
{
   return d->timeToLive;
}

/*!
    Returns the weight for this service record.

    The weight field specifies a relative weight for entries with the same
    priority. Entries with higher weights should be selected with a higher
    probability.
*/

quint16 QDnsServiceRecord::weight() const
{
   return d->weight;
}

/*!
    Assigns the data of the \a other object to this record object,
    and returns a reference to it.
*/

QDnsServiceRecord &QDnsServiceRecord::operator=(const QDnsServiceRecord &other)
{
   d = other.d;
   return *this;
}
/*!
    \fn void QDnsServiceRecord::swap(QDnsServiceRecord &other)

    Swaps this service record instance with \a other. This function is
    very fast and never fails.
*/

/*!
    \class QDnsTextRecord
    \brief The QDnsTextRecord class stores information about a DNS TXT record.

    \inmodule QtNetwork
    \ingroup network
    \ingroup shared

    When performing a text lookup, zero or more records will be
    returned. Each record is represented by a QDnsTextRecord instance.

    The meaning of the fields is defined in
    \l{http://www.rfc-editor.org/rfc/rfc1035.txt}{RFC 1035}.

    \sa QDnsLookup
*/

/*!
    Constructs an empty text record object.
*/

QDnsTextRecord::QDnsTextRecord()
   : d(new QDnsTextRecordPrivate)
{
}

/*!
    Constructs a copy of \a other.
*/

QDnsTextRecord::QDnsTextRecord(const QDnsTextRecord &other)
   : d(other.d)
{
}

/*!
    Destroys a text record.
*/

QDnsTextRecord::~QDnsTextRecord()
{
}

/*!
    Returns the name for this text record.
*/

QString QDnsTextRecord::name() const
{
   return d->name;
}

/*!
    Returns the duration in seconds for which this record is valid.
*/

quint32 QDnsTextRecord::timeToLive() const
{
   return d->timeToLive;
}

/*!
    Returns the values for this text record.
*/

QList<QByteArray> QDnsTextRecord::values() const
{
   return d->values;
}

/*!
    Assigns the data of the \a other object to this record object,
    and returns a reference to it.
*/

QDnsTextRecord &QDnsTextRecord::operator=(const QDnsTextRecord &other)
{
   d = other.d;
   return *this;
}
/*!
    \fn void QDnsTextRecord::swap(QDnsTextRecord &other)

    Swaps this text record instance with \a other. This function is
    very fast and never fails.
*/

void QDnsLookupPrivate::_q_lookupFinished(const QDnsLookupReply &_reply)
{
   Q_Q(QDnsLookup);
   if (runnable == q->sender()) {
#ifdef QDNSLOOKUP_DEBUG
      qDebug("DNS reply for %s: %i (%s)", qPrintable(name), _reply.error, qPrintable(_reply.errorString));
#endif
      reply = _reply;
      runnable = 0;
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
   if (!theDnsLookupSeedStorage()->hasLocalData()) {
      qsrand(QTime(0, 0, 0).msecsTo(QTime::currentTime()) ^ reinterpret_cast<quintptr>(this));
      theDnsLookupSeedStorage()->setLocalData(new bool(true));
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
            qWarning("QDnsLookup requires a QCoreApplication");
            delete runnable;
            return;
         }

         moveToThread(app->thread());
         connect(app, SIGNAL(destroyed()), SLOT(_q_applicationDestroyed()), Qt::DirectConnection);
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

QT_END_NAMESPACE
