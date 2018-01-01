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

#ifndef QJSONARRAY_H
#define QJSONARRAY_H

#include <qjsonvalue.h>

class QDebug;
class QStringList;

template <typename T> class QList;
typedef QList<QVariant> QVariantList;

class Q_CORE_EXPORT QJsonArray
{
 public:

   using size_type       = int;
   using value_type      = QJsonValue;
   using pointer         = value_type *;
   using const_pointer   = const value_type *;
   using reference       = QJsonValueRef ;
   using const_reference = QJsonValue;
   using difference_type = int;

   QJsonArray();
   ~QJsonArray();

   QJsonArray(const QJsonArray &other);
   QJsonArray &operator =(const QJsonArray &other);

   static QJsonArray fromStringList(const QStringList &list);
   static QJsonArray fromVariantList(const QVariantList &list);
   QVariantList toVariantList() const;

   int size() const;
   int count() const {
      return size();
   }

   bool isEmpty() const;
   QJsonValue at(int i) const;
   QJsonValue first() const;
   QJsonValue last() const;

   void prepend(const QJsonValue &value);
   void append(const QJsonValue &value);
   void removeAt(int i);
   QJsonValue takeAt(int i);

   void removeFirst() {
      removeAt(0);
   }

   void removeLast() {
      removeAt(size() - 1);
   }

   void insert(int i, const QJsonValue &value);
   void replace(int i, const QJsonValue &value);

   bool contains(const QJsonValue &element) const;

   QJsonValueRef operator[](int i);
   QJsonValue operator[](int i) const;

   bool operator==(const QJsonArray &other) const;
   bool operator!=(const QJsonArray &other) const;

   QJsonArray operator+(const QJsonValue &v) const
   {
      QJsonArray n = *this;
      n += v; return n;
   }

   QJsonArray &operator+=(const QJsonValue &v)
   {
      append(v);
      return *this;
   }

   QJsonArray &operator<< (const QJsonValue &v)
   {
      append(v);
      return *this;
   }

   class const_iterator;

   class iterator
   {
    public:
      QJsonArray *a;
      int i;

      typedef std::random_access_iterator_tag  iterator_category;
      typedef int difference_type;
      typedef QJsonValue value_type;
      typedef QJsonValueRefPtr pointer;
      typedef QJsonValueRef reference;

      inline iterator() : a(0), i(0) { }
      explicit inline iterator(QJsonArray *array, int index) : a(array), i(index) { }

      inline QJsonValueRef operator*() const {
         return QJsonValueRef(a, i);
      }

      inline QJsonValueRefPtr operator->() const {
         return QJsonValueRefPtr(a, i);
      }

      inline QJsonValueRef operator[](int n) const {
         return QJsonValueRef(a, i + n);
      }

      inline bool operator==(const iterator &other) const {
         return i == other.i;
      }

      inline bool operator!=(const iterator &other) const {
         return i != other.i;
      }

      inline bool operator<(const iterator &other) const {
         return i < other.i;
      }

      inline bool operator<=(const iterator &other) const {
         return i <= other.i;
      }

      inline bool operator>(const iterator &other) const {
         return i > other.i;
      }

      inline bool operator>=(const iterator &other) const {
         return i >= other.i;
      }

      inline bool operator==(const const_iterator &other) const {
         return i == other.i;
      }

      inline bool operator!=(const const_iterator &other) const {
         return i != other.i;
      }

      inline bool operator<(const const_iterator &other) const {
         return i < other.i;
      }

      inline bool operator<=(const const_iterator &other) const {
         return i <= other.i;
      }

      inline bool operator>(const const_iterator &other) const {
         return i > other.i;
      }

      inline bool operator>=(const const_iterator &other) const {
         return i >= other.i;
      }

      inline iterator &operator++() {
         ++i;
         return *this;
      }

      inline iterator operator++(int) {
         iterator n = *this;
         ++i;
         return n;
      }

      inline iterator &operator--() {
         i--;
         return *this;
      }

      inline iterator operator--(int) {
         iterator n = *this;
         i--;
         return n;
      }

      inline iterator &operator+=(int n) {
         i += n;
         return *this;
      }

      inline iterator &operator-=(int n) {
         i -= n;
         return *this;
      }

      inline iterator operator+(int n) const {
         return iterator(a, i + n);
      }

      inline iterator operator-(int n) const {
         return iterator(a, i - n);
      }

      inline int operator-(iterator n) const {
         return i - n.i;
      }
   };

   friend class iterator;

   class const_iterator
   {
    public:
      const QJsonArray *a;
      int i;
      typedef std::random_access_iterator_tag  iterator_category;
      typedef qptrdiff difference_type;
      typedef QJsonValue value_type;
      typedef QJsonValuePtr pointer;
      typedef QJsonValue reference;

      inline const_iterator() : a(0), i(0) { }
      explicit inline const_iterator(const QJsonArray *array, int index) : a(array), i(index) { }
      inline const_iterator(const const_iterator &other) : a(other.a), i(other.i) {}
      inline const_iterator(const iterator &other) : a(other.a), i(other.i) {}

      inline QJsonValue operator*() const {
         return a->at(i);
      }

      inline QJsonValuePtr operator->() const {
         return QJsonValuePtr(a->at(i));
      }

      inline QJsonValue operator[](int j) const {
         return a->at(i + j);
      }

      inline bool operator==(const const_iterator &other) const {
         return i == other.i;
      }

      inline bool operator!=(const const_iterator &other) const {
         return i != other.i;
      }

      inline bool operator<(const const_iterator &other) const {
         return i < other.i;
      }

      inline bool operator<=(const const_iterator &other) const {
         return i <= other.i;
      }

      inline bool operator>(const const_iterator &other) const {
         return i > other.i;
      }

      inline bool operator>=(const const_iterator &other) const {
         return i >= other.i;
      }

      inline const_iterator &operator++() {
         ++i;
         return *this;
      }

      inline const_iterator operator++(int) {
         const_iterator n = *this;
         ++i;
         return n;
      }
      inline const_iterator &operator--() {
         i--;
         return *this;
      }
      inline const_iterator operator--(int) {
         const_iterator n = *this;
         i--;
         return n;
      }
      inline const_iterator &operator+=(int n) {
         i += n;
         return *this;
      }

      inline const_iterator &operator-=(int n) {
         i -= n;
         return *this;
      }

      inline const_iterator operator+(int n) const {
         return const_iterator(a, i + n);
      }

      inline const_iterator operator-(int n) const {
         return const_iterator(a, i - n);
      }

      inline int operator-(const_iterator n) const {
         return i - n.i;
      }
   };

   friend class const_iterator;

   // stl style
   inline iterator begin() {
      detach();
      return iterator(this, 0);
   }

   inline const_iterator begin() const {
      return const_iterator(this, 0);
   }

   inline const_iterator constBegin() const {
      return const_iterator(this, 0);
   }

   inline iterator end() {
      detach();
      return iterator(this, size());
   }

   inline const_iterator end() const {
      return const_iterator(this, size());
   }

   inline const_iterator constEnd() const {
      return const_iterator(this, size());
   }

   iterator insert(iterator before, const QJsonValue &value) {
      insert(before.i, value);
      return before;
   }

   iterator erase(iterator it) {
      removeAt(it.i);
      return it;
   }

   typedef iterator Iterator;
   typedef const_iterator ConstIterator;

   // stl compatibility
   void push_back(const QJsonValue &t) {
      append(t);
   }

   void push_front(const QJsonValue &t) {
      prepend(t);
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
   friend class QJsonPrivate::Data;
   friend class QJsonValue;
   friend class QJsonDocument;
   friend Q_CORE_EXPORT QDebug operator<<(QDebug, const QJsonArray &);

   QJsonArray(QJsonPrivate::Data *data, QJsonPrivate::Array *array);
   void compact();
   void detach(uint reserve = 0);

   QJsonPrivate::Data *d;
   QJsonPrivate::Array *a;
};

Q_CORE_EXPORT QDebug operator<<(QDebug, const QJsonArray &);

#endif
