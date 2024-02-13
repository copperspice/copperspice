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

#include <qjsondocument.h>

#include <qstringlist.h>
#include <qvariant.h>

#include <qjsonparser_p.h>
#include <qjsonwriter_p.h>

QJsonDocument::QJsonDocument()
   : m_data(std::make_shared<QJsonValue>())
{
}

QJsonDocument::QJsonDocument(const QJsonObject &object)
   : m_data(std::make_shared<QJsonValue>(object))
{
}

QJsonDocument::QJsonDocument(const QJsonArray &array)
   : m_data(std::make_shared<QJsonValue>(array))
{
}

QJsonDocument::QJsonDocument(const QJsonDocument &other)
{
   m_data = other.m_data;
}

QJsonDocument::QJsonDocument(QJsonDocument &&other)
{
   m_data = std::move(other.m_data);
}

QJsonDocument &QJsonDocument::operator =(const QJsonDocument &other)
{
   m_data = other.m_data;
   return *this;
}

QJsonDocument::~QJsonDocument()
{
}

QJsonDocument QJsonDocument::fromVariant(const QVariant &variant)
{
   QJsonDocument doc;

   if (variant.type() == QVariant::Map) {
      doc.setObject(QJsonObject::fromVariantMap(variant.toMap()));

   } else if (variant.type() == QVariant::List) {
      doc.setArray(QJsonArray::fromVariantList(variant.toList()));

   } else if (variant.type() == QVariant::StringList) {
      doc.setArray(QJsonArray::fromStringList(variant.toStringList()));
   }

   return doc;
}

QVariant QJsonDocument::toVariant() const
{
   if (m_data->isArray()) {
      return m_data->toArray().toVariantList();

   } else {
      return m_data->toObject().toVariantMap();
   }
}

QString QJsonDocument::toJsonString(JsonFormat format) const
{
   QString retval;

   if (m_data->isArray()) {
      retval = QJsonWriter::arrayToString(m_data->toArray(), 0, format);

   } else if (m_data->isObject()) {
      retval = QJsonWriter::objectToString(m_data->toObject(), 0, format);
   }

   return retval;
}

QJsonDocument QJsonDocument::fromJson(const QByteArray &json, QJsonParseError *error)
{
   QString str = QString::fromUtf8(json);
   return fromJson(str, error);
}

QJsonDocument QJsonDocument::fromJson(QStringView json, QJsonParseError *error)
{
   return QJsonParser(json).parse(error);
}

bool QJsonDocument::isEmpty() const
{
   return m_data->isNull();
}

bool QJsonDocument::isArray() const
{
   return m_data->isArray();
}

bool QJsonDocument::isObject() const
{
   return m_data->isObject();
}

QJsonObject QJsonDocument::object() const
{
   return m_data->toObject();
}

QJsonArray QJsonDocument::array() const
{
   return m_data->toArray();
}

void QJsonDocument::setObject(const QJsonObject &object)
{
   m_data = std::make_shared<QJsonValue>(object);
}

void QJsonDocument::setArray(const QJsonArray &array)
{
   m_data = std::make_shared<QJsonValue>(array);
}

bool QJsonDocument::operator==(const QJsonDocument &other) const
{
   if (*m_data == *other.m_data) {
      return true;
   }

   return false;
}

bool QJsonDocument::isNull() const
{
   return m_data->isNull();
}
