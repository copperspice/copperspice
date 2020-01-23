/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include "qscriptdebuggercommandschedulerfrontend_p.h"
#include "qscriptdebuggercommandschedulerinterface_p.h"
#include "qscriptdebuggercommand_p.h"

QT_BEGIN_NAMESPACE

QScriptDebuggerCommandSchedulerFrontend::QScriptDebuggerCommandSchedulerFrontend(
   QScriptDebuggerCommandSchedulerInterface *scheduler,
   QScriptDebuggerResponseHandlerInterface *responseHandler)
   : m_scheduler(scheduler), m_responseHandler(responseHandler)
{
}

QScriptDebuggerCommandSchedulerFrontend::~QScriptDebuggerCommandSchedulerFrontend()
{
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleCommand(const QScriptDebuggerCommand &command)
{
   return m_scheduler->scheduleCommand(command, m_responseHandler);
}

/*!
  Instructs the front-end to break at the next script statement, and
  returns a unique identifier associated with this command.

  When the next script statement is encountered, the client will be
  notified, and the front-end will be ready to accept commands.

  \sa scheduleContinue()
*/
int QScriptDebuggerCommandSchedulerFrontend::scheduleInterrupt()
{
   return scheduleCommand(QScriptDebuggerCommand::interruptCommand());
}

/*!
  Instructs the front-end to continue evaluation, and returns a unique
  identifier associated with this command.

  \sa scheduleBreak()
*/
int QScriptDebuggerCommandSchedulerFrontend::scheduleContinue()
{
   return scheduleCommand(QScriptDebuggerCommand::continueCommand());
}

/*!
  Instructs the front-end to step into the next script statement, and
  returns a unique identifier associated with this command.

  Evaluation will automatically be continued, and the client()'s event()
  function will be called when the statement has been stepped into.
*/
int QScriptDebuggerCommandSchedulerFrontend::scheduleStepInto(int count)
{
   return scheduleCommand(QScriptDebuggerCommand::stepIntoCommand(count));
}

/*!
  Instructs the front-end to step over the next script statement, and
  returns a unique identifier associated with this command.

  Evaluation will automatically be continued, and the client()'s event()
  function will be called when the statement has been stepped over.
*/
int QScriptDebuggerCommandSchedulerFrontend::scheduleStepOver(int count)
{
   return scheduleCommand(QScriptDebuggerCommand::stepOverCommand(count));
}

/*!
  Instructs the front-end to step out of the current script function, and
  returns a unique identifier associated with this command.

  Evaluation will automatically be continued, and the client()'s
  event() function will be called when the script function has been
  stepped out of.
*/
int QScriptDebuggerCommandSchedulerFrontend::scheduleStepOut()
{
   return scheduleCommand(QScriptDebuggerCommand::stepOutCommand());
}

/*!
  Instructs the front-end to continue evaluation until the location
  specified by the given \a fileName and \a lineNumber is reached.
*/
int QScriptDebuggerCommandSchedulerFrontend::scheduleRunToLocation(const QString &fileName, int lineNumber)
{
   return scheduleCommand(QScriptDebuggerCommand::runToLocationCommand(fileName, lineNumber));
}

/*!
  Instructs the front-end to continue evaluation until the location
  specified by the given \a scriptId and \a lineNumber is reached.
*/
int QScriptDebuggerCommandSchedulerFrontend::scheduleRunToLocation(qint64 scriptId, int lineNumber)
{
   return scheduleCommand(QScriptDebuggerCommand::runToLocationCommand(scriptId, lineNumber));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleForceReturn(int contextIndex, const QScriptDebuggerValue &value)
{
   return scheduleCommand(QScriptDebuggerCommand::forceReturnCommand(contextIndex, value));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleSetBreakpoint(const QString &fileName, int lineNumber)
{
   return scheduleCommand(QScriptDebuggerCommand::setBreakpointCommand(fileName, lineNumber));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleSetBreakpoint(const QScriptBreakpointData &data)
{
   return scheduleCommand(QScriptDebuggerCommand::setBreakpointCommand(data));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleDeleteBreakpoint(int id)
{
   return scheduleCommand(QScriptDebuggerCommand::deleteBreakpointCommand(id));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleDeleteAllBreakpoints()
{
   return scheduleCommand(QScriptDebuggerCommand::deleteAllBreakpointsCommand());
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleGetBreakpoints()
{
   return scheduleCommand(QScriptDebuggerCommand::getBreakpointsCommand());
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleGetBreakpointData(int id)
{
   return scheduleCommand(QScriptDebuggerCommand::getBreakpointDataCommand(id));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleSetBreakpointData(int id, const QScriptBreakpointData &data)
{
   return scheduleCommand(QScriptDebuggerCommand::setBreakpointDataCommand(id, data));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleGetScripts()
{
   return scheduleCommand(QScriptDebuggerCommand::getScriptsCommand());
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleGetScriptData(qint64 id)
{
   return scheduleCommand(QScriptDebuggerCommand::getScriptDataCommand(id));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleScriptsCheckpoint()
{
   return scheduleCommand(QScriptDebuggerCommand::scriptsCheckpointCommand());
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleGetScriptsDelta()
{
   return scheduleCommand(QScriptDebuggerCommand::getScriptsDeltaCommand());
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleResolveScript(const QString &fileName)
{
   return scheduleCommand(QScriptDebuggerCommand::resolveScriptCommand(fileName));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleGetBacktrace()
{
   return scheduleCommand(QScriptDebuggerCommand::getBacktraceCommand());
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleGetContextCount()
{
   return scheduleCommand(QScriptDebuggerCommand::getContextCountCommand());
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleGetContextState(int contextIndex)
{
   return scheduleCommand(QScriptDebuggerCommand::getContextStateCommand(contextIndex));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleGetContextInfo(int contextIndex)
{
   return scheduleCommand(QScriptDebuggerCommand::getContextInfoCommand(contextIndex));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleGetContextId(int contextIndex)
{
   return scheduleCommand(QScriptDebuggerCommand::getContextIdCommand(contextIndex));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleGetThisObject(int contextIndex)
{
   return scheduleCommand(QScriptDebuggerCommand::getThisObjectCommand(contextIndex));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleGetActivationObject(int contextIndex)
{
   return scheduleCommand(QScriptDebuggerCommand::getActivationObjectCommand(contextIndex));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleGetScopeChain(int contextIndex)
{
   return scheduleCommand(QScriptDebuggerCommand::getScopeChainCommand(contextIndex));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleContextsCheckpoint()
{
   return scheduleCommand(QScriptDebuggerCommand::contextsCheckpoint());
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleGetPropertyExpressionValue(
   int contextIndex, int lineNumber, const QStringList &path)
{
   return scheduleCommand(QScriptDebuggerCommand::getPropertyExpressionValue(contextIndex, lineNumber, path));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleGetCompletions(int contextIndex, const QStringList &path)
{
   return scheduleCommand(QScriptDebuggerCommand::getCompletions(contextIndex, path));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleEvaluate(int contextIndex,
      const QString &program,
      const QString &fileName,
      int lineNumber)
{
   return scheduleCommand(QScriptDebuggerCommand::evaluateCommand(contextIndex, program, fileName, lineNumber));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleNewScriptValueIterator(const QScriptDebuggerValue &object)
{
   return scheduleCommand(QScriptDebuggerCommand::newScriptValueIteratorCommand(object));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleGetPropertiesByIterator(int id, int count)
{
   return scheduleCommand(QScriptDebuggerCommand::getPropertiesByIteratorCommand(id, count));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleDeleteScriptValueIterator(int id)
{
   return scheduleCommand(QScriptDebuggerCommand::deleteScriptValueIteratorCommand(id));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleScriptValueToString(const QScriptDebuggerValue &value)
{
   return scheduleCommand(QScriptDebuggerCommand::scriptValueToStringCommand(value));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleSetScriptValueProperty(const QScriptDebuggerValue &object,
      const QString &name,
      const QScriptDebuggerValue &value)
{
   return scheduleCommand(QScriptDebuggerCommand::setScriptValuePropertyCommand(object, name, value));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleClearExceptions()
{
   return scheduleCommand(QScriptDebuggerCommand::clearExceptionsCommand());
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleNewScriptObjectSnapshot()
{
   return scheduleCommand(QScriptDebuggerCommand::newScriptObjectSnapshotCommand());
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleScriptObjectSnapshotCapture(int id,
      const QScriptDebuggerValue &object)
{
   return scheduleCommand(QScriptDebuggerCommand::scriptObjectSnapshotCaptureCommand(id, object));
}

int QScriptDebuggerCommandSchedulerFrontend::scheduleDeleteScriptObjectSnapshot(int id)
{
   return scheduleCommand(QScriptDebuggerCommand::deleteScriptObjectSnapshotCommand(id));
}

QT_END_NAMESPACE
