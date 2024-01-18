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

#include "private/qjsdebugservice_p.h"
#include "private/qjsdebuggeragent_p.h"

#include <QtCore/qdatastream.h>
#include <QtCore/qdebug.h>
#include <QtCore/qstringlist.h>
#include <QtDeclarative/qdeclarativeengine.h>

Q_GLOBAL_STATIC(QJSDebugService, serviceInstance)

// convert to a QByteArray that can be sent to the debug client
QByteArray JSAgentCoverageData::toByteArray() const
{
   QByteArray data;
   //### using QDataStream is relatively expensive
   QDataStream ds(&data, QIODevice::WriteOnly);
   ds << prefix << time << messageType << scriptId << program << fileName << baseLineNumber
      << lineNumber << columnNumber << returnValue;
   return data;
}

QJSDebugService::QJSDebugService(QObject *parent)
   : QDeclarativeDebugService(QLatin1String("JSDebugger"), parent)
   , m_agent(0), m_deferredSend(true)
{
   m_timer.start();
}

QJSDebugService::~QJSDebugService()
{
   delete m_agent;
}

QJSDebugService *QJSDebugService::instance()
{
   return serviceInstance();
}

void QJSDebugService::addEngine(QDeclarativeEngine *engine)
{
   Q_ASSERT(engine);
   Q_ASSERT(!m_engines.contains(engine));

   m_engines.append(engine);

   if (status() == Enabled && !m_engines.isEmpty() && !m_agent) {
      m_agent = new QJSDebuggerAgent(engine, engine);
      connect(m_agent, SIGNAL(stopped(bool, QString)),
              this, SLOT(executionStopped(bool, QString)));

      while (!m_agent->isInitialized()) {
         waitForMessage();
      }
   }
}

void QJSDebugService::removeEngine(QDeclarativeEngine *engine)
{
   Q_ASSERT(engine);
   Q_ASSERT(m_engines.contains(engine));

   m_engines.removeAll(engine);
}

void QJSDebugService::statusChanged(Status status)
{
   if (status == Enabled && !m_engines.isEmpty() && !m_agent) {
      // Multiple engines are currently unsupported
      QDeclarativeEngine *engine = m_engines.first();
      m_agent = new QJSDebuggerAgent(engine, engine);

      connect(m_agent, SIGNAL(stopped(bool, QString)),
              this, SLOT(executionStopped(bool, QString)));

   } else if (status != Enabled && m_agent) {
      delete m_agent;
      m_agent = 0;
   }
}

void QJSDebugService::messageReceived(const QByteArray &message)
{
   if (!m_agent) {
      qWarning() << "QJSDebugService::messageReceived: No QJSDebuggerAgent available";
      return;
   }

   QDataStream ds(message);
   QByteArray command;
   ds >> command;
   if (command == "BREAKPOINTS") {
      JSAgentBreakpoints breakpoints;
      ds >> breakpoints;
      m_agent->setBreakpoints(breakpoints);

      //qDebug() << "BREAKPOINTS";
      //foreach (const JSAgentBreakpointData &bp, breakpoints)
      //    qDebug() << "BREAKPOINT: " << bp.fileUrl << bp.lineNumber;
   } else if (command == "WATCH_EXPRESSIONS") {
      QStringList watchExpressions;
      ds >> watchExpressions;
      m_agent->setWatchExpressions(watchExpressions);
   } else if (command == "STEPOVER") {
      m_agent->stepOver();
   } else if (command == "STEPINTO" || command == "INTERRUPT") {
      m_agent->stepInto();
   } else if (command == "STEPOUT") {
      m_agent->stepOut();
   } else if (command == "CONTINUE") {
      m_agent->continueExecution();
   } else if (command == "EXEC") {
      QByteArray id;
      QString expr;
      ds >> id >> expr;

      JSAgentWatchData data = m_agent->executeExpression(expr);

      QByteArray reply;
      QDataStream rs(&reply, QIODevice::WriteOnly);
      rs << QByteArray("RESULT") << id << data;
      sendMessage(reply);
   } else if (command == "EXPAND") {
      QByteArray requestId;
      quint64 objectId;
      ds >> requestId >> objectId;

      QList<JSAgentWatchData> result = m_agent->expandObjectById(objectId);

      QByteArray reply;
      QDataStream rs(&reply, QIODevice::WriteOnly);
      rs << QByteArray("EXPANDED") << requestId << result;
      sendMessage(reply);
   } else if (command == "ACTIVATE_FRAME") {
      int frameId;
      ds >> frameId;

      QList<JSAgentWatchData> locals = m_agent->localsAtFrame(frameId);

      QByteArray reply;
      QDataStream rs(&reply, QIODevice::WriteOnly);
      rs << QByteArray("LOCALS") << frameId << locals;
      sendMessage(reply);
   } else if (command == "SET_PROPERTY") {
      QByteArray id;
      qint64 objectId;
      QString property;
      QString value;
      ds >> id >> objectId >> property >> value;

      m_agent->setProperty(objectId, property, value);

      //TODO: feedback
   } else if (command == "PING") {
      int ping;
      ds >> ping;
      QByteArray reply;
      QDataStream rs(&reply, QIODevice::WriteOnly);
      rs << QByteArray("PONG") << ping;
      sendMessage(reply);
   } else if (command == "COVERAGE") {
      bool enabled;
      ds >> enabled;
      m_agent->setCoverageEnabled(enabled);
      if (!enabled) {
         sendMessages();
      }
   } else {
      qDebug() << Q_FUNC_INFO << "Unknown command" << command;
   }

   QDeclarativeDebugService::messageReceived(message);
}

void QJSDebugService::executionStopped(bool becauseOfException,
                                       const QString &exception)
{
   const QList<JSAgentStackData> backtrace = m_agent->backtrace();
   const QList<JSAgentWatchData> watches = m_agent->watches();
   const QList<JSAgentWatchData> locals = m_agent->locals();

   QByteArray reply;
   QDataStream rs(&reply, QIODevice::WriteOnly);
   rs << QByteArray("STOPPED") << backtrace << watches << locals
      << becauseOfException << exception;
   sendMessage(reply);
}

/*
    Either send the message directly, or queue up
    a list of messages to send later (via sendMessages)
*/
void QJSDebugService::processMessage(const JSAgentCoverageData &message)
{
   if (m_deferredSend) {
      m_data.append(message);
   } else {
      sendMessage(message.toByteArray());
   }
}

/*
    Send the messages queued up by processMessage
*/
void QJSDebugService::sendMessages()
{
   if (m_deferredSend) {
      //### this is a suboptimal way to send batched messages
      for (int i = 0; i < m_data.count(); ++i) {
         sendMessage(m_data.at(i).toByteArray());
      }
      m_data.clear();

      //indicate completion
      QByteArray data;
      QDataStream ds(&data, QIODevice::WriteOnly);
      ds << QByteArray("COVERAGE") << (qint64) - 1 << (int)CoverageComplete;
      sendMessage(data);
   }
}

