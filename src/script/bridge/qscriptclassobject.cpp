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
#include "Error.h"
#include "PropertyNameArray.h"

#include <qscriptclassobject_p.h>

#include <qscriptengine.h>
#include <qscriptcontext.h>
#include <qscriptclass.h>
#include <qscriptclasspropertyiterator.h>

#include <qscriptengine_p.h>
#include <qscriptcontext_p.h>

namespace QScript {

ClassObjectDelegate::ClassObjectDelegate(QScriptClass *scriptClass)
   : m_scriptClass(scriptClass)
{
}

ClassObjectDelegate::~ClassObjectDelegate()
{
}

QScriptObjectDelegate::Type ClassObjectDelegate::type() const
{
   return ClassObject;
}

bool ClassObjectDelegate::getOwnPropertySlot(QScriptObject *object,
   JSC::ExecState *exec, const JSC::Identifier &propertyName, JSC::PropertySlot &slot)
{
   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
   QScript::SaveFrameHelper saveFrame(engine, exec);

   // for compatibility with the old back-end, normal JS properties are queried first

   if (QScriptObjectDelegate::getOwnPropertySlot(object, exec, propertyName, slot)) {
      return true;
   }

   QScriptValue scriptObject = engine->scriptValueFromJSCValue(object);
   QScriptString scriptName;
   QScriptStringPrivate scriptName_d(engine, propertyName, QScriptStringPrivate::StackAllocated);
   QScriptStringPrivate::init(scriptName, &scriptName_d);
   uint id = 0;

   QScriptClass::QueryFlags flags = m_scriptClass->queryProperty(
         scriptObject, scriptName, QScriptClass::HandlesReadAccess, &id);

   if (flags & QScriptClass::HandlesReadAccess) {
      QScriptValue value = m_scriptClass->property(scriptObject, scriptName, id);

      if (! value.isValid()) {
         // The class claims to have the property, but returned an invalid
         // value. Silently convert to undefined to avoid the invalid value
         // "escaping" into JS.
         value = QScriptValue(QScriptValue::UndefinedValue);
      }

      slot.setValue(engine->scriptValueToJSCValue(value));
      return true;
   }

   return false;
}

bool ClassObjectDelegate::getOwnPropertyDescriptor(QScriptObject *object,
   JSC::ExecState *exec, const JSC::Identifier &propertyName, JSC::PropertyDescriptor &descriptor)
{
   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
   QScript::SaveFrameHelper saveFrame(engine, exec);

   // for compatibility with the old back-end, normal JS properties are queried first
   if (QScriptObjectDelegate::getOwnPropertyDescriptor(object, exec, propertyName, descriptor)) {
      return true;
   }

   QScriptValue scriptObject = engine->scriptValueFromJSCValue(object);
   QScriptString scriptName;
   QScriptStringPrivate scriptName_d(engine, propertyName, QScriptStringPrivate::StackAllocated);
   QScriptStringPrivate::init(scriptName, &scriptName_d);
   uint id = 0;

   QScriptClass::QueryFlags qflags = m_scriptClass->queryProperty(
         scriptObject, scriptName, QScriptClass::HandlesReadAccess, &id);

   if (qflags & QScriptClass::HandlesReadAccess) {
      QScriptValue::PropertyFlags pflags = m_scriptClass->propertyFlags(scriptObject, scriptName, id);
      unsigned attribs = 0;

      if (pflags & QScriptValue::ReadOnly) {
         attribs |= JSC::ReadOnly;
      }

      if (pflags & QScriptValue::SkipInEnumeration) {
         attribs |= JSC::DontEnum;
      }

      if (pflags & QScriptValue::Undeletable) {
         attribs |= JSC::DontDelete;
      }

      if (pflags & QScriptValue::PropertyGetter) {
         attribs |= JSC::Getter;
      }

      if (pflags & QScriptValue::PropertySetter) {
         attribs |= JSC::Setter;
      }

      attribs |= pflags & QScriptValue::UserRange;
      // Rather than calling the getter, we could return an access descriptor here.
      QScriptValue value = m_scriptClass->property(scriptObject, scriptName, id);
      if (!value.isValid()) {
         // The class claims to have the property, but returned an invalid
         // value. Silently convert to undefined to avoid the invalid value
         // "escaping" into JS.
         value = QScriptValue(QScriptValue::UndefinedValue);
      }

      descriptor.setDescriptor(engine->scriptValueToJSCValue(value), attribs);
      return true;
   }

   return false;
}

void ClassObjectDelegate::put(QScriptObject *object, JSC::ExecState *exec,
   const JSC::Identifier &propertyName, JSC::JSValue value, JSC::PutPropertySlot &slot)
{
   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
   QScript::SaveFrameHelper saveFrame(engine, exec);
   QScriptValue scriptObject = engine->scriptValueFromJSCValue(object);
   QScriptString scriptName;
   QScriptStringPrivate scriptName_d(engine, propertyName, QScriptStringPrivate::StackAllocated);
   QScriptStringPrivate::init(scriptName, &scriptName_d);
   uint id = 0;

   QScriptClass::QueryFlags flags = m_scriptClass->queryProperty(
         scriptObject, scriptName, QScriptClass::HandlesWriteAccess, &id);

   if (flags & QScriptClass::HandlesWriteAccess) {
      m_scriptClass->setProperty(scriptObject, scriptName, id, engine->scriptValueFromJSCValue(value));
      return;
   }

   QScriptObjectDelegate::put(object, exec, propertyName, value, slot);
}

bool ClassObjectDelegate::deleteProperty(QScriptObject *object, JSC::ExecState *exec,
   const JSC::Identifier &propertyName)
{
   // ### avoid duplication of put()
   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
   QScript::SaveFrameHelper saveFrame(engine, exec);
   QScriptValue scriptObject = engine->scriptValueFromJSCValue(object);
   QScriptString scriptName;
   QScriptStringPrivate scriptName_d(engine, propertyName, QScriptStringPrivate::StackAllocated);
   QScriptStringPrivate::init(scriptName, &scriptName_d);
   uint id = 0;

   QScriptClass::QueryFlags flags = m_scriptClass->queryProperty(
         scriptObject, scriptName, QScriptClass::HandlesWriteAccess, &id);

   if (flags & QScriptClass::HandlesWriteAccess) {
      if (m_scriptClass->propertyFlags(scriptObject, scriptName, id) & QScriptValue::Undeletable) {
         return false;
      }

      m_scriptClass->setProperty(scriptObject, scriptName, id, QScriptValue());
      return true;
   }

   return QScriptObjectDelegate::deleteProperty(object, exec, propertyName);
}

void ClassObjectDelegate::getOwnPropertyNames(QScriptObject *object, JSC::ExecState *exec,
   JSC::PropertyNameArray &propertyNames, JSC::EnumerationMode mode)
{
   // For compatibility with the old back-end, normal JS properties are added first.
   QScriptObjectDelegate::getOwnPropertyNames(object, exec, propertyNames, mode);

   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
   QScript::SaveFrameHelper saveFrame(engine, exec);
   QScriptValue scriptObject = engine->scriptValueFromJSCValue(object);
   QScriptClassPropertyIterator *it = m_scriptClass->newIterator(scriptObject);

   if (it != nullptr) {
      while (it->hasNext()) {
         it->next();
         QString name = it->name().toString();
         propertyNames.add(JSC::Identifier(exec, name));
      }

      delete it;
   }
}

JSC::CallType ClassObjectDelegate::getCallData(QScriptObject *, JSC::CallData &callData)
{
   if (! m_scriptClass->supportsExtension(QScriptClass::Callable)) {
      return JSC::CallTypeNone;
   }

   callData.native.function = call;

   return JSC::CallTypeHost;
}

JSC::JSValue JSC_HOST_CALL ClassObjectDelegate::call(JSC::ExecState *exec, JSC::JSObject *callee,
   JSC::JSValue thisValue, const JSC::ArgList &args)
{
   if (! callee->inherits(&QScriptObject::info)) {
      return JSC::throwError(exec, JSC::TypeError, "callee is not a ClassObject object");
   }

   QScriptObject *obj = static_cast<QScriptObject *>(callee);
   QScriptObjectDelegate *delegate = obj->delegate();

   if (! delegate || (delegate->type() != QScriptObjectDelegate::ClassObject)) {
      return JSC::throwError(exec, JSC::TypeError, "callee is not a ClassObject object");
   }

   QScriptClass *scriptClass   = static_cast<ClassObjectDelegate *>(delegate)->scriptClass();
   QScriptEnginePrivate *eng_p = scriptEngineFromExec(exec);

   JSC::ExecState *oldFrame = eng_p->currentFrame;
   eng_p->pushContext(exec, thisValue, args, callee);

   QScriptContext *ctx = eng_p->contextForFrame(eng_p->currentFrame);
   QScriptValue scriptObject = eng_p->scriptValueFromJSCValue(obj);
   QVariant result = scriptClass->extension(QScriptClass::Callable, QVariant::fromValue(ctx));

   eng_p->popContext();
   eng_p->currentFrame = oldFrame;

   return QScriptEnginePrivate::jscValueFromVariant(exec, result);
}

JSC::ConstructType ClassObjectDelegate::getConstructData(QScriptObject *, JSC::ConstructData &constructData)
{
   if (!m_scriptClass->supportsExtension(QScriptClass::Callable)) {
      return JSC::ConstructTypeNone;
   }

   constructData.native.function = construct;
   return JSC::ConstructTypeHost;
}

JSC::JSObject *ClassObjectDelegate::construct(JSC::ExecState *exec, JSC::JSObject *callee, const JSC::ArgList &args)
{
   Q_ASSERT(callee->inherits(&QScriptObject::info));
   QScriptObject *obj = static_cast<QScriptObject *>(callee);
   QScriptObjectDelegate *delegate = obj->delegate();
   QScriptClass *scriptClass = static_cast<ClassObjectDelegate *>(delegate)->scriptClass();

   QScriptEnginePrivate *eng_p = scriptEngineFromExec(exec);
   JSC::ExecState *oldFrame = eng_p->currentFrame;
   eng_p->pushContext(exec, JSC::JSValue(), args, callee, true);
   QScriptContext *ctx = eng_p->contextForFrame(eng_p->currentFrame);

   QScriptValue defaultObject = ctx->thisObject();

   QVariant variant    = scriptClass->extension(QScriptClass::Callable, QVariant::fromValue(ctx));
   QScriptValue result = variant.value<QScriptValue>();

   if (! result.isObject()) {
      result = defaultObject;
   }

   eng_p->popContext();
   eng_p->currentFrame = oldFrame;

   return JSC::asObject(eng_p->scriptValueToJSCValue(result));
}

bool ClassObjectDelegate::hasInstance(QScriptObject *object, JSC::ExecState *exec,
   JSC::JSValue value, JSC::JSValue proto)
{
   if (! scriptClass()->supportsExtension(QScriptClass::HasInstance)) {
      return QScriptObjectDelegate::hasInstance(object, exec, value, proto);
   }

   QList<QScriptValue> args;
   QScriptEnginePrivate *eng_p = scriptEngineFromExec(exec);

   QScript::SaveFrameHelper saveFrame(eng_p, exec);
   args << eng_p->scriptValueFromJSCValue(object) << eng_p->scriptValueFromJSCValue(value);

   QVariant result = scriptClass()->extension(QScriptClass::HasInstance, QVariant::fromValue(args));

   return result.toBool();
}

} // namespace QScript

