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

#ifndef QJSDEBUGSERVICE_P_H
#define QJSDEBUGSERVICE_P_H

#include <QtCore/QPointer>
#include <QElapsedTimer>
#include "qdeclarativedebugservice_p.h"

QT_BEGIN_NAMESPACE

class QDeclarativeEngine;
class QJSDebuggerAgent;

struct JSAgentCoverageData {
   QByteArray prefix;
   qint64 time;
   int messageType;

   qint64 scriptId;
   QString program;
   QString fileName;
   int baseLineNumber;
   int lineNumber;
   int columnNumber;
   QString returnValue;

   QByteArray toByteArray() const;
};

class QJSDebugService : public QDeclarativeDebugService
{
   DECL_CS_OBJECT(QJSDebugService)

 public:
   QJSDebugService(QObject *parent = nullptr);
   ~QJSDebugService();

   static QJSDebugService *instance();

   void addEngine(QDeclarativeEngine *);
   void removeEngine(QDeclarativeEngine *);
   void processMessage(const JSAgentCoverageData &message);

   QElapsedTimer m_timer;

 protected:
   void statusChanged(Status status);
   void messageReceived(const QByteArray &);

 private :
   DECL_CS_SLOT_1(Private, void executionStopped(bool becauseOfException, const QString &exception))
   DECL_CS_SLOT_2(executionStopped)

   void sendMessages();
   QList<QDeclarativeEngine *> m_engines;
   QPointer<QJSDebuggerAgent> m_agent;
   bool m_deferredSend;
   QList<JSAgentCoverageData> m_data;
};

QT_END_NAMESPACE

#endif // QJSDEBUGSERVICE_P_H
