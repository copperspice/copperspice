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

#ifndef QJSONVALUE_H
#define QJSONVALUE_H

#include <qglobal.h>
#include <qstring.h>

class QDebug;
class QVariant;
class QJsonArray;
class QJsonObject;

namespace QJsonPrivate {
class Data;
class Base;
class Object;
class Header;
class Array;
class Value;
class Entry;
}

class Q_CORE_EXPORT QJsonValue
{
 public:
   enum Type {
      Null =  0x0,
      Bool = 0x1,
      Double = 0x2,
      String = 0x3,
      Array = 0x4,
      Object = 0x5,
      Undefined = 0x80
   };

   QJsonValue(Type = Null);
   QJsonValue(bool b);
   QJsonValue(double n);
   QJsonValue(int n);
   QJsonValue(qint64 n);
   QJsonValue(const QString &str);
   QJsonValue(QLatin1String str);
   QJsonValue(const QJsonArray &arr);
   QJsonValue(const QJsonObject &obj);

   ~QJsonValue();

   QJsonValue(const QJsonValue &other);
   QJsonValue &operator =(const QJsonValue &other);

   static QJsonValue fromVariant(const QVariant &variant);
   QVariant toVariant() const;

   Type type() const;
   inline bool isNull() const {
      return type() == Null;
   }

   inline bool isBool() const {
      return type() == Bool;
   }

   inline bool isDouble() const {
      return type() == Double;
   }

   inline bool isString() const {
      return type() == String;
   }

   inline bool isArray() const {
      return type() == Array;
   }

   inline bool isObject() const {
      return type() == Object;
   }

   inline bool isUndefined() const {
      return type() == Undefined;
   }

   bool toBool(bool defaultValue = false) const;
   int toInt(int defaultValue = 0) const;
   double toDouble(double defaultValue = 0) const;
   QString toString(const QString &defaultValue = QString()) const;
   QJsonArray toArray() const;
   QJsonArray toArray(const QJsonArray &defaultValue) const;
   QJsonObject toObject() const;
   QJsonObject toObject(const QJsonObject &defaultValue) const;

   bool operator==(const QJsonValue &other) const;
   bool operator!=(const QJsonValue &other) const;

 private:
   // avoid implicit conversions from char * to bool
   inline QJsonValue(const void *) {}
   friend class QJsonPrivate::Value;
   friend class QJsonArray;
   friend class QJsonObject;
   friend Q_CORE_EXPORT QDebug operator<<(QDebug, const QJsonValue &);

   QJsonValue(QJsonPrivate::Data *d, QJsonPrivate::Base *b, const QJsonPrivate::Value &v);

   void detach();

   union {
      quint64 ui;
      bool b;
      double dbl;

      QString *m_stringData;
      QJsonPrivate::Base *base;
   };

   QJsonPrivate::Data *d; // needed for Objects and Arrays
   Type t;
};

class Q_CORE_EXPORT QJsonValueRef
{
 public:
   QJsonValueRef(QJsonArray *array, int idx)
      : a(array), is_object(false), index(idx) {}

   QJsonValueRef(QJsonObject *object, int idx)
      : o(object), is_object(true), index(idx) {}

   inline operator QJsonValue() const {
      return toValue();
   }
   QJsonValueRef &operator = (const QJsonValue &val);
   QJsonValueRef &operator = (const QJsonValueRef &val);

   inline QJsonValue::Type type() const {
      return toValue().type();
   }

   inline bool isNull() const {
      return type() == QJsonValue::Null;
   }

   inline bool isBool() const {
      return type() == QJsonValue::Bool;
   }

   inline bool isDouble() const {
      return type() == QJsonValue::Double;
   }

   inline bool isString() const {
      return type() == QJsonValue::String;
   }

   inline bool isArray() const {
      return type() == QJsonValue::Array;
   }

   inline bool isObject() const {
      return type() == QJsonValue::Object;
   }

   inline bool isUndefined() const {
      return type() == QJsonValue::Undefined;
   }

   inline bool toBool() const {
      return toValue().toBool();
   }

   inline int toInt() const {
      return toValue().toInt();
   }

   inline double toDouble() const {
      return toValue().toDouble();
   }

   inline QString toString() const {
      return toValue().toString();
   }

   QJsonArray toArray() const;
   QJsonObject toObject() const;

   inline bool operator==(const QJsonValue &other) const {
      return toValue() == other;
   }

   inline bool operator!=(const QJsonValue &other) const {
      return toValue() != other;
   }

 private:
   QJsonValue toValue() const;

   union {
      QJsonArray *a;
      QJsonObject *o;
   };

   uint is_object : 1;
   uint index : 31;
};

class Q_CORE_EXPORT QJsonValueRefPtr {
 public:
    template<typename T1, typename T2>
    QJsonValueRefPtr(T1 a, T2 b)
        : m_ref(a, b) {
    };

    QJsonValueRef* operator->() const { return &m_ref; };

 private:
    mutable QJsonValueRef m_ref;
};

class Q_CORE_EXPORT QJsonValuePtr {
 public:

    QJsonValuePtr(QJsonValue val)
   : m_value(val) {
    };

    const QJsonValue * operator->() const { return &m_value; };

 private:
    const QJsonValue m_value;
};

Q_CORE_EXPORT QDebug operator<<(QDebug, const QJsonValue &);

#endif
