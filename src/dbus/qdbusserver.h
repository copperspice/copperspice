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

#ifndef QDBUSSERVER_H
#define QDBUSSERVER_H

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtDBus/qdbusmacros.h>

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

class QDBusConnectionPrivate;
class QDBusError;
class QDBusConnection;

class Q_DBUS_EXPORT QDBusServer: public QObject
{
   CS_OBJECT(QDBusServer)

   public:
       QDBusServer(const QString &address = QLatin1String("unix:tmpdir=/tmp"), QObject *parent = nullptr);
       virtual ~QDBusServer();
   
       bool isConnected() const;
       QDBusError lastError() const;
       QString address() const;
   
   public:
       CS_SIGNAL_1(Public, void newConnection(const QDBusConnection & connection))
       CS_SIGNAL_2(newConnection,connection) 
   
   private:
       Q_DISABLE_COPY(QDBusServer)
       QDBusConnectionPrivate *d;
};

QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif
