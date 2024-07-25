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

#ifndef QNETWORKINTERFACE_P_H
#define QNETWORKINTERFACE_P_H

#include <qabstractsocket.h>
#include <qatomic.h>
#include <qhostaddress.h>
#include <qlist.h>
#include <qreadwritelock.h>
#include <qstring.h>

#include <qhostaddress_p.h>

#ifndef QT_NO_NETWORKINTERFACE

class QNetworkAddressEntryPrivate
{
 public:
   QHostAddress address;
   QNetmaskAddress netmask;
   QHostAddress broadcast;
};

class QNetworkInterfacePrivate: public QSharedData
{
 public:
   QNetworkInterfacePrivate()
      : index(0), flags(Qt::EmptyFlag)
   {
   }

   ~QNetworkInterfacePrivate()
   {
   }

   int index;                                 // interface index, if know
   QNetworkInterface::InterfaceFlags flags;

   QString name;
   QString friendlyName;
   QString hardwareAddress;

   QList<QNetworkAddressEntry> addressEntries;

   static QString makeHwAddress(int len, uchar *data);

 private:
   // disallow copying -- avoid detaching
   QNetworkInterfacePrivate &operator=(const QNetworkInterfacePrivate &other);
   QNetworkInterfacePrivate(const QNetworkInterfacePrivate &other);
};

class QNetworkInterfaceManager
{
 public:
   QNetworkInterfaceManager();
   ~QNetworkInterfaceManager();

   QSharedDataPointer<QNetworkInterfacePrivate> interfaceFromName(const QString &name);
   QSharedDataPointer<QNetworkInterfacePrivate> interfaceFromIndex(int index);
   QList<QSharedDataPointer<QNetworkInterfacePrivate> > allInterfaces();

   // convenience
   QSharedDataPointer<QNetworkInterfacePrivate> empty;

 private:
   QList<QNetworkInterfacePrivate *> scan();
};

#endif // QT_NO_NETWORKINTERFACE

#endif
