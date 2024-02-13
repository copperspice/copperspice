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

#ifndef QJSONARRAY_H
#define QJSONARRAY_H

#include <qjsonvalue.h>
#include <qstringlist.h>
#include <qvariant.h>

#include <initializer_list>

class QJsonData;
class QJsonDataArray;

class Q_CORE_EXPORT QJsonArray
{
 public:
   using iterator           = QVector<QJsonValue>::iterator;
   using const_iterator     = QVector<QJsonValue>::const_iterator;

   using size_type          = QVector<QJsonValue>::size_type;
   using difference_type    = QVector<QJsonValue>::difference_type;

   using pointer            = QJsonValue *;
   using const_pointer      = const QJsonValue *;

   using reference          = QJsonValue &;
   using const_reference    = QJsonValue;

   QJsonArray();
   QJsonArray(const_iterator iter_begin, const_iterator iter_end);
   QJsonArray(const QJsonArray &other);
   QJsonArray(QJsonArray &&other);

   QJsonArray(std::initializer_list<QJsonValue> args);
   ~QJsonArray();

   static QJsonArray fromStringList(const QStringList &list);
   static QJsonArray fromVariantList(const QList<QVariant> &list);

   // methods
   const QJsonValue &at(size_type index) const;
   void append(QJsonValue value);

   bool contains(const QJsonValue &value) const;

   size_type count() const {
      return size();
   }

   iterator erase(const_iterator iter);
   const QJsonValue &first() const;

   bool isEmpty() const;

   void insert(size_type index, QJsonValue value);
   iterator insert(iterator before, QJsonValue value);

   const QJsonValue &last() const;

   void prepend(QJsonValue value);

   void replace(size_type index, QJsonValue value);
   void removeAt(size_type index);

   void removeFirst() {
      removeAt(0);
   }

   void removeLast() {
      removeAt(size() - 1);
   }

   size_type size() const;

   QJsonValue takeAt(size_type index);
   QList<QVariant> toVariantList() const;

   // operators
   QJsonArray &operator =(const QJsonArray &other);

   QJsonValue &operator[](size_type index);
   const QJsonValue &operator[](size_type index) const;

   bool operator==(const QJsonArray &other) const;
   bool operator!=(const QJsonArray &other) const;

   QJsonArray operator+(QJsonValue value) const {
      QJsonArray retval = *this;
      retval += std::move(value);

      return retval;
   }

   QJsonArray &operator+=(QJsonValue value);
   QJsonArray &operator<< (const QJsonValue value);

   // iterators
   iterator begin();
   const_iterator begin() const;
   const_iterator constBegin() const;

   iterator end();
   const_iterator end() const;
   const_iterator constEnd() const;

   // methods
   void push_back(const QJsonValue &value) {
      append(value);
   }

   void push_front(const QJsonValue &value) {
      prepend(value);
   }

   void pop_front() {
      removeFirst();
   }

   void pop_back() {
      removeLast();
   }

   bool empty() const {
      return isEmpty();
   }

 private:
   friend class QJsonValue;
   std::shared_ptr<QJsonDataArray> m_array;
};

#endif
