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

#include <qscriptdeclarativeobject_p.h>

#include <qscriptengine.h>
#include <qscriptcontext.h>
#include <qscriptclass.h>
#include <qscriptclasspropertyiterator.h>
#include <qstringlist.h>

#include <qscriptcontext_p.h>
#include <qscriptengine_p.h>

#include "Error.h"
#include "PropertyNameArray.h"

namespace QScript {

DeclarativeObjectDelegate::DeclarativeObjectDelegate(QScriptDeclarativeClass *c, QScriptDeclarativeClass::Object *o)
   : m_class(c), m_object(o)
{
}

DeclarativeObjectDelegate::~DeclarativeObjectDelegate()
{
   delete m_object;
}

QScriptObjectDelegate::Type DeclarativeObjectDelegate::type() const
{
   return DeclarativeClassObject;
}

bool DeclarativeObjectDelegate::getOwnPropertySlot(QScriptObject *object, JSC::ExecState *exec,
            const JSC::Identifier &propertyName, JSC::PropertySlot &slot)
{
   QScriptDeclarativeClass::Identifier identifier = (void *)propertyName.ustring().rep();

   QScriptDeclarativeClassPrivate *p = QScriptDeclarativeClassPrivate::get(m_class);
   p->context = reinterpret_cast<QScriptContext *>(exec);

   QScriptClass::QueryFlags flags = m_class->queryProperty(m_object, identifier, QScriptClass::HandlesReadAccess);

   if (flags & QScriptClass::HandlesReadAccess) {
      QScriptDeclarativeClass::Value val = m_class->property(m_object, identifier);
      p->context = nullptr;
      slot.setValue((const JSC::JSValue &)val);
      return true;
   }

   p->context = nullptr;

   return QScriptObjectDelegate::getOwnPropertySlot(object, exec, propertyName, slot);
}

void DeclarativeObjectDelegate::put(QScriptObject *object, JSC::ExecState *exec,
            const JSC::Identifier &propertyName, JSC::JSValue value, JSC::PutPropertySlot &slot)
{
   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
   QScript::SaveFrameHelper saveFrame(engine, exec);
   QScriptDeclarativeClass::Identifier identifier = (void *)propertyName.ustring().rep();

   QScriptDeclarativeClassPrivate *p = QScriptDeclarativeClassPrivate::get(m_class);
   p->context = reinterpret_cast<QScriptContext *>(exec);
   QScriptClass::QueryFlags flags = m_class->queryProperty(m_object, identifier, QScriptClass::HandlesWriteAccess);

   if (flags & QScriptClass::HandlesWriteAccess) {
      m_class->setProperty(m_object, identifier, engine->scriptValueFromJSCValue(value));
      p->context = nullptr;
      return;
   }

   p->context = nullptr;

   QScriptObjectDelegate::put(object, exec, propertyName, value, slot);
}

bool DeclarativeObjectDelegate::deleteProperty(QScriptObject *object, JSC::ExecState *exec,
            const JSC::Identifier &propertyName)
{
   return QScriptObjectDelegate::deleteProperty(object, exec, propertyName);
}

void DeclarativeObjectDelegate::getOwnPropertyNames(QScriptObject *object, JSC::ExecState *exec,
            JSC::PropertyNameArray &propertyNames, JSC::EnumerationMode mode)
{
   QStringList properties = m_class->propertyNames(m_object);
   for (int ii = 0; ii < properties.count(); ++ii) {
      const QString &name = properties.at(ii);
      propertyNames.add(JSC::Identifier(exec, name));
   }

   QScriptObjectDelegate::getOwnPropertyNames(object, exec, propertyNames, mode);
}

JSC::CallType DeclarativeObjectDelegate::getCallData(QScriptObject *object, JSC::CallData &callData)
{
   (void) object;

   if (! QScriptDeclarativeClassPrivate::get(m_class)->supportsCall) {
      return JSC::CallTypeNone;
   }

   callData.native.function = call;

   return JSC::CallTypeHost;
}

JSC::JSValue DeclarativeObjectDelegate::call(JSC::ExecState *exec, JSC::JSObject *callee,
            JSC::JSValue thisValue, const JSC::ArgList &args)
{
   if (!callee->inherits(&QScriptObject::info)) {
      return JSC::throwError(exec, JSC::TypeError, "callee is not a DeclarativeObject object");
   }

   QScriptObject *obj = static_cast<QScriptObject *>(callee);
   QScriptObjectDelegate *delegate = obj->delegate();

   if (!delegate || (delegate->type() != QScriptObjectDelegate::DeclarativeClassObject)) {
      return JSC::throwError(exec, JSC::TypeError, "callee is not a DeclarativeObject object");
   }

   QScriptDeclarativeClass *scriptClass = static_cast<DeclarativeObjectDelegate *>(delegate)->m_class;
   QScriptEnginePrivate *eng_p = scriptEngineFromExec(exec);

   QScript::SaveFrameHelper saveFrame(eng_p, exec);
   eng_p->pushContext(exec, thisValue, args, callee);
   QScriptContext *ctxt = eng_p->contextForFrame(eng_p->currentFrame);

   QScriptValue scriptObject = eng_p->scriptValueFromJSCValue(obj);
   QScriptDeclarativeClass::Value result =
      scriptClass->call(static_cast<DeclarativeObjectDelegate *>(delegate)->m_object, ctxt);

   eng_p->popContext();

   return (JSC::JSValue &)(result);
}

JSC::ConstructType DeclarativeObjectDelegate::getConstructData(QScriptObject *object, JSC::ConstructData &constructData)
{
   return QScriptObjectDelegate::getConstructData(object, constructData);
}

bool DeclarativeObjectDelegate::hasInstance(QScriptObject *object, JSC::ExecState *exec,
   JSC::JSValue value, JSC::JSValue proto)
{
   return QScriptObjectDelegate::hasInstance(object, exec, value, proto);
}

bool DeclarativeObjectDelegate::compareToObject(QScriptObject *o, JSC::ExecState *exec, JSC::JSObject *o2)
{
   (void) o;
   (void) exec;

   if (! o2->inherits(&QScriptObject::info)) {
      return false;
   }

   QScriptObject *scriptObject = static_cast<QScriptObject *>(o2);
   QScriptObjectDelegate *delegate = scriptObject->delegate();
   if (!delegate || (delegate->type() != QScriptObjectDelegate::DeclarativeClassObject)) {
      return false;
   }

   DeclarativeObjectDelegate *other = static_cast<DeclarativeObjectDelegate *>(delegate);
   if (m_class != other->m_class) {
      return false;
   } else {
      return m_class->compare(m_object, other->m_object);
   }
}

} // namespace QScript


