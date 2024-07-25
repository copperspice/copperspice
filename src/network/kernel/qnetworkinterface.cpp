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

#include <qnetworkinterface.h>
#include <qnetworkinterface_p.h>

#include <qdebug.h>
#include <qendian.h>

#ifndef QT_NO_NETWORKINTERFACE

QNetworkInterfaceManager *cs_Manager()
{
   static QNetworkInterfaceManager retval;
   return &retval;
}

static QList<QNetworkInterfacePrivate *> postProcess(QList<QNetworkInterfacePrivate *> list)
{
   // Some platforms report a netmask but don't report a broadcast address
   // Go through all available addresses and calculate the broadcast address
   // from the IP and the netmask
   //
   // This is an IPv4-only thing -- IPv6 has no concept of broadcasts
   // The math is:
   //    broadcast = IP | ~netmask

   QList<QNetworkInterfacePrivate *>::iterator it        = list.begin();
   const QList<QNetworkInterfacePrivate *>::iterator end = list.end();

   for ( ; it != end; ++it) {
      QList<QNetworkAddressEntry>::iterator addr_it        = (*it)->addressEntries.begin();
      const QList<QNetworkAddressEntry>::iterator addr_end = (*it)->addressEntries.end();

      for ( ; addr_it != addr_end; ++addr_it) {
         if (addr_it->ip().protocol() != QAbstractSocket::IPv4Protocol) {
            continue;
         }

         if (!addr_it->netmask().isNull() && addr_it->broadcast().isNull()) {
            QHostAddress bcast = addr_it->ip();
            bcast = QHostAddress(bcast.toIPv4Address() | ~addr_it->netmask().toIPv4Address());
            addr_it->setBroadcast(bcast);
         }
      }
   }

   return list;
}

QNetworkInterfaceManager::QNetworkInterfaceManager()
{
}

QNetworkInterfaceManager::~QNetworkInterfaceManager()
{
}

QSharedDataPointer<QNetworkInterfacePrivate> QNetworkInterfaceManager::interfaceFromName(const QString &name)
{
   QList<QSharedDataPointer<QNetworkInterfacePrivate> > interfaceList = allInterfaces();
   QList<QSharedDataPointer<QNetworkInterfacePrivate> >::const_iterator it = interfaceList.constBegin();

   bool ok;
   uint index = name.toInteger<uint>(&ok);

   for ( ; it != interfaceList.constEnd(); ++it) {
      if (ok && (*it)->index == int(index)) {
         return *it;

      } else if ((*it)->name == name)  {
         return *it;
      }
   }

   return empty;
}

QSharedDataPointer<QNetworkInterfacePrivate> QNetworkInterfaceManager::interfaceFromIndex(int index)
{
   QList<QSharedDataPointer<QNetworkInterfacePrivate> > interfaceList      = allInterfaces();
   QList<QSharedDataPointer<QNetworkInterfacePrivate> >::const_iterator it = interfaceList.constBegin();

   for ( ; it != interfaceList.constEnd(); ++it)
      if ((*it)->index == index) {
         return *it;
      }

   return empty;
}

QList<QSharedDataPointer<QNetworkInterfacePrivate> > QNetworkInterfaceManager::allInterfaces()
{
   QList<QNetworkInterfacePrivate *> list = postProcess(scan());
   QList<QSharedDataPointer<QNetworkInterfacePrivate> > result;

   for (QNetworkInterfacePrivate *ptr : list) {
      result << QSharedDataPointer<QNetworkInterfacePrivate>(ptr);
   }

   return result;
}

QString QNetworkInterfacePrivate::makeHwAddress(int len, uchar *data)
{
   QString retval;

   for (int i = 0; i < len; ++i) {
      if (i != 0) {
         retval += QChar(':');
      }

      retval += QString("%1X").formatArg(data[i], 2, '0');
   }

   return retval;
}

QNetworkAddressEntry::QNetworkAddressEntry()
   : d(new QNetworkAddressEntryPrivate)
{
}

QNetworkAddressEntry::QNetworkAddressEntry(const QNetworkAddressEntry &other)
   : d(new QNetworkAddressEntryPrivate(*other.d.data()))
{
}

QNetworkAddressEntry &QNetworkAddressEntry::operator=(const QNetworkAddressEntry &other)
{
   *d.data() = *other.d.data();
   return *this;
}

QNetworkAddressEntry::~QNetworkAddressEntry()
{
}

bool QNetworkAddressEntry::operator==(const QNetworkAddressEntry &other) const
{
   if (d == other.d) {
      return true;
   }

   if (!d || !other.d) {
      return false;
   }
   return d->address == other.d->address &&
          d->netmask == other.d->netmask &&
          d->broadcast == other.d->broadcast;
}

QHostAddress QNetworkAddressEntry::ip() const
{
   return d->address;
}

void QNetworkAddressEntry::setIp(const QHostAddress &newIp)
{
   d->address = newIp;
}

QHostAddress QNetworkAddressEntry::netmask() const
{
   return d->netmask;
}

void QNetworkAddressEntry::setNetmask(const QHostAddress &newNetmask)
{
   if (newNetmask.protocol() != ip().protocol()) {
      d->netmask = QNetmaskAddress();
      return;
   }

   d->netmask.setAddress(newNetmask);
}

int QNetworkAddressEntry::prefixLength() const
{
   return d->netmask.prefixLength();
}

void QNetworkAddressEntry::setPrefixLength(int length)
{
   d->netmask.setPrefixLength(d->address.protocol(), length);
}

QHostAddress QNetworkAddressEntry::broadcast() const
{
   return d->broadcast;
}

void QNetworkAddressEntry::setBroadcast(const QHostAddress &newBroadcast)
{
   d->broadcast = newBroadcast;
}

QNetworkInterface::QNetworkInterface()
   : d(nullptr)
{
}

QNetworkInterface::~QNetworkInterface()
{
}

QNetworkInterface::QNetworkInterface(const QNetworkInterface &other)
   : d(other.d)
{
}

QNetworkInterface &QNetworkInterface::operator=(const QNetworkInterface &other)
{
   d = other.d;
   return *this;
}

bool QNetworkInterface::isValid() const
{
   return !name().isEmpty();
}

int QNetworkInterface::index() const
{
   return d ? d->index : 0;
}

QString QNetworkInterface::name() const
{
   return d ? d->name : QString();
}

QString QNetworkInterface::humanReadableName() const
{
   return d ? !d->friendlyName.isEmpty() ? d->friendlyName : name() : QString();
}

QNetworkInterface::InterfaceFlags QNetworkInterface::flags() const
{
   return d ? d->flags : InterfaceFlags(Qt::EmptyFlag);
}

QString QNetworkInterface::hardwareAddress() const
{
   return d ? d->hardwareAddress : QString();
}

QList<QNetworkAddressEntry> QNetworkInterface::addressEntries() const
{
   return d ? d->addressEntries : QList<QNetworkAddressEntry>();
}

QNetworkInterface QNetworkInterface::interfaceFromName(const QString &name)
{
   QNetworkInterface result;
   result.d = cs_Manager()->interfaceFromName(name);

   return result;
}

QNetworkInterface QNetworkInterface::interfaceFromIndex(int index)
{
   QNetworkInterface result;
   result.d = cs_Manager()->interfaceFromIndex(index);

   return result;
}

QList<QNetworkInterface> QNetworkInterface::allInterfaces()
{
   QList<QSharedDataPointer<QNetworkInterfacePrivate> > privs = cs_Manager()->allInterfaces();
   QList<QNetworkInterface> result;

   for (const QSharedDataPointer<QNetworkInterfacePrivate> &p : privs) {
      QNetworkInterface item;
      item.d = p;
      result << item;
   }

   return result;
}

QList<QHostAddress> QNetworkInterface::allAddresses()
{
   QList<QSharedDataPointer<QNetworkInterfacePrivate> > privs = cs_Manager()->allInterfaces();
   QList<QHostAddress> result;

   for (const QSharedDataPointer<QNetworkInterfacePrivate> &p : privs) {
      for (const QNetworkAddressEntry &entry : p->addressEntries) {
         result += entry.ip();
      }
   }

   return result;
}

static inline QDebug flagsDebug(QDebug debug, QNetworkInterface::InterfaceFlags flags)
{
   if (flags & QNetworkInterface::IsUp) {
      debug << "IsUp ";
   }

   if (flags & QNetworkInterface::IsRunning) {
      debug << "IsRunning ";
   }

   if (flags & QNetworkInterface::CanBroadcast) {
      debug << "CanBroadcast ";
   }

   if (flags & QNetworkInterface::IsLoopBack) {
      debug << "IsLoopBack ";
   }

   if (flags & QNetworkInterface::IsPointToPoint) {
      debug << "IsPointToPoint ";
   }

   if (flags & QNetworkInterface::CanMulticast) {
      debug << "CanMulticast ";
   }
   return debug;
}

static inline QDebug operator<<(QDebug debug, const QNetworkAddressEntry &entry)
{
   debug << "(address = " << entry.ip();

   if (!entry.netmask().isNull()) {
      debug << ", netmask = " << entry.netmask();
   }

   if (!entry.broadcast().isNull()) {
      debug << ", broadcast = " << entry.broadcast();
   }

   debug << ')';

   return debug;
}

QDebug operator<<(QDebug debug, const QNetworkInterface &networkInterface)
{
   // QDebugStateSaver saver(debug);
   // debug.resetFormat().nospace();

   debug << "QNetworkInterface(name = " << networkInterface.name()
         << ", hardware address = " << networkInterface.hardwareAddress()
         << ", flags = ";
   flagsDebug(debug, networkInterface.flags());

   debug << ", entries = " << networkInterface.addressEntries()
         << ")\n";

   return debug;
}

#endif // QT_NO_NETWORKINTERFACE
