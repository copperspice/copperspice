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

#ifndef QSCRIPTVALUE_H
#define QSCRIPTVALUE_H

#include <qstring.h>
#include <qlist.h>
#include <qsharedpointer.h>

class QScriptClass;
class QScriptValue;
class QScriptEngine;
class QScriptString;
class QVariant;
class QObject;
class QDateTime;

class QMetaObject;

using qsreal = double;

class QScriptValuePrivate;
class QScriptEnginePrivate;
struct QScriptValuePrivatePointerDeleter;

class Q_SCRIPT_EXPORT QScriptValue
{
 public:
   enum ResolveFlag {
      ResolveLocal        = 0x00,
      ResolvePrototype    = 0x01,
      ResolveScope        = 0x02,
      ResolveFull         = ResolvePrototype | ResolveScope
   };

   using ResolveFlags = QFlags<ResolveFlag>;

   enum PropertyFlag {
      ReadOnly            = 0x00000001,
      Undeletable         = 0x00000002,
      SkipInEnumeration   = 0x00000004,

      PropertyGetter      = 0x00000008,
      PropertySetter      = 0x00000010,

      QObjectMember       = 0x00000020,

      KeepExistingFlags   = 0x00000800,

      UserRange           = 0xff000000            // Users may use these as they see fit.
   };
   using PropertyFlags = QFlags<PropertyFlag>;

   enum SpecialValue {
      NullValue,
      UndefinedValue
   };

 public:
   QScriptValue();
   QScriptValue(const QScriptValue &other);

   QScriptValue(QScriptEngine *engine, SpecialValue value);
   QScriptValue(QScriptEngine *engine, bool value);
   QScriptValue(QScriptEngine *engine, int value);
   QScriptValue(QScriptEngine *engine, uint value);
   QScriptValue(QScriptEngine *engine, qsreal value);
   QScriptValue(QScriptEngine *engine, const QString &value);

   QScriptValue(SpecialValue value);
   QScriptValue(bool value);
   QScriptValue(int value);
   QScriptValue(uint value);
   QScriptValue(qsreal value);
   QScriptValue(const QString &value);

   ~QScriptValue();

   QScriptValue &operator=(const QScriptValue &other);

   QScriptEngine *engine() const;

   bool isValid() const;
   bool isBool() const;
   bool isBoolean() const;
   bool isNumber() const;
   bool isFunction() const;
   bool isNull() const;
   bool isString() const;
   bool isUndefined() const;
   bool isVariant() const;
   bool isQObject() const;
   bool isQMetaObject() const;
   bool isObject() const;
   bool isDate() const;
   bool isRegExp() const;
   bool isArray() const;
   bool isError() const;

   QString toString() const;
   qsreal toNumber() const;
   bool toBool() const;
   bool toBoolean() const;
   qsreal toInteger() const;
   qint32 toInt32() const;
   quint32 toUInt32() const;
   quint16 toUInt16() const;
   QVariant toVariant() const;
   QObject *toQObject() const;
   const QMetaObject *toQMetaObject() const;
   QScriptValue toObject() const;
   QDateTime toDateTime() const;

   QRegularExpression toRegExp() const;

   bool instanceOf(const QScriptValue &other) const;

   bool lessThan(const QScriptValue &other) const;
   bool equals(const QScriptValue &other) const;
   bool strictlyEquals(const QScriptValue &other) const;

   QScriptValue prototype() const;
   void setPrototype(const QScriptValue &prototype);

   QScriptValue scope() const;
   void setScope(const QScriptValue &scope);

   QScriptValue property(const QString &name,
      const ResolveFlags &mode = ResolvePrototype) const;
   void setProperty(const QString &name, const QScriptValue &value,
      const PropertyFlags &flags = KeepExistingFlags);

   QScriptValue property(quint32 arrayIndex,
      const ResolveFlags &mode = ResolvePrototype) const;
   void setProperty(quint32 arrayIndex, const QScriptValue &value,
      const PropertyFlags &flags = KeepExistingFlags);

   QScriptValue property(const QScriptString &name,
      const ResolveFlags &mode = ResolvePrototype) const;
   void setProperty(const QScriptString &name, const QScriptValue &value,
      const PropertyFlags &flags = KeepExistingFlags);

   QScriptValue::PropertyFlags propertyFlags(
      const QString &name, const ResolveFlags &mode = ResolvePrototype) const;
   QScriptValue::PropertyFlags propertyFlags(
      const QScriptString &name, const ResolveFlags &mode = ResolvePrototype) const;

   QScriptValue call(const QScriptValue &thisObject = QScriptValue(),
      const QList<QScriptValue> &args = QList<QScriptValue>());

   QScriptValue call(const QScriptValue &thisObject, const QScriptValue &arguments);

   QScriptValue construct(const QList<QScriptValue> &args = QList<QScriptValue>());
   QScriptValue construct(const QScriptValue &arguments);

   QScriptValue data() const;
   void setData(const QScriptValue &data);

   QScriptClass *scriptClass() const;
   void setScriptClass(QScriptClass *scriptClass);

   qint64 objectId() const;

   bool operator==(const QScriptValue &other) const;

 private:
   // force compile error, prevent QScriptValue(bool) to be called
   QScriptValue(void *);

   // force compile error, prevent QScriptValue(QScriptEngine*, bool) to be called
   QScriptValue(QScriptEngine *, void *);

   QScriptValue(QScriptValuePrivate *);

   QExplicitlySharedDataPointer<QScriptValuePrivate> d_ptr;

   Q_DECLARE_PRIVATE(QScriptValue)

   friend class QScriptEnginePrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QScriptValue::ResolveFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QScriptValue::PropertyFlags)

CS_DECLARE_METATYPE(QScriptValue)

#endif
