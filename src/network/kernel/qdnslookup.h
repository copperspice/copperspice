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

#ifndef QDNSLOOKUP_H
#define QDNSLOOKUP_H

#include <qlist.h>
#include <qobject.h>
#include <qshareddata.h>
#include <qsharedpointer.h>
#include <qstring.h>



class QHostAddress;
class QDnsLookupPrivate;
class QDnsDomainNameRecordPrivate;
class QDnsHostAddressRecordPrivate;
class QDnsMailExchangeRecordPrivate;
class QDnsServiceRecordPrivate;
class QDnsTextRecordPrivate;
class QDnsLookupReply;

class Q_NETWORK_EXPORT QDnsDomainNameRecord
{
 public:
    QDnsDomainNameRecord();
    QDnsDomainNameRecord(const QDnsDomainNameRecord &other);
    ~QDnsDomainNameRecord();

    QDnsDomainNameRecord &operator=(const QDnsDomainNameRecord &other);

    QDnsDomainNameRecord &operator=(QDnsDomainNameRecord &&other) {
      swap(other);
      return *this;
    }


    void swap(QDnsDomainNameRecord &other) {
      qSwap(d, other.d);
    }

    QString name() const;
    quint32 timeToLive() const;
    QString value() const;

 private:
    QSharedDataPointer<QDnsDomainNameRecordPrivate> d;
    friend class QDnsLookupRunnable;
};

class Q_NETWORK_EXPORT QDnsHostAddressRecord
{
 public:
    QDnsHostAddressRecord();
    QDnsHostAddressRecord(const QDnsHostAddressRecord &other);
    ~QDnsHostAddressRecord();

    QDnsHostAddressRecord &operator=(const QDnsHostAddressRecord &other);

    QDnsHostAddressRecord &operator=(QDnsHostAddressRecord &&other)  {
      swap(other);
      return *this;
   }


   void swap(QDnsHostAddressRecord &other)  {
      qSwap(d, other.d);
   }

   QString name() const;
   quint32 timeToLive() const;
   QHostAddress value() const;

 private:
    QSharedDataPointer<QDnsHostAddressRecordPrivate> d;
    friend class QDnsLookupRunnable;
};

class Q_NETWORK_EXPORT QDnsMailExchangeRecord
{
 public:
    QDnsMailExchangeRecord();
    QDnsMailExchangeRecord(const QDnsMailExchangeRecord &other);
   ~QDnsMailExchangeRecord();

   QDnsMailExchangeRecord &operator=(const QDnsMailExchangeRecord &other);

    QDnsMailExchangeRecord &operator=(QDnsMailExchangeRecord &&other)  {
      swap(other);
      return *this;
    }

    void swap(QDnsMailExchangeRecord &other) {
      qSwap(d, other.d);
   }

    QString exchange() const;
    QString name() const;
    quint16 preference() const;
    quint32 timeToLive() const;

 private:
    QSharedDataPointer<QDnsMailExchangeRecordPrivate> d;
    friend class QDnsLookupRunnable;
};

class Q_NETWORK_EXPORT QDnsServiceRecord
{
 public:
    QDnsServiceRecord();
    QDnsServiceRecord(const QDnsServiceRecord &other);
    ~QDnsServiceRecord();

    QDnsServiceRecord &operator=(QDnsServiceRecord &&other)  {
      swap(other);
      return *this;
    }

    QDnsServiceRecord &operator=(const QDnsServiceRecord &other);

    void swap(QDnsServiceRecord &other) {
      qSwap(d, other.d);
    }

    QString name() const;
    quint16 port() const;
    quint16 priority() const;
    QString target() const;
    quint32 timeToLive() const;
    quint16 weight() const;

 private:
    QSharedDataPointer<QDnsServiceRecordPrivate> d;
    friend class QDnsLookupRunnable;
};

class Q_NETWORK_EXPORT QDnsTextRecord
{
 public:
    QDnsTextRecord();
    QDnsTextRecord(const QDnsTextRecord &other);
    ~QDnsTextRecord();

    QDnsTextRecord &operator=(const QDnsTextRecord &other);

    QDnsTextRecord &operator=(QDnsTextRecord &&other) {
      swap(other);
      return *this;
    }

    QString name() const;
    quint32 timeToLive() const;
    QList<QByteArray> values() const;

    void swap(QDnsTextRecord &other) {
      qSwap(d, other.d);
    }

 private:
    QSharedDataPointer<QDnsTextRecordPrivate> d;
    friend class QDnsLookupRunnable;
};

class Q_NETWORK_EXPORT QDnsLookup : public QObject
{
    NET_CS_OBJECT(QDnsLookup)

    NET_CS_PROPERTY_READ(error, error)
    NET_CS_PROPERTY_NOTIFY(error, finished)

    NET_CS_PROPERTY_READ(errorString, errorString)
    NET_CS_PROPERTY_NOTIFY(errorString, finished)

    NET_CS_PROPERTY_READ(name, name)
    NET_CS_PROPERTY_WRITE(name, setName)
    NET_CS_PROPERTY_NOTIFY(name, nameChanged)

    NET_CS_PROPERTY_READ(type, type)
    NET_CS_PROPERTY_WRITE(type, setType)
    NET_CS_PROPERTY_NOTIFY(type, typeChanged)

    NET_CS_PROPERTY_READ(nameserver, nameserver)
    NET_CS_PROPERTY_WRITE(nameserver, setNameserver)
    NET_CS_PROPERTY_NOTIFY(nameserver, nameserverChanged)

 public:
    NET_CS_REGISTER_ENUM(enum Error
       {
           NoError = 0,
           ResolverError,
           OperationCancelledError,
           InvalidRequestError,
           InvalidReplyError,
           ServerFailureError,
           ServerRefusedError,
           NotFoundError
       };
    )

    NET_CS_REGISTER_ENUM( enum Type
       {
           A = 1,
           AAAA = 28,
           ANY = 255,
           CNAME = 5,
           MX = 15,
           NS = 2,
           PTR = 12,
           SRV = 33,
           TXT = 16
       };
    )

    explicit QDnsLookup(QObject *parent = nullptr);
    QDnsLookup(Type type, const QString &name, QObject *parent = nullptr);
    QDnsLookup(Type type, const QString &name, const QHostAddress &nameserver, QObject *parent = nullptr);
    ~QDnsLookup();

    Error error() const;
    QString errorString() const;
    bool isFinished() const;

    QString name() const;
    void setName(const QString &name);

    Type type() const;
    void setType(QDnsLookup::Type);

    QHostAddress nameserver() const;
    void setNameserver(const QHostAddress &nameserver);

    QList<QDnsDomainNameRecord> canonicalNameRecords() const;
    QList<QDnsHostAddressRecord> hostAddressRecords() const;
    QList<QDnsMailExchangeRecord> mailExchangeRecords() const;
    QList<QDnsDomainNameRecord> nameServerRecords() const;
    QList<QDnsDomainNameRecord> pointerRecords() const;
    QList<QDnsServiceRecord> serviceRecords() const;
    QList<QDnsTextRecord> textRecords() const;

    NET_CS_SIGNAL_1(Public, void finished())
    NET_CS_SIGNAL_2(finished)

    NET_CS_SIGNAL_1(Public, void nameChanged(const QString &name))
    NET_CS_SIGNAL_2(nameChanged, name)

    NET_CS_SIGNAL_1(Public, void typeChanged(Type type))
    NET_CS_SIGNAL_2(typeChanged, type)

    NET_CS_SIGNAL_1(Public, void nameserverChanged(const QHostAddress &nameserver))
    NET_CS_SIGNAL_2(nameserverChanged, nameserver)

    NET_CS_SLOT_1(Public, void abort())
    NET_CS_SLOT_2(abort)

    NET_CS_SLOT_1(Public, void lookup())
    NET_CS_SLOT_2(lookup)

  protected:
    QScopedPointer<QDnsLookupPrivate> d_ptr;

  private:
    Q_DECLARE_PRIVATE(QDnsLookup)

    NET_CS_SLOT_1(Private, void _q_lookupFinished(const QDnsLookupReply &reply))
    NET_CS_SLOT_2(_q_lookupFinished)
};



#endif // QDNSLOOKUP_H
