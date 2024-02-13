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

#include <qjsonarray.h>

#include <qjson.h>

#include <qjsonwriter_p.h>

QJsonArray::QJsonArray()
{
   m_array = std::make_shared<QJsonDataArray>();
}

QJsonArray::QJsonArray(const_iterator iter_begin, const_iterator iter_end)
{
   m_array = std::make_shared<QJsonDataArray>();
   m_array->m_vector = QVector<QJsonValue>(iter_begin, iter_end);
}

QJsonArray::QJsonArray(const QJsonArray &other)
{
   m_array = std::make_shared<QJsonDataArray>(*other.m_array);
}

QJsonArray::QJsonArray(QJsonArray &&other)
{
   m_array = std::move(other.m_array);
}

QJsonArray::QJsonArray(std::initializer_list<QJsonValue> args)
{

   m_array = std::make_shared<QJsonDataArray>();

   for (const auto &item : args) {
      append(item);
   }
}

QJsonArray::~QJsonArray()
{
}

void QJsonArray::append(QJsonValue value)
{
   m_array->m_vector.append(std::move(value));
}

QJsonArray &QJsonArray::operator =(const QJsonArray &other)
{
   *m_array = *other.m_array;

   return *this;
}

const QJsonValue &QJsonArray::at(size_type index) const
{
   return m_array->m_vector.at(index);
}

bool QJsonArray::contains(const QJsonValue &value) const
{
   return m_array->m_vector.contains(value);
}

QJsonArray::iterator QJsonArray::erase(const_iterator iter)
{
   return m_array->m_vector.erase(iter);
}

const QJsonValue &QJsonArray::first() const
{
   return m_array->m_vector.first();
}

void QJsonArray::insert(size_type index, QJsonValue value)
{
   return m_array->m_vector.insert(index, std::move(value));
}

QJsonArray::iterator QJsonArray::insert(iterator before, QJsonValue value)
{
   return m_array->m_vector.insert(before, std::move(value));
}

bool QJsonArray::isEmpty() const
{
   return ! m_array->m_vector.size();
}

const QJsonValue &QJsonArray::last() const
{
   return m_array->m_vector.last();
}

void QJsonArray::prepend(QJsonValue value)
{
   m_array->m_vector.append(std::move(value));
}

void QJsonArray::removeAt(size_type index)
{
   m_array->m_vector.removeAt(index);
}

QJsonArray::size_type QJsonArray::size() const
{
   return m_array->m_vector.size();
}

QJsonValue QJsonArray::takeAt(size_type index)
{
   return m_array->m_vector.takeAt(index);
}

QJsonArray QJsonArray::fromStringList(const QStringList &list)
{
   QJsonArray array;

   for (const auto &item : list) {
      array.append(QJsonValue(item));
   }

   return array;
}

QJsonArray QJsonArray::fromVariantList(const QList<QVariant> &list)
{
   QJsonArray array;

   for (const auto &item : list) {
      array.append(QJsonValue::fromVariant(item));
   }

   return array;
}

QList<QVariant> QJsonArray::toVariantList() const
{
   QList<QVariant> list;

   for (const auto &item : *this) {
      list.append(item.toVariant());
   }

   return list;
}

void QJsonArray::replace(size_type index, QJsonValue value)
{
   m_array->m_vector.replace(index, std::move(value));
}

QJsonValue &QJsonArray::operator [](size_type index)
{
   return m_array->m_vector[index];
}

const QJsonValue &QJsonArray::operator[](size_type index) const
{
   return m_array->m_vector[index];
}

bool QJsonArray::operator==(const QJsonArray &other) const
{
   return m_array->m_vector == other.m_array->m_vector;
}

bool QJsonArray::operator!=(const QJsonArray &other) const
{
   return !(*this == other);
}

QJsonArray &QJsonArray::operator+=(QJsonValue value)
{
   m_array->m_vector.append(std::move(value));
   return *this;
}

QJsonArray &QJsonArray::operator<< (const QJsonValue value)
{
   m_array->m_vector.append(std::move(value));
   return *this;
}

// iterators
QJsonArray::iterator QJsonArray::begin()
{
   return m_array->m_vector.begin();
}

QJsonArray::const_iterator QJsonArray::begin() const
{
   return m_array->m_vector.begin();
}

QJsonArray::const_iterator QJsonArray::constBegin() const
{
   return m_array->m_vector.constBegin();
}

QJsonArray::iterator QJsonArray::end()
{
   return m_array->m_vector.end();
}

QJsonArray::const_iterator QJsonArray::end() const
{
   return m_array->m_vector.end();
}

QJsonArray::const_iterator QJsonArray::constEnd() const
{
   return m_array->m_vector.constEnd();
}
