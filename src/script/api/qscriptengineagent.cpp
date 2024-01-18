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

#include "config.h"

#include <qscriptengineagent.h>
#include <qscriptengine.h>

#include <qscriptengineagent_p.h>
#include <qscriptengine_p.h>

#include "CodeBlock.h"
#include "Instruction.h"

void QScriptEngineAgentPrivate::attach()
{
   if (engine->originalGlobalObject()->debugger()) {
      engine->originalGlobalObject()->setDebugger(nullptr);
   }

   JSC::Debugger::attach(engine->originalGlobalObject());
   if (!QScriptEnginePrivate::get(engine)->isEvaluating()) {
      JSC::Debugger::recompileAllJSFunctions(engine->globalData);
   }
}

void QScriptEngineAgentPrivate::detach()
{
   JSC::Debugger::detach(engine->originalGlobalObject());
}

void QScriptEngineAgentPrivate::returnEvent(const JSC::DebuggerCallFrame &frame, intptr_t sourceID, int lineno)
{
   (void) frame;
   (void) lineno;
   (void) sourceID;
}

void QScriptEngineAgentPrivate::exceptionThrow(const JSC::DebuggerCallFrame &frame, intptr_t sourceID, bool hasHandler)
{
   JSC::CallFrame *oldFrame = engine->currentFrame;
   int oldAgentLineNumber = engine->agentLineNumber;
   engine->currentFrame = frame.callFrame();
   QScriptValue value(engine->scriptValueFromJSCValue(frame.exception()));
   engine->agentLineNumber = value.property(QLatin1String("lineNumber")).toInt32();
   q_ptr->exceptionThrow(sourceID, value, hasHandler);
   engine->agentLineNumber = oldAgentLineNumber;
   engine->currentFrame = oldFrame;
   engine->setCurrentException(value);
};

void QScriptEngineAgentPrivate::exceptionCatch(const JSC::DebuggerCallFrame &frame, intptr_t sourceID)
{
   JSC::CallFrame *oldFrame = engine->currentFrame;
   engine->currentFrame = frame.callFrame();
   QScriptValue value(engine->scriptValueFromJSCValue(frame.exception()));
   q_ptr->exceptionCatch(sourceID, value);
   engine->currentFrame = oldFrame;
   engine->clearCurrentException();
}

void QScriptEngineAgentPrivate::atStatement(const JSC::DebuggerCallFrame &frame, intptr_t sourceID,
   int lineno/*, int column*/)
{
   QScript::UStringSourceProviderWithFeedback *source = engine->loadedScripts.value(sourceID);
   if (! source) {
      // QTBUG-6108: We don't have the source for this script, so ignore.
      return;
   }

   //    column = source->columnNumberFromOffset(column);
   int column = 1;
   JSC::CallFrame *oldFrame = engine->currentFrame;
   int oldAgentLineNumber = engine->agentLineNumber;
   engine->currentFrame = frame.callFrame();
   engine->agentLineNumber = lineno;
   q_ptr->positionChange(sourceID, lineno, column);
   engine->currentFrame = oldFrame;
   engine->agentLineNumber = oldAgentLineNumber;
}

void QScriptEngineAgentPrivate::functionExit(const JSC::JSValue &returnValue, intptr_t sourceID)
{
   QScriptValue result = engine->scriptValueFromJSCValue(returnValue);
   q_ptr->functionExit(sourceID, result);
   q_ptr->contextPop();
}

void QScriptEngineAgentPrivate::evaluateStop(const JSC::JSValue &returnValue, intptr_t sourceID)
{
   QScriptValue result = engine->scriptValueFromJSCValue(returnValue);
   q_ptr->functionExit(sourceID, result);
}

void QScriptEngineAgentPrivate::didReachBreakpoint(const JSC::DebuggerCallFrame &frame,
   intptr_t sourceID, int lineno/*, int column*/)
{
   if (q_ptr->supportsExtension(QScriptEngineAgent::DebuggerInvocationRequest)) {
      QScript::UStringSourceProviderWithFeedback *source = engine->loadedScripts.value(sourceID);
      if (!source) {
         // QTBUG-6108: We don't have the source for this script, so ignore.
         return;
      }
      //        column = source->columnNumberFromOffset(column);
      int column = 1;
      JSC::CallFrame *oldFrame = engine->currentFrame;
      int oldAgentLineNumber = engine->agentLineNumber;
      engine->currentFrame = frame.callFrame();
      engine->agentLineNumber = lineno;
      QList<QVariant> args;
      args << qint64(sourceID) << lineno << column;
      q_ptr->extension(QScriptEngineAgent::DebuggerInvocationRequest, args);
      engine->currentFrame = oldFrame;
      engine->agentLineNumber = oldAgentLineNumber;
   }
};

QScriptEngineAgent::QScriptEngineAgent(QScriptEngine *engine)
   : d_ptr(new QScriptEngineAgentPrivate())
{
   d_ptr->q_ptr = this;
   d_ptr->engine = QScriptEnginePrivate::get(engine);
   d_ptr->engine->ownedAgents.append(this);
}

/*!
  \internal
*/
QScriptEngineAgent::QScriptEngineAgent(QScriptEngineAgentPrivate &dd, QScriptEngine *engine)
   : d_ptr(&dd)
{
   d_ptr->q_ptr = this;
   d_ptr->engine = QScriptEnginePrivate::get(engine);
}

QScriptEngineAgent::~QScriptEngineAgent()
{
   d_ptr->engine->agentDeleted(this); //### TODO: Can this throw?
}

void QScriptEngineAgent::scriptLoad(qint64 id, const QString &program,
   const QString &fileName, int baseLineNumber)
{
   (void) id;
   (void) program;
   (void) fileName;
   (void) baseLineNumber;
}

void QScriptEngineAgent::scriptUnload(qint64 id)
{
   (void) id;
}

void QScriptEngineAgent::contextPush()
{
}

void QScriptEngineAgent::contextPop()
{
}

void QScriptEngineAgent::functionEntry(qint64 scriptId)
{
   (void) scriptId;
}

void QScriptEngineAgent::functionExit(qint64 scriptId, const QScriptValue &returnValue)
{
   (void) scriptId;
   (void) returnValue;
}

void QScriptEngineAgent::positionChange(qint64 scriptId, int lineNumber, int columnNumber)
{
   (void) scriptId;
   (void) lineNumber;
   (void) columnNumber;
}

void QScriptEngineAgent::exceptionThrow(qint64 scriptId, const QScriptValue &exception, bool hasHandler)
{
   (void) scriptId;
   (void) exception;
   (void) hasHandler;
}

void QScriptEngineAgent::exceptionCatch(qint64 scriptId,
   const QScriptValue &exception)
{
   (void) scriptId;
   (void) exception;
}

bool QScriptEngineAgent::supportsExtension(Extension extension) const
{
   (void) extension;

   return false;
}

QVariant QScriptEngineAgent::extension(Extension extension, const QVariant &argument)
{
   (void) extension;
   (void) argument;

   return QVariant();
}

QScriptEngine *QScriptEngineAgent::engine() const
{
   Q_D(const QScriptEngineAgent);
   return QScriptEnginePrivate::get(d->engine);
}

