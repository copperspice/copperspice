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

#ifndef QDECLARATIVEInspectorService_P_H
#define QDECLARATIVEInspectorService_P_H

#include <qdeclarativedebugservice_p.h>
#include <qdeclarativeglobal_p.h>
#include <QtCore/QList>

QT_BEGIN_NAMESPACE

class QDeclarativeView;
class QDeclarativeInspectorInterface;

class Q_DECLARATIVE_EXPORT QDeclarativeInspectorService : public QDeclarativeDebugService
{
   DECL_CS_OBJECT(QDeclarativeInspectorService)

 public:
   QDeclarativeInspectorService();
   static QDeclarativeInspectorService *instance();

   void addView(QDeclarativeView *);
   void removeView(QDeclarativeView *);
   QList<QDeclarativeView *> views() const {
      return m_views;
   }

   void sendMessage(const QByteArray &message);

   DECL_CS_SIGNAL_1(Public, void gotMessage(const QByteArray &message))
   DECL_CS_SIGNAL_2(gotMessage, message)

 protected:
   virtual void statusChanged(Status status);
   virtual void messageReceived(const QByteArray &);

 private:
   void updateStatus();

   static QDeclarativeInspectorInterface *loadInspectorPlugin();

   QList<QDeclarativeView *> m_views;
   QDeclarativeInspectorInterface *m_inspectorPlugin;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEOBSERVERSERVICE_H
