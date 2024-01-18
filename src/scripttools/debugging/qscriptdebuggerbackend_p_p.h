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

#ifndef QSCRIPTDEBUGGERBACKEND_P_P_H
#define QSCRIPTDEBUGGERBACKEND_P_P_H

#include <QtCore/qobjectdefs.h>
#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtScript/qscriptvalue.h>
#include <qscriptdebuggerbackend_p.h>

QT_BEGIN_NAMESPACE

class QEvent;
class QString;
class QScriptContext;
class QScriptEngine;
class QScriptValueIterator;
class QScriptObjectSnapshot;
class QScriptDebuggerAgent;
class QScriptDebuggerCommandExecutor;
class QScriptDebuggerBackend;

class QScriptDebuggerBackendPrivate
{
   Q_DECLARE_PUBLIC(QScriptDebuggerBackend)

 public:
   QScriptDebuggerBackendPrivate();
   virtual ~QScriptDebuggerBackendPrivate();

   void postEvent(QEvent *e);
   virtual bool event(QEvent *e);

   // events reported by agent
   virtual void stepped(qint64 scriptId, int lineNumber, int columnNumber,
                        const QScriptValue &result);
   virtual void locationReached(qint64 scriptId, int lineNumber, int columnNumber);
   virtual void interrupted(qint64 scriptId, int lineNumber, int columnNumber);
   virtual void breakpoint(qint64 scriptId, int lineNumber, int columnNumber,
                           int breakpointId);
   virtual void exception(qint64 scriptId, const QScriptValue &exception,
                          bool hasHandler);
   virtual void debuggerInvocationRequest(qint64 scriptId, int lineNumber,
                                          int columnNumber);
   virtual void forcedReturn(qint64 scriptId, int lineNumber, int columnNumber,
                             const QScriptValue &value);

   static QScriptValue trace(QScriptContext *context,
                             QScriptEngine *engine);
   static QScriptValue qsassert(QScriptContext *context,
                                QScriptEngine *engine);
   static QScriptValue fileName(QScriptContext *context,
                                QScriptEngine *engine);
   static QScriptValue lineNumber(QScriptContext *context,
                                  QScriptEngine *engine);

   void agentDestroyed(QScriptDebuggerAgent *);

   QScriptDebuggerAgent *agent;
   QScriptDebuggerCommandExecutor *commandExecutor;

   int pendingEvaluateContextIndex;
   QString pendingEvaluateProgram;
   QString pendingEvaluateFileName;
   int pendingEvaluateLineNumber;
   bool ignoreExceptions;

   int nextScriptValueIteratorId;
   QMap<int, QScriptValueIterator *> scriptValueIterators;

   int nextScriptObjectSnapshotId;
   QMap<int, QScriptObjectSnapshot *> scriptObjectSnapshots;

   QObject *eventReceiver;

   QScriptDebuggerBackend *q_ptr;

   QScriptValue origTraceFunction;
   QScriptValue origFileNameFunction;
   QScriptValue origLineNumberFunction;
};

QT_END_NAMESPACE

#endif
