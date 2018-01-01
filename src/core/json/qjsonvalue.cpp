/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qjsonobject.h>
#include <qjsonvalue.h>
#include <qjsonarray.h>
#include <qjsonarray.h>
#include <qvariant.h>
#include <qstringlist.h>
#include <qdebug.h>

#include <qjson_p.h>

QJsonValue::QJsonValue(Type type)
   : ui(0), d(0), t(type)
{
}

/*!
    \internal
 */
QJsonValue::QJsonValue(QJsonPrivate::Data *data, QJsonPrivate::Base *base, const QJsonPrivate::Value &v)
   : d(0)
{
   t = (Type)(uint)v.type;
   switch (t) {
      case Undefined:
      case Null:
         dbl = 0;
         break;

      case Bool:
         b = v.toBoolean();
         break;

      case Double:
         dbl = v.toDouble(base);
         break;

      case String: {
         QString *s = new QString(v.toString(base));
         m_stringData = s;
         break;
      }

      case Array:
      case Object:
         d = data;
         this->base = v.base(base);
         break;
   }
   if (d) {
      d->ref.ref();
   }
}

QJsonValue::QJsonValue(bool b)
   : d(0), t(Bool)
{
   this->b = b;
}

/*!
    Creates a value of type Double, with value \a n.
 */
QJsonValue::QJsonValue(double n)
   : d(0), t(Double)
{
   this->dbl = n;
}

/*!
    \overload
    Creates a value of type Double, with value \a n.
 */
QJsonValue::QJsonValue(int n)
   : d(0), t(Double)
{
   this->dbl = n;
}

/*!
    \overload
    Creates a value of type Double, with value \a n.
    NOTE: the integer limits for IEEE 754 double precision data is 2^53 (-9007199254740992 to +9007199254740992).
    If you pass in values outside this range expect a loss of precision to occur.
 */
QJsonValue::QJsonValue(qint64 n)
   : d(0), t(Double)
{
   this->dbl = n;
}

/*!
    Creates a value of type String, with value \a s.
 */
QJsonValue::QJsonValue(const QString &s)
   : d(0), t(String)
{
   m_stringData = new QString(s);
}

/*!
    Creates a value of type String, with value \a s.
 */
QJsonValue::QJsonValue(QLatin1String s)
   : d(0), t(String)
{
   m_stringData = new QString(s);
}

/*!
    Creates a value of type Array, with value \a a.
 */
QJsonValue::QJsonValue(const QJsonArray &a)
   : d(a.d), t(Array)
{
   base = a.a;
   if (d) {
      d->ref.ref();
   }
}

/*!
    Creates a value of type Object, with value \a o.
 */
QJsonValue::QJsonValue(const QJsonObject &o)
   : d(o.d), t(Object)
{
   base = o.o;
   if (d) {
      d->ref.ref();
   }
}


/*!
    Destroys the value.
 */
QJsonValue::~QJsonValue()
{
   if (t == String) {
      delete m_stringData;
   }

   if (d && ! d->ref.deref()) {
      delete d;
   }
}

/*!
    Creates a copy of \a other.
 */
QJsonValue::QJsonValue(const QJsonValue &other)
{
   t  = other.t;
   d  = other.d;
   ui = other.ui;

   if (d) {
      d->ref.ref();
   }

   if (t == String && m_stringData) {
      QString *tmp = new QString(*m_stringData);
      m_stringData = tmp;
   }
}

/*!
    Assigns the value stored in \a other to this object.
 */
QJsonValue &QJsonValue::operator =(const QJsonValue &other)
{
   if (t == String) {
      delete m_stringData;
   }

   t = other.t;
   dbl = other.dbl;

   if (d != other.d) {

      if (d && !d->ref.deref()) {
         delete d;
      }
      d = other.d;
      if (d) {
         d->ref.ref();
      }

   }

   if (t == String && m_stringData) {
      QString *tmp = new QString(*m_stringData);
      m_stringData = tmp;
   }

   return *this;
}

QJsonValue QJsonValue::fromVariant(const QVariant &variant)
{
   switch (variant.type()) {
      case QVariant::Bool:
         return QJsonValue(variant.toBool());
      case QVariant::Int:
      case QVariant::Double:
      case QVariant::LongLong:
      case QVariant::ULongLong:
      case QVariant::UInt:
         return QJsonValue(variant.toDouble());
      case QVariant::String:
         return QJsonValue(variant.toString());
      case QVariant::StringList:
         return QJsonValue(QJsonArray::fromStringList(variant.toStringList()));
      case QVariant::List:
         return QJsonValue(QJsonArray::fromVariantList(variant.toList()));
      case QVariant::Map:
         return QJsonValue(QJsonObject::fromVariantMap(variant.toMap()));
      default:
         break;
   }
   QString string = variant.toString();
   if (string.isEmpty()) {
      return QJsonValue();
   }
   return QJsonValue(string);
}

/*!
    Converts the value to a QVariant.

    The QJsonValue types will be converted as follows:

    \value Null     QVariant()
    \value Bool     QVariant::Bool
    \value Double   QVariant::Double
    \value String   QVariant::String
    \value Array    QVariantList
    \value Object   QVariantMap
    \value Undefined QVariant()

    \sa fromVariant()
 */
QVariant QJsonValue::toVariant() const
{
   switch (t) {
      case Bool:
         return b;
      case Double:
         return dbl;
      case String:
         return toString();
      case Array:
         return d ?
                QJsonArray(d, static_cast<QJsonPrivate::Array *>(base)).toVariantList() :
                QVariantList();
      case Object:
         return d ?
                QJsonObject(d, static_cast<QJsonPrivate::Object *>(base)).toVariantMap() :
                QVariantMap();
      case Null:
      case Undefined:
         break;
   }
   return QVariant();
}

/*!
    \enum QJsonValue::Type

    This enum describes the type of the JSON value.

    \value Null     A Null value
    \value Bool     A boolean value. Use toBool() to convert to a bool.
    \value Double   A double. Use toDouble() to convert to a double.
    \value String   A string. Use toString() to convert to a QString.
    \value Array    An array. Use toArray() to convert to a QJsonArray.
    \value Object   An object. Use toObject() to convert to a QJsonObject.
    \value Undefined The value is undefined. This is usually returned as an
                    error condition, when trying to read an out of bounds value
                    in an array or a non existent key in an object.
*/

/*!
    Returns the type of the value.

    \sa QJsonValue::Type
 */
QJsonValue::Type QJsonValue::type() const
{
   return t;
}

/*!
    Converts the value to a bool and returns it.

    If type() is not bool, the \a defaultValue will be returned.
 */
bool QJsonValue::toBool(bool defaultValue) const
{
   if (t != Bool) {
      return defaultValue;
   }
   return b;
}

/*!
    Converts the value to an int and returns it.

    If type() is not Double or the value is not a whole number,
    the \a defaultValue will be returned.
 */
int QJsonValue::toInt(int defaultValue) const
{
   if (t == Double && int(dbl) == dbl) {
      return dbl;
   }
   return defaultValue;
}

/*!
    Converts the value to a double and returns it.

    If type() is not Double, the \a defaultValue will be returned.
 */
double QJsonValue::toDouble(double defaultValue) const
{
   if (t != Double) {
      return defaultValue;
   }
   return dbl;
}

/*!
    Converts the value to a QString and returns it.

    If type() is not String, the \a defaultValue will be returned.
 */
QString QJsonValue::toString(const QString &defaultValue) const
{
   if (t != String) {
      return defaultValue;
   }

   return *m_stringData;
}

/*!
    Converts the value to an array and returns it.

    If type() is not Array, the \a defaultValue will be returned.
 */
QJsonArray QJsonValue::toArray(const QJsonArray &defaultValue) const
{
   if (! d || t != Array) {
      return defaultValue;
   }

   return QJsonArray(d, static_cast<QJsonPrivate::Array *>(base));
}

/*!
    \overload

    Converts the value to an array and returns it.

    If type() is not Array, a QJsonArray() will be returned.
 */
QJsonArray QJsonValue::toArray() const
{
   return toArray(QJsonArray());
}

/*!
    Converts the value to an object and returns it.

    If type() is not Object, the \a defaultValue will be returned.
 */
QJsonObject QJsonValue::toObject(const QJsonObject &defaultValue) const
{
   if (!d || t != Object) {
      return defaultValue;
   }

   return QJsonObject(d, static_cast<QJsonPrivate::Object *>(base));
}

/*!
    \overload

    Converts the value to an object and returns it.

    If type() is not Object, the QJsonObject() will be returned.
 */
QJsonObject QJsonValue::toObject() const
{
   return toObject(QJsonObject());
}

/*!
    Returns true if the value is equal to \a other.
 */
bool QJsonValue::operator==(const QJsonValue &other) const
{
   if (t != other.t) {
      return false;
   }

   switch (t) {
      case Undefined:
      case Null:
         break;
      case Bool:
         return b == other.b;
      case Double:
         return dbl == other.dbl;
      case String:
         return toString() == other.toString();
      case Array:
         if (base == other.base) {
            return true;
         }
         if (!base || !other.base) {
            return false;
         }
         return QJsonArray(d, static_cast<QJsonPrivate::Array *>(base))
                == QJsonArray(other.d, static_cast<QJsonPrivate::Array *>(other.base));
      case Object:
         if (base == other.base) {
            return true;
         }
         if (!base || !other.base) {
            return false;
         }
         return QJsonObject(d, static_cast<QJsonPrivate::Object *>(base))
                == QJsonObject(other.d, static_cast<QJsonPrivate::Object *>(other.base));
   }
   return true;
}

/*!
    Returns true if the value is not equal to \a other.
 */
bool QJsonValue::operator!=(const QJsonValue &other) const
{
   return !(*this == other);
}

/*!
    \internal
 */
void QJsonValue::detach()
{
   if (!d) {
      return;
   }

   QJsonPrivate::Data *x = d->clone(base);
   x->ref.ref();
   if (!d->ref.deref()) {
      delete d;
   }
   d = x;
   base = static_cast<QJsonPrivate::Object *>(d->header->root());
}

QJsonValueRef &QJsonValueRef::operator =(const QJsonValue &val)
{
   if (is_object) {
      o->setValueAt(index, val);
   } else {
      a->replace(index, val);
   }

   return *this;
}

QJsonValueRef &QJsonValueRef::operator =(const QJsonValueRef &ref)
{
   if (is_object) {
      o->setValueAt(index, ref);
   } else {
      a->replace(index, ref);
   }

   return *this;
}

QJsonArray QJsonValueRef::toArray() const
{
   return toValue().toArray();
}

QJsonObject QJsonValueRef::toObject() const
{
   return toValue().toObject();
}

QJsonValue QJsonValueRef::toValue() const
{
   if (!is_object) {
      return a->at(index);
   }
   return o->valueAt(index);
}

QDebug operator<<(QDebug dbg, const QJsonValue &o)
{
   switch (o.t) {
      case QJsonValue::Undefined:
         dbg.nospace() << "QJsonValue(undefined)";
         break;
      case QJsonValue::Null:
         dbg.nospace() << "QJsonValue(null)";
         break;
      case QJsonValue::Bool:
         dbg.nospace() << "QJsonValue(bool, " << o.toBool() << ")";
         break;
      case QJsonValue::Double:
         dbg.nospace() << "QJsonValue(double, " << o.toDouble() << ")";
         break;
      case QJsonValue::String:
         dbg.nospace() << "QJsonValue(string, " << o.toString() << ")";
         break;
      case QJsonValue::Array:
         dbg.nospace() << "QJsonValue(array, ";
         dbg.nospace() << o.toArray();
         dbg.nospace() << ")";
         break;
      case QJsonValue::Object:
         dbg.nospace() << "QJsonValue(object, ";
         dbg.nospace() << o.toObject();
         dbg.nospace() << ")";
         break;
   }
   return dbg.space();
}

