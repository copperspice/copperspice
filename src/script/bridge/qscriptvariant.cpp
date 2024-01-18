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
#include "qscriptvariant_p.h"

#include "../api/qscriptengine.h"
#include "../api/qscriptengine_p.h"

#include "Error.h"
#include "PrototypeFunction.h"
#include "JSFunction.h"
#include "NativeFunctionWrapper.h"
#include "JSString.h"

namespace JSC {
ASSERT_CLASS_FITS_IN_CELL(QScript::QVariantPrototype);
}

namespace QScript {

QVariantDelegate::QVariantDelegate(const QVariant &value)
   : m_value(value)
{
}

QVariantDelegate::~QVariantDelegate()
{
}

QVariant &QVariantDelegate::value()
{
   return m_value;
}

void QVariantDelegate::setValue(const QVariant &value)
{
   m_value = value;
}

QScriptObjectDelegate::Type QVariantDelegate::type() const
{
   return Variant;
}

static JSC::JSValue JSC_HOST_CALL variantProtoFuncValueOf(JSC::ExecState *exec, JSC::JSObject *,
   JSC::JSValue thisValue, const JSC::ArgList &)
{
   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
   thisValue = engine->toUsableValue(thisValue);
   if (!thisValue.inherits(&QScriptObject::info)) {
      return throwError(exec, JSC::TypeError);
   }
   QScriptObjectDelegate *delegate = static_cast<QScriptObject *>(JSC::asObject(thisValue))->delegate();
   if (!delegate || (delegate->type() != QScriptObjectDelegate::Variant)) {
      return throwError(exec, JSC::TypeError);
   }
   const QVariant &v = static_cast<QVariantDelegate *>(delegate)->value();
   switch (v.type()) {
      case QVariant::Invalid:
         return JSC::jsUndefined();
      case QVariant::String:
         return JSC::jsString(exec, v.toString());

      case QVariant::Int:
         return JSC::jsNumber(exec, v.toInt());

      case QVariant::Bool:
         return JSC::jsBoolean(v.toBool());

      case QVariant::Double:
         return JSC::jsNumber(exec, v.toDouble());

      //    case QVariant::Char:
      //        return JSC::jsNumber(exec, v.toChar().unicode());

      case QVariant::UInt:
         return JSC::jsNumber(exec, v.toUInt());

      default:
         ;
   }
   return thisValue;
}

static JSC::JSValue JSC_HOST_CALL variantProtoFuncToString(JSC::ExecState *exec, JSC::JSObject *callee,
   JSC::JSValue thisValue, const JSC::ArgList &args)
{
   QScriptEnginePrivate *engine = scriptEngineFromExec(exec);
   thisValue = engine->toUsableValue(thisValue);
   if (!thisValue.inherits(&QScriptObject::info)) {
      return throwError(exec, JSC::TypeError, "This object is not a QVariant");
   }
   QScriptObjectDelegate *delegate = static_cast<QScriptObject *>(JSC::asObject(thisValue))->delegate();
   if (!delegate || (delegate->type() != QScriptObjectDelegate::Variant)) {
      return throwError(exec, JSC::TypeError, "This object is not a QVariant");
   }
   const QVariant &v = static_cast<QVariantDelegate *>(delegate)->value();
   JSC::UString result;
   JSC::JSValue value = variantProtoFuncValueOf(exec, callee, thisValue, args);

   if (value.isObject()) {
      result = v.toString();

      if (result.isEmpty() && !v.canConvert(QVariant::String)) {
         result = QString("QVariant(%0)").formatArg(v.typeName());
      }

   } else {
      result = value.toString(exec);
   }

   return JSC::jsString(exec, result);
}

bool QVariantDelegate::compareToObject(QScriptObject *, JSC::ExecState *exec, JSC::JSObject *o2)
{
   const QVariant &variant1 = value();
   return variant1 == QScriptEnginePrivate::toVariant(exec, o2);
}

QVariantPrototype::QVariantPrototype(JSC::ExecState *exec, WTF::PassRefPtr<JSC::Structure> structure,
   JSC::Structure *prototypeFunctionStructure)
   : QScriptObject(structure)
{
   setDelegate(new QVariantDelegate(QVariant()));

   putDirectFunction(exec, new (exec) JSC::NativeFunctionWrapper(exec, prototypeFunctionStructure, 0,
         exec->propertyNames().toString, variantProtoFuncToString), JSC::DontEnum);
   putDirectFunction(exec, new (exec) JSC::NativeFunctionWrapper(exec, prototypeFunctionStructure, 0,
         exec->propertyNames().valueOf, variantProtoFuncValueOf), JSC::DontEnum);
}


} // namespace QScript
