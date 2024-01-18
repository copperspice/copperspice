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
#include "qscriptfunction_p.h"

#include "qscriptengine_p.h"
#include "qscriptcontext.h"
#include "qscriptcontext_p.h"
#include "qscriptvalue_p.h"
#include "qscriptactivationobject_p.h"
#include "qscriptobject_p.h"

#include "JSGlobalObject.h"
#include "DebuggerCallFrame.h"
#include "Debugger.h"

namespace JSC {
ASSERT_CLASS_FITS_IN_CELL(QT_PREPEND_NAMESPACE(QScript::FunctionWrapper));
ASSERT_CLASS_FITS_IN_CELL(QT_PREPEND_NAMESPACE(QScript::FunctionWithArgWrapper));
}

namespace QScript {

const JSC::ClassInfo FunctionWrapper::info = { "QtNativeFunctionWrapper", &PrototypeFunction::info, nullptr, nullptr };
const JSC::ClassInfo FunctionWithArgWrapper::info = { "QtNativeFunctionWithArgWrapper", &PrototypeFunction::info, nullptr, nullptr };

FunctionWrapper::FunctionWrapper(JSC::ExecState *exec, int length, const JSC::Identifier &name,
   QScriptEngine::FunctionSignature function)
   : JSC::PrototypeFunction(exec, length, name, proxyCall), data(new Data())
{
   data->function = function;
}

FunctionWrapper::~FunctionWrapper()
{
   delete data;
}

JSC::ConstructType FunctionWrapper::getConstructData(JSC::ConstructData &consData)
{
   consData.native.function = proxyConstruct;
   consData.native.function.doNotCallDebuggerFunctionExit();
   return JSC::ConstructTypeHost;
}

JSC::JSValue FunctionWrapper::proxyCall(JSC::ExecState *exec, JSC::JSObject *callee,
   JSC::JSValue thisObject, const JSC::ArgList &args)
{
   FunctionWrapper *self = static_cast<FunctionWrapper *>(callee);
   QScriptEnginePrivate *eng_p = QScript::scriptEngineFromExec(exec);

   JSC::ExecState *oldFrame = eng_p->currentFrame;
   eng_p->pushContext(exec, thisObject, args, callee);
   QScriptContext *ctx = eng_p->contextForFrame(eng_p->currentFrame);

   QScriptValue result = self->data->function(ctx, QScriptEnginePrivate::get(eng_p));
   if (!result.isValid()) {
      result = QScriptValue(QScriptValue::UndefinedValue);
   }

   eng_p->popContext();
   eng_p->currentFrame = oldFrame;

   return eng_p->scriptValueToJSCValue(result);
}

JSC::JSObject *FunctionWrapper::proxyConstruct(JSC::ExecState *exec, JSC::JSObject *callee,
   const JSC::ArgList &args)
{
   FunctionWrapper *self = static_cast<FunctionWrapper *>(callee);
   QScriptEnginePrivate *eng_p = QScript::scriptEngineFromExec(exec);

   JSC::ExecState *oldFrame = eng_p->currentFrame;
   eng_p->pushContext(exec, JSC::JSValue(), args, callee, true);
   QScriptContext *ctx = eng_p->contextForFrame(eng_p->currentFrame);

   QScriptValue result = self->data->function(ctx, QScriptEnginePrivate::get(eng_p));

   if (JSC::Debugger *debugger = eng_p->originalGlobalObject()->debugger()) {
      debugger->functionExit(QScriptValuePrivate::get(result)->jscValue, -1);
   }

   if (!result.isObject()) {
      result = ctx->thisObject();
   }

   eng_p->popContext();
   eng_p->currentFrame = oldFrame;

   return JSC::asObject(eng_p->scriptValueToJSCValue(result));
}

FunctionWithArgWrapper::FunctionWithArgWrapper(JSC::ExecState *exec, int length, const JSC::Identifier &name,
   QScriptEngine::FunctionWithArgSignature function, void *arg)
   : JSC::PrototypeFunction(exec, length, name, proxyCall),
     data(new Data())
{
   data->function = function;
   data->arg = arg;
}

FunctionWithArgWrapper::~FunctionWithArgWrapper()
{
   delete data;
}

JSC::ConstructType FunctionWithArgWrapper::getConstructData(JSC::ConstructData &consData)
{
   consData.native.function = proxyConstruct;
   return JSC::ConstructTypeHost;
}

JSC::JSValue FunctionWithArgWrapper::proxyCall(JSC::ExecState *exec, JSC::JSObject *callee,
   JSC::JSValue thisObject, const JSC::ArgList &args)
{
   FunctionWithArgWrapper *self = static_cast<FunctionWithArgWrapper *>(callee);
   QScriptEnginePrivate *eng_p = QScript::scriptEngineFromExec(exec);

   JSC::ExecState *oldFrame = eng_p->currentFrame;
   eng_p->pushContext(exec, thisObject, args, callee);
   QScriptContext *ctx = eng_p->contextForFrame(eng_p->currentFrame);

   QScriptValue result = self->data->function(ctx, QScriptEnginePrivate::get(eng_p), self->data->arg);

   eng_p->popContext();
   eng_p->currentFrame = oldFrame;

   return eng_p->scriptValueToJSCValue(result);
}

JSC::JSObject *FunctionWithArgWrapper::proxyConstruct(JSC::ExecState *exec, JSC::JSObject *callee,
   const JSC::ArgList &args)
{
   FunctionWithArgWrapper *self = static_cast<FunctionWithArgWrapper *>(callee);
   QScriptEnginePrivate *eng_p = QScript::scriptEngineFromExec(exec);

   JSC::ExecState *oldFrame = eng_p->currentFrame;
   eng_p->pushContext(exec, JSC::JSValue(), args, callee, true);
   QScriptContext *ctx = eng_p->contextForFrame(eng_p->currentFrame);

   QScriptValue result = self->data->function(ctx, QScriptEnginePrivate::get(eng_p), self->data->arg);
   if (!result.isObject()) {
      result = ctx->thisObject();
   }

   eng_p->popContext();
   eng_p->currentFrame = oldFrame;

   return JSC::asObject(eng_p->scriptValueToJSCValue(result));
}

} // namespace
