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

#include <qscriptvalue.h>

#include <qvariant.h>
#include <qvarlengtharray.h>
#include <qnumeric.h>
#include <qscriptengine.h>

#include <qscriptvalue_p.h>
#include <qscriptengine_p.h>
#include <qscriptstring_p.h>

#include "JSGlobalObject.h"
#include "JSImmediate.h"
#include "JSObject.h"
#include "JSValue.h"
#include "JSFunction.h"
#include "Identifier.h"
#include "Operations.h"
#include "Arguments.h"

void QScriptValuePrivate::detachFromEngine()
{
   if (isJSC()) {
      jscValue = JSC::JSValue();
   }

   engine = nullptr;
}

// internal
QScriptValue::QScriptValue(QScriptValuePrivate *d)
   : d_ptr(d)
{
}

QScriptValue::QScriptValue()
   : d_ptr(nullptr)
{
}

QScriptValue::~QScriptValue()
{
}

QScriptValue::QScriptValue(const QScriptValue &other)
   : d_ptr(other.d_ptr)
{
}

// obsolete
QScriptValue::QScriptValue(QScriptEngine *engine, QScriptValue::SpecialValue value)
   : d_ptr(new (QScriptEnginePrivate::get(engine)) QScriptValuePrivate(QScriptEnginePrivate::get(engine)))
{
   switch (value) {
      case NullValue:
         d_ptr->initFrom(JSC::jsNull());
         break;

      case UndefinedValue:
         d_ptr->initFrom(JSC::jsUndefined());
         break;
   }
}

// obsolete, remove code (emerald)
QScriptValue::QScriptValue(QScriptEngine *engine, bool val)
   : d_ptr(new (QScriptEnginePrivate::get(engine)) QScriptValuePrivate(QScriptEnginePrivate::get(engine)))
{
   d_ptr->initFrom(JSC::jsBoolean(val));
}

QScriptValue::QScriptValue(QScriptEngine *engine, int val)
   : d_ptr(new (QScriptEnginePrivate::get(engine)) QScriptValuePrivate(QScriptEnginePrivate::get(engine)))
{
   if (engine) {
      QScript::APIShim shim(d_ptr->engine);
      JSC::ExecState *exec = d_ptr->engine->currentFrame;
      d_ptr->initFrom(JSC::jsNumber(exec, val));
   } else {
      d_ptr->initFrom(val);
   }
}

QScriptValue::QScriptValue(QScriptEngine *engine, uint val)
   : d_ptr(new (QScriptEnginePrivate::get(engine)) QScriptValuePrivate(QScriptEnginePrivate::get(engine)))
{
   if (engine) {
      QScript::APIShim shim(d_ptr->engine);
      JSC::ExecState *exec = d_ptr->engine->currentFrame;
      d_ptr->initFrom(JSC::jsNumber(exec, val));
   } else {
      d_ptr->initFrom(val);
   }
}

QScriptValue::QScriptValue(QScriptEngine *engine, qsreal val)
   : d_ptr(new (QScriptEnginePrivate::get(engine)) QScriptValuePrivate(QScriptEnginePrivate::get(engine)))
{
   if (engine) {
      QScript::APIShim shim(d_ptr->engine);
      JSC::ExecState *exec = d_ptr->engine->currentFrame;
      d_ptr->initFrom(JSC::jsNumber(exec, val));
   } else {
      d_ptr->initFrom(val);
   }
}

QScriptValue::QScriptValue(QScriptEngine *engine, const QString &val)
   : d_ptr(new (QScriptEnginePrivate::get(engine)) QScriptValuePrivate(QScriptEnginePrivate::get(engine)))
{
   if (engine) {
      QScript::APIShim shim(d_ptr->engine);
      JSC::ExecState *exec = d_ptr->engine->currentFrame;
      d_ptr->initFrom(JSC::jsString(exec, val));

   } else {
      d_ptr->initFrom(val);
   }
}

QScriptValue::QScriptValue(SpecialValue value)
   : d_ptr(new (nullptr) QScriptValuePrivate(nullptr))
{
   switch (value) {
      case NullValue:
         d_ptr->initFrom(JSC::jsNull());
         break;

      case UndefinedValue:
         d_ptr->initFrom(JSC::jsUndefined());
         break;
   }
}

QScriptValue::QScriptValue(bool value)
   : d_ptr(new (nullptr) QScriptValuePrivate(nullptr))
{
   d_ptr->initFrom(JSC::jsBoolean(value));
}

QScriptValue::QScriptValue(int value)
   : d_ptr(new (nullptr) QScriptValuePrivate(nullptr))
{
   d_ptr->initFrom(value);
}

QScriptValue::QScriptValue(uint value)
   : d_ptr(new (nullptr) QScriptValuePrivate(nullptr))
{
   d_ptr->initFrom(value);
}

QScriptValue::QScriptValue(qsreal value)
   : d_ptr(new (nullptr) QScriptValuePrivate(nullptr))
{
   d_ptr->initFrom(value);
}

QScriptValue::QScriptValue(const QString &value)
   : d_ptr(new (nullptr) QScriptValuePrivate(nullptr))
{
   d_ptr->initFrom(value);
}

QScriptValue &QScriptValue::operator=(const QScriptValue &other)
{
   d_ptr = other.d_ptr;
   return *this;
}

bool QScriptValue::isError() const
{
   Q_D(const QScriptValue);
   if (! d || ! d->isJSC()) {
      return false;
   }

   return QScriptEnginePrivate::isError(d->jscValue);
}

bool QScriptValue::isArray() const
{
   Q_D(const QScriptValue);
   if (! d || ! d->isJSC()) {
      return false;
   }
   return QScriptEnginePrivate::isArray(d->jscValue);
}

bool QScriptValue::isDate() const
{
   Q_D(const QScriptValue);

   if (! d || ! d->isJSC()) {
      return false;
   }

   return QScriptEnginePrivate::isDate(d->jscValue);
}

bool QScriptValue::isRegExp() const
{
   Q_D(const QScriptValue);
   if (!d || !d->isJSC()) {
      return false;
   }

   return QScriptEnginePrivate::isRegExp(d->jscValue);
}

QScriptValue QScriptValue::prototype() const
{
   Q_D(const QScriptValue);
   if (! d || ! d->isObject()) {
      return QScriptValue();
   }

   return d->engine->scriptValueFromJSCValue(JSC::asObject(d->jscValue)->prototype());
}

void QScriptValue::setPrototype(const QScriptValue &prototype)
{
   Q_D(QScriptValue);
   if (! d || ! d->isObject()) {
      return;
   }

   JSC::JSValue other = d->engine->scriptValueToJSCValue(prototype);
   if (! other || !(other.isObject() || other.isNull())) {
      return;
   }

   if (QScriptValuePrivate::getEngine(prototype)
      && (QScriptValuePrivate::getEngine(prototype) != d->engine)) {
      qWarning("QScriptValue::setPrototype() failed: can not set a prototype created in a different engine");
      return;
   }
   JSC::JSObject *thisObject = JSC::asObject(d->jscValue);

   // check for cycle
   JSC::JSValue nextPrototypeValue = other;
   while (nextPrototypeValue && nextPrototypeValue.isObject()) {
      JSC::JSObject *nextPrototype = JSC::asObject(nextPrototypeValue);
      if (nextPrototype == thisObject) {
         qWarning("QScriptValue::setPrototype() failed: cyclic prototype value");
         return;
      }
      nextPrototypeValue = nextPrototype->prototype();
   }

   thisObject->setPrototype(other);

   // Sync the internal Global Object prototype if appropriate.
   if (((thisObject == d->engine->originalGlobalObjectProxy)
         && !d->engine->customGlobalObject()) || (thisObject == d->engine->customGlobalObject())) {
      d->engine->originalGlobalObject()->setPrototype(other);
   }
}

// internal
QScriptValue QScriptValue::scope() const
{
   Q_D(const QScriptValue);
   if (!d || !d->isObject()) {
      return QScriptValue();
   }

   QScript::APIShim shim(d->engine);

   // ### make hidden property
   JSC::JSValue result = d->property("__qt_scope__", QScriptValue::ResolveLocal);
   return d->engine->scriptValueFromJSCValue(result);
}

// internal
void QScriptValue::setScope(const QScriptValue &scope)
{
   Q_D(QScriptValue);
   if (!d || !d->isObject()) {
      return;
   }
   if (scope.isValid() && QScriptValuePrivate::getEngine(scope)
      && (QScriptValuePrivate::getEngine(scope) != d->engine)) {
      qWarning("QScriptValue::setScope() failed: can not set a scope object created in a different engine");
      return;
   }

   JSC::JSValue other = d->engine->scriptValueToJSCValue(scope);
   JSC::ExecState *exec = d->engine->currentFrame;
   JSC::Identifier id = JSC::Identifier(exec, "__qt_scope__");

   if (!scope.isValid()) {
      JSC::asObject(d->jscValue)->removeDirect(id);
   } else {
      // ### make hidden property
      JSC::asObject(d->jscValue)->putDirect(id, other);
   }
}

bool QScriptValue::instanceOf(const QScriptValue &other) const
{
   Q_D(const QScriptValue);
   if (!d || !d->isObject() || !other.isObject()) {
      return false;
   }

   if (QScriptValuePrivate::getEngine(other) != d->engine) {
      qWarning("QScriptValue::instanceof: can not perform operation on a value created in a different engine");
      return false;
   }

   JSC::JSValue jscProto = d->engine->scriptValueToJSCValue(other.property(QLatin1String("prototype")));
   if (!jscProto) {
      jscProto = JSC::jsUndefined();
   }

   JSC::ExecState *exec = d->engine->currentFrame;
   JSC::JSValue jscOther = d->engine->scriptValueToJSCValue(other);
   return JSC::asObject(jscOther)->hasInstance(exec, d->jscValue, jscProto);
}

// ### move

namespace QScript {

enum Type {
   Undefined,
   Null,
   Boolean,
   String,
   Number,
   Object
};

static Type type(const QScriptValue &v)
{
   if (v.isUndefined()) {
      return Undefined;

   } else if (v.isNull()) {
      return Null;

   } else if (v.isBoolean()) {
      return Boolean;

   } else if (v.isString()) {
      return String;

   } else if (v.isNumber()) {
      return Number;
   }

   Q_ASSERT(v.isObject());

   return Object;
}

static QScriptValue ToPrimitive(const QScriptValue &object, JSC::PreferredPrimitiveType hint = JSC::NoPreference)
{
   Q_ASSERT(object.isObject());
   QScriptValuePrivate *pp = QScriptValuePrivate::get(object);

   Q_ASSERT(pp->engine != nullptr);

   QScript::APIShim shim(pp->engine);
   JSC::ExecState *exec = pp->engine->currentFrame;
   JSC::JSValue savedException;

   QScriptEnginePrivate::saveException(exec, &savedException);
   JSC::JSValue result = JSC::asObject(pp->jscValue)->toPrimitive(exec, hint);

   QScriptEnginePrivate::restoreException(exec, savedException);

   return pp->engine->scriptValueFromJSCValue(result);
}

static bool IsNumerical(const QScriptValue &value)
{
   return value.isNumber() || value.isBool();
}

static bool LessThan(QScriptValue lhs, QScriptValue rhs)
{
   if (type(lhs) == type(rhs)) {
      switch (type(lhs)) {
         case Undefined:
         case Null:
            return false;

         case Number:
            return lhs.toNumber() < rhs.toNumber();

         case Boolean:
            return lhs.toBool() < rhs.toBool();

         case String:
            return lhs.toString() < rhs.toString();

         case Object:
            break;
      } // switch
   }

   if (lhs.isObject()) {
      lhs = ToPrimitive(lhs, JSC::PreferNumber);
   }

   if (rhs.isObject()) {
      rhs = ToPrimitive(rhs, JSC::PreferNumber);
   }

   if (lhs.isString() && rhs.isString()) {
      return lhs.toString() < rhs.toString();
   }

   return lhs.toNumber() < rhs.toNumber();
}

static bool Equals(QScriptValue lhs, QScriptValue rhs)
{
   if (type(lhs) == type(rhs)) {
      switch (type(lhs)) {
         case QScript::Undefined:
         case QScript::Null:
            return true;

         case QScript::Number:
            return lhs.toNumber() == rhs.toNumber();

         case QScript::Boolean:
            return lhs.toBool() == rhs.toBool();

         case QScript::String:
            return lhs.toString() == rhs.toString();

         case QScript::Object:
            if (lhs.isVariant()) {
               return lhs.strictlyEquals(rhs) || (lhs.toVariant() == rhs.toVariant());
            }

            else if (lhs.isQObject()) {
               return (lhs.strictlyEquals(rhs)) || (lhs.toQObject() == rhs.toQObject());
            }

            else {
               return lhs.strictlyEquals(rhs);
            }
      }
   }

   if (lhs.isNull() && rhs.isUndefined()) {
      return true;

   } else if (lhs.isUndefined() && rhs.isNull()) {
      return true;

   } else if (IsNumerical(lhs) && rhs.isString()) {
      return lhs.toNumber() == rhs.toNumber();

   } else if (lhs.isString() && IsNumerical(rhs)) {
      return lhs.toNumber() == rhs.toNumber();

   } else if (lhs.isBool()) {
      return Equals(lhs.toNumber(), rhs);

   }  else if (rhs.isBool()) {
      return Equals(lhs, rhs.toNumber());

   } else if (lhs.isObject() && !rhs.isNull()) {
      lhs = ToPrimitive(lhs);

      if (lhs.isValid() && !lhs.isObject()) {
         return Equals(lhs, rhs);
      }

   } else if (rhs.isObject() && ! lhs.isNull()) {
      rhs = ToPrimitive(rhs);
      if (rhs.isValid() && !rhs.isObject()) {
         return Equals(lhs, rhs);
      }
   }

   return false;
}

} // namespace QScript

bool QScriptValue::lessThan(const QScriptValue &other) const
{
   Q_D(const QScriptValue);

   // no equivalent function in JSC? There's a jsLess() in VM/Machine.cpp
   if (! isValid() || ! other.isValid()) {
      return false;
   }

   if (QScriptValuePrivate::getEngine(other) && d->engine
         && (QScriptValuePrivate::getEngine(other) != d->engine)) {
      qWarning("QScriptValue::lessThan: can not compare to a value created in a different engine");
      return false;
   }

   return QScript::LessThan(*this, other);
}

bool QScriptValue::equals(const QScriptValue &other) const
{
   Q_D(const QScriptValue);

   if (! d || ! other.d_ptr) {
      return (d_ptr == other.d_ptr);
   }

   if (QScriptValuePrivate::getEngine(other) && d->engine
         && (QScriptValuePrivate::getEngine(other) != d->engine)) {
      qWarning("QScriptValue::equals: can not compare to a value created in a different engine");
      return false;
   }

   if (d->isJSC() && other.d_ptr->isJSC()) {
      QScriptEnginePrivate *eng_p = d->engine;
      if (!eng_p) {
         eng_p = other.d_ptr->engine;
      }

      if (eng_p) {
         QScript::APIShim shim(eng_p);
         JSC::ExecState *exec = eng_p->currentFrame;
         JSC::JSValue savedException;
         QScriptEnginePrivate::saveException(exec, &savedException);
         bool result = JSC::JSValue::equal(exec, d->jscValue, other.d_ptr->jscValue);
         QScriptEnginePrivate::restoreException(exec, savedException);
         return result;
      }
   }
   return QScript::Equals(*this, other);
}

bool QScriptValue::strictlyEquals(const QScriptValue &other) const
{
   Q_D(const QScriptValue);
   if (! d || ! other.d_ptr) {
      return (d_ptr == other.d_ptr);
   }

   if (QScriptValuePrivate::getEngine(other) && d->engine
         && (QScriptValuePrivate::getEngine(other) != d->engine)) {
      qWarning("QScriptValue::strictlyEquals: can not compare to a value created in a different engine");
      return false;
   }

   if (d->type != other.d_ptr->type) {
      if (d->type == QScriptValuePrivate::JavaScriptCore) {
         QScriptEnginePrivate *eng_p = d->engine ? d->engine : other.d_ptr->engine;
         if (eng_p) {
            return JSC::JSValue::strictEqual(eng_p->currentFrame, d->jscValue, eng_p->scriptValueToJSCValue(other));
         }

      } else if (other.d_ptr->type == QScriptValuePrivate::JavaScriptCore) {
         QScriptEnginePrivate *eng_p = other.d_ptr->engine ? other.d_ptr->engine : d->engine;
         if (eng_p) {
            return JSC::JSValue::strictEqual(eng_p->currentFrame, eng_p->scriptValueToJSCValue(*this), other.d_ptr->jscValue);
         }
      }

      return false;
   }

   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore: {
         QScriptEnginePrivate *eng_p = d->engine ? d->engine : other.d_ptr->engine;
         JSC::ExecState *exec = eng_p ? eng_p->currentFrame : nullptr;
         return JSC::JSValue::strictEqual(exec, d->jscValue, other.d_ptr->jscValue);
      }

      case QScriptValuePrivate::Number:
         return (d->numberValue == other.d_ptr->numberValue);

      case QScriptValuePrivate::String:
         return (d->stringValue == other.d_ptr->stringValue);
   }

   return false;
}

QString QScriptValue::toString() const
{
   Q_D(const QScriptValue);
   if (! d) {
      return QString();
   }

   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore: {
         if (d->engine) {
            QScript::APIShim shim(d->engine);
            return QScriptEnginePrivate::toString(d->engine->currentFrame, d->jscValue);

         } else {
            return QScriptEnginePrivate::toString(nullptr, d->jscValue);
         }
      }

      case QScriptValuePrivate::Number:
         return QScript::ToString(d->numberValue);

      case QScriptValuePrivate::String:
         return d->stringValue;
   }

   return QString();
}

qsreal QScriptValue::toNumber() const
{
   Q_D(const QScriptValue);
   if (! d) {
      return 0;
   }

   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore: {
         if (d->engine) {
            QScript::APIShim shim(d->engine);
            return QScriptEnginePrivate::toNumber(d->engine->currentFrame, d->jscValue);

         } else {
            return QScriptEnginePrivate::toNumber(nullptr, d->jscValue);
         }
      }

      case QScriptValuePrivate::Number:
         return d->numberValue;

      case QScriptValuePrivate::String:
         return QScript::ToNumber(d->stringValue);
   }

   return 0;
}

// obsolete
bool QScriptValue::toBoolean() const
{
   Q_D(const QScriptValue);

   if (!d) {
      return false;
   }

   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore: {
         if (d->engine) {
            QScript::APIShim shim(d->engine);
            return QScriptEnginePrivate::toBool(d->engine->currentFrame, d->jscValue);

         } else {
            return QScriptEnginePrivate::toBool(nullptr, d->jscValue);
         }
      }

      case QScriptValuePrivate::Number:
         return QScript::ToBool(d->numberValue);

      case QScriptValuePrivate::String:
         return QScript::ToBool(d->stringValue);
   }

   return false;
}

bool QScriptValue::toBool() const
{
   Q_D(const QScriptValue);
   if (!d) {
      return false;
   }
   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore: {
         if (d->engine) {
            QScript::APIShim shim(d->engine);
            return QScriptEnginePrivate::toBool(d->engine->currentFrame, d->jscValue);
         } else {
            return QScriptEnginePrivate::toBool(nullptr, d->jscValue);
         }
      }

      case QScriptValuePrivate::Number:
         return QScript::ToBool(d->numberValue);

      case QScriptValuePrivate::String:
         return QScript::ToBool(d->stringValue);
   }
   return false;
}

qint32 QScriptValue::toInt32() const
{
   Q_D(const QScriptValue);
   if (!d) {
      return 0;
   }

   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore: {
         if (d->engine) {
            QScript::APIShim shim(d->engine);
            return QScriptEnginePrivate::toInt32(d->engine->currentFrame, d->jscValue);

         } else {
            return QScriptEnginePrivate::toInt32(nullptr, d->jscValue);
         }
      }

      case QScriptValuePrivate::Number:
         return QScript::ToInt32(d->numberValue);

      case QScriptValuePrivate::String:
         return QScript::ToInt32(d->stringValue);
   }

   return 0;
}

quint32 QScriptValue::toUInt32() const
{
   Q_D(const QScriptValue);
   if (! d) {
      return 0;
   }

   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore: {
         if (d->engine) {
            QScript::APIShim shim(d->engine);
            return QScriptEnginePrivate::toUInt32(d->engine->currentFrame, d->jscValue);
         } else {
            return QScriptEnginePrivate::toUInt32(nullptr, d->jscValue);
         }
      }

      case QScriptValuePrivate::Number:
         return QScript::ToUInt32(d->numberValue);

      case QScriptValuePrivate::String:
         return QScript::ToUInt32(d->stringValue);
   }

   return 0;
}

quint16 QScriptValue::toUInt16() const
{
   Q_D(const QScriptValue);
   if (! d) {
      return 0;
   }

   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore: {
         if (d->engine) {
            QScript::APIShim shim(d->engine);
            return QScriptEnginePrivate::toUInt16(d->engine->currentFrame, d->jscValue);
         } else {
            return QScriptEnginePrivate::toUInt16(nullptr, d->jscValue);
         }
      }

      case QScriptValuePrivate::Number:
         return QScript::ToUInt16(d->numberValue);

      case QScriptValuePrivate::String:
         return QScript::ToUInt16(d->stringValue);
   }
   return 0;
}

qsreal QScriptValue::toInteger() const
{
   Q_D(const QScriptValue);
   if (! d) {
      return 0;
   }

   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore: {
         if (d->engine) {
            QScript::APIShim shim(d->engine);
            return QScriptEnginePrivate::toInteger(d->engine->currentFrame, d->jscValue);
         } else {
            return QScriptEnginePrivate::toInteger(nullptr, d->jscValue);
         }
      }

      case QScriptValuePrivate::Number:
         return QScript::ToInteger(d->numberValue);

      case QScriptValuePrivate::String:
         return QScript::ToInteger(d->stringValue);
   }

   return 0;
}

QVariant QScriptValue::toVariant() const
{
   Q_D(const QScriptValue);
   if (! d) {
      return QVariant();
   }

   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore: {
         if (d->engine) {
            QScript::APIShim shim(d->engine);
            return QScriptEnginePrivate::toVariant(d->engine->currentFrame, d->jscValue);
         } else {
            return QScriptEnginePrivate::toVariant(nullptr, d->jscValue);
         }
      }

      case QScriptValuePrivate::Number:
         return QVariant(d->numberValue);

      case QScriptValuePrivate::String:
         return QVariant(d->stringValue);
   }

   return QVariant();
}

// obsolete
QScriptValue QScriptValue::toObject() const
{
   Q_D(const QScriptValue);
   if (! d || ! d->engine) {
      return QScriptValue();
   }
   return engine()->toObject(*this);
}

QDateTime QScriptValue::toDateTime() const
{
   Q_D(const QScriptValue);
   if (! d || ! d->engine) {
      return QDateTime();
   }

   QScript::APIShim shim(d->engine);

   return QScriptEnginePrivate::toDateTime(d->engine->currentFrame, d->jscValue);
}

QRegularExpression QScriptValue::toRegExp() const
{
   Q_D(const QScriptValue);

   if (! d || ! d->engine) {
      return QRegularExpression();
   }

   QScript::APIShim shim(d->engine);

   return QScriptEnginePrivate::toRegExp(d->engine->currentFrame, d->jscValue);
}

QObject *QScriptValue::toQObject() const
{
   Q_D(const QScriptValue);
   if (! d || ! d->engine) {
      return nullptr;
   }

   QScript::APIShim shim(d->engine);
   return QScriptEnginePrivate::toQObject(d->engine->currentFrame, d->jscValue);
}

const QMetaObject *QScriptValue::toQMetaObject() const
{
   Q_D(const QScriptValue);
   if (! d || ! d->engine) {
      return nullptr;
   }

   QScript::APIShim shim(d->engine);

   return QScriptEnginePrivate::toQMetaObject(d->engine->currentFrame, d->jscValue);
}

void QScriptValue::setProperty(const QString &name, const QScriptValue &value,
   const PropertyFlags &flags)
{
   Q_D(QScriptValue);
   if (! d || ! d->isObject()) {
      return;
   }

   QScript::APIShim shim(d->engine);
   QScriptEnginePrivate *valueEngine = QScriptValuePrivate::getEngine(value);

   if (valueEngine && (valueEngine != d->engine)) {
      qWarning("QScriptValue::setProperty(%s) failed: can not set value created in a different engine", csPrintable(name));
      return;
   }

   JSC::JSValue jsValue = d->engine->scriptValueToJSCValue(value);
   d->setProperty(name, jsValue, flags);
}

QScriptValue QScriptValue::property(const QString &name,
   const ResolveFlags &mode) const
{
   Q_D(const QScriptValue);
   if (! d || ! d->isObject()) {
      return QScriptValue();
   }

   QScript::APIShim shim(d->engine);
   return d->engine->scriptValueFromJSCValue(d->property(name, mode));
}

QScriptValue QScriptValue::property(quint32 arrayIndex,
   const ResolveFlags &mode) const
{
   Q_D(const QScriptValue);
   if (! d || ! d->isObject()) {
      return QScriptValue();
   }

   QScript::APIShim shim(d->engine);

   return d->engine->scriptValueFromJSCValue(d->property(arrayIndex, mode));
}

void QScriptValue::setProperty(quint32 arrayIndex, const QScriptValue &value,
   const PropertyFlags &flags)
{
   Q_D(QScriptValue);
   if (! d || !d->isObject()) {
      return;
   }

   if (QScriptValuePrivate::getEngine(value)
         && (QScriptValuePrivate::getEngine(value) != d->engine)) {
      qWarning("QScriptValue::setProperty() failed: can not set value created in a different engine");
      return;
   }

   QScript::APIShim shim(d->engine);
   JSC::JSValue jsValue = d->engine->scriptValueToJSCValue(value);
   d->setProperty(arrayIndex, jsValue, flags);
}

QScriptValue QScriptValue::property(const QScriptString &name,
   const ResolveFlags &mode) const
{
   Q_D(const QScriptValue);
   if (! d || ! d->isObject() || !QScriptStringPrivate::isValid(name)) {
      return QScriptValue();
   }

   QScript::APIShim shim(d->engine);
   return d->engine->scriptValueFromJSCValue(d->property(name.d_ptr->identifier, mode));
}

void QScriptValue::setProperty(const QScriptString &name, const QScriptValue &value, const PropertyFlags &flags)
{
   Q_D(QScriptValue);
   if (! d || ! d->isObject() || !QScriptStringPrivate::isValid(name)) {
      return;
   }

   QScriptEnginePrivate *valueEngine = QScriptValuePrivate::getEngine(value);
   if (valueEngine && (valueEngine != d->engine)) {
      qWarning("QScriptValue::setProperty(%s) failed: can not set value created in a different engine", csPrintable(name.toString()));
      return;
   }

   QScript::APIShim shim(d->engine);
   JSC::JSValue jsValue = d->engine->scriptValueToJSCValue(value);
   d->setProperty(name.d_ptr->identifier, jsValue, flags);
}

QScriptValue::PropertyFlags QScriptValue::propertyFlags(const QString &name, const ResolveFlags &mode) const
{
   Q_D(const QScriptValue);
   if (! d || ! d->isObject()) {
      return Qt::EmptyFlag;
   }

   QScript::APIShim shim(d->engine);
   JSC::ExecState *exec = d->engine->currentFrame;

   return d->propertyFlags(JSC::Identifier(exec, name), mode);
}

QScriptValue::PropertyFlags QScriptValue::propertyFlags(const QScriptString &name, const ResolveFlags &mode) const
{
   Q_D(const QScriptValue);
   if (! d || ! d->isObject() || ! QScriptStringPrivate::isValid(name)) {
      return Qt::EmptyFlag;
   }

   return d->propertyFlags(name.d_ptr->identifier, mode);
}

QScriptValue QScriptValue::call(const QScriptValue &thisObject, const QList<QScriptValue> &args)
{
   Q_D(const QScriptValue);

   if (! d || ! d->isObject()) {
      return QScriptValue();
   }

   QScript::APIShim shim(d->engine);
   JSC::JSValue callee = d->jscValue;
   JSC::CallData callData;
   JSC::CallType callType = callee.getCallData(callData);

   if (callType == JSC::CallTypeNone) {
      return QScriptValue();
   }

   if (QScriptValuePrivate::getEngine(thisObject)
      && (QScriptValuePrivate::getEngine(thisObject) != d->engine)) {
      qWarning("QScriptValue::call() failed: can not call function with thisObject created in a different engine");
      return QScriptValue();
   }

   JSC::ExecState *exec = d->engine->currentFrame;

   JSC::JSValue jscThisObject = d->engine->scriptValueToJSCValue(thisObject);
   if (!jscThisObject || !jscThisObject.isObject()) {
      jscThisObject = d->engine->globalObject();
   }

   QVarLengthArray<JSC::JSValue, 8> argsVector(args.size());

   for (int i = 0; i < args.size(); ++i) {
      const QScriptValue &arg = args.at(i);

      if (!arg.isValid()) {
         argsVector[i] = JSC::jsUndefined();

      } else if (QScriptValuePrivate::getEngine(arg)
            && (QScriptValuePrivate::getEngine(arg) != d->engine)) {
         qWarning("QScriptValue::call() failed: can not call function with argument created in a different engine");
         return QScriptValue();

      } else {
         argsVector[i] = d->engine->scriptValueToJSCValue(arg);
      }
   }
   JSC::ArgList jscArgs(argsVector.data(), argsVector.size());

   JSC::JSValue savedException;
   QScriptEnginePrivate::saveException(exec, &savedException);
   JSC::JSValue result = JSC::call(exec, callee, callType, callData, jscThisObject, jscArgs);
   if (exec->hadException()) {
      result = exec->exception();
   } else {
      QScriptEnginePrivate::restoreException(exec, savedException);
   }
   return d->engine->scriptValueFromJSCValue(result);
}

QScriptValue QScriptValue::call(const QScriptValue &thisObject, const QScriptValue &arguments)
{
   Q_D(QScriptValue);

   if (!d || !d->isObject()) {
      return QScriptValue();
   }
   QScript::APIShim shim(d->engine);
   JSC::JSValue callee = d->jscValue;
   JSC::CallData callData;
   JSC::CallType callType = callee.getCallData(callData);
   if (callType == JSC::CallTypeNone) {
      return QScriptValue();
   }

   if (QScriptValuePrivate::getEngine(thisObject)
      && (QScriptValuePrivate::getEngine(thisObject) != d->engine)) {
      qWarning("QScriptValue::call() failed: can not call function with thisObject created in a different engine");
      return QScriptValue();
   }

   JSC::ExecState *exec = d->engine->currentFrame;

   JSC::JSValue jscThisObject = d->engine->scriptValueToJSCValue(thisObject);
   if (!jscThisObject || !jscThisObject.isObject()) {
      jscThisObject = d->engine->globalObject();
   }

   JSC::JSValue array = d->engine->scriptValueToJSCValue(arguments);
   // copied from runtime/FunctionPrototype.cpp, functionProtoFuncApply()
   JSC::MarkedArgumentBuffer applyArgs;
   if (!array.isUndefinedOrNull()) {
      if (!array.isObject()) {
         return d->engine->scriptValueFromJSCValue(JSC::throwError(exec, JSC::TypeError, "Arguments must be an array"));
      }
      if (JSC::asObject(array)->classInfo() == &JSC::Arguments::info) {
         JSC::asArguments(array)->fillArgList(exec, applyArgs);
      } else if (JSC::isJSArray(&exec->globalData(), array)) {
         JSC::asArray(array)->fillArgList(exec, applyArgs);
      } else if (JSC::asObject(array)->inherits(&JSC::JSArray::info)) {
         unsigned length = JSC::asArray(array)->get(exec, exec->propertyNames().length).toUInt32(exec);
         for (unsigned i = 0; i < length; ++i) {
            applyArgs.append(JSC::asArray(array)->get(exec, i));
         }
      } else {
         return d->engine->scriptValueFromJSCValue(JSC::throwError(exec, JSC::TypeError, "Arguments must be an array"));
      }
   }

   JSC::JSValue savedException;
   QScriptEnginePrivate::saveException(exec, &savedException);
   JSC::JSValue result = JSC::call(exec, callee, callType, callData, jscThisObject, applyArgs);

   if (exec->hadException()) {
      result = exec->exception();
   } else {
      QScriptEnginePrivate::restoreException(exec, savedException);
   }
   return d->engine->scriptValueFromJSCValue(result);
}

QScriptValue QScriptValue::construct(const QList<QScriptValue> &args)
{
   Q_D(const QScriptValue);

   if (! d || ! d->isObject()) {
      return QScriptValue();
   }

   QScript::APIShim shim(d->engine);
   JSC::JSValue callee = d->jscValue;
   JSC::ConstructData constructData;
   JSC::ConstructType constructType = callee.getConstructData(constructData);

   if (constructType == JSC::ConstructTypeNone) {
      return QScriptValue();
   }

   JSC::ExecState *exec = d->engine->currentFrame;

   QVarLengthArray<JSC::JSValue, 8> argsVector(args.size());
   for (int i = 0; i < args.size(); ++i) {
      QScriptValue arg = args.at(i);
      if (QScriptValuePrivate::getEngine(arg) != d->engine && QScriptValuePrivate::getEngine(arg)) {
         qWarning("QScriptValue::construct() failed: can not construct function with argument created in a different engine");
         return QScriptValue();
      }

      if (!arg.isValid()) {
         argsVector[i] = JSC::jsUndefined();
      } else {
         argsVector[i] = d->engine->scriptValueToJSCValue(args.at(i));
      }
   }

   JSC::ArgList jscArgs(argsVector.data(), argsVector.size());

   JSC::JSValue savedException;
   QScriptEnginePrivate::saveException(exec, &savedException);
   JSC::JSValue result;
   JSC::JSObject *newObject = JSC::construct(exec, callee, constructType, constructData, jscArgs);
   if (exec->hadException()) {
      result = exec->exception();
   } else {
      result = newObject;
      QScriptEnginePrivate::restoreException(exec, savedException);
   }
   return d->engine->scriptValueFromJSCValue(result);
}

QScriptValue QScriptValue::construct(const QScriptValue &arguments)
{
   Q_D(QScriptValue);
   if (!d || !d->isObject()) {
      return QScriptValue();
   }
   QScript::APIShim shim(d->engine);
   JSC::JSValue callee = d->jscValue;
   JSC::ConstructData constructData;
   JSC::ConstructType constructType = callee.getConstructData(constructData);
   if (constructType == JSC::ConstructTypeNone) {
      return QScriptValue();
   }

   JSC::ExecState *exec = d->engine->currentFrame;

   if (QScriptValuePrivate::getEngine(arguments) != d->engine && QScriptValuePrivate::getEngine(arguments)) {
      qWarning("QScriptValue::construct() failed: can not construct function with argument created in a different engine");
      return QScriptValue();
   }
   JSC::JSValue array = d->engine->scriptValueToJSCValue(arguments);
   // copied from runtime/FunctionPrototype.cpp, functionProtoFuncApply()
   JSC::MarkedArgumentBuffer applyArgs;
   if (! array.isUndefinedOrNull()) {
      if (!array.isObject()) {
         return d->engine->scriptValueFromJSCValue(JSC::throwError(exec, JSC::TypeError, "Arguments must be an array"));
      }
      if (JSC::asObject(array)->classInfo() == &JSC::Arguments::info) {
         JSC::asArguments(array)->fillArgList(exec, applyArgs);
      } else if (JSC::isJSArray(&exec->globalData(), array)) {
         JSC::asArray(array)->fillArgList(exec, applyArgs);
      } else if (JSC::asObject(array)->inherits(&JSC::JSArray::info)) {
         unsigned length = JSC::asArray(array)->get(exec, exec->propertyNames().length).toUInt32(exec);
         for (unsigned i = 0; i < length; ++i) {
            applyArgs.append(JSC::asArray(array)->get(exec, i));
         }
      } else {
         return d->engine->scriptValueFromJSCValue(JSC::throwError(exec, JSC::TypeError, "Arguments must be an array"));
      }
   }

   JSC::JSValue savedException;
   QScriptEnginePrivate::saveException(exec, &savedException);
   JSC::JSValue result;
   JSC::JSObject *newObject = JSC::construct(exec, callee, constructType, constructData, applyArgs);
   if (exec->hadException()) {
      result = exec->exception();
   } else {
      result = newObject;
      QScriptEnginePrivate::restoreException(exec, savedException);
   }
   return d->engine->scriptValueFromJSCValue(result);
}

QScriptEngine *QScriptValue::engine() const
{
   Q_D(const QScriptValue);
   if (!d) {
      return nullptr;
   }
   return QScriptEnginePrivate::get(d->engine);
}

// obsolete
bool QScriptValue::isBoolean() const
{
   Q_D(const QScriptValue);
   return d && d->isJSC() && d->jscValue.isBoolean();
}

bool QScriptValue::isBool() const
{
   Q_D(const QScriptValue);
   return d && d->isJSC() && d->jscValue.isBoolean();
}

bool QScriptValue::isNumber() const
{
   Q_D(const QScriptValue);
   if (!d) {
      return false;
   }
   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore:
         return d->jscValue.isNumber();
      case QScriptValuePrivate::Number:
         return true;
      case QScriptValuePrivate::String:
         return false;
   }
   return false;
}

bool QScriptValue::isString() const
{
   Q_D(const QScriptValue);
   if (!d) {
      return false;
   }
   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore:
         return d->jscValue.isString();
      case QScriptValuePrivate::Number:
         return false;
      case QScriptValuePrivate::String:
         return true;
   }
   return false;
}

bool QScriptValue::isFunction() const
{
   Q_D(const QScriptValue);
   if (!d || !d->isJSC()) {
      return false;
   }
   return QScript::isFunction(d->jscValue);
}

bool QScriptValue::isNull() const
{
   Q_D(const QScriptValue);
   return d && d->isJSC() && d->jscValue.isNull();
}

bool QScriptValue::isUndefined() const
{
   Q_D(const QScriptValue);
   return d && d->isJSC() && d->jscValue.isUndefined();
}

bool QScriptValue::isObject() const
{
   Q_D(const QScriptValue);
   return d && d->isObject();
}

bool QScriptValue::isVariant() const
{
   Q_D(const QScriptValue);
   if (!d || !d->isJSC()) {
      return false;
   }
   return QScriptEnginePrivate::isVariant(d->jscValue);
}

bool QScriptValue::isQObject() const
{
   Q_D(const QScriptValue);
   if (!d || !d->isJSC()) {
      return false;
   }
   return QScriptEnginePrivate::isQObject(d->jscValue);
}

bool QScriptValue::isQMetaObject() const
{
   Q_D(const QScriptValue);
   if (!d || !d->isJSC()) {
      return false;
   }
   return QScriptEnginePrivate::isQMetaObject(d->jscValue);
}

bool QScriptValue::isValid() const
{
   Q_D(const QScriptValue);
   return d && (!d->isJSC() || !!d->jscValue);
}

QScriptValue QScriptValue::data() const
{
   Q_D(const QScriptValue);
   if (!d || !d->isObject()) {
      return QScriptValue();
   }
   if (d->jscValue.inherits(&QScriptObject::info)) {
      QScriptObject *scriptObject = static_cast<QScriptObject *>(JSC::asObject(d->jscValue));
      return d->engine->scriptValueFromJSCValue(scriptObject->data());
   } else {
      // ### make hidden property
      return property(QLatin1String("__qt_data__"), QScriptValue::ResolveLocal);
   }
}

void QScriptValue::setData(const QScriptValue &data)
{
   Q_D(QScriptValue);
   if (! d || ! d->isObject()) {
      return;
   }

   QScript::APIShim shim(d->engine);
   JSC::JSValue other = d->engine->scriptValueToJSCValue(data);

   if (d->jscValue.inherits(&QScriptObject::info)) {
      QScriptObject *scriptObject = static_cast<QScriptObject *>(JSC::asObject(d->jscValue));
      scriptObject->setData(other);
   } else {
      JSC::ExecState *exec = d->engine->currentFrame;
      JSC::Identifier id = JSC::Identifier(exec, "__qt_data__");
      if (!data.isValid()) {
         JSC::asObject(d->jscValue)->removeDirect(id);
      } else {
         // ### make hidden property
         JSC::asObject(d->jscValue)->putDirect(id, other);
      }
   }
}

QScriptClass *QScriptValue::scriptClass() const
{
   Q_D(const QScriptValue);
   if (! d || ! d->isJSC() || !d->jscValue.inherits(&QScriptObject::info)) {
      return nullptr;
   }

   QScriptObject *scriptObject = static_cast<QScriptObject *>(JSC::asObject(d->jscValue));
   QScriptObjectDelegate *delegate = scriptObject->delegate();

   if (! delegate || (delegate->type() != QScriptObjectDelegate::ClassObject)) {
      return nullptr;
   }

   return static_cast<QScript::ClassObjectDelegate *>(delegate)->scriptClass();
}

void QScriptValue::setScriptClass(QScriptClass *scriptClass)
{
   Q_D(QScriptValue);

   if (! d || ! d->isObject()) {
      return;
   }
   if (! d->jscValue.inherits(&QScriptObject::info)) {
      qWarning("QScriptValue::setScriptClass() failed, argument does not inherit from QScriptObject");
      return;
   }

   QScriptObject *scriptObject = static_cast<QScriptObject *>(JSC::asObject(d->jscValue));

   if (! scriptClass) {
      scriptObject->setDelegate(nullptr);

   } else {
      QScriptObjectDelegate *delegate = scriptObject->delegate();

      if (! delegate || (delegate->type() != QScriptObjectDelegate::ClassObject)) {
         delegate = new QScript::ClassObjectDelegate(scriptClass);
         scriptObject->setDelegate(delegate);
      }

      static_cast<QScript::ClassObjectDelegate *>(delegate)->setScriptClass(scriptClass);
   }
}

// internal - Returns the ID of this object, or -1 if this QScriptValue is not an object.
qint64 QScriptValue::objectId() const
{
   return d_ptr ? d_ptr->objectId() : -1;
}

bool QScriptValue::operator==(const QScriptValue &other) const
{
   bool retval = true;

   if (this->d_ptr == other.d_ptr) {
      retval = true;

   } else if (this->d_ptr->type != other.d_ptr->type) {
      retval = false;

   } else if (this->d_ptr->type == QScriptValuePrivate::JavaScriptCore) {
      if (this->d_ptr->jscValue != other.d_ptr->jscValue) {
         retval = false;
      }

   } else if (this->d_ptr->type == QScriptValuePrivate::Number) {
      if (this->d_ptr->numberValue != other.d_ptr->numberValue) {
         retval = false;
      }

   } else if (this->d_ptr->type == QScriptValuePrivate::String) {
      if (this->d_ptr->stringValue != other.d_ptr->stringValue) {
         retval = false;
      }
   }

   return retval;
}
