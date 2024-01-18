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

#ifndef QSCRIPTCLASSOBJECT_P_H
#define QSCRIPTCLASSOBJECT_P_H

#include "qscriptobject_p.h"

class QScriptClass;

namespace QScript {

class ClassObjectDelegate : public QScriptObjectDelegate
{
 public:
   ClassObjectDelegate(QScriptClass *scriptClass);
   ~ClassObjectDelegate();

   inline QScriptClass *scriptClass() const;
   inline void setScriptClass(QScriptClass *scriptClass);

   virtual Type type() const;

   virtual bool getOwnPropertySlot(QScriptObject *, JSC::ExecState *,
      const JSC::Identifier &propertyName,
      JSC::PropertySlot &);
   virtual bool getOwnPropertyDescriptor(QScriptObject *, JSC::ExecState *,
      const JSC::Identifier &propertyName,
      JSC::PropertyDescriptor &);
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
   static JSC::JSObject *construct(JSC::ExecState *, JSC::JSObject *,
      const JSC::ArgList &);

   virtual bool hasInstance(QScriptObject *, JSC::ExecState *,
      JSC::JSValue value, JSC::JSValue proto);

 private:
   QScriptClass *m_scriptClass;
};

inline QScriptClass *ClassObjectDelegate::scriptClass() const
{
   return m_scriptClass;
}

inline void ClassObjectDelegate::setScriptClass(QScriptClass *scriptClass)
{
   Q_ASSERT(scriptClass != nullptr);
   m_scriptClass = scriptClass;
}

} // namespace QScript

#endif
