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

#ifndef QDECLARATIVEDEBUGSERVICE_P_H
#define QDECLARATIVEDEBUGSERVICE_P_H

#include <qobject.h>
#include <qdeclarativeglobal_p.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QDeclarativeDebugServicePrivate;

class Q_DECLARATIVE_EXPORT QDeclarativeDebugService : public QObject
{
   DECL_CS_OBJECT(QDeclarativeDebugService)

   Q_DECLARE_PRIVATE(QDeclarativeDebugService)
   Q_DISABLE_COPY(QDeclarativeDebugService)

 public:
   explicit QDeclarativeDebugService(const QString &, QObject *parent = nullptr);
   ~QDeclarativeDebugService();

   QString name() const;

   enum Status { NotConnected, Unavailable, Enabled };
   Status status() const;

   void sendMessage(const QByteArray &);
   bool waitForMessage();

   static int idForObject(QObject *);
   static QObject *objectForId(int);

   static QString objectToString(QObject *obj);

   static bool isDebuggingEnabled();
   static bool hasDebuggingClient();

 protected:
   virtual void statusChanged(Status);
   virtual void messageReceived(const QByteArray &);

   QScopedPointer<QDeclarativeDebugServicePrivate> d_ptr;

 private:
   friend class QDeclarativeDebugServer;
   friend class QDeclarativeDebugServerPrivate;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEDEBUGSERVICE_H

