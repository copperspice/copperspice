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

#ifndef QSCRIPTDECLARATIVEOBJECT_P_H
#define QSCRIPTDECLARATIVEOBJECT_P_H

#include "config.h"
#include "qscriptobject_p.h"
#include "qscriptdeclarativeclass_p.h"

class QScriptClass;

class QScriptDeclarativeClassPrivate
{
 public:
   QScriptDeclarativeClassPrivate()
      : engine(nullptr), q_ptr(nullptr), context(nullptr), supportsCall(false)
   {
   }

   QScriptEngine *engine;
   QScriptDeclarativeClass *q_ptr;
   QScriptContext *context;
   bool supportsCall: 1;

   static QScriptDeclarativeClassPrivate *get(QScriptDeclarativeClass *c) {
      return c->d_ptr.data();
   }
};

namespace QScript {

class DeclarativeObjectDelegate : public QScriptObjectDelegate
{
 public:
   DeclarativeObjectDelegate(QScriptDeclarativeClass *c, QScriptDeclarativeClass::Object *o);
   ~DeclarativeObjectDelegate();

   virtual Type type() const;

   QScriptDeclarativeClass *scriptClass() const {
      return m_class;
   }

   QScriptDeclarativeClass::Object *object() const {
      return m_object;
   }

   virtual bool getOwnPropertySlot(QScriptObject *, JSC::ExecState *,
      const JSC::Identifier &propertyName,
      JSC::PropertySlot &);
   virtual void put(QScriptObject *, JSC::ExecState *exec,
      const JSC::Identifier &propertyName,
      JSC::JSValue, JSC::PutPropertySlot &);
   virtual bool deleteProperty(QScriptObject *, JSC::ExecState *,
      const JSC::Identifier &propertyName);
   virtual void getOwnPropertyNames(QScriptObject *, JSC::ExecState *,
      JSC::PropertyNameArray &,
      JSC::EnumerationMode mode = JSC::ExcludeDontEnumProperties);

   virtual JSC::CallType getCallData(QScriptObject *, JSC::CallData &);
   static JSC::JSValue JSC_HOST_CALL call(JSC::ExecState *, JSC::JSObject *,
      JSC::JSValue, const JSC::ArgList &);

   virtual JSC::ConstructType getConstructData(QScriptObject *, JSC::ConstructData &);

   virtual bool hasInstance(QScriptObject *, JSC::ExecState *,
      JSC::JSValue value, JSC::JSValue proto);

   bool compareToObject(QScriptObject *, JSC::ExecState *, JSC::JSObject *);

 private:
   QScriptDeclarativeClass *m_class;
   QScriptDeclarativeClass::Object *m_object;
};

} // namespace QScript


#endif
