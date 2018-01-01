/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "config.h"
#include "qscriptengineagent.h"
#include "qscriptengineagent_p.h"
#include "qscriptengine.h"
#include "qscriptengine_p.h"

#include "CodeBlock.h"
#include "Instruction.h"

QT_BEGIN_NAMESPACE

/*!
  \since 4.4
  \class QScriptEngineAgent

  \brief The QScriptEngineAgent class provides an interface to report events pertaining to QScriptEngine execution.

  \ingroup script


  The QScriptEngineAgent class is the basis of tools that monitor and/or control the execution of a
  QScriptEngine, such as debuggers and profilers.

  To process script loading and unloading events, reimplement the
  scriptLoad() and scriptUnload() functions. scriptLoad() is called
  after the input to QScriptEngine::evaluate() has been parsed, right
  before the given script is executed. The engine assigns each
  script an ID, which is available as one of the arguments to
  scriptLoad(); subsequently, other event handlers can use the ID to
  identify a particular script. One common usage of scriptLoad() is
  to retain the script text, filename and base line number (the
  original input to QScriptEngine::evaluate()), so that other event
  handlers can e.g. map a line number to the corresponding line of
  text.

  scriptUnload() is called when the QScriptEngine has no further use
  for a script; the QScriptEngineAgent may at this point safely
  discard any resources associated with the script (such as the
  script text). Note that after scriptUnload() has been called, the
  QScriptEngine may reuse the relevant script ID for new scripts
  (i.e. as argument to a subsequent call to scriptLoad()).

  Evaluating the following script will result in scriptUnload()
  being called immediately after evaluation has completed:

  \snippet doc/src/snippets/code/src_script_qscriptengineagent.cpp 0

  Evaluating the following script will \b{not} result in a call to
  scriptUnload() when evaluation has completed:

  \snippet doc/src/snippets/code/src_script_qscriptengineagent.cpp 1

  The script isn't unloaded because it defines a function (\c{cube})
  that remains in the script environment after evaluation has
  completed. If a subsequent script removed the \c{cube} function
  (e.g. by setting it to \c{null}), scriptUnload() would be called
  when the function is garbage collected. In general terms, a script
  isn't unloaded until the engine has determined that none of its
  contents is referenced.

  To process script function calls and returns, reimplement the
  functionEntry() and functionExit() functions. functionEntry() is
  called when a script function is about to be executed;
  functionExit() is called when a script function is about to return,
  either normally or due to an exception.

  To process individual script statements, reimplement
  positionChange(). positionChange() is called each time the engine is
  about to execute a new statement of a script, and thus offers the
  finest level of script monitoring.

  To process exceptions, reimplement exceptionThrow() and
  exceptionCatch(). exceptionThrow() is called when a script exception
  is thrown, before it has been handled. exceptionCatch() is called
  when an exception handler is present, and execution is about to be
  resumed at the handler code.

  \sa QScriptEngine::setAgent(), QScriptContextInfo
*/

/*!
  \enum QScriptEngineAgent::Extension

  This enum specifies the possible extensions to a QScriptEngineAgent.

  \value DebuggerInvocationRequest The agent handles \c{debugger} script statements.

  \sa extension()
*/


void QScriptEngineAgentPrivate::attach()
{
   if (engine->originalGlobalObject()->debugger()) {
      engine->originalGlobalObject()->setDebugger(0);
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
   Q_UNUSED(frame);
   Q_UNUSED(lineno);
   Q_UNUSED(sourceID);
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
   if (!source) {
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

/*!
    Constructs a QScriptEngineAgent object for the given \a engine.

    The engine takes ownership of the agent.

    Call QScriptEngine::setAgent() to make this agent the active
    agent.
*/
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

/*!
  Destroys this QScriptEngineAgent.
*/
QScriptEngineAgent::~QScriptEngineAgent()
{
   d_ptr->engine->agentDeleted(this); //### TODO: Can this throw?
}

/*!

  This function is called when the engine has parsed a script and has
  associated it with the given \a id. The id can be used to identify
  this particular script in subsequent event notifications.

  \a program, \a fileName and \a baseLineNumber are the original
  arguments to the QScriptEngine::evaluate() call that triggered this
  event.

  This function is called just before the script is about to be
  evaluated.

  You can reimplement this function to record information about the
  script; for example, by retaining the script text, you can obtain
  the line of text corresponding to a line number in a subsequent
  call to positionChange().

  The default implementation does nothing.

  \sa scriptUnload()
*/
void QScriptEngineAgent::scriptLoad(qint64 id, const QString &program,
                                    const QString &fileName, int baseLineNumber)
{
   Q_UNUSED(id);
   Q_UNUSED(program);
   Q_UNUSED(fileName);
   Q_UNUSED(baseLineNumber);
}

/*!
  This function is called when the engine has discarded the script
  identified by the given \a id.

  You can reimplement this function to clean up any resources you have
  associated with the script.

  The default implementation does nothing.

  \sa scriptLoad()
*/
void QScriptEngineAgent::scriptUnload(qint64 id)
{
   Q_UNUSED(id);
}

/*!
  This function is called when a new script context has been pushed.

  The default implementation does nothing.

  \sa contextPop(), functionEntry()
*/
void QScriptEngineAgent::contextPush()
{
}

/*!
  This function is called when the current script context is about to
  be popped.

  The default implementation does nothing.

  \sa contextPush(), functionExit()
*/
void QScriptEngineAgent::contextPop()
{
}

/*!
  This function is called when a script function is called in the
  engine. If the script function is not a native Qt Script function,
  it resides in the script identified by \a scriptId; otherwise, \a
  scriptId is -1.

  This function is called just before execution of the script function
  begins.  You can obtain the QScriptContext associated with the
  function call with QScriptEngine::currentContext(). The arguments
  passed to the function are available.

  Reimplement this function to handle this event. For example, a
  debugger implementation could reimplement this function (and
  functionExit()) to keep track of the call stack and provide
  step-over functionality.

  The default implementation does nothing.

  \sa functionExit(), positionChange(), QScriptEngine::currentContext()
*/
void QScriptEngineAgent::functionEntry(qint64 scriptId)
{
   Q_UNUSED(scriptId);
}

/*!
  This function is called when the currently executing script function
  is about to return. If the script function is not a native Qt Script
  function, it resides in the script identified by \a scriptId;
  otherwise, \a scriptId is -1. The \a returnValue is the value that
  the script function will return.

  This function is called just before the script function returns.
  You can still access the QScriptContext associated with the
  script function call with QScriptEngine::currentContext().

  If the engine's
  \l{QScriptEngine::hasUncaughtException()}{hasUncaughtException}()
  function returns true, the script function is exiting due to an
  exception; otherwise, the script function is returning normally.

  Reimplement this function to handle this event; typically you will
  then also want to reimplement functionEntry().

  The default implementation does nothing.

  \sa functionEntry(), QScriptEngine::hasUncaughtException()
*/
void QScriptEngineAgent::functionExit(qint64 scriptId,
                                      const QScriptValue &returnValue)
{
   Q_UNUSED(scriptId);
   Q_UNUSED(returnValue);
}

/*!
  This function is called when the engine is about to execute a new
  statement in the script identified by \a scriptId.  The statement
  begins on the line and column specified by \a lineNumber
  This event is not generated for native Qt Script functions.

  Reimplement this function to handle this event. For example, a
  debugger implementation could reimplement this function to provide
  line-by-line stepping, and a profiler implementation could use it to
  count the number of times each statement is executed.

  The default implementation does nothing.

  \note \a columnNumber is undefined

  \sa scriptLoad(), functionEntry()
*/
void QScriptEngineAgent::positionChange(qint64 scriptId,
                                        int lineNumber, int columnNumber)
{
   Q_UNUSED(scriptId);
   Q_UNUSED(lineNumber);
   Q_UNUSED(columnNumber);
}

/*!
  This function is called when the given \a exception has occurred in
  the engine, in the script identified by \a scriptId. If the
  exception was thrown by a native Qt Script function, \a scriptId is
  -1.

  If \a hasHandler is true, there is a \c{catch} or \c{finally} block
  that will handle the exception. If \a hasHandler is false, there is
  no handler for the exception.

  Reimplement this function if you want to handle this event. For
  example, a debugger can notify the user when an uncaught exception
  occurs (i.e. \a hasHandler is false).

  The default implementation does nothing.

  \sa exceptionCatch()
*/
void QScriptEngineAgent::exceptionThrow(qint64 scriptId,
                                        const QScriptValue &exception,
                                        bool hasHandler)
{
   Q_UNUSED(scriptId);
   Q_UNUSED(exception);
   Q_UNUSED(hasHandler);
}

/*!
  This function is called when the given \a exception is about to be
  caught, in the script identified by \a scriptId.

  Reimplement this function if you want to handle this event.

  The default implementation does nothing.

  \sa exceptionThrow()
*/
void QScriptEngineAgent::exceptionCatch(qint64 scriptId,
                                        const QScriptValue &exception)
{
   Q_UNUSED(scriptId);
   Q_UNUSED(exception);
}


/*!
  Returns true if the QScriptEngineAgent supports the given \a
  extension; otherwise, false is returned. By default, no extensions
  are supported.

  \sa extension()
*/
bool QScriptEngineAgent::supportsExtension(Extension extension) const
{
   Q_UNUSED(extension);
   return false;
}

/*!
  This virtual function can be reimplemented in a QScriptEngineAgent
  subclass to provide support for extensions. The optional \a argument
  can be provided as input to the \a extension; the result must be
  returned in the form of a QVariant. You can call supportsExtension()
  to check if an extension is supported by the QScriptEngineAgent.  By
  default, no extensions are supported, and this function returns an
  invalid QVariant.

  If you implement the DebuggerInvocationRequest extension, Qt Script
  will call this function when a \c{debugger} statement is encountered
  in a script. The \a argument is a QVariantList containing three
  items: The first item is the scriptId (a qint64), the second item is
  the line number (an int), and the third item is the column number
  (an int).

  \sa supportsExtension()
*/
QVariant QScriptEngineAgent::extension(Extension extension,
                                       const QVariant &argument)
{
   Q_UNUSED(extension);
   Q_UNUSED(argument);
   return QVariant();
}

/*!
  Returns the QScriptEngine that this agent is associated with.
*/
QScriptEngine *QScriptEngineAgent::engine() const
{
   Q_D(const QScriptEngineAgent);
   return QScriptEnginePrivate::get(d->engine);
}

QT_END_NAMESPACE
