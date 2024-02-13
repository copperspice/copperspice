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

#include <qjsonobject.h>

#include <qjson.h>

#include <qjsonwriter_p.h>

QJsonObject::QJsonObject()
{
   m_object = std::make_shared<QJsonDataObject>();
}

QJsonObject::QJsonObject(const_iterator iter_begin, const_iterator iter_end)
{
   m_object = std::make_shared<QJsonDataObject>();
   m_object->m_map = QFlatMap<QString, QJsonValue>(iter_begin, iter_end);
}

QJsonObject::QJsonObject(std::initializer_list<QPair<QString, QJsonValue>> list)
{
   m_object = std::make_shared<QJsonDataObject>();
   m_object->m_map = QFlatMap<QString, QJsonValue>(list.begin(), list.end());
}

QJsonObject::QJsonObject(const QJsonObject &other)
{
   m_object = std::make_shared<QJsonDataObject>(*other.m_object);
}

QJsonObject::QJsonObject(QJsonObject &&other)
{
   m_object = std::move(other.m_object);
}

QJsonObject::~QJsonObject()
{
}

QJsonObject &QJsonObject::operator =(const QJsonObject &other)
{
   *m_object = *other.m_object;

   return *this;
}

QJsonObject QJsonObject::fromVariantHash(const QVariantHash &hash)
{
   QJsonObject object;

   for (QVariantHash::const_iterator iter = hash.constBegin(); iter != hash.constEnd(); ++iter) {
      object.insert(iter.key(), QJsonValue::fromVariant(iter.value()));
   }

   return object;
}

QVariantHash QJsonObject::toVariantHash() const
{
   QVariantHash hash;

   for (QJsonObject::const_iterator iter = this->constBegin(); iter != this->constEnd(); ++iter) {
      hash.insert(iter.key(), iter.value().toVariant());
   }

   return hash;
}

QJsonObject QJsonObject::fromVariantMap(const QVariantMap &map)
{
   QJsonObject object;

   for (QVariantMap::const_iterator iter = map.constBegin(); iter != map.constEnd(); ++iter) {
      object.insert(iter.key(), QJsonValue::fromVariant(iter.value()));
   }

   return object;
}

QVariantMap QJsonObject::toVariantMap() const
{
   QVariantMap map;

   for (QJsonObject::const_iterator iter = this->constBegin(); iter != this->constEnd(); ++iter) {
      map.insert(iter.key(), iter.value().toVariant());
   }

   return map;
}

QStringList QJsonObject::keys() const
{
   return m_object->m_map.keys();
}

QJsonObject::size_type QJsonObject::size() const
{
   return m_object->m_map.size();
}

bool QJsonObject::isEmpty() const
{
   return ! m_object->m_map.size();
}

const QJsonValue &QJsonObject::value(const QString &key) const
{
   return m_object->m_map[key];
}

const QJsonValue &QJsonObject::operator [](const QString &key) const
{
   return m_object->m_map[key];
}

QJsonValue &QJsonObject::operator [](const QString &key)
{
   return m_object->m_map[key];
}

QJsonObject::iterator QJsonObject::insert(const QString &key, QJsonValue value)
{
   if (value.type() == QJsonValue::Undefined) {
      remove(key);
      return end();
   }

   return m_object->m_map.insert(key, std::move(value));
}

void QJsonObject::remove(const QString &key)
{
   m_object->m_map.remove(key);
}

QJsonValue QJsonObject::take(const QString &key)
{
   return m_object->m_map.take(key);
}

bool QJsonObject::contains(const QString &key) const
{
   return m_object->m_map.contains(key);
}

bool QJsonObject::operator==(const QJsonObject &other) const
{
   return m_object->m_map == other.m_object->m_map;
}

bool QJsonObject::operator!=(const QJsonObject &other) const
{
   return !(*this == other);
}

QJsonObject::iterator QJsonObject::erase(QJsonObject::const_iterator iter)
{
   return m_object->m_map.erase(iter);
}

QJsonObject::iterator QJsonObject::find(const QString &key)
{
   return m_object->m_map.find(key);
}

QJsonObject::const_iterator QJsonObject::constFind(const QString &key) const
{
   return m_object->m_map.find(key);
}

// iterators
QJsonObject::iterator QJsonObject::begin()
{
   return m_object->m_map.begin();
}

QJsonObject::const_iterator QJsonObject::begin() const
{
   return m_object->m_map.begin();
}

QJsonObject::const_iterator QJsonObject::constBegin() const
{
   return m_object->m_map.constBegin();
}

QJsonObject::iterator QJsonObject::end()
{
   return m_object->m_map.end();
}

QJsonObject::const_iterator QJsonObject::end() const
{
   return m_object->m_map.end();
}

QJsonObject::const_iterator QJsonObject::constEnd() const
{
   return m_object->m_map.constEnd();
}
