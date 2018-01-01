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

#ifndef QJSONOBJECT_H
#define QJSONOBJECT_H

#include <qjsonvalue.h>
#include <qcontainerfwd.h>

class QDebug;
typedef QMap<QString, QVariant> QVariantMap;

class Q_CORE_EXPORT QJsonObject
{
 public:
   typedef QJsonValue mapped_type;
   typedef QString key_type;
   typedef int size_type;

   QJsonObject();
   ~QJsonObject();

   QJsonObject(const QJsonObject &other);
   QJsonObject &operator =(const QJsonObject &other);

   static QJsonObject fromVariantMap(const QVariantMap &map);
   QVariantMap toVariantMap() const;

   QStringList keys() const;
   int size() const;

   int count() const {
      return size();
   }

   int length() const {
      return size();
   }
   bool isEmpty() const;

   QJsonValue value(const QString &key) const;
   QJsonValue operator[] (const QString &key) const;
   QJsonValueRef operator[] (const QString &key);

   void remove(const QString &key);
   QJsonValue take(const QString &key);
   bool contains(const QString &key) const;

   bool operator==(const QJsonObject &other) const;
   bool operator!=(const QJsonObject &other) const;

   class const_iterator;

   class iterator
   {
      friend class const_iterator;
      friend class QJsonObject;
      QJsonObject *o;
      int i;

    public:
      typedef std::bidirectional_iterator_tag iterator_category;
      typedef int difference_type;
      typedef QJsonValue value_type;
      typedef QJsonValueRefPtr pointer;
      typedef QJsonValueRef reference;

      constexpr inline iterator() : o(0), i(0) {}
      constexpr inline iterator(QJsonObject *obj, int index) : o(obj), i(index) {}

      inline QString key() const {
         return o->keyAt(i);
      }

      inline QJsonValueRef value() const {
         return QJsonValueRef(o, i);
      }

      inline QJsonValueRef operator*() const {
         return QJsonValueRef(o, i);
      }

      inline QJsonValueRefPtr operator->() const {
        return QJsonValueRefPtr(o, i);
      }

      inline bool operator==(const iterator &other) const {
         return i == other.i;
      }
      inline bool operator!=(const iterator &other) const {
         return i != other.i;
      }

      inline iterator &operator++() {
         ++i;
         return *this;
      }
      inline iterator operator++(int) {
         iterator r = *this;
         ++i;
         return r;
      }
      inline iterator &operator--() {
         --i;
         return *this;
      }
      inline iterator operator--(int) {
         iterator r = *this;
         --i;
         return r;
      }

      inline iterator operator+(int n) const {
         iterator r = *this;
         r.i += n;
         return r;
      }

      inline iterator operator-(int n) const {
         return operator+(-n);
      }

      inline iterator &operator+=(int n) {
         i += n;
         return *this;
      }

      inline iterator &operator-=(int n) {
         i -= n;
         return *this;
      }

    public:
      bool operator==(const const_iterator &other) const {
         return i == other.i;
      }

      bool operator!=(const const_iterator &other) const {
         return i != other.i;
      }
   };
   friend class iterator;

   class const_iterator
   {
      friend class iterator;
      const QJsonObject *o;
      int i;

    public:
      typedef std::bidirectional_iterator_tag iterator_category;
      typedef int difference_type;
      typedef QJsonValue*   value_type;
      typedef QJsonValuePtr pointer;
      typedef QJsonValue    reference;

      constexpr const_iterator() : o(0), i(0) {}

      constexpr const_iterator(const QJsonObject *obj, int index)
         : o(obj), i(index) { }

      const_iterator(const iterator &other)
         : o(other.o), i(other.i) { }

      QString key() const {
         return o->keyAt(i);
      }

      QJsonValue value() const {
         return o->valueAt(i);
      }

      QJsonValue operator*() const {
         return o->valueAt(i);
      }

      QJsonValuePtr operator->() const {
         return  QJsonValuePtr(o->valueAt(i));
      }

      bool operator==(const const_iterator &other) const {
         return i == other.i;
      }

      bool operator!=(const const_iterator &other) const {
         return i != other.i;
      }

      const_iterator &operator++() {
         ++i;
         return *this;
      }

      const_iterator operator++(int) {
         const_iterator r = *this;
         ++i;
         return r;
      }

      const_iterator &operator--() {
         --i;
         return *this;
      }

      const_iterator operator--(int) {
         const_iterator r = *this;
         --i;
         return r;
      }

      const_iterator operator+(int n) const {
         const_iterator r = *this;
         r.i += n;
         return r;
      }

      const_iterator operator-(int n) const {
         return operator+(-n);
      }

      const_iterator &operator+=(int n) {
         i += n;
         return *this;
      }

      const_iterator &operator-=(int n) {
         i -= n;
         return *this;
      }

      bool operator==(const iterator &other) const {
         return i == other.i;
      }

      bool operator!=(const iterator &other) const {
         return i != other.i;
      }
   };

   friend class const_iterator;

   // STL style
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
   iterator erase(iterator it);

   typedef iterator Iterator;
   typedef const_iterator ConstIterator;

   iterator find(const QString &key);

   const_iterator find(const QString &key) const {
      return constFind(key);
   }

   const_iterator constFind(const QString &key) const;
   iterator insert(const QString &key, const QJsonValue &value);

   inline bool empty() const {
      return isEmpty();
   }

 private:
   friend class QJsonPrivate::Data;
   friend class QJsonValue;
   friend class QJsonDocument;
   friend class QJsonValueRef;

   friend Q_CORE_EXPORT QDebug operator<<(QDebug, const QJsonObject &);

   QJsonObject(QJsonPrivate::Data *data, QJsonPrivate::Object *object);
   void detach(uint reserve = 0);
   void compact();

   QString keyAt(int i) const;
   QJsonValue valueAt(int i) const;
   void setValueAt(int i, const QJsonValue &val);

   QJsonPrivate::Data *d;
   QJsonPrivate::Object *o;
};

Q_CORE_EXPORT QDebug operator<<(QDebug, const QJsonObject &);

#endif