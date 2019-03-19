/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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
#include "qscriptvalue.h"

#include "qscriptvalue_p.h"
#include "qscriptengine.h"
#include "qscriptengine_p.h"
#include "qscriptstring_p.h"

#include "JSGlobalObject.h"
#include "JSImmediate.h"
#include "JSObject.h"
#include "JSValue.h"
#include "JSFunction.h"
#include "Identifier.h"
#include "Operations.h"
#include "Arguments.h"

#include <qvariant.h>
#include <qvarlengtharray.h>
#include <qnumeric.h>

QT_BEGIN_NAMESPACE

void QScriptValuePrivate::detachFromEngine()
{
   if (isJSC()) {
      jscValue = JSC::JSValue();
   }
   engine = 0;
}

/*!
  \internal
*/
QScriptValue::QScriptValue(QScriptValuePrivate *d)
   : d_ptr(d)
{
}

/*!
  Constructs an invalid QScriptValue.
*/
QScriptValue::QScriptValue()
   : d_ptr(0)
{
}

/*!
  Destroys this QScriptValue.
*/
QScriptValue::~QScriptValue()
{
}

/*!
  Constructs a new QScriptValue that is a copy of \a other.

  Note that if \a other is an object (i.e., isObject() would return
  true), then only a reference to the underlying object is copied into
  the new script value (i.e., the object itself is not copied).
*/
QScriptValue::QScriptValue(const QScriptValue &other)
   : d_ptr(other.d_ptr)
{
}

/*!
  \obsolete

  Constructs a new QScriptValue with the special \a value and
  registers it with the script \a engine.
*/
QScriptValue::QScriptValue(QScriptEngine *engine, QScriptValue::SpecialValue value)
   : d_ptr(new (QScriptEnginePrivate::get(engine))QScriptValuePrivate(QScriptEnginePrivate::get(engine)))
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

/*!
  \obsolete

  \fn QScriptValue::QScriptValue(QScriptEngine *engine, bool value)

  Constructs a new QScriptValue with the boolean \a value and
  registers it with the script \a engine.
*/
QScriptValue::QScriptValue(QScriptEngine *engine, bool val)
   : d_ptr(new (QScriptEnginePrivate::get(engine))QScriptValuePrivate(QScriptEnginePrivate::get(engine)))
{
   d_ptr->initFrom(JSC::jsBoolean(val));
}

/*!
  \fn QScriptValue::QScriptValue(QScriptEngine *engine, int value)
  \obsolete

  Constructs a new QScriptValue with the integer \a value and
  registers it with the script \a engine.
*/
QScriptValue::QScriptValue(QScriptEngine *engine, int val)
   : d_ptr(new (QScriptEnginePrivate::get(engine))QScriptValuePrivate(QScriptEnginePrivate::get(engine)))
{
   if (engine) {
      QScript::APIShim shim(d_ptr->engine);
      JSC::ExecState *exec = d_ptr->engine->currentFrame;
      d_ptr->initFrom(JSC::jsNumber(exec, val));
   } else {
      d_ptr->initFrom(val);
   }
}

/*!
  \fn QScriptValue::QScriptValue(QScriptEngine *engine, uint value)
  \obsolete

  Constructs a new QScriptValue with the unsigned integer \a value and
  registers it with the script \a engine.
 */
QScriptValue::QScriptValue(QScriptEngine *engine, uint val)
   : d_ptr(new (QScriptEnginePrivate::get(engine))QScriptValuePrivate(QScriptEnginePrivate::get(engine)))
{
   if (engine) {
      QScript::APIShim shim(d_ptr->engine);
      JSC::ExecState *exec = d_ptr->engine->currentFrame;
      d_ptr->initFrom(JSC::jsNumber(exec, val));
   } else {
      d_ptr->initFrom(val);
   }
}

/*!
  \fn QScriptValue::QScriptValue(QScriptEngine *engine, qsreal value)
  \obsolete

  Constructs a new QScriptValue with the qsreal \a value and
  registers it with the script \a engine.
*/
QScriptValue::QScriptValue(QScriptEngine *engine, qsreal val)
   : d_ptr(new (QScriptEnginePrivate::get(engine))QScriptValuePrivate(QScriptEnginePrivate::get(engine)))
{
   if (engine) {
      QScript::APIShim shim(d_ptr->engine);
      JSC::ExecState *exec = d_ptr->engine->currentFrame;
      d_ptr->initFrom(JSC::jsNumber(exec, val));
   } else {
      d_ptr->initFrom(val);
   }
}

/*!
  \fn QScriptValue::QScriptValue(QScriptEngine *engine, const QString &value)
  \obsolete

  Constructs a new QScriptValue with the string \a value and
  registers it with the script \a engine.
*/
QScriptValue::QScriptValue(QScriptEngine *engine, const QString &val)
   : d_ptr(new (QScriptEnginePrivate::get(engine))QScriptValuePrivate(QScriptEnginePrivate::get(engine)))
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
   : d_ptr(new (/*engine=*/0)QScriptValuePrivate(/*engine=*/0))
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

/*!
  \since 4.5

  Constructs a new QScriptValue with a boolean \a value.
*/
QScriptValue::QScriptValue(bool value)
   : d_ptr(new (/*engine=*/0)QScriptValuePrivate(/*engine=*/0))
{
   d_ptr->initFrom(JSC::jsBoolean(value));
}

/*!
  \since 4.5

  Constructs a new QScriptValue with a number \a value.
*/
QScriptValue::QScriptValue(int value)
   : d_ptr(new (/*engine=*/0)QScriptValuePrivate(/*engine=*/0))
{
   d_ptr->initFrom(value);
}

QScriptValue::QScriptValue(uint value)
   : d_ptr(new (/*engine=*/0)QScriptValuePrivate(/*engine=*/0))
{
   d_ptr->initFrom(value);
}

QScriptValue::QScriptValue(qsreal value)
   : d_ptr(new (/*engine=*/0)QScriptValuePrivate(/*engine=*/0))
{
   d_ptr->initFrom(value);
}

QScriptValue::QScriptValue(const QString &value)
   : d_ptr(new (/*engine=*/0)QScriptValuePrivate(/*engine=*/0))
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
   if (!d || !d->isJSC()) {
      return false;
   }
   return QScriptEnginePrivate::isError(d->jscValue);
}

bool QScriptValue::isArray() const
{
   Q_D(const QScriptValue);
   if (!d || !d->isJSC()) {
      return false;
   }
   return QScriptEnginePrivate::isArray(d->jscValue);
}

bool QScriptValue::isDate() const
{
   Q_D(const QScriptValue);
   if (!d || !d->isJSC()) {
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
   if (!d || !d->isObject()) {
      return QScriptValue();
   }
   return d->engine->scriptValueFromJSCValue(JSC::asObject(d->jscValue)->prototype());
}

void QScriptValue::setPrototype(const QScriptValue &prototype)
{
   Q_D(QScriptValue);
   if (!d || !d->isObject()) {
      return;
   }

   JSC::JSValue other = d->engine->scriptValueToJSCValue(prototype);
   if (!other || !(other.isObject() || other.isNull())) {
      return;
   }

   if (QScriptValuePrivate::getEngine(prototype)
         && (QScriptValuePrivate::getEngine(prototype) != d->engine)) {
      qWarning("QScriptValue::setPrototype() failed: "
               "cannot set a prototype created in "
               "a different engine");
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
         && !d->engine->customGlobalObject())
         || (thisObject == d->engine->customGlobalObject())) {
      d->engine->originalGlobalObject()->setPrototype(other);
   }
}

/*!
  \internal
*/
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

/*!
  \internal
*/
void QScriptValue::setScope(const QScriptValue &scope)
{
   Q_D(QScriptValue);
   if (!d || !d->isObject()) {
      return;
   }
   if (scope.isValid() && QScriptValuePrivate::getEngine(scope)
         && (QScriptValuePrivate::getEngine(scope) != d->engine)) {
      qWarning("QScriptValue::setScope() failed: "
               "cannot set a scope object created in "
               "a different engine");
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

/*!
  Returns true if this QScriptValue is an instance of
  \a other; otherwise returns false.

  This QScriptValue is considered to be an instance of \a other if
  \a other is a function and the value of the \c{prototype}
  property of \a other is in the prototype chain of this
  QScriptValue.
*/
bool QScriptValue::instanceOf(const QScriptValue &other) const
{
   Q_D(const QScriptValue);
   if (!d || !d->isObject() || !other.isObject()) {
      return false;
   }
   if (QScriptValuePrivate::getEngine(other) != d->engine) {
      qWarning("QScriptValue::instanceof: "
               "cannot perform operation on a value created in "
               "a different engine");
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
   Q_ASSERT(pp->engine != 0);
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
   }

   else if (lhs.isUndefined() && rhs.isNull()) {
      return true;
   }

   else if (IsNumerical(lhs) && rhs.isString()) {
      return lhs.toNumber() == rhs.toNumber();
   }

   else if (lhs.isString() && IsNumerical(rhs)) {
      return lhs.toNumber() == rhs.toNumber();
   }

   else if (lhs.isBool()) {
      return Equals(lhs.toNumber(), rhs);
   }

   else if (rhs.isBool()) {
      return Equals(lhs, rhs.toNumber());
   }

   else if (lhs.isObject() && !rhs.isNull()) {
      lhs = ToPrimitive(lhs);

      if (lhs.isValid() && !lhs.isObject()) {
         return Equals(lhs, rhs);
      }
   }

   else if (rhs.isObject() && ! lhs.isNull()) {
      rhs = ToPrimitive(rhs);
      if (rhs.isValid() && !rhs.isObject()) {
         return Equals(lhs, rhs);
      }
   }

   return false;
}

} // namespace QScript

/*!
  Returns true if this QScriptValue is less than \a other, otherwise
  returns false.  The comparison follows the behavior described in
  \l{ECMA-262} section 11.8.5, "The Abstract Relational Comparison
  Algorithm".

  Note that if this QScriptValue or the \a other value are objects,
  calling this function has side effects on the script engine, since
  the engine will call the object's valueOf() function (and possibly
  toString()) in an attempt to convert the object to a primitive value
  (possibly resulting in an uncaught script exception).

  \sa equals()
*/
bool QScriptValue::lessThan(const QScriptValue &other) const
{
   Q_D(const QScriptValue);
   // no equivalent function in JSC? There's a jsLess() in VM/Machine.cpp
   if (!isValid() || !other.isValid()) {
      return false;
   }
   if (QScriptValuePrivate::getEngine(other) && d->engine
         && (QScriptValuePrivate::getEngine(other) != d->engine)) {
      qWarning("QScriptValue::lessThan: "
               "cannot compare to a value created in "
               "a different engine");
      return false;
   }
   return QScript::LessThan(*this, other);
}

/*!
  Returns true if this QScriptValue is equal to \a other, otherwise
  returns false. The comparison follows the behavior described in
  \l{ECMA-262} section 11.9.3, "The Abstract Equality Comparison
  Algorithm".

  This function can return true even if the type of this QScriptValue
  is different from the type of the \a other value; i.e. the
  comparison is not strict.  For example, comparing the number 9 to
  the string "9" returns true; comparing an undefined value to a null
  value returns true; comparing a \c{Number} object whose primitive
  value is 6 to a \c{String} object whose primitive value is "6"
  returns true; and comparing the number 1 to the boolean value
  \c{true} returns true. If you want to perform a comparison
  without such implicit value conversion, use strictlyEquals().

  Note that if this QScriptValue or the \a other value are objects,
  calling this function has side effects on the script engine, since
  the engine will call the object's valueOf() function (and possibly
  toString()) in an attempt to convert the object to a primitive value
  (possibly resulting in an uncaught script exception).

  \sa strictlyEquals(), lessThan()
*/
bool QScriptValue::equals(const QScriptValue &other) const
{
   Q_D(const QScriptValue);
   if (!d || !other.d_ptr) {
      return (d_ptr == other.d_ptr);
   }
   if (QScriptValuePrivate::getEngine(other) && d->engine
         && (QScriptValuePrivate::getEngine(other) != d->engine)) {
      qWarning("QScriptValue::equals: "
               "cannot compare to a value created in "
               "a different engine");
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

/*!
  Returns true if this QScriptValue is equal to \a other using strict
  comparison (no conversion), otherwise returns false. The comparison
  follows the behavior described in \l{ECMA-262} section 11.9.6, "The
  Strict Equality Comparison Algorithm".

  If the type of this QScriptValue is different from the type of the
  \a other value, this function returns false. If the types are equal,
  the result depends on the type, as shown in the following table:

    \table
    \header \o Type \o Result
    \row    \o Undefined  \o true
    \row    \o Null       \o true
    \row    \o Boolean    \o true if both values are true, false otherwise
    \row    \o Number     \o false if either value is NaN (Not-a-Number); true if values are equal, false otherwise
    \row    \o String     \o true if both values are exactly the same sequence of characters, false otherwise
    \row    \o Object     \o true if both values refer to the same object, false otherwise
    \endtable

  \sa equals()
*/
bool QScriptValue::strictlyEquals(const QScriptValue &other) const
{
   Q_D(const QScriptValue);
   if (!d || !other.d_ptr) {
      return (d_ptr == other.d_ptr);
   }
   if (QScriptValuePrivate::getEngine(other) && d->engine
         && (QScriptValuePrivate::getEngine(other) != d->engine)) {
      qWarning("QScriptValue::strictlyEquals: "
               "cannot compare to a value created in "
               "a different engine");
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
         JSC::ExecState *exec = eng_p ? eng_p->currentFrame : 0;
         return JSC::JSValue::strictEqual(exec, d->jscValue, other.d_ptr->jscValue);
      }
      case QScriptValuePrivate::Number:
         return (d->numberValue == other.d_ptr->numberValue);
      case QScriptValuePrivate::String:
         return (d->stringValue == other.d_ptr->stringValue);
   }
   return false;
}

/*!
  Returns the string value of this QScriptValue, as defined in
  \l{ECMA-262} section 9.8, "ToString".

  Note that if this QScriptValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's toString() function (and possibly valueOf()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa isString()
*/
QString QScriptValue::toString() const
{
   Q_D(const QScriptValue);
   if (!d) {
      return QString();
   }
   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore: {
         if (d->engine) {
            QScript::APIShim shim(d->engine);
            return QScriptEnginePrivate::toString(d->engine->currentFrame, d->jscValue);
         } else {
            return QScriptEnginePrivate::toString(0, d->jscValue);
         }
      }
      case QScriptValuePrivate::Number:
         return QScript::ToString(d->numberValue);
      case QScriptValuePrivate::String:
         return d->stringValue;
   }
   return QString();
}

/*!
  Returns the number value of this QScriptValue, as defined in
  \l{ECMA-262} section 9.3, "ToNumber".

  Note that if this QScriptValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's valueOf() function (and possibly toString()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa isNumber(), toInteger(), toInt32(), toUInt32(), toUInt16()
*/
qsreal QScriptValue::toNumber() const
{
   Q_D(const QScriptValue);
   if (!d) {
      return 0;
   }
   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore: {
         if (d->engine) {
            QScript::APIShim shim(d->engine);
            return QScriptEnginePrivate::toNumber(d->engine->currentFrame, d->jscValue);
         } else {
            return QScriptEnginePrivate::toNumber(0, d->jscValue);
         }
      }
      case QScriptValuePrivate::Number:
         return d->numberValue;
      case QScriptValuePrivate::String:
         return QScript::ToNumber(d->stringValue);
   }
   return 0;
}

/*!
  \obsolete

  Use toBool() instead.
*/
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
            return QScriptEnginePrivate::toBool(0, d->jscValue);
         }
      }
      case QScriptValuePrivate::Number:
         return QScript::ToBool(d->numberValue);
      case QScriptValuePrivate::String:
         return QScript::ToBool(d->stringValue);
   }
   return false;
}

/*!
  \since 4.5

  Returns the boolean value of this QScriptValue, using the conversion
  rules described in \l{ECMA-262} section 9.2, "ToBoolean".

  Note that if this QScriptValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's valueOf() function (and possibly toString()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa isBool()
*/
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
            return QScriptEnginePrivate::toBool(0, d->jscValue);
         }
      }
      case QScriptValuePrivate::Number:
         return QScript::ToBool(d->numberValue);
      case QScriptValuePrivate::String:
         return QScript::ToBool(d->stringValue);
   }
   return false;
}

/*!
  Returns the signed 32-bit integer value of this QScriptValue, using
  the conversion rules described in \l{ECMA-262} section 9.5, "ToInt32".

  Note that if this QScriptValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's valueOf() function (and possibly toString()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa toNumber(), toUInt32()
*/
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
            return QScriptEnginePrivate::toInt32(0, d->jscValue);
         }
      }
      case QScriptValuePrivate::Number:
         return QScript::ToInt32(d->numberValue);
      case QScriptValuePrivate::String:
         return QScript::ToInt32(d->stringValue);
   }
   return 0;
}

/*!
  Returns the unsigned 32-bit integer value of this QScriptValue, using
  the conversion rules described in \l{ECMA-262} section 9.6, "ToUint32".

  Note that if this QScriptValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's valueOf() function (and possibly toString()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa toNumber(), toInt32()
*/
quint32 QScriptValue::toUInt32() const
{
   Q_D(const QScriptValue);
   if (!d) {
      return 0;
   }
   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore: {
         if (d->engine) {
            QScript::APIShim shim(d->engine);
            return QScriptEnginePrivate::toUInt32(d->engine->currentFrame, d->jscValue);
         } else {
            return QScriptEnginePrivate::toUInt32(0, d->jscValue);
         }
      }
      case QScriptValuePrivate::Number:
         return QScript::ToUInt32(d->numberValue);
      case QScriptValuePrivate::String:
         return QScript::ToUInt32(d->stringValue);
   }
   return 0;
}

/*!
  Returns the unsigned 16-bit integer value of this QScriptValue, using
  the conversion rules described in \l{ECMA-262} section 9.7, "ToUint16".

  Note that if this QScriptValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's valueOf() function (and possibly toString()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa toNumber()
*/
quint16 QScriptValue::toUInt16() const
{
   Q_D(const QScriptValue);
   if (!d) {
      return 0;
   }
   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore: {
         if (d->engine) {
            QScript::APIShim shim(d->engine);
            return QScriptEnginePrivate::toUInt16(d->engine->currentFrame, d->jscValue);
         } else {
            return QScriptEnginePrivate::toUInt16(0, d->jscValue);
         }
      }
      case QScriptValuePrivate::Number:
         return QScript::ToUInt16(d->numberValue);
      case QScriptValuePrivate::String:
         return QScript::ToUInt16(d->stringValue);
   }
   return 0;
}

/*!
  Returns the integer value of this QScriptValue, using the conversion
  rules described in \l{ECMA-262} section 9.4, "ToInteger".

  Note that if this QScriptValue is an object, calling this function
  has side effects on the script engine, since the engine will call
  the object's valueOf() function (and possibly toString()) in an
  attempt to convert the object to a primitive value (possibly
  resulting in an uncaught script exception).

  \sa toNumber()
*/
qsreal QScriptValue::toInteger() const
{
   Q_D(const QScriptValue);
   if (!d) {
      return 0;
   }
   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore: {
         if (d->engine) {
            QScript::APIShim shim(d->engine);
            return QScriptEnginePrivate::toInteger(d->engine->currentFrame, d->jscValue);
         } else {
            return QScriptEnginePrivate::toInteger(0, d->jscValue);
         }
      }
      case QScriptValuePrivate::Number:
         return QScript::ToInteger(d->numberValue);
      case QScriptValuePrivate::String:
         return QScript::ToInteger(d->stringValue);
   }
   return 0;
}

/*!
  Returns the QVariant value of this QScriptValue, if it can be
  converted to a QVariant; otherwise returns an invalid QVariant.
  The conversion is performed according to the following table:

    \table
    \header \o Input Type \o Result
    \row    \o Undefined  \o An invalid QVariant.
    \row    \o Null       \o An invalid QVariant.
    \row    \o Boolean    \o A QVariant containing the value of the boolean.
    \row    \o Number     \o A QVariant containing the value of the number.
    \row    \o String     \o A QVariant containing the value of the string.
    \row    \o QVariant Object \o The result is the QVariant value of the object (no conversion).
    \row    \o QObject Object \o A QVariant containing a pointer to the QObject.
    \row    \o Date Object \o A QVariant containing the date value (toDateTime()).
    \row    \o RegExp Object \o A QVariant containing the regular expression value (toRegExp()).
    \row    \o Array Object \o The array is converted to a QVariantList. Each element is converted to a QVariant, recursively; cyclic references are not followed.
    \row    \o Object     \o The object is converted to a QVariantMap. Each property is converted to a QVariant, recursively; cyclic references are not followed.
    \endtable

  \sa isVariant()
*/
QVariant QScriptValue::toVariant() const
{
   Q_D(const QScriptValue);
   if (!d) {
      return QVariant();
   }
   switch (d->type) {
      case QScriptValuePrivate::JavaScriptCore: {
         if (d->engine) {
            QScript::APIShim shim(d->engine);
            return QScriptEnginePrivate::toVariant(d->engine->currentFrame, d->jscValue);
         } else {
            return QScriptEnginePrivate::toVariant(0, d->jscValue);
         }
      }
      case QScriptValuePrivate::Number:
         return QVariant(d->numberValue);
      case QScriptValuePrivate::String:
         return QVariant(d->stringValue);
   }
   return QVariant();
}

/*!
  \obsolete

  This function is obsolete; use QScriptEngine::toObject() instead.
*/
QScriptValue QScriptValue::toObject() const
{
   Q_D(const QScriptValue);
   if (!d || !d->engine) {
      return QScriptValue();
   }
   return engine()->toObject(*this);
}

/*!
  Returns a QDateTime representation of this value, in local time.
  If this QScriptValue is not a date, or the value of the date is NaN
  (Not-a-Number), an invalid QDateTime is returned.

  \sa isDate()
*/
QDateTime QScriptValue::toDateTime() const
{
   Q_D(const QScriptValue);
   if (!d || !d->engine) {
      return QDateTime();
   }

   QScript::APIShim shim(d->engine);

   return QScriptEnginePrivate::toDateTime(d->engine->currentFrame, d->jscValue);
}

QRegularExpression QScriptValue::toRegExp() const
{
   Q_D(const QScriptValue);

   if (! d || !d->engine) {
      return QRegularExpression();
   }

   QScript::APIShim shim(d->engine);

   return QScriptEnginePrivate::toRegExp(d->engine->currentFrame, d->jscValue);
}

QObject *QScriptValue::toQObject() const
{
   Q_D(const QScriptValue);
   if (!d || !d->engine) {
      return 0;
   }
   QScript::APIShim shim(d->engine);
   return QScriptEnginePrivate::toQObject(d->engine->currentFrame, d->jscValue);
}

/*!
  If this QScriptValue is a QMetaObject, returns the QMetaObject pointer
  that the QScriptValue represents; otherwise, returns 0.

  \sa isQMetaObject()
*/
const QMetaObject *QScriptValue::toQMetaObject() const
{
   Q_D(const QScriptValue);
   if (!d || !d->engine) {
      return 0;
   }
   QScript::APIShim shim(d->engine);
   return QScriptEnginePrivate::toQMetaObject(d->engine->currentFrame, d->jscValue);
}

/*!
  Sets the value of this QScriptValue's property with the given \a name to
  the given \a value.

  If this QScriptValue is not an object, this function does nothing.

  If this QScriptValue does not already have a property with name \a name,
  a new property is created; the given \a flags then specify how this
  property may be accessed by script code.

  If \a value is invalid, the property is removed.

  If the property is implemented using a setter function (i.e. has the
  PropertySetter flag set), calling setProperty() has side-effects on
  the script engine, since the setter function will be called with the
  given \a value as argument (possibly resulting in an uncaught script
  exception).

  Note that you cannot specify custom getter or setter functions for
  built-in properties, such as the \c{length} property of Array objects
  or meta properties of QObject objects.

  \sa property()
*/

void QScriptValue::setProperty(const QString &name, const QScriptValue &value,
                               const PropertyFlags &flags)
{
   Q_D(QScriptValue);
   if (!d || !d->isObject()) {
      return;
   }
   QScript::APIShim shim(d->engine);
   QScriptEnginePrivate *valueEngine = QScriptValuePrivate::getEngine(value);
   if (valueEngine && (valueEngine != d->engine)) {
      qWarning("QScriptValue::setProperty(%s) failed: "
               "cannot set value created in a different engine",
               qPrintable(name));
      return;
   }
   JSC::JSValue jsValue = d->engine->scriptValueToJSCValue(value);
   d->setProperty(name, jsValue, flags);
}

/*!
  Returns the value of this QScriptValue's property with the given \a name,
  using the given \a mode to resolve the property.

  If no such property exists, an invalid QScriptValue is returned.

  If the property is implemented using a getter function (i.e. has the
  PropertyGetter flag set), calling property() has side-effects on the
  script engine, since the getter function will be called (possibly
  resulting in an uncaught script exception). If an exception
  occurred, property() returns the value that was thrown (typically
  an \c{Error} object).

  \sa setProperty(), propertyFlags(), QScriptValueIterator
*/
QScriptValue QScriptValue::property(const QString &name,
                                    const ResolveFlags &mode) const
{
   Q_D(const QScriptValue);
   if (!d || !d->isObject()) {
      return QScriptValue();
   }
   QScript::APIShim shim(d->engine);
   return d->engine->scriptValueFromJSCValue(d->property(name, mode));
}

/*!
  \overload

  Returns the property at the given \a arrayIndex, using the given \a
  mode to resolve the property.

  This function is provided for convenience and performance when
  working with array objects.

  If this QScriptValue is not an Array object, this function behaves
  as if property() was called with the string representation of \a
  arrayIndex.
*/
QScriptValue QScriptValue::property(quint32 arrayIndex,
                                    const ResolveFlags &mode) const
{
   Q_D(const QScriptValue);
   if (!d || !d->isObject()) {
      return QScriptValue();
   }
   QScript::APIShim shim(d->engine);
   return d->engine->scriptValueFromJSCValue(d->property(arrayIndex, mode));
}

/*!
  \overload

  Sets the property at the given \a arrayIndex to the given \a value.

  This function is provided for convenience and performance when
  working with array objects.

  If this QScriptValue is not an Array object, this function behaves
  as if setProperty() was called with the string representation of \a
  arrayIndex.
*/
void QScriptValue::setProperty(quint32 arrayIndex, const QScriptValue &value,
                               const PropertyFlags &flags)
{
   Q_D(QScriptValue);
   if (!d || !d->isObject()) {
      return;
   }
   if (QScriptValuePrivate::getEngine(value)
         && (QScriptValuePrivate::getEngine(value) != d->engine)) {
      qWarning("QScriptValue::setProperty() failed: "
               "cannot set value created in a different engine");
      return;
   }
   QScript::APIShim shim(d->engine);
   JSC::JSValue jsValue = d->engine->scriptValueToJSCValue(value);
   d->setProperty(arrayIndex, jsValue, flags);
}

/*!
  \since 4.4

  Returns the value of this QScriptValue's property with the given \a name,
  using the given \a mode to resolve the property.

  This overload of property() is useful when you need to look up the
  same property repeatedly, since the lookup can be performed faster
  when the name is represented as an interned string.

  \sa QScriptEngine::toStringHandle(), setProperty()
*/
QScriptValue QScriptValue::property(const QScriptString &name,
                                    const ResolveFlags &mode) const
{
   Q_D(const QScriptValue);
   if (!d || !d->isObject() || !QScriptStringPrivate::isValid(name)) {
      return QScriptValue();
   }
   QScript::APIShim shim(d->engine);
   return d->engine->scriptValueFromJSCValue(d->property(name.d_ptr->identifier, mode));
}

/*!
  \since 4.4

  Sets the value of this QScriptValue's property with the given \a
  name to the given \a value. The given \a flags specify how this
  property may be accessed by script code.

  This overload of setProperty() is useful when you need to set the
  same property repeatedly, since the operation can be performed
  faster when the name is represented as an interned string.

  \sa QScriptEngine::toStringHandle()
*/
void QScriptValue::setProperty(const QScriptString &name,
                               const QScriptValue &value,
                               const PropertyFlags &flags)
{
   Q_D(QScriptValue);
   if (!d || !d->isObject() || !QScriptStringPrivate::isValid(name)) {
      return;
   }
   QScriptEnginePrivate *valueEngine = QScriptValuePrivate::getEngine(value);
   if (valueEngine && (valueEngine != d->engine)) {
      qWarning("QScriptValue::setProperty(%s) failed: "
               "cannot set value created in a different engine",
               qPrintable(name.toString()));
      return;
   }
   QScript::APIShim shim(d->engine);
   JSC::JSValue jsValue = d->engine->scriptValueToJSCValue(value);
   d->setProperty(name.d_ptr->identifier, jsValue, flags);
}

/*!
  Returns the flags of the property with the given \a name, using the
  given \a mode to resolve the property.

  \sa property()
*/
QScriptValue::PropertyFlags QScriptValue::propertyFlags(const QString &name,
      const ResolveFlags &mode) const
{
   Q_D(const QScriptValue);
   if (!d || !d->isObject()) {
      return 0;
   }
   QScript::APIShim shim(d->engine);
   JSC::ExecState *exec = d->engine->currentFrame;
   return d->propertyFlags(JSC::Identifier(exec, name), mode);

}

/*!
  \since 4.4

  Returns the flags of the property with the given \a name, using the
  given \a mode to resolve the property.

  \sa property()
*/
QScriptValue::PropertyFlags QScriptValue::propertyFlags(const QScriptString &name,
      const ResolveFlags &mode) const
{
   Q_D(const QScriptValue);
   if (!d || !d->isObject() || !QScriptStringPrivate::isValid(name)) {
      return 0;
   }
   return d->propertyFlags(name.d_ptr->identifier, mode);
}

/*!
  Calls this QScriptValue as a function, using \a thisObject as
  the `this' object in the function call, and passing \a args
  as arguments to the function. Returns the value returned from
  the function.

  If this QScriptValue is not a function, call() does nothing
  and returns an invalid QScriptValue.

  Note that if \a thisObject is not an object, the global object
  (see \l{QScriptEngine::globalObject()}) will be used as the
  `this' object.

  Calling call() can cause an exception to occur in the script engine;
  in that case, call() returns the value that was thrown (typically an
  \c{Error} object). You can call
  QScriptEngine::hasUncaughtException() to determine if an exception
  occurred.

  \snippet doc/src/snippets/code/src_script_qscriptvalue.cpp 2

  \sa construct()
*/
QScriptValue QScriptValue::call(const QScriptValue &thisObject,
                                const QScriptValueList &args)
{
   Q_D(const QScriptValue);
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
      qWarning("QScriptValue::call() failed: "
               "cannot call function with thisObject created in "
               "a different engine");
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
         qWarning("QScriptValue::call() failed: "
                  "cannot call function with argument created in "
                  "a different engine");
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

/*!
  Calls this QScriptValue as a function, using \a thisObject as
  the `this' object in the function call, and passing \a arguments
  as arguments to the function. Returns the value returned from
  the function.

  If this QScriptValue is not a function, call() does nothing
  and returns an invalid QScriptValue.

  \a arguments can be an arguments object, an array, null or
  undefined; any other type will cause a TypeError to be thrown.

  Note that if \a thisObject is not an object, the global object
  (see \l{QScriptEngine::globalObject()}) will be used as the
  `this' object.

  One common usage of this function is to forward native function
  calls to another function:

  \snippet doc/src/snippets/code/src_script_qscriptvalue.cpp 3

  \sa construct(), QScriptContext::argumentsObject()
*/
QScriptValue QScriptValue::call(const QScriptValue &thisObject,
                                const QScriptValue &arguments)
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
      qWarning("QScriptValue::call() failed: "
               "cannot call function with thisObject created in "
               "a different engine");
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

/*!
  Creates a new \c{Object} and calls this QScriptValue as a
  constructor, using the created object as the `this' object and
  passing \a args as arguments. If the return value from the
  constructor call is an object, then that object is returned;
  otherwise the default constructed object is returned.

  If this QScriptValue is not a function, construct() does nothing
  and returns an invalid QScriptValue.

  Calling construct() can cause an exception to occur in the script
  engine; in that case, construct() returns the value that was thrown
  (typically an \c{Error} object). You can call
  QScriptEngine::hasUncaughtException() to determine if an exception
  occurred.

  \sa call(), QScriptEngine::newObject()
*/
QScriptValue QScriptValue::construct(const QScriptValueList &args)
{
   Q_D(const QScriptValue);
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

   QVarLengthArray<JSC::JSValue, 8> argsVector(args.size());
   for (int i = 0; i < args.size(); ++i) {
      QScriptValue arg = args.at(i);
      if (QScriptValuePrivate::getEngine(arg) != d->engine && QScriptValuePrivate::getEngine(arg)) {
         qWarning("QScriptValue::construct() failed: "
                  "cannot construct function with argument created in "
                  "a different engine");
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

/*!
  Creates a new \c{Object} and calls this QScriptValue as a
  constructor, using the created object as the `this' object and
  passing \a arguments as arguments. If the return value from the
  constructor call is an object, then that object is returned;
  otherwise the default constructed object is returned.

  If this QScriptValue is not a function, construct() does nothing
  and returns an invalid QScriptValue.

  \a arguments can be an arguments object, an array, null or
  undefined. Any other type will cause a TypeError to be thrown.

  \sa call(), QScriptEngine::newObject(), QScriptContext::argumentsObject()
*/
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
      qWarning("QScriptValue::construct() failed: "
               "cannot construct function with argument created in "
               "a different engine");
      return QScriptValue();
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

/*!
  Returns the QScriptEngine that created this QScriptValue,
  or 0 if this QScriptValue is invalid or the value is not
  associated with a particular engine.
*/
QScriptEngine *QScriptValue::engine() const
{
   Q_D(const QScriptValue);
   if (!d) {
      return 0;
   }
   return QScriptEnginePrivate::get(d->engine);
}

/*!
  \obsolete

  Use isBool() instead.
*/
bool QScriptValue::isBoolean() const
{
   Q_D(const QScriptValue);
   return d && d->isJSC() && d->jscValue.isBoolean();
}

/*!
  \since 4.5

  Returns true if this QScriptValue is of the primitive type Boolean;
  otherwise returns false.

  \sa toBool()
*/
bool QScriptValue::isBool() const
{
   Q_D(const QScriptValue);
   return d && d->isJSC() && d->jscValue.isBoolean();
}

/*!
  Returns true if this QScriptValue is of the primitive type Number;
  otherwise returns false.

  \sa toNumber()
*/
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

/*!
  Returns true if this QScriptValue is of the primitive type String;
  otherwise returns false.

  \sa toString()
*/
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

/*!
  Returns true if this QScriptValue is a function; otherwise returns
  false.

  \sa call()
*/
bool QScriptValue::isFunction() const
{
   Q_D(const QScriptValue);
   if (!d || !d->isJSC()) {
      return false;
   }
   return QScript::isFunction(d->jscValue);
}

/*!
  Returns true if this QScriptValue is of the primitive type Null;
  otherwise returns false.

  \sa QScriptEngine::nullValue()
*/
bool QScriptValue::isNull() const
{
   Q_D(const QScriptValue);
   return d && d->isJSC() && d->jscValue.isNull();
}

/*!
  Returns true if this QScriptValue is of the primitive type Undefined;
  otherwise returns false.

  \sa QScriptEngine::undefinedValue()
*/
bool QScriptValue::isUndefined() const
{
   Q_D(const QScriptValue);
   return d && d->isJSC() && d->jscValue.isUndefined();
}

/*!
  Returns true if this QScriptValue is of the Object type; otherwise
  returns false.

  Note that function values, variant values, and QObject values are
  objects, so this function returns true for such values.

  \sa toObject(), QScriptEngine::newObject()
*/
bool QScriptValue::isObject() const
{
   Q_D(const QScriptValue);
   return d && d->isObject();
}

/*!
  Returns true if this QScriptValue is a variant value;
  otherwise returns false.

  \sa toVariant(), QScriptEngine::newVariant()
*/
bool QScriptValue::isVariant() const
{
   Q_D(const QScriptValue);
   if (!d || !d->isJSC()) {
      return false;
   }
   return QScriptEnginePrivate::isVariant(d->jscValue);
}

/*!
  Returns true if this QScriptValue is a QObject; otherwise returns
  false.

  Note: This function returns true even if the QObject that this
  QScriptValue wraps has been deleted.

  \sa toQObject(), QScriptEngine::newQObject()
*/
bool QScriptValue::isQObject() const
{
   Q_D(const QScriptValue);
   if (!d || !d->isJSC()) {
      return false;
   }
   return QScriptEnginePrivate::isQObject(d->jscValue);
}

/*!
  Returns true if this QScriptValue is a QMetaObject; otherwise returns
  false.

  \sa toQMetaObject(), QScriptEngine::newQMetaObject()
*/
bool QScriptValue::isQMetaObject() const
{
   Q_D(const QScriptValue);
   if (!d || !d->isJSC()) {
      return false;
   }
   return QScriptEnginePrivate::isQMetaObject(d->jscValue);
}

/*!
  Returns true if this QScriptValue is valid; otherwise returns
  false.
*/
bool QScriptValue::isValid() const
{
   Q_D(const QScriptValue);
   return d && (!d->isJSC() || !!d->jscValue);
}

/*!
  \since 4.4

  Returns the internal data of this QScriptValue object. QtScript uses
  this property to store the primitive value of Date, String, Number
  and Boolean objects. For other types of object, custom data may be
  stored using setData().
*/
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

/*!
  \since 4.4

  Sets the internal \a data of this QScriptValue object. You can use
  this function to set object-specific data that won't be directly
  accessible to scripts, but may be retrieved in C++ using the data()
  function.

  \sa QScriptEngine::reportAdditionalMemoryCost()
*/
void QScriptValue::setData(const QScriptValue &data)
{
   Q_D(QScriptValue);
   if (!d || !d->isObject()) {
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

/*!
  \since 4.4

  Returns the custom script class that this script object is an
  instance of, or 0 if the object is not of a custom class.

  \sa setScriptClass()
*/
QScriptClass *QScriptValue::scriptClass() const
{
   Q_D(const QScriptValue);
   if (!d || !d->isJSC() || !d->jscValue.inherits(&QScriptObject::info)) {
      return 0;
   }
   QScriptObject *scriptObject = static_cast<QScriptObject *>(JSC::asObject(d->jscValue));
   QScriptObjectDelegate *delegate = scriptObject->delegate();
   if (!delegate || (delegate->type() != QScriptObjectDelegate::ClassObject)) {
      return 0;
   }
   return static_cast<QScript::ClassObjectDelegate *>(delegate)->scriptClass();
}

/*!
  \since 4.4

  Sets the custom script class of this script object to \a scriptClass.
  This can be used to "promote" a plain script object (e.g. created
  by the "new" operator in a script, or by QScriptEngine::newObject() in C++)
  to an object of a custom type.

  If \a scriptClass is 0, the object will be demoted to a plain
  script object.

  \sa scriptClass(), setData()
*/
void QScriptValue::setScriptClass(QScriptClass *scriptClass)
{
   Q_D(QScriptValue);
   if (!d || !d->isObject()) {
      return;
   }
   if (!d->jscValue.inherits(&QScriptObject::info)) {
      qWarning("QScriptValue::setScriptClass() failed: "
               "cannot change class of non-QScriptObject");
      return;
   }
   QScriptObject *scriptObject = static_cast<QScriptObject *>(JSC::asObject(d->jscValue));
   if (!scriptClass) {
      scriptObject->setDelegate(0);
   } else {
      QScriptObjectDelegate *delegate = scriptObject->delegate();
      if (!delegate || (delegate->type() != QScriptObjectDelegate::ClassObject)) {
         delegate = new QScript::ClassObjectDelegate(scriptClass);
         scriptObject->setDelegate(delegate);
      }
      static_cast<QScript::ClassObjectDelegate *>(delegate)->setScriptClass(scriptClass);
   }
}

/*!
  \internal

  Returns the ID of this object, or -1 if this QScriptValue is not an
  object.

  \sa QScriptEngine::objectById()
*/
qint64 QScriptValue::objectId() const
{
   return d_ptr ? d_ptr->objectId() : -1;
}
QT_END_NAMESPACE
