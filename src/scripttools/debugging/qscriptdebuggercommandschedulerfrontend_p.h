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

#ifndef QSCRIPTDEBUGGERCOMMANDSCHEDULERFRONTEND_P_H
#define QSCRIPTDEBUGGERCOMMANDSCHEDULERFRONTEND_P_H

#include <QtCore/qobjectdefs.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QScriptDebuggerCommandSchedulerInterface;
class QScriptDebuggerResponseHandlerInterface;
class QScriptDebuggerCommand;
class QScriptDebuggerValue;
class QScriptBreakpointData;

class QScriptDebuggerCommandSchedulerFrontend
{
 public:
   QScriptDebuggerCommandSchedulerFrontend(
      QScriptDebuggerCommandSchedulerInterface *scheduler,
      QScriptDebuggerResponseHandlerInterface *responseHandler);
   ~QScriptDebuggerCommandSchedulerFrontend();

   // execution control
   int scheduleInterrupt();
   int scheduleContinue();
   int scheduleStepInto(int count = 1);
   int scheduleStepOver(int count = 1);
   int scheduleStepOut();
   int scheduleRunToLocation(const QString &fileName, int lineNumber);
   int scheduleRunToLocation(qint64 scriptId, int lineNumber);
   int scheduleForceReturn(int contextIndex, const QScriptDebuggerValue &value);

   // breakpoints
   int scheduleSetBreakpoint(const QString &fileName, int lineNumber);
   int scheduleSetBreakpoint(const QScriptBreakpointData &data);
   int scheduleDeleteBreakpoint(int id);
   int scheduleDeleteAllBreakpoints();
   int scheduleGetBreakpoints();
   int scheduleGetBreakpointData(int id);
   int scheduleSetBreakpointData(int id, const QScriptBreakpointData &data);

   // scripts
   int scheduleGetScripts();
   int scheduleGetScriptData(qint64 id);
   int scheduleScriptsCheckpoint();
   int scheduleGetScriptsDelta();
   int scheduleResolveScript(const QString &fileName);

   // stack
   int scheduleGetBacktrace();
   int scheduleGetContextCount();
   int scheduleGetContextState(int contextIndex);
   int scheduleGetContextInfo(int contextIndex);
   int scheduleGetContextId(int contextIndex);
   int scheduleGetThisObject(int contextIndex);
   int scheduleGetActivationObject(int contextIndex);
   int scheduleGetScopeChain(int contextIndex);
   int scheduleContextsCheckpoint();
   int scheduleGetPropertyExpressionValue(int contextIndex, int lineNumber,
                                          const QStringList &path);
   int scheduleGetCompletions(int contextIndex, const QStringList &path);

   // iteration
   int scheduleNewScriptValueIterator(const QScriptDebuggerValue &object);
   int scheduleGetPropertiesByIterator(int id, int count);
   int scheduleDeleteScriptValueIterator(int id);

   // evaluation
   int scheduleEvaluate(int contextIndex, const QString &program,
                        const QString &fileName = QString(),
                        int lineNumber = 1);

   int scheduleScriptValueToString(const QScriptDebuggerValue &value);
   int scheduleSetScriptValueProperty(const QScriptDebuggerValue &object,
                                      const QString &name,
                                      const QScriptDebuggerValue &value);

   int scheduleClearExceptions();

   int scheduleNewScriptObjectSnapshot();
   int scheduleScriptObjectSnapshotCapture(int id, const QScriptDebuggerValue &object);
   int scheduleDeleteScriptObjectSnapshot(int id);

 private:
   int scheduleCommand(const QScriptDebuggerCommand &command);

   QScriptDebuggerCommandSchedulerInterface *m_scheduler;
   QScriptDebuggerResponseHandlerInterface *m_responseHandler;

   Q_DISABLE_COPY(QScriptDebuggerCommandSchedulerFrontend)
};

QT_END_NAMESPACE

#endif
