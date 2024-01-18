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

#ifndef QDECLARATIVEDEBUGSERVERCONNECTION_P_H
#define QDECLARATIVEDEBUGSERVERCONNECTION_P_H

#include <qdeclarativeglobal_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeDebugServer;
class Q_DECLARATIVE_EXPORT QDeclarativeDebugServerConnection
{
 public:
   QDeclarativeDebugServerConnection() {}
   virtual ~QDeclarativeDebugServerConnection() {}

   virtual void setServer(QDeclarativeDebugServer *server) = 0;
   virtual void setPort(int port, bool bock) = 0;
   virtual bool isConnected() const = 0;
   virtual void send(const QByteArray &message) = 0;
   virtual void disconnect() = 0;
   virtual bool waitForMessage() = 0;
};

CS_DECLARE_INTERFACE(QDeclarativeDebugServerConnection, "com.copperspice.QDeclarativeDebugServerConnection/1.0")

QT_END_NAMESPACE

#endif // QDECLARATIVEDEBUGSERVERCONNECTION_H
