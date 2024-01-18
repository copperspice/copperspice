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
#include <qscriptglobalobject_p.h>

#include <qscriptengine.h>
#include <qscriptengine_p.h>

namespace JSC {
ASSERT_CLASS_FITS_IN_CELL(QScript::GlobalObject);
ASSERT_CLASS_FITS_IN_CELL(QScript::OriginalGlobalObjectProxy);

} // namespace JSC

namespace QScript {

GlobalObject::GlobalObject()
   : JSC::JSGlobalObject(), customGlobalObject(nullptr)
{
}

GlobalObject::~GlobalObject()
{
}

void GlobalObject::markChildren(JSC::MarkStack &markStack)
{
   JSC::JSGlobalObject::markChildren(markStack);
   if (customGlobalObject) {
      markStack.append(customGlobalObject);
   }
}

bool GlobalObject::getOwnPropertySlot(JSC::ExecState *exec,
   const JSC::Identifier &propertyName,
   JSC::PropertySlot &slot)
{
   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
   if (propertyName == exec->propertyNames().arguments && engine->currentFrame->argumentCount() > 0) {
      JSC::JSValue args = engine->scriptValueToJSCValue(engine->contextForFrame(engine->currentFrame)->argumentsObject());
      slot.setValue(args);
      return true;
   }
   if (customGlobalObject) {
      return customGlobalObject->getOwnPropertySlot(exec, propertyName, slot);
   }
   return JSC::JSGlobalObject::getOwnPropertySlot(exec, propertyName, slot);
}

bool GlobalObject::getOwnPropertyDescriptor(JSC::ExecState *exec,
   const JSC::Identifier &propertyName,
   JSC::PropertyDescriptor &descriptor)
{
   // Must match the logic of getOwnPropertySlot().
   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
   if (propertyName == exec->propertyNames().arguments && engine->currentFrame->argumentCount() > 0) {
      // ### Can we get rid of this special handling of the arguments property?
      JSC::JSValue args = engine->scriptValueToJSCValue(engine->contextForFrame(engine->currentFrame)->argumentsObject());
      descriptor.setValue(args);
      return true;
   }
   if (customGlobalObject) {
      return customGlobalObject->getOwnPropertyDescriptor(exec, propertyName, descriptor);
   }
   return JSC::JSGlobalObject::getOwnPropertyDescriptor(exec, propertyName, descriptor);
}

void GlobalObject::put(JSC::ExecState *exec, const JSC::Identifier &propertyName,
   JSC::JSValue value, JSC::PutPropertySlot &slot)
{
   if (customGlobalObject) {
      customGlobalObject->put(exec, propertyName, value, slot);
   } else {
      JSC::JSGlobalObject::put(exec, propertyName, value, slot);
   }
}

void GlobalObject::putWithAttributes(JSC::ExecState *exec, const JSC::Identifier &propertyName,
   JSC::JSValue value, unsigned attributes)
{
   if (customGlobalObject) {
      customGlobalObject->putWithAttributes(exec, propertyName, value, attributes);
   } else {
      JSC::JSGlobalObject::putWithAttributes(exec, propertyName, value, attributes);
   }
}

bool GlobalObject::deleteProperty(JSC::ExecState *exec, const JSC::Identifier &propertyName)
{
   if (customGlobalObject) {
      return customGlobalObject->deleteProperty(exec, propertyName);
   }
   return JSC::JSGlobalObject::deleteProperty(exec, propertyName);
}

void GlobalObject::getOwnPropertyNames(JSC::ExecState *exec, JSC::PropertyNameArray &propertyNames,
   JSC::EnumerationMode mode)
{
   if (customGlobalObject) {
      customGlobalObject->getOwnPropertyNames(exec, propertyNames, mode);
   } else {
      JSC::JSGlobalObject::getOwnPropertyNames(exec, propertyNames, mode);
   }
}

void GlobalObject::defineGetter(JSC::ExecState *exec, const JSC::Identifier &propertyName,
   JSC::JSObject *getterFunction, unsigned attributes)
{
   if (customGlobalObject) {
      customGlobalObject->defineGetter(exec, propertyName, getterFunction, attributes);
   } else {
      JSC::JSGlobalObject::defineGetter(exec, propertyName, getterFunction, attributes);
   }
}

void GlobalObject::defineSetter(JSC::ExecState *exec, const JSC::Identifier &propertyName,
   JSC::JSObject *setterFunction, unsigned attributes)
{
   if (customGlobalObject) {
      customGlobalObject->defineSetter(exec, propertyName, setterFunction, attributes);
   } else {
      JSC::JSGlobalObject::defineSetter(exec, propertyName, setterFunction, attributes);
   }
}

JSC::JSValue GlobalObject::lookupGetter(JSC::ExecState *exec, const JSC::Identifier &propertyName)
{
   if (customGlobalObject) {
      return customGlobalObject->lookupGetter(exec, propertyName);
   }
   return JSC::JSGlobalObject::lookupGetter(exec, propertyName);
}

JSC::JSValue GlobalObject::lookupSetter(JSC::ExecState *exec, const JSC::Identifier &propertyName)
{
   if (customGlobalObject) {
      return customGlobalObject->lookupSetter(exec, propertyName);
   }
   return JSC::JSGlobalObject::lookupSetter(exec, propertyName);
}

} // namespace QScript

