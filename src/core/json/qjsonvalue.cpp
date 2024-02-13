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

#include <qjsonvalue.h>

#include <qjson.h>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qjsonobject.h>

QJsonValue::QJsonValue(Type type)
{
   if (type == Type::Bool) {
      m_data = std::make_shared<QJsonDataBool>(false);

   } else if (type == Type::Double) {
      m_data = std::make_shared<QJsonDataNumber>(0);

   } else if (type == Type::String) {
      m_data = std::make_shared<QJsonDataString>(QString());

   } else if (type == Type::Array) {
      m_data = std::make_shared<QJsonDataArray>();

   } else if (type == Type::Object) {
      m_data = std::make_shared<QJsonDataObject>();

   } else {
      // type is null or unknown
      m_data = std::make_shared<QJsonDataNull>();

   }
}

QJsonValue::QJsonValue(bool b)
   : m_data(std::make_shared<QJsonDataBool>(b))
{
}

QJsonValue::QJsonValue(double n)
   : m_data(std::make_shared<QJsonDataNumber>(n))
{
}

QJsonValue::QJsonValue(int n)
   : m_data(std::make_shared<QJsonDataNumber>(n))
{
}

QJsonValue::QJsonValue(qint64 n)
   : m_data(std::make_shared<QJsonDataNumber>(n))
{
}

QJsonValue::QJsonValue(QString str)
   : m_data(std::make_shared<QJsonDataString>(std::move(str)))
{
}

QJsonValue::QJsonValue(QJsonArray array)
{
   m_data = std::move(array.m_array);
}

QJsonValue::QJsonValue(QJsonObject obj)
{
   m_data = std::move(obj.m_object);
}

QJsonValue::~QJsonValue()
{
}

QJsonValue::QJsonValue(const QJsonValue &other)
{
   m_data = other.m_data->clone();
}

QJsonValue &QJsonValue::operator =(const QJsonValue &other)
{
   m_data = other.m_data->clone();
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

      case QVariant::Hash:
         return QJsonValue(QJsonObject::fromVariantHash(variant.toHash()));

      case QVariant::Map:
         return QJsonValue(QJsonObject::fromVariantMap(variant.toMap()));

      case QVariant::JsonValue:
         return variant.toJsonValue();

      case QVariant::JsonObject:
         return variant.toJsonObject();

      case QVariant::JsonArray:
         return variant.toJsonArray();

      case QVariant::JsonDocument: {
         QJsonDocument doc = variant.toJsonDocument();
         return doc.isArray() ? QJsonValue(doc.array()) : QJsonValue(doc.object());
      }

      default:
         break;
   }

   QString string = variant.toString();

   if (string.isEmpty()) {
      return QJsonValue();
   }

   return QJsonValue(string);
}

QVariant QJsonValue::toVariant() const
{
   switch (type()) {
      case Type::Bool:
         return toBool();

      case Type::Double:
         return toDouble();

      case Type::String:
         return toString();

      case Type::Array:
         return toArray().toVariantList();

      case Type::Object:
         return toObject().toVariantMap();

      default:
         break;
   }

   return QVariant();
}

QJsonValue::Type QJsonValue::type() const
{
   return m_data->type();
}

bool QJsonValue::toBool(bool defaultValue) const
{
   return m_data->toBool(defaultValue);
}

int QJsonValue::toInt(int defaultValue) const
{
   return m_data->toInt(defaultValue);
}

double QJsonValue::toDouble(double defaultValue) const
{
   return m_data->toDouble(defaultValue);
}

QString QJsonValue::toString(const QString &defaultValue) const
{
   return m_data->toString(defaultValue);
}

QJsonArray QJsonValue::toArray() const
{
   return m_data->toArray(QJsonArray());
}

QJsonArray QJsonValue::toArray(const QJsonArray &defaultValue) const
{
   return m_data->toArray(defaultValue);
}

QJsonObject QJsonValue::toObject() const
{
   return m_data->toObject(QJsonObject());
}

QJsonObject QJsonValue::toObject(const QJsonObject &defaultValue) const
{
   return m_data->toObject(defaultValue);
}

bool QJsonValue::operator==(const QJsonValue &other) const
{
   if (type() != other.type()) {
      return false;
   }

   switch (type()) {
      case Undefined:
      case Null:
         break;

      case Bool:
         return toBool() == other.toBool();

      case Double:
         return toDouble() == other.toDouble();

      case String:
         return toString() == other.toString();

      case Array:
         return toArray() == other.toArray();

      case Object:
         return toObject() == other.toObject();
   }

   return true;
}

bool QJsonValue::operator!=(const QJsonValue &other) const
{
   return !(*this == other);
}
