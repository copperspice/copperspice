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

#include <qscriptcontext.h>

#include <qscriptengine.h>
#include <qscriptcontextinfo.h>
#include <qstringlist.h>

#include <qscriptcontext_p.h>
#include <qscriptengine_p.h>
#include <qscriptactivationobject_p.h>

#include "Arguments.h"
#include "CodeBlock.h"
#include "Error.h"
#include "JSFunction.h"
#include "JSObject.h"
#include "JSGlobalObject.h"

QScriptContext::QScriptContext()
{
   //QScriptContext doesn't exist,  pointer to QScriptContext are just pointer to  JSC::CallFrame
   Q_ASSERT(false);
}

QScriptValue QScriptContext::throwValue(const QScriptValue &value)
{
   JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);

   QScriptEnginePrivate *engine = QScript::scriptEngineFromExec(frame);
   QScript::APIShim shim(engine);

   JSC::JSValue jscValue = engine->scriptValueToJSCValue(value);
   engine->clearCurrentException();
   frame->setException(jscValue);
   return value;
}

QScriptValue QScriptContext::throwError(Error error, const QString &text)
{
   JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   QScriptEnginePrivate *engine = QScript::scriptEngineFromExec(frame);
   QScript::APIShim shim(engine);

   JSC::ErrorType jscError = JSC::GeneralError;
   switch (error) {
      case UnknownError:
         break;

      case ReferenceError:
         jscError = JSC::ReferenceError;
         break;

      case SyntaxError:
         jscError = JSC::SyntaxError;
         break;

      case TypeError:
         jscError = JSC::TypeError;
         break;

      case RangeError:
         jscError = JSC::RangeError;
         break;

      case URIError:
         jscError = JSC::URIError;
         break;
   }

   JSC::JSObject *result = JSC::throwError(frame, jscError, text);
   engine->clearCurrentException();
   return engine->scriptValueFromJSCValue(result);
}

QScriptValue QScriptContext::throwError(const QString &text)
{
   JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   QScriptEnginePrivate *engine = QScript::scriptEngineFromExec(frame);
   QScript::APIShim shim(engine);
   engine->clearCurrentException();
   JSC::JSObject *result = JSC::throwError(frame, JSC::GeneralError, text);
   return engine->scriptValueFromJSCValue(result);
}

QScriptContext::~QScriptContext()
{
   //QScriptContext doesn't exist,  pointer to QScriptContext are just pointer to JSC::CallFrame
   Q_ASSERT(false);
}

QScriptEngine *QScriptContext::engine() const
{
   const JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   return QScriptEnginePrivate::get(QScript::scriptEngineFromExec(frame));
}

QScriptValue QScriptContext::argument(int index) const
{
   if (index < 0) {
      return QScriptValue();
   }
   if (index >= argumentCount()) {
      return QScriptValue(QScriptValue::UndefinedValue);
   }
   QScriptValue v = argumentsObject().property(index);
   return v;
}

QScriptValue QScriptContext::callee() const
{
   const JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   QScriptEnginePrivate *eng = QScript::scriptEngineFromExec(frame);
   QScript::APIShim shim(eng);
   if (frame->callee() == eng->originalGlobalObject()) {
      // This is a pushContext()-created context; the callee is a lie.
      Q_ASSERT(QScriptEnginePrivate::contextFlags(const_cast<JSC::CallFrame *>(frame)) & QScriptEnginePrivate::NativeContext);
      return QScriptValue();
   }
   return eng->scriptValueFromJSCValue(frame->callee());
}

QScriptValue QScriptContext::argumentsObject() const
{
   JSC::CallFrame *frame = const_cast<JSC::ExecState *>(QScriptEnginePrivate::frameForContext(this));
   QScript::APIShim shim(QScript::scriptEngineFromExec(frame));

   if (frame == frame->lexicalGlobalObject()->globalExec()) {
      // <global> context doesn't have arguments. return an empty object
      return QScriptEnginePrivate::get(QScript::scriptEngineFromExec(frame))->newObject();
   }

   //for a js function
   if (frame->codeBlock() && frame->callee()) {
      if (!QScriptEnginePrivate::hasValidCodeBlockRegister(frame)) {
         // We have a built-in JS host call.
         // codeBlock is needed by retrieveArguments(), but since it
         // contains junk, we would crash. Return an invalid value for now.
         return QScriptValue();
      }
      JSC::JSValue result = frame->interpreter()->retrieveArguments(frame, JSC::asFunction(frame->callee()));
      return QScript::scriptEngineFromExec(frame)->scriptValueFromJSCValue(result);
   }

   if (frame->callerFrame()->hasHostCallFrameFlag()) {
      // <eval> context doesn't have arguments. return an empty object
      return QScriptEnginePrivate::get(QScript::scriptEngineFromExec(frame))->newObject();
   }

   //for a native function
   if (!frame->optionalCalleeArguments()
      && QScriptEnginePrivate::hasValidCodeBlockRegister(frame)) { // Make sure we don't go here for host JSFunctions
      Q_ASSERT(frame->argumentCount() > 0); //we need at least 'this' otherwise we'll crash later
      JSC::Arguments *arguments = new (&frame->globalData())JSC::Arguments(frame, JSC::Arguments::NoParameters);
      frame->setCalleeArguments(arguments);
   }
   return QScript::scriptEngineFromExec(frame)->scriptValueFromJSCValue(frame->optionalCalleeArguments());
}

bool QScriptContext::isCalledAsConstructor() const
{
   JSC::CallFrame *frame = const_cast<JSC::ExecState *>(QScriptEnginePrivate::frameForContext(this));
   QScript::APIShim shim(QScript::scriptEngineFromExec(frame));

   //For native functions, look up flags.
   uint flags = QScriptEnginePrivate::contextFlags(frame);
   if (flags & QScriptEnginePrivate::NativeContext) {
      return flags & QScriptEnginePrivate::CalledAsConstructorContext;
   }

   //Not a native function, try to look up in the bytecode if we where called from op_construct
   JSC::Instruction *returnPC = frame->returnPC();

   if (!returnPC) {
      return false;
   }

   JSC::CallFrame *callerFrame = QScriptEnginePrivate::frameForContext(parentContext());
   if (!callerFrame) {
      return false;
   }

   if (returnPC[-JSC::op_construct_length].u.opcode == frame->interpreter()->getOpcode(JSC::op_construct)) {
      //We are maybe called from the op_construct opcode which has 6 opperands.
      //But we need to check we are not called from op_call with 4 opperands

      //we make sure that the returnPC[-1] (thisRegister) is smaller than the returnPC[-3] (registerOffset)
      //as if it was an op_call, the returnPC[-1] would be the registerOffset, bigger than returnPC[-3] (funcRegister)
      return returnPC[-1].u.operand < returnPC[-3].u.operand;
   }
   return false;
}

QScriptContext *QScriptContext::parentContext() const
{
   const JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   QScript::APIShim shim(QScript::scriptEngineFromExec(frame));
   JSC::CallFrame *callerFrame = frame->callerFrame()->removeHostCallFrameFlag();
   return QScriptEnginePrivate::contextForFrame(callerFrame);
}

int QScriptContext::argumentCount() const
{
   const JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   int argc = frame->argumentCount();
   if (argc != 0) {
      --argc;   // -1 due to "this"
   }
   return argc;
}

// internal (cs)
QScriptValue QScriptContext::returnValue() const
{
   qWarning("QScriptContext::returnValue() not implemented");
   return QScriptValue();
}

// internal (cs)
void QScriptContext::setReturnValue(const QScriptValue &result)
{
   JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   JSC::CallFrame *callerFrame = frame->callerFrame();
   if (!callerFrame->codeBlock()) {
      return;
   }

   Q_ASSERT_X(false, Q_FUNC_INFO, "check me");

   int dst = frame->registers()[JSC::RegisterFile::ReturnValueRegister].i(); // returnValueRegister() is private
   callerFrame[dst] = QScript::scriptEngineFromExec(frame)->scriptValueToJSCValue(result);
}

QScriptValue QScriptContext::activationObject() const
{
   JSC::CallFrame *frame = const_cast<JSC::ExecState *>(QScriptEnginePrivate::frameForContext(this));
   QScript::APIShim shim(QScript::scriptEngineFromExec(frame));
   JSC::JSObject *result = nullptr;

   uint flags = QScriptEnginePrivate::contextFlags(frame);
   if ((flags & QScriptEnginePrivate::NativeContext) && !(flags & QScriptEnginePrivate::HasScopeContext)) {
      //For native functions, lazily create it if needed
      QScript::QScriptActivationObject *scope = new (frame) QScript::QScriptActivationObject(frame);
      frame->setScopeChain(frame->scopeChain()->copy()->push(scope));
      result = scope;
      QScriptEnginePrivate::setContextFlags(frame, flags | QScriptEnginePrivate::HasScopeContext);
   } else {
      // look in scope chain
      JSC::ScopeChainNode *node = frame->scopeChain();
      JSC::ScopeChainIterator it(node);
      for (it = node->begin(); it != node->end(); ++it) {
         if ((*it) && (*it)->isVariableObject()) {
            result = *it;
            break;
         }
      }
   }
   if (!result) {
      if (!parentContext()) {
         return engine()->globalObject();
      }

      qWarning("QScriptContext::activationObject:  could not get activation object for frame");
      return QScriptValue();

      /*JSC::CodeBlock *codeBlock = frame->codeBlock();
      if (!codeBlock) {
          // non-Qt native function
          Q_ASSERT(true); //### this should in theorry not happen
          result = new (frame)QScript::QScriptActivationObject(frame);
      } else {
          // ### this is wrong
          JSC::FunctionBodyNode *body = static_cast<JSC::FunctionBodyNode*>(codeBlock->ownerNode());
          result = new (frame)JSC::JSActivation(frame, body);
      }*/
   }

   if (result && result->inherits(&QScript::QScriptActivationObject::info)
      && (static_cast<QScript::QScriptActivationObject *>(result)->delegate() != nullptr)) {
      // Return the object that property access is being delegated to
      result = static_cast<QScript::QScriptActivationObject *>(result)->delegate();
   }

   return QScript::scriptEngineFromExec(frame)->scriptValueFromJSCValue(result);
}

void QScriptContext::setActivationObject(const QScriptValue &activation)
{
   if (!activation.isObject()) {
      return;
   } else if (activation.engine() != engine()) {
      qWarning("QScriptContext::setActivationObject() failed: can not set an object created in a different engine");
      return;
   }

   JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   QScriptEnginePrivate *engine = QScript::scriptEngineFromExec(frame);
   QScript::APIShim shim(engine);
   JSC::JSObject *object = JSC::asObject(engine->scriptValueToJSCValue(activation));

   if (object == engine->originalGlobalObjectProxy) {
      object = engine->originalGlobalObject();
   }

   uint flags = QScriptEnginePrivate::contextFlags(frame);
   if ((flags & QScriptEnginePrivate::NativeContext) && !(flags & QScriptEnginePrivate::HasScopeContext)) {
      //For native functions, we create a scope node
      JSC::JSObject *scope = object;
      if (!scope->isVariableObject()) {
         // Create a QScriptActivationObject that acts as a proxy
         scope = new (frame) QScript::QScriptActivationObject(frame, scope);
      }
      frame->setScopeChain(frame->scopeChain()->copy()->push(scope));
      QScriptEnginePrivate::setContextFlags(frame, flags | QScriptEnginePrivate::HasScopeContext);
      return;
   }

   // else replace the first activation object in the scope chain
   JSC::ScopeChainNode *node = frame->scopeChain();
   while (node != nullptr) {
      if (node->object && node->object->isVariableObject()) {
         if (!object->isVariableObject()) {
            if (node->object->inherits(&QScript::QScriptActivationObject::info)) {
               static_cast<QScript::QScriptActivationObject *>(node->object)->setDelegate(object);
            } else {
               // Create a QScriptActivationObject that acts as a proxy
               node->object = new (frame) QScript::QScriptActivationObject(frame, object);
            }
         } else {
            node->object = object;
         }
         break;
      }
      node = node->next;
   }
}

QScriptValue QScriptContext::thisObject() const
{
   JSC::CallFrame *frame = const_cast<JSC::ExecState *>(QScriptEnginePrivate::frameForContext(this));
   QScriptEnginePrivate *engine = QScript::scriptEngineFromExec(frame);
   QScript::APIShim shim(engine);
   JSC::JSValue result = engine->thisForContext(frame);

   if (!result || result.isNull()) {
      result = frame->globalThisValue();
   }

   return engine->scriptValueFromJSCValue(result);
}

void QScriptContext::setThisObject(const QScriptValue &thisObject)
{
   JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   QScript::APIShim shim(QScript::scriptEngineFromExec(frame));
   if (!thisObject.isObject()) {
      return;
   }

   if (thisObject.engine() != engine()) {
      qWarning("QScriptContext::setThisObject() failed: "
         "cannot set an object created in "
         "a different engine");
      return;
   }

   if (frame == frame->lexicalGlobalObject()->globalExec()) {
      engine()->setGlobalObject(thisObject);
      return;
   }

   JSC::JSValue jscThisObject = QScript::scriptEngineFromExec(frame)->scriptValueToJSCValue(thisObject);
   JSC::CodeBlock *cb = frame->codeBlock();

   if (cb != nullptr) {
      frame[cb->thisRegister()] = jscThisObject;
   } else {
      JSC::Register *thisRegister = QScriptEnginePrivate::thisRegisterForFrame(frame);
      thisRegister[0] = jscThisObject;
   }
}

QScriptContext::ExecutionState QScriptContext::state() const
{
   const JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   if (frame->hadException()) {
      return QScriptContext::ExceptionState;
   }
   return QScriptContext::NormalState;
}

QStringList QScriptContext::backtrace() const
{
   QStringList result;
   const QScriptContext *ctx = this;
   while (ctx) {
      result.append(ctx->toString());
      ctx = ctx->parentContext();
   }
   return result;
}

QString QScriptContext::toString() const
{
   QScriptContextInfo info(this);
   QString result;

   QString functionName = info.functionName();
   if (functionName.isEmpty()) {
      if (parentContext()) {
         const JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
         if (info.functionType() == QScriptContextInfo::ScriptFunction) {
            result.append(QLatin1String("<anonymous>"));
         } else if (frame->callerFrame()->hasHostCallFrameFlag()) {
            result.append(QLatin1String("<eval>"));
         } else {
            result.append(QLatin1String("<native>"));
         }
      } else {
         result.append(QLatin1String("<global>"));
      }
   } else {
      result.append(functionName);
   }

   QStringList parameterNames = info.functionParameterNames();
   result.append(QLatin1Char('('));
   for (int i = 0; i < argumentCount(); ++i) {
      if (i > 0) {
         result.append(QLatin1String(", "));
      }
      if (i < parameterNames.count()) {
         result.append(parameterNames.at(i));
         result.append(QLatin1String(" = "));
      }
      QScriptValue arg = argument(i);
      if (arg.isString()) {
         result.append(QLatin1Char('\''));
      }
      result.append(arg.toString());
      if (arg.isString()) {
         result.append(QLatin1Char('\''));
      }

   }
   result.append(QLatin1Char(')'));

   QString fileName = info.fileName();
   int lineNumber = info.lineNumber();
   result.append(QLatin1String(" at "));
   if (!fileName.isEmpty()) {
      result.append(fileName);
      result.append(QLatin1Char(':'));
   }
   result.append(QString::number(lineNumber));
   return result;
}

QList<QScriptValue> QScriptContext::scopeChain() const
{
   activationObject(); //ensure the creation of the normal scope for native context
   const JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   QScriptEnginePrivate *engine = QScript::scriptEngineFromExec(frame);
   QScript::APIShim shim(engine);

   QList<QScriptValue> result;
   JSC::ScopeChainNode *node = frame->scopeChain();
   JSC::ScopeChainIterator it(node);

   for (it = node->begin(); it != node->end(); ++it) {
      JSC::JSObject *object = *it;
      if (!object) {
         continue;
      }

      if (object->inherits(&QScript::QScriptActivationObject::info)
         && (static_cast<QScript::QScriptActivationObject *>(object)->delegate() != nullptr)) {
         // Return the object that property access is being delegated to
         object = static_cast<QScript::QScriptActivationObject *>(object)->delegate();
      }
      result.append(engine->scriptValueFromJSCValue(object));
   }
   return result;
}

// internal (cs)
void QScriptContext::pushScope(const QScriptValue &object)
{
   activationObject(); //ensure the creation of the normal scope for native context

   if (! object.isObject()) {
      return;

   } else if (object.engine() != engine()) {
      qWarning("QScriptContext::pushScope() failed: can not push an object created in a different engine");

      return;
   }

   JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   QScriptEnginePrivate *engine = QScript::scriptEngineFromExec(frame);
   QScript::APIShim shim(engine);
   JSC::JSObject *jscObject = JSC::asObject(engine->scriptValueToJSCValue(object));

   if (jscObject == engine->originalGlobalObjectProxy) {
      jscObject = engine->originalGlobalObject();
   }

   JSC::ScopeChainNode *scope = frame->scopeChain();

   Q_ASSERT(scope != nullptr);
   if (!scope->object) {
      // pushing to an "empty" chain
      if (!jscObject->isGlobalObject()) {
         qWarning("QScriptContext::pushScope() failed: initial object in scope chain has to be the Global Object");
         return;
      }
      scope->object = jscObject;

   } else {
      frame->setScopeChain(scope->push(jscObject));
   }
}

// internal (cs)
QScriptValue QScriptContext::popScope()
{
   activationObject(); //ensure the creation of the normal scope for native context
   JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   JSC::ScopeChainNode *scope = frame->scopeChain();

   Q_ASSERT(scope != nullptr);
   QScriptEnginePrivate *engine = QScript::scriptEngineFromExec(frame);
   QScript::APIShim shim(engine);
   QScriptValue result = engine->scriptValueFromJSCValue(scope->object);

   if (! scope->next) {
      // We cannot have a null scope chain, so just zap the object pointer.
      scope->object = nullptr;
   } else {
      frame->setScopeChain(scope->pop());
   }

   return result;
}
