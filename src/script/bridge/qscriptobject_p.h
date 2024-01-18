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

#ifndef QSCRIPTOBJECT_P_H
#define QSCRIPTOBJECT_P_H

#include "JSObject.h"

class QScriptObjectDelegate;

class QScriptObject : public JSC::JSObject
{
 public:
   // work around CELL_SIZE limitation
   struct Data {
      JSC::JSValue data; // QScriptValue::data
      QScriptObjectDelegate *delegate;
      bool isMarking; // recursion guard

      Data()
         : delegate(nullptr), isMarking(false)
      {
      }

      ~Data();
   };

   explicit QScriptObject(WTF::PassRefPtr<JSC::Structure> sid);
   virtual ~QScriptObject();

   virtual bool getOwnPropertySlot(JSC::ExecState *,
      const JSC::Identifier &propertyName,
      JSC::PropertySlot &);
   virtual bool getOwnPropertyDescriptor(JSC::ExecState *, const JSC::Identifier &, JSC::PropertyDescriptor &);
   virtual void put(JSC::ExecState *exec, const JSC::Identifier &propertyName,
      JSC::JSValue, JSC::PutPropertySlot &);
   virtual bool deleteProperty(JSC::ExecState *,
      const JSC::Identifier &propertyName);
   virtual void getOwnPropertyNames(JSC::ExecState *, JSC::PropertyNameArray &,
      JSC::EnumerationMode mode = JSC::ExcludeDontEnumProperties);
   virtual void markChildren(JSC::MarkStack &markStack);
   virtual JSC::CallType getCallData(JSC::CallData &);
   virtual JSC::ConstructType getConstructData(JSC::ConstructData &);
   virtual bool hasInstance(JSC::ExecState *, JSC::JSValue value, JSC::JSValue proto);
   virtual bool compareToObject(JSC::ExecState *, JSC::JSObject *);

   virtual const JSC::ClassInfo *classInfo() const {
      return &info;
   }
   static const JSC::ClassInfo info;

   static WTF::PassRefPtr<JSC::Structure> createStructure(JSC::JSValue prototype) {
      return JSC::Structure::create(prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags));
   }

   inline JSC::JSValue data() const;
   inline void setData(JSC::JSValue data);

   inline QScriptObjectDelegate *delegate() const;
   inline void setDelegate(QScriptObjectDelegate *delegate);

 protected:
   static const unsigned StructureFlags = JSC::ImplementsHasInstance | JSC::OverridesHasInstance |
      JSC::OverridesGetOwnPropertySlot | JSC::OverridesMarkChildren | JSC::OverridesGetPropertyNames |
      JSObject::StructureFlags;

   Data *d;
};

class QScriptObjectPrototype : public QScriptObject
{
 public:
   QScriptObjectPrototype(JSC::ExecState *, WTF::PassRefPtr<JSC::Structure>,
      JSC::Structure *prototypeFunctionStructure);
};

class QScriptObjectDelegate
{
 public:
   enum Type {
      QtObject,
      Variant,
      ClassObject,
      DeclarativeClassObject
   };

   QScriptObjectDelegate();

   QScriptObjectDelegate(const QScriptObjectDelegate &) = delete;
   QScriptObjectDelegate &operator=(const QScriptObjectDelegate &) = delete;

   virtual ~QScriptObjectDelegate();

   virtual Type type() const = 0;

   virtual bool getOwnPropertySlot(QScriptObject *, JSC::ExecState *,
      const JSC::Identifier &propertyName, JSC::PropertySlot &);

   virtual bool getOwnPropertyDescriptor(QScriptObject *, JSC::ExecState *,
      const JSC::Identifier &propertyName, JSC::PropertyDescriptor &);

   virtual void put(QScriptObject *, JSC::ExecState *exec, const JSC::Identifier &propertyName,
      JSC::JSValue, JSC::PutPropertySlot &);

   virtual bool deleteProperty(QScriptObject *, JSC::ExecState *, const JSC::Identifier &propertyName);

   virtual void getOwnPropertyNames(QScriptObject *, JSC::ExecState *, JSC::PropertyNameArray &,
      JSC::EnumerationMode mode = JSC::ExcludeDontEnumProperties);
   virtual void markChildren(QScriptObject *, JSC::MarkStack &markStack);
   virtual JSC::CallType getCallData(QScriptObject *, JSC::CallData &);
   virtual JSC::ConstructType getConstructData(QScriptObject *, JSC::ConstructData &);

   virtual bool hasInstance(QScriptObject *, JSC::ExecState *, JSC::JSValue value, JSC::JSValue proto);
   virtual bool compareToObject(QScriptObject *, JSC::ExecState *, JSC::JSObject *);
};

inline JSC::JSValue QScriptObject::data() const
{
   if (!d) {
      return JSC::JSValue();
   }

   return d->data;
}

inline void QScriptObject::setData(JSC::JSValue data)
{
   if (!d) {
      d = new Data();
   }
   d->data = data;
}

inline QScriptObjectDelegate *QScriptObject::delegate() const
{
   if (!d) {
      return nullptr;
   }
   return d->delegate;
}

inline void QScriptObject::setDelegate(QScriptObjectDelegate *delegate)
{
   if (!d) {
      d = new Data();
   } else {
      delete d->delegate;
   }
   d->delegate = delegate;
}

#endif
