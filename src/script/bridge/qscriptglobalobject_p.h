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

#ifndef QSCRIPTGLOBALOBJECT_P_H
#define QSCRIPTGLOBALOBJECT_P_H

#include "JSGlobalObject.h"

namespace QScript {

class GlobalObject : public JSC::JSGlobalObject
{
 public:
   GlobalObject();
   virtual ~GlobalObject();
   virtual JSC::UString className() const {
      return "global";
   }
   virtual void markChildren(JSC::MarkStack &);
   virtual bool getOwnPropertySlot(JSC::ExecState *,
      const JSC::Identifier &propertyName,
      JSC::PropertySlot &);
   virtual bool getOwnPropertyDescriptor(JSC::ExecState *,
      const JSC::Identifier &propertyName,
      JSC::PropertyDescriptor &);
   virtual void put(JSC::ExecState *exec, const JSC::Identifier &propertyName,
      JSC::JSValue, JSC::PutPropertySlot &);
   virtual void putWithAttributes(JSC::ExecState *exec, const JSC::Identifier &propertyName,
      JSC::JSValue value, unsigned attributes);
   virtual bool deleteProperty(JSC::ExecState *,
      const JSC::Identifier &propertyName);
   virtual void getOwnPropertyNames(JSC::ExecState *, JSC::PropertyNameArray &,
      JSC::EnumerationMode mode = JSC::ExcludeDontEnumProperties);
   virtual void defineGetter(JSC::ExecState *, const JSC::Identifier &propertyName, JSC::JSObject *getterFunction,
      unsigned attributes = 0);
   virtual void defineSetter(JSC::ExecState *, const JSC::Identifier &propertyName, JSC::JSObject *setterFunction,
      unsigned attributes = 0);
   virtual JSC::JSValue lookupGetter(JSC::ExecState *, const JSC::Identifier &propertyName);
   virtual JSC::JSValue lookupSetter(JSC::ExecState *, const JSC::Identifier &propertyName);

 public:
   JSC::JSObject *customGlobalObject;
};

class OriginalGlobalObjectProxy : public JSC::JSObject
{
 public:
   explicit OriginalGlobalObjectProxy(WTF::PassRefPtr<JSC::Structure> sid,
      JSC::JSGlobalObject *object)
      : JSC::JSObject(sid), originalGlobalObject(object) {
   }
   virtual ~OriginalGlobalObjectProxy() {
   }
   virtual JSC::UString className() const {
      return originalGlobalObject->className();
   }
   virtual void markChildren(JSC::MarkStack &markStack) {
      markStack.append(originalGlobalObject);
      JSC::JSObject::markChildren(markStack);
   }
   virtual bool getOwnPropertySlot(JSC::ExecState *exec,
      const JSC::Identifier &propertyName,
      JSC::PropertySlot &slot) {
      return originalGlobalObject->JSC::JSGlobalObject::getOwnPropertySlot(exec, propertyName, slot);
   }
   virtual bool getOwnPropertyDescriptor(JSC::ExecState *exec,
      const JSC::Identifier &propertyName,
      JSC::PropertyDescriptor &descriptor) {
      return originalGlobalObject->JSC::JSGlobalObject::getOwnPropertyDescriptor(exec, propertyName, descriptor);
   }
   virtual void put(JSC::ExecState *exec, const JSC::Identifier &propertyName,
      JSC::JSValue value, JSC::PutPropertySlot &slot) {
      originalGlobalObject->JSC::JSGlobalObject::put(exec, propertyName, value, slot);
   }
   virtual void putWithAttributes(JSC::ExecState *exec, const JSC::Identifier &propertyName, JSC::JSValue value,
      unsigned attributes) {
      originalGlobalObject->JSC::JSGlobalObject::putWithAttributes(exec, propertyName, value, attributes);
   }
   virtual bool deleteProperty(JSC::ExecState *exec,
      const JSC::Identifier &propertyName) {
      return originalGlobalObject->JSC::JSGlobalObject::deleteProperty(exec, propertyName);
   }
   virtual void getOwnPropertyNames(JSC::ExecState *exec, JSC::PropertyNameArray &propertyNames,
      JSC::EnumerationMode mode = JSC::ExcludeDontEnumProperties) {
      originalGlobalObject->JSC::JSGlobalObject::getOwnPropertyNames(exec, propertyNames, mode);
   }
   virtual void defineGetter(JSC::ExecState *exec, const JSC::Identifier &propertyName, JSC::JSObject *getterFunction,
      unsigned attributes) {
      originalGlobalObject->JSC::JSGlobalObject::defineGetter(exec, propertyName, getterFunction, attributes);
   }
   virtual void defineSetter(JSC::ExecState *exec, const JSC::Identifier &propertyName, JSC::JSObject *setterFunction,
      unsigned attributes) {
      originalGlobalObject->JSC::JSGlobalObject::defineSetter(exec, propertyName, setterFunction, attributes);
   }
   virtual JSC::JSValue lookupGetter(JSC::ExecState *exec, const JSC::Identifier &propertyName) {
      return originalGlobalObject->JSC::JSGlobalObject::lookupGetter(exec, propertyName);
   }
   virtual JSC::JSValue lookupSetter(JSC::ExecState *exec, const JSC::Identifier &propertyName) {
      return originalGlobalObject->JSC::JSGlobalObject::lookupSetter(exec, propertyName);
   }

 private:
   JSC::JSGlobalObject *originalGlobalObject;
};

} // namespace QScript

#endif
