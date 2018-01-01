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
#include "qscriptcontext.h"

#include "qscriptcontext_p.h"
#include "qscriptcontextinfo.h"
#include "qscriptengine.h"
#include "qscriptengine_p.h"
#include "../bridge/qscriptactivationobject_p.h"

#include "Arguments.h"
#include "CodeBlock.h"
#include "Error.h"
#include "JSFunction.h"
#include "JSObject.h"
#include "JSGlobalObject.h"

#include <QtCore/qstringlist.h>

QT_BEGIN_NAMESPACE

/*!
  \since 4.3
  \class QScriptContext

  \brief The QScriptContext class represents a Qt Script function invocation.

  \ingroup script
  \mainclass

  A QScriptContext provides access to the `this' object and arguments
  passed to a script function. You typically want to access this
  information when you're writing a native (C++) function (see
  QScriptEngine::newFunction()) that will be called from script
  code. For example, when the script code

  \snippet doc/src/snippets/code/src_script_qscriptcontext.cpp 0

  is evaluated, a QScriptContext will be created, and the context will
  carry the arguments as QScriptValues; in this particular case, the
  arguments will be one QScriptValue containing the number 20.5, a second
  QScriptValue containing the string \c{"hello"}, and a third QScriptValue
  containing a Qt Script object.

  Use argumentCount() to get the number of arguments passed to the
  function, and argument() to get an argument at a certain index. The
  argumentsObject() function returns a Qt Script array object
  containing all the arguments; you can use the QScriptValueIterator
  to iterate over its elements, or pass the array on as arguments to
  another script function using QScriptValue::call().

  Use thisObject() to get the `this' object associated with the function call,
  and setThisObject() to set the `this' object. If you are implementing a
  native "instance method", you typically fetch the thisObject() and access
  one or more of its properties:

  \snippet doc/src/snippets/code/src_script_qscriptcontext.cpp 1

  Use isCalledAsConstructor() to determine if the function was called
  as a constructor (e.g. \c{"new foo()"} (as constructor) or just
  \c{"foo()"}).  When a function is called as a constructor, the
  thisObject() contains the newly constructed object that the function
  is expected to initialize.

  Use throwValue() or throwError() to throw an exception.

  Use callee() to obtain the QScriptValue that represents the function being
  called. This can for example be used to call the function recursively.

  Use parentContext() to get a pointer to the context that precedes
  this context in the activation stack. This is mostly useful for
  debugging purposes (e.g. when constructing some form of backtrace).

  The activationObject() function returns the object that is used to
  hold the local variables associated with this function call. You can
  replace the activation object by calling setActivationObject(). A
  typical usage of these functions is when you want script code to be
  evaluated in the context of the parent context, e.g. to implement an
  include() function:

  \snippet doc/src/snippets/code/src_script_qscriptcontext.cpp 2

  Use backtrace() to get a human-readable backtrace associated with
  this context. This can be useful for debugging purposes when
  implementing native functions. The toString() function provides a
  string representation of the context. (QScriptContextInfo provides
  more detailed debugging-related information about the
  QScriptContext.)

  Use engine() to obtain a pointer to the QScriptEngine that this context
  resides in.

  \sa QScriptContextInfo, QScriptEngine::newFunction(), QScriptable
*/

/*!
    \enum QScriptContext::ExecutionState

    This enum specifies the frameution state of the context.

    \value NormalState The context is in a normal state.

    \value ExceptionState The context is in an exceptional state.
*/

/*!
    \enum QScriptContext::Error

    This enum specifies types of error.

    \value ReferenceError A reference error.

    \value SyntaxError A syntax error.

    \value TypeError A type error.

    \value RangeError A range error.

    \value URIError A URI error.

    \value UnknownError An unknown error.
*/

/*!
  \internal
*/
QScriptContext::QScriptContext()
{
   //QScriptContext doesn't exist,  pointer to QScriptContext are just pointer to  JSC::CallFrame
   Q_ASSERT(false);
}

/*!
  Throws an exception with the given \a value.
  Returns the value thrown (the same as the argument).

  \sa throwError(), state()
*/
QScriptValue QScriptContext::throwValue(const QScriptValue &value)
{
   JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   QScript::APIShim shim(QScript::scriptEngineFromExec(frame));
   JSC::JSValue jscValue = QScript::scriptEngineFromExec(frame)->scriptValueToJSCValue(value);
   frame->setException(jscValue);
   return value;
}

/*!
  Throws an \a error with the given \a text.
  Returns the created error object.

  The \a text will be stored in the \c{message} property of the error
  object.

  The error object will be initialized to contain information about
  the location where the error occurred; specifically, it will have
  properties \c{lineNumber}, \c{fileName} and \c{stack}. These
  properties are described in \l {QtScript Extensions to ECMAScript}.

  \sa throwValue(), state()
*/
QScriptValue QScriptContext::throwError(Error error, const QString &text)
{
   JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   QScript::APIShim shim(QScript::scriptEngineFromExec(frame));
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
   return QScript::scriptEngineFromExec(frame)->scriptValueFromJSCValue(result);
}

/*!
  \overload

  Throws an error with the given \a text.
  Returns the created error object.

  \sa throwValue(), state()
*/
QScriptValue QScriptContext::throwError(const QString &text)
{
   JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   QScript::APIShim shim(QScript::scriptEngineFromExec(frame));
   JSC::JSObject *result = JSC::throwError(frame, JSC::GeneralError, text);
   return QScript::scriptEngineFromExec(frame)->scriptValueFromJSCValue(result);
}

/*!
  Destroys this QScriptContext.
*/
QScriptContext::~QScriptContext()
{
   //QScriptContext doesn't exist,  pointer to QScriptContext are just pointer to JSC::CallFrame
   Q_ASSERT(false);
}

/*!
  Returns the QScriptEngine that this QScriptContext belongs to.
*/
QScriptEngine *QScriptContext::engine() const
{
   const JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   return QScriptEnginePrivate::get(QScript::scriptEngineFromExec(frame));
}

/*!
  Returns the function argument at the given \a index.

  If \a index >= argumentCount(), a QScriptValue of
  the primitive type Undefined is returned.

  \sa argumentCount()
*/
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

/*!
  Returns the callee. The callee is the function object that this
  QScriptContext represents an invocation of.
*/
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

/*!
  Returns the arguments object of this QScriptContext.

  The arguments object has properties \c callee (equal to callee())
  and \c length (equal to argumentCount()), and properties \c 0, \c 1,
  ..., argumentCount() - 1 that provide access to the argument
  values. Initially, property \c P (0 <= \c P < argumentCount()) has
  the same value as argument(\c P). In the case when \c P is less
  than the number of formal parameters of the function, \c P shares
  its value with the corresponding property of the activation object
  (activationObject()). This means that changing this property changes
  the corresponding property of the activation object and vice versa.

  \sa argument(), activationObject()
*/
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

/*!
  Returns true if the function was called as a constructor
  (e.g. \c{"new foo()"}); otherwise returns false.

  When a function is called as constructor, the thisObject()
  contains the newly constructed object to be initialized.

  \note This function is only guaranteed to work for a context
  corresponding to native functions.
*/
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

/*!
  Returns the parent context of this QScriptContext.
*/
QScriptContext *QScriptContext::parentContext() const
{
   const JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   QScript::APIShim shim(QScript::scriptEngineFromExec(frame));
   JSC::CallFrame *callerFrame = frame->callerFrame()->removeHostCallFrameFlag();
   return QScriptEnginePrivate::contextForFrame(callerFrame);
}

/*!
  Returns the number of arguments passed to the function
  in this invocation.

  Note that the argument count can be different from the
  formal number of arguments (the \c{length} property of
  callee()).

  \sa argument()
*/
int QScriptContext::argumentCount() const
{
   const JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   int argc = frame->argumentCount();
   if (argc != 0) {
      --argc;   // -1 due to "this"
   }
   return argc;
}

/*!
  \internal
*/
QScriptValue QScriptContext::returnValue() const
{
   qWarning("QScriptContext::returnValue() not implemented");
   return QScriptValue();
}

/*!
  \internal
*/
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

/*!
  Returns the activation object of this QScriptContext. The activation
  object provides access to the local variables associated with this
  context.

  \note The activation object might not be available if there is no
  active QScriptEngineAgent, as it might be optimized.

  \sa argument(), argumentsObject()
*/

QScriptValue QScriptContext::activationObject() const
{
   JSC::CallFrame *frame = const_cast<JSC::ExecState *>(QScriptEnginePrivate::frameForContext(this));
   QScript::APIShim shim(QScript::scriptEngineFromExec(frame));
   JSC::JSObject *result = 0;

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
         && (static_cast<QScript::QScriptActivationObject *>(result)->delegate() != 0)) {
      // Return the object that property access is being delegated to
      result = static_cast<QScript::QScriptActivationObject *>(result)->delegate();
   }

   return QScript::scriptEngineFromExec(frame)->scriptValueFromJSCValue(result);
}

/*!
  Sets the activation object of this QScriptContext to be the given \a
  activation.

  If \a activation is not an object, this function does nothing.

  \note For a context corresponding to a JavaScript function, this is only
  guaranteed to work if there was an QScriptEngineAgent active on the
  engine while the function was evaluated.
*/
void QScriptContext::setActivationObject(const QScriptValue &activation)
{
   if (!activation.isObject()) {
      return;
   } else if (activation.engine() != engine()) {
      qWarning("QScriptContext::setActivationObject() failed: "
               "cannot set an object created in "
               "a different engine");
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
   while (node != 0) {
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

/*!
  Returns the `this' object associated with this QScriptContext.
*/
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

/*!
  Sets the `this' object associated with this QScriptContext to be
  \a thisObject.

  If \a thisObject is not an object, this function does nothing.
*/
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
   if (cb != 0) {
      frame[cb->thisRegister()] = jscThisObject;
   } else {
      JSC::Register *thisRegister = QScriptEnginePrivate::thisRegisterForFrame(frame);
      thisRegister[0] = jscThisObject;
   }
}

/*!
  Returns the frameution state of this QScriptContext.
*/
QScriptContext::ExecutionState QScriptContext::state() const
{
   const JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   if (frame->hadException()) {
      return QScriptContext::ExceptionState;
   }
   return QScriptContext::NormalState;
}

/*!
  Returns a human-readable backtrace of this QScriptContext.

  Each line is of the form \c{<function-name>(<arguments>)@<file-name>:<line-number>}.

  To access individual pieces of debugging-related information (for
  example, to construct your own backtrace representation), use
  QScriptContextInfo.

  \sa QScriptEngine::uncaughtExceptionBacktrace(), QScriptContextInfo, toString()
*/
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

/*!
  \since 4.4

  Returns a string representation of this context.
  This is useful for debugging.

  \sa backtrace()
*/
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

/*!
  \internal
  \since 4.5

  Returns the scope chain of this QScriptContext.
*/
QScriptValueList QScriptContext::scopeChain() const
{
   activationObject(); //ensure the creation of the normal scope for native context
   const JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   QScriptEnginePrivate *engine = QScript::scriptEngineFromExec(frame);
   QScript::APIShim shim(engine);
   QScriptValueList result;
   JSC::ScopeChainNode *node = frame->scopeChain();
   JSC::ScopeChainIterator it(node);
   for (it = node->begin(); it != node->end(); ++it) {
      JSC::JSObject *object = *it;
      if (!object) {
         continue;
      }
      if (object->inherits(&QScript::QScriptActivationObject::info)
            && (static_cast<QScript::QScriptActivationObject *>(object)->delegate() != 0)) {
         // Return the object that property access is being delegated to
         object = static_cast<QScript::QScriptActivationObject *>(object)->delegate();
      }
      result.append(engine->scriptValueFromJSCValue(object));
   }
   return result;
}

/*!
  \internal
  \since 4.5

  Adds the given \a object to the front of this context's scope chain.

  If \a object is not an object, this function does nothing.
*/
void QScriptContext::pushScope(const QScriptValue &object)
{
   activationObject(); //ensure the creation of the normal scope for native context
   if (!object.isObject()) {
      return;
   } else if (object.engine() != engine()) {
      qWarning("QScriptContext::pushScope() failed: "
               "cannot push an object created in "
               "a different engine");
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
   Q_ASSERT(scope != 0);
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

/*!
  \internal
  \since 4.5

  Removes the front object from this context's scope chain, and
  returns the removed object.

  If the scope chain is already empty, this function returns an
  invalid QScriptValue.
*/
QScriptValue QScriptContext::popScope()
{
   activationObject(); //ensure the creation of the normal scope for native context
   JSC::CallFrame *frame = QScriptEnginePrivate::frameForContext(this);
   JSC::ScopeChainNode *scope = frame->scopeChain();
   Q_ASSERT(scope != 0);
   QScriptEnginePrivate *engine = QScript::scriptEngineFromExec(frame);
   QScript::APIShim shim(engine);
   QScriptValue result = engine->scriptValueFromJSCValue(scope->object);
   if (!scope->next) {
      // We cannot have a null scope chain, so just zap the object pointer.
      scope->object = 0;
   } else {
      frame->setScopeChain(scope->pop());
   }
   return result;
}

QT_END_NAMESPACE
