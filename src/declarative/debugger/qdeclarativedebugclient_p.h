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

#ifndef QDECLARATIVEDEBUGCLIENT_P_H
#define QDECLARATIVEDEBUGCLIENT_P_H

#include <QtNetwork/qtcpsocket.h>
#include <qdeclarativeglobal_p.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QDeclarativeDebugConnectionPrivate;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDebugConnection : public QTcpSocket
{
   DECL_CS_OBJECT(QDeclarativeDebugConnection)
   Q_DISABLE_COPY(QDeclarativeDebugConnection)

 public:
   QDeclarativeDebugConnection(QObject * = 0);
   ~QDeclarativeDebugConnection();

   bool isConnected() const;
 private:
   QDeclarativeDebugConnectionPrivate *d;
   friend class QDeclarativeDebugClient;
   friend class QDeclarativeDebugClientPrivate;
};

class QDeclarativeDebugClientPrivate;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDebugClient : public QObject
{
   DECL_CS_OBJECT(QDeclarativeDebugClient)
   Q_DECLARE_PRIVATE(QDeclarativeDebugClient)
   Q_DISABLE_COPY(QDeclarativeDebugClient)

 public:
   enum Status { NotConnected, Unavailable, Enabled };

   QDeclarativeDebugClient(const QString &, QDeclarativeDebugConnection *parent);
   ~QDeclarativeDebugClient();

   QString name() const;

   Status status() const;

   void sendMessage(const QByteArray &);

 protected:
   virtual void statusChanged(Status);
   virtual void messageReceived(const QByteArray &);

   QScopedPointer<QDeclarativeDebugClientPrivate> d_ptr;

 private:
   friend class QDeclarativeDebugConnection;
   friend class QDeclarativeDebugConnectionPrivate;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEDEBUGCLIENT_H
