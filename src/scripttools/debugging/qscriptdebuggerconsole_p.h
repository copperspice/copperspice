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

#ifndef QSCRIPTDEBUGGERCONSOLE_P_H
#define QSCRIPTDEBUGGERCONSOLE_P_H

#include <QtCore/qobjectdefs.h>
#include <QtCore/qscopedpointer.h>
#include <qscriptdebuggerconsolehistorianinterface_p.h>

QT_BEGIN_NAMESPACE

class QString;
class QScriptDebuggerConsoleCommandJob;
class QScriptMessageHandlerInterface;
class QScriptDebuggerCommandSchedulerInterface;
class QScriptDebuggerConsoleCommandManager;
class QScriptDebuggerConsolePrivate;

class QScriptDebuggerConsole : public QScriptDebuggerConsoleHistorianInterface
{
 public:
   QScriptDebuggerConsole();
   ~QScriptDebuggerConsole();

   void loadScriptedCommands(const QString &scriptsPath, QScriptMessageHandlerInterface *messageHandler);
   void showDebuggerInfoMessage(QScriptMessageHandlerInterface *messageHandler);

   QScriptDebuggerConsoleCommandManager *commandManager() const;

   QScriptDebuggerConsoleCommandJob *consumeInput(
      const QString &input, QScriptMessageHandlerInterface *messageHandler,
      QScriptDebuggerCommandSchedulerInterface *commandScheduler);

   bool hasIncompleteInput() const;
   QString incompleteInput() const;
   void setIncompleteInput(const QString &input);
   QString commandPrefix() const;

   int historyCount() const;
   QString historyAt(int index) const;
   void changeHistoryAt(int index, const QString &newHistory);

   int currentFrameIndex() const;
   void setCurrentFrameIndex(int index);

   qint64 currentScriptId() const;
   void setCurrentScriptId(qint64 id);

   int currentLineNumber() const;
   void setCurrentLineNumber(int lineNumber);

   int evaluateAction() const;
   void setEvaluateAction(int action);

   qint64 sessionId() const;
   void bumpSessionId();

 private:
   QScopedPointer<QScriptDebuggerConsolePrivate> d_ptr;

   Q_DECLARE_PRIVATE(QScriptDebuggerConsole)
   Q_DISABLE_COPY(QScriptDebuggerConsole)
};

QT_END_NAMESPACE

#endif
