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

#ifndef QDNSLOOKUP_P_H
#define QDNSLOOKUP_P_H

#include <qmutex.h>
#include <qrunnable.h>
#include <qsharedpointer.h>
#include <qthreadpool.h>
#include <qdnslookup.h>
#include <qhostaddress.h>

class QDnsLookupRunnable;

class QDnsLookupReply
{
 public:
    QDnsLookupReply()
      : error(QDnsLookup::NoError)
    { }

    QDnsLookup::Error error;
    QString errorString;

    QList<QDnsDomainNameRecord> canonicalNameRecords;
    QList<QDnsHostAddressRecord> hostAddressRecords;
    QList<QDnsMailExchangeRecord> mailExchangeRecords;
    QList<QDnsDomainNameRecord> nameServerRecords;
    QList<QDnsDomainNameRecord> pointerRecords;
    QList<QDnsServiceRecord> serviceRecords;
    QList<QDnsTextRecord> textRecords;
};

class QDnsLookupPrivate
{
   Q_DECLARE_PUBLIC(QDnsLookup)

 public:
    QDnsLookupPrivate()
        : isFinished(false), type(QDnsLookup::A), runnable(nullptr)
    { }

    void _q_lookupFinished(const QDnsLookupReply &reply);

    static const char *msgNoIpV6NameServerAdresses;

    bool isFinished;
    QString name;
    QDnsLookup::Type type;
    QHostAddress nameserver;
    QDnsLookupReply reply;
    QDnsLookupRunnable *runnable;

 protected:
   QDnsLookup *q_ptr;
};

class QDnsLookupRunnable : public QObject, public QRunnable
{
    NET_CS_OBJECT(QDnsLookupRunnable)

 public:
    QDnsLookupRunnable(QDnsLookup::Type type, const QByteArray &name, const QHostAddress &nameserver)
        : requestType(type), requestName(name) , nameserver(nameserver)
    { }

    void run() override;

    NET_CS_SIGNAL_1(Public, void finished(const QDnsLookupReply &reply))
    NET_CS_SIGNAL_2(finished, reply)

 private:
    static void query(const int requestType, const QByteArray &requestName,
                  const QHostAddress &nameserver, QDnsLookupReply *reply);

    QDnsLookup::Type requestType;
    QByteArray requestName;
    QHostAddress nameserver;
};

class QDnsLookupThreadPool : public QThreadPool
{
    NET_CS_OBJECT(QDnsLookupThreadPool)

 public:
    QDnsLookupThreadPool();
    void start(QRunnable *runnable);

 private:
    QMutex signalsMutex;
    bool signalsConnected;

    NET_CS_SLOT_1(Private, void _q_applicationDestroyed())
    NET_CS_SLOT_2(_q_applicationDestroyed)
};

class QDnsRecordPrivate : public QSharedData
{
 public:
    QDnsRecordPrivate()
        : timeToLive(0)
    { }

    QString name;
    quint32 timeToLive;
};

class QDnsDomainNameRecordPrivate : public QDnsRecordPrivate
{
 public:
    QDnsDomainNameRecordPrivate()
    { }

    QString value;
};

class QDnsHostAddressRecordPrivate : public QDnsRecordPrivate
{
 public:
    QDnsHostAddressRecordPrivate()
    { }

    QHostAddress value;
};

class QDnsMailExchangeRecordPrivate : public QDnsRecordPrivate
{
 public:
    QDnsMailExchangeRecordPrivate()
        : preference(0)
    { }

    QString exchange;
    quint16 preference;
};

class QDnsServiceRecordPrivate : public QDnsRecordPrivate
{
 public:
    QDnsServiceRecordPrivate()
        : port(0), priority(0), weight(0)
    { }

    QString target;
    quint16 port;
    quint16 priority;
    quint16 weight;
};

class QDnsTextRecordPrivate : public QDnsRecordPrivate
{
 public:
    QDnsTextRecordPrivate()
    { }

    QList<QByteArray> values;
};

#endif
