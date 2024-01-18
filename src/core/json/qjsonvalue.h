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

#ifndef QJSONVALUE_H
#define QJSONVALUE_H

#include <qglobal.h>
#include <qstring.h>
#include <qvariant.h>

class QJsonData;
class QJsonArray;
class QJsonObject;

class Q_CORE_EXPORT QJsonValue
{
 public:
   enum Type {
      Null      = 0x0,
      Bool      = 0x1,
      Double    = 0x2,
      String    = 0x3,
      Array     = 0x4,
      Object    = 0x5,
      Undefined = 0x80
   };

   QJsonValue(Type type = Null);

   QJsonValue(bool b);
   QJsonValue(double n);
   QJsonValue(int n);
   QJsonValue(qint64 n);

   QJsonValue(QString str);
   QJsonValue(QJsonArray array);
   QJsonValue(QJsonObject object);

   QJsonValue(const QJsonValue &other);

   ~QJsonValue();

   // methods
   static QJsonValue fromVariant(const QVariant &variant);

   bool isNull() const {
      return type() == Type::Null;
   }

   bool isBool() const {
      return type() == Type::Bool;
   }

   bool isDouble() const {
      return type() == Type::Double;
   }

   bool isString() const {
      return type() == Type::String;
   }

   bool isArray() const {
      return type() == Type::Array;
   }

   bool isObject() const {
      return type() == Type::Object;
   }

   bool isUndefined() const {
      return type() == Type::Undefined;
   }

   bool toBool(bool defaultValue = false) const;
   int toInt(int defaultValue = 0) const;
   double toDouble(double defaultValue = 0) const;
   QString toString(const QString &defaultValue = QString()) const;

   QJsonArray toArray() const;
   QJsonArray toArray(const QJsonArray &defaultValue) const;

   QJsonObject toObject() const;
   QJsonObject toObject(const QJsonObject &defaultValue) const;

   QVariant toVariant() const;

   Type type() const;

   // operators
   QJsonValue &operator =(const QJsonValue &other);

   bool operator==(const QJsonValue &other) const;
   bool operator!=(const QJsonValue &other) const;

 private:
   // avoid implicit conversions from char * to bool
   inline QJsonValue(const void *)
   {}

   std::shared_ptr<QJsonData> m_data;
};

#endif
