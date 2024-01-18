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

#include <algorithm>

#include "qscriptdebuggercommandexecutor_p.h"

#include "qscriptdebuggerbackend_p.h"
#include "qscriptdebuggercommand_p.h"
#include "qscriptdebuggerresponse_p.h"
#include "qscriptdebuggervalue_p.h"
#include "qscriptdebuggervalueproperty_p.h"
#include "qscriptbreakpointdata_p.h"
#include "qscriptobjectsnapshot_p.h"
#include "qscriptdebuggerobjectsnapshotdelta_p.h"

#include <QtCore/qstringlist.h>
#include <QtScript/qscriptengine.h>
#include <QtScript/qscriptcontextinfo.h>
#include <QtScript/qscriptvalueiterator.h>
#include <QtCore/qdebug.h>

Q_DECLARE_METATYPE(QScriptScriptsDelta)
Q_DECLARE_METATYPE(QScriptDebuggerValueProperty)
Q_DECLARE_METATYPE(QScriptDebuggerValuePropertyList)
Q_DECLARE_METATYPE(QScriptDebuggerObjectSnapshotDelta)

QT_BEGIN_NAMESPACE

/*!
  \since 4.5
  \class QScriptDebuggerCommandExecutor
  \internal

  \brief The QScriptDebuggerCommandExecutor applies debugger commands to a back-end.

  The execute() function takes a command (typically produced by a
  QScriptDebuggerFrontend) and applies it to a QScriptDebuggerBackend.

  \sa QScriptDebuggerCommmand
*/

class QScriptDebuggerCommandExecutorPrivate
{
 public:
   QScriptDebuggerCommandExecutorPrivate();
   ~QScriptDebuggerCommandExecutorPrivate();
};

QScriptDebuggerCommandExecutorPrivate::QScriptDebuggerCommandExecutorPrivate()
{
}

QScriptDebuggerCommandExecutorPrivate::~QScriptDebuggerCommandExecutorPrivate()
{
}

QScriptDebuggerCommandExecutor::QScriptDebuggerCommandExecutor()
   : d_ptr(new QScriptDebuggerCommandExecutorPrivate())
{
}

QScriptDebuggerCommandExecutor::~QScriptDebuggerCommandExecutor()
{
}

static bool isPrefixOf(const QString &prefix, const QString &what)
{
   return ((what.length() > prefix.length())
           && what.startsWith(prefix));
}

/*!
  Applies the given \a command to the given \a backend.
*/
QScriptDebuggerResponse QScriptDebuggerCommandExecutor::execute(
   QScriptDebuggerBackend *backend,
   const QScriptDebuggerCommand &command)
{
   QScriptDebuggerResponse response;
   switch (command.type()) {
      case QScriptDebuggerCommand::None:
         break;

      case QScriptDebuggerCommand::Interrupt:
         backend->interruptEvaluation();
         break;

      case QScriptDebuggerCommand::Continue:
         if (backend->engine()->isEvaluating()) {
            backend->continueEvalution();
            response.setAsync(true);
         }
         break;

      case QScriptDebuggerCommand::StepInto: {
         QVariant attr = command.attribute(QScriptDebuggerCommand::StepCount);
         int count = attr.isValid() ? attr.toInt() : 1;
         backend->stepInto(count);
         response.setAsync(true);
      }
      break;

      case QScriptDebuggerCommand::StepOver: {
         QVariant attr = command.attribute(QScriptDebuggerCommand::StepCount);
         int count = attr.isValid() ? attr.toInt() : 1;
         backend->stepOver(count);
         response.setAsync(true);
      }
      break;

      case QScriptDebuggerCommand::StepOut:
         backend->stepOut();
         response.setAsync(true);
         break;

      case QScriptDebuggerCommand::RunToLocation:
         backend->runToLocation(command.fileName(), command.lineNumber());
         response.setAsync(true);
         break;

      case QScriptDebuggerCommand::RunToLocationByID:
         backend->runToLocation(command.scriptId(), command.lineNumber());
         response.setAsync(true);
         break;

      case QScriptDebuggerCommand::ForceReturn: {
         int contextIndex = command.contextIndex();
         QScriptDebuggerValue value = command.scriptValue();
         QScriptEngine *engine = backend->engine();
         QScriptValue realValue = value.toScriptValue(engine);
         backend->returnToCaller(contextIndex, realValue);
         response.setAsync(true);
      }
      break;

      case QScriptDebuggerCommand::Resume:
         backend->resume();
         response.setAsync(true);
         break;

      case QScriptDebuggerCommand::SetBreakpoint: {
         QScriptBreakpointData data = command.breakpointData();
         if (!data.isValid()) {
            data = QScriptBreakpointData(command.fileName(), command.lineNumber());
         }
         int id = backend->setBreakpoint(data);
         response.setResult(id);
      }
      break;

      case QScriptDebuggerCommand::DeleteBreakpoint: {
         int id = command.breakpointId();
         if (!backend->deleteBreakpoint(id)) {
            response.setError(QScriptDebuggerResponse::InvalidBreakpointID);
         }
      }
      break;

      case QScriptDebuggerCommand::DeleteAllBreakpoints:
         backend->deleteAllBreakpoints();
         break;

      case QScriptDebuggerCommand::GetBreakpoints: {
         QScriptBreakpointMap bps = backend->breakpoints();
         if (!bps.isEmpty()) {
            response.setResult(bps);
         }
      }
      break;

      case QScriptDebuggerCommand::GetBreakpointData: {
         int id = command.breakpointId();
         QScriptBreakpointData data = backend->breakpointData(id);
         if (data.isValid()) {
            response.setResult(data);
         } else {
            response.setError(QScriptDebuggerResponse::InvalidBreakpointID);
         }
      }
      break;

      case QScriptDebuggerCommand::SetBreakpointData: {
         int id = command.breakpointId();
         QScriptBreakpointData data = command.breakpointData();
         if (!backend->setBreakpointData(id, data)) {
            response.setError(QScriptDebuggerResponse::InvalidBreakpointID);
         }
      }
      break;

      case QScriptDebuggerCommand::GetScripts: {
         QScriptScriptMap scripts = backend->scripts();
         if (!scripts.isEmpty()) {
            response.setResult(scripts);
         }
      }
      break;

      case QScriptDebuggerCommand::GetScriptData: {
         qint64 id = command.scriptId();
         QScriptScriptData data = backend->scriptData(id);
         if (data.isValid()) {
            response.setResult(data);
         } else {
            response.setError(QScriptDebuggerResponse::InvalidScriptID);
         }
      }
      break;

      case QScriptDebuggerCommand::ScriptsCheckpoint:
         backend->scriptsCheckpoint();
         response.setResult(QVariant::fromValue(backend->scriptsDelta()));
         break;

      case QScriptDebuggerCommand::GetScriptsDelta:
         response.setResult(QVariant::fromValue(backend->scriptsDelta()));
         break;

      case QScriptDebuggerCommand::ResolveScript:
         response.setResult(backend->resolveScript(command.fileName()));
         break;

      case QScriptDebuggerCommand::GetBacktrace:
         response.setResult(backend->backtrace());
         break;

      case QScriptDebuggerCommand::GetContextCount:
         response.setResult(backend->contextCount());
         break;

      case QScriptDebuggerCommand::GetContextState: {
         QScriptContext *ctx = backend->context(command.contextIndex());
         if (ctx) {
            response.setResult(static_cast<int>(ctx->state()));
         } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
         }
      }
      break;

      case QScriptDebuggerCommand::GetContextID: {
         int idx = command.contextIndex();
         if ((idx >= 0) && (idx < backend->contextCount())) {
            response.setResult(backend->contextIds()[idx]);
         } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
         }
      }
      break;

      case QScriptDebuggerCommand::GetContextInfo: {
         QScriptContext *ctx = backend->context(command.contextIndex());
         if (ctx) {
            response.setResult(QScriptContextInfo(ctx));
         } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
         }
      }
      break;

      case QScriptDebuggerCommand::GetThisObject: {
         QScriptContext *ctx = backend->context(command.contextIndex());
         if (ctx) {
            response.setResult(ctx->thisObject());
         } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
         }
      }
      break;

      case QScriptDebuggerCommand::GetActivationObject: {
         QScriptContext *ctx = backend->context(command.contextIndex());
         if (ctx) {
            response.setResult(ctx->activationObject());
         } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
         }
      }
      break;

      case QScriptDebuggerCommand::GetScopeChain: {
         QScriptContext *ctx = backend->context(command.contextIndex());
         if (ctx) {
            QScriptDebuggerValueList dest;
            QScriptValueList src = ctx->scopeChain();
            for (int i = 0; i < src.size(); ++i) {
               dest.append(src.at(i));
            }
            response.setResult(dest);
         } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
         }
      }
      break;

      case QScriptDebuggerCommand::ContextsCheckpoint: {
         response.setResult(QVariant::fromValue(backend->contextsCheckpoint()));
      }
      break;

      case QScriptDebuggerCommand::GetPropertyExpressionValue: {
         QScriptContext *ctx = backend->context(command.contextIndex());
         int lineNumber = command.lineNumber();
         QVariant attr = command.attribute(QScriptDebuggerCommand::UserAttribute);
         QStringList path = attr.toStringList();
         if (!ctx || path.isEmpty()) {
            break;
         }
         QScriptContextInfo ctxInfo(ctx);
         if (ctx->callee().isValid()
               && ((lineNumber < ctxInfo.functionStartLineNumber())
                   || (lineNumber > ctxInfo.functionEndLineNumber()))) {
            break;
         }
         QScriptValueList objects;
         int pathIndex = 0;
         if (path.at(0) == QLatin1String("this")) {
            objects.append(ctx->thisObject());
            ++pathIndex;
         } else {
            objects << ctx->scopeChain();
         }
         for (int i = 0; i < objects.size(); ++i) {
            QScriptValue val = objects.at(i);
            for (int j = pathIndex; val.isValid() && (j < path.size()); ++j) {
               val = val.property(path.at(j));
            }
            if (val.isValid()) {
               bool hadException = (ctx->state() == QScriptContext::ExceptionState);
               QString str = val.toString();
               if (!hadException && backend->engine()->hasUncaughtException()) {
                  backend->engine()->clearExceptions();
               }
               response.setResult(str);
               break;
            }
         }
      }
      break;

      case QScriptDebuggerCommand::GetCompletions: {
         QScriptContext *ctx = backend->context(command.contextIndex());
         QVariant attr = command.attribute(QScriptDebuggerCommand::UserAttribute);
         QStringList path = attr.toStringList();
         if (!ctx || path.isEmpty()) {
            break;
         }
         QScriptValueList objects;
         QString prefix = path.last();
         QSet<QString> matches;
         if (path.size() > 1) {
            const QString &topLevelIdent = path.at(0);
            QScriptValue obj;
            if (topLevelIdent == QLatin1String("this")) {
               obj = ctx->thisObject();
            } else {
               QScriptValueList scopeChain;
               scopeChain = ctx->scopeChain();
               for (int i = 0; i < scopeChain.size(); ++i) {
                  QScriptValue oo = scopeChain.at(i).property(topLevelIdent);
                  if (oo.isObject()) {
                     obj = oo;
                     break;
                  }
               }
            }
            for (int i = 1; obj.isObject() && (i < path.size() - 1); ++i) {
               obj = obj.property(path.at(i));
            }
            if (obj.isValid()) {
               objects.append(obj);
            }
         } else {
            objects << ctx->scopeChain();
            QStringList keywords;
            keywords.append(QString::fromLatin1("this"));
            keywords.append(QString::fromLatin1("true"));
            keywords.append(QString::fromLatin1("false"));
            keywords.append(QString::fromLatin1("null"));
            for (int i = 0; i < keywords.size(); ++i) {
               const QString &kwd = keywords.at(i);
               if (isPrefixOf(prefix, kwd)) {
                  matches.insert(kwd);
               }
            }
         }

         for (int i = 0; i < objects.size(); ++i) {
            QScriptValue obj = objects.at(i);
            while (obj.isObject()) {
               QScriptValueIterator it(obj);
               while (it.hasNext()) {
                  it.next();
                  QString propertyName = it.name();
                  if (isPrefixOf(prefix, propertyName)) {
                     matches.insert(propertyName);
                  }
               }
               obj = obj.prototype();
            }
         }

         QStringList matchesList = matches.toList();
         std::stable_sort(matchesList);

         response.setResult(matchesList);
      }
      break;

      case QScriptDebuggerCommand::NewScriptObjectSnapshot: {
         int id = backend->newScriptObjectSnapshot();
         response.setResult(id);
      }
      break;

      case QScriptDebuggerCommand::ScriptObjectSnapshotCapture: {
         int id = command.snapshotId();
         QScriptObjectSnapshot *snap = backend->scriptObjectSnapshot(id);
         Q_ASSERT(snap != 0);
         QScriptDebuggerValue object = command.scriptValue();
         Q_ASSERT(object.type() == QScriptDebuggerValue::ObjectValue);
         QScriptEngine *engine = backend->engine();
         QScriptValue realObject = object.toScriptValue(engine);
         Q_ASSERT(realObject.isObject());
         QScriptObjectSnapshot::Delta delta = snap->capture(realObject);
         QScriptDebuggerObjectSnapshotDelta result;
         result.removedProperties = delta.removedProperties;
         bool didIgnoreExceptions = backend->ignoreExceptions();
         backend->setIgnoreExceptions(true);
         for (int i = 0; i < delta.changedProperties.size(); ++i) {
            const QScriptValueProperty &src = delta.changedProperties.at(i);
            bool hadException = engine->hasUncaughtException();
            QString str = src.value().toString();
            if (!hadException && engine->hasUncaughtException()) {
               engine->clearExceptions();
            }
            QScriptDebuggerValueProperty dest(src.name(), src.value(), str, src.flags());
            result.changedProperties.append(dest);
         }
         for (int j = 0; j < delta.addedProperties.size(); ++j) {
            const QScriptValueProperty &src = delta.addedProperties.at(j);
            bool hadException = engine->hasUncaughtException();
            QString str = src.value().toString();
            if (!hadException && engine->hasUncaughtException()) {
               engine->clearExceptions();
            }
            QScriptDebuggerValueProperty dest(src.name(), src.value(), str, src.flags());
            result.addedProperties.append(dest);
         }
         backend->setIgnoreExceptions(didIgnoreExceptions);
         response.setResult(QVariant::fromValue(result));
      }
      break;

      case QScriptDebuggerCommand::DeleteScriptObjectSnapshot: {
         int id = command.snapshotId();
         backend->deleteScriptObjectSnapshot(id);
      }
      break;

      case QScriptDebuggerCommand::NewScriptValueIterator: {
         QScriptDebuggerValue object = command.scriptValue();
         Q_ASSERT(object.type() == QScriptDebuggerValue::ObjectValue);
         QScriptEngine *engine = backend->engine();
         QScriptValue realObject = object.toScriptValue(engine);
         Q_ASSERT(realObject.isObject());
         int id = backend->newScriptValueIterator(realObject);
         response.setResult(id);
      }
      break;

      case QScriptDebuggerCommand::GetPropertiesByIterator: {
         int id = command.iteratorId();
         int count = 1000;
         QScriptValueIterator *it = backend->scriptValueIterator(id);
         Q_ASSERT(it != 0);
         QScriptDebuggerValuePropertyList props;
         for (int i = 0; (i < count) && it->hasNext(); ++i) {
            it->next();
            QString name = it->name();
            QScriptValue value = it->value();
            QString valueAsString = value.toString();
            QScriptValue::PropertyFlags flags = it->flags();
            QScriptDebuggerValueProperty prp(name, value, valueAsString, flags);
            props.append(prp);
         }
         response.setResult(props);
      }
      break;

      case QScriptDebuggerCommand::DeleteScriptValueIterator: {
         int id = command.iteratorId();
         backend->deleteScriptValueIterator(id);
      }
      break;

      case QScriptDebuggerCommand::Evaluate: {
         int contextIndex = command.contextIndex();
         QString program = command.program();
         QString fileName = command.fileName();
         int lineNumber = command.lineNumber();
         backend->evaluate(contextIndex, program, fileName, lineNumber);
         response.setAsync(true);
      }
      break;

      case QScriptDebuggerCommand::ScriptValueToString: {
         QScriptDebuggerValue value = command.scriptValue();
         QScriptEngine *engine = backend->engine();
         QScriptValue realValue = value.toScriptValue(engine);
         response.setResult(realValue.toString());
      }
      break;

      case QScriptDebuggerCommand::SetScriptValueProperty: {
         QScriptDebuggerValue object = command.scriptValue();
         QScriptEngine *engine = backend->engine();
         QScriptValue realObject = object.toScriptValue(engine);
         QScriptDebuggerValue value = command.subordinateScriptValue();
         QScriptValue realValue = value.toScriptValue(engine);
         QString name = command.name();
         realObject.setProperty(name, realValue);
      }
      break;

      case QScriptDebuggerCommand::ClearExceptions:
         backend->engine()->clearExceptions();
         break;

      case QScriptDebuggerCommand::UserCommand:
      case QScriptDebuggerCommand::MaxUserCommand:
         break;
   }
   return response;
}

QT_END_NAMESPACE
