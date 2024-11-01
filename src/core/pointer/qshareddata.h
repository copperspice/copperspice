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

#ifndef QSHAREDDATA_H
#define QSHAREDDATA_H

#include <qatomic.h>
#include <qglobal.h>

template <class T>
class QSharedDataPointer;

class Q_CORE_EXPORT QSharedData
{
 public:
   QSharedData()
      : ref(0)
   {
   }

   QSharedData(const QSharedData &other)
      : ref(0)
   {
      (void) other;
   }

   mutable QAtomicInt ref;

 private:
   // using the assignment operator would lead to corruption in the ref-counting
   QSharedData &operator=(const QSharedData &);
};

template <class T>
class QSharedDataPointer
{
 public:
   using Type    = T;
   using pointer = T *;

   QSharedDataPointer() {
      d = nullptr;
   }

   explicit QSharedDataPointer(T *data);

   QSharedDataPointer(const QSharedDataPointer<T> &other)
      : d(other.d)
   {
      if (d) {
         d->ref.ref();
      }
   }

   QSharedDataPointer(QSharedDataPointer &&other)
      : d(other.d) {
      other.d = nullptr;
   }

   ~QSharedDataPointer() {
      if (d && ! d->ref.deref()) {
         delete d;
      }
   }

   // methods
   T *data() {
      detach();
      return d;
   }

   const T *data() const {
      return d;
   }

   const T *constData() const {
      return d;
   }

   void detach() {
      if (d && d->ref.load() != 1) {
         detach_helper();
      }
   }

   void swap(QSharedDataPointer &other) {
      qSwap(d, other.d);
   }

   // operators
   QSharedDataPointer &operator=(T *other) {
      if (other != d) {
         if (other) {
            other->ref.ref();
         }

         T *old = d;
         d = other;

         if (old && ! old->ref.deref()) {
            delete old;
         }
      }

      return *this;
   }

   QSharedDataPointer<T> &operator=(QSharedDataPointer<T> &&other) {
      qSwap(d, other.d);
      return *this;
   }

   QSharedDataPointer<T> &operator=(const QSharedDataPointer<T> &other) {
      if (other.d != d) {
         if (other.d) {
            other.d->ref.ref();
         }

         T *old = d;
         d = other.d;

         if (old && !old->ref.deref()) {
            delete old;
         }
      }

      return *this;
   }

   bool operator==(const QSharedDataPointer<T> &other) const {
      return d == other.d;
   }

   bool operator!=(const QSharedDataPointer<T> &other) const {
      return d != other.d;
   }

   T &operator*() {
      detach();
      return *d;
   }

   const T &operator*() const {
      return *d;
   }

   T *operator->() {
      detach();
      return d;
   }

   const T *operator->() const {
      return d;
   }

   operator T *() {
      detach();
      return d;
   }

   operator const T *() const {
      return d;
   }

   bool operator!() const {
      return ! d;
   }

 protected:
   T *clone();

 private:
   void detach_helper();

   T *d;
};

template <class T>
class QExplicitlySharedDataPointer
{
 public:
   using Type    = T;
   using pointer = T *;

   QExplicitlySharedDataPointer() {
      d = nullptr;
   }

   explicit QExplicitlySharedDataPointer(T *data);

   QExplicitlySharedDataPointer(const QExplicitlySharedDataPointer<T> &other)
      : d(other.d)
   {
      if (d) {
         d->ref.ref();
      }
   }

   template <class X>
   QExplicitlySharedDataPointer(const QExplicitlySharedDataPointer<X> &other)
      : d(static_cast<T *>(other.data())) {
      if (d) {
         d->ref.ref();
      }
   }

   QExplicitlySharedDataPointer(QExplicitlySharedDataPointer &&other)
      : d(other.d)
   {
      other.d = nullptr;
   }

   ~QExplicitlySharedDataPointer() {
      if (d && ! d->ref.deref())
   {
         delete d;
      }
   }

   // methods
   T *data() const {
      return d;
   }

   const T *constData() const {
      return d;
   }

   void detach() {
      if (d && d->ref.load() != 1) {
         detach_helper();
      }
   }

   void reset() {
      if (d && !d->ref.deref()) {
         delete d;
      }

      d = nullptr;
   }

   void swap(QExplicitlySharedDataPointer &other) {
      qSwap(d, other.d);
   }

   // operators
   QExplicitlySharedDataPointer<T> &operator=(const QExplicitlySharedDataPointer<T> &other) {
      if (other.d != d) {
         if (other.d) {
            other.d->ref.ref();
         }

         T *old = d;
         d = other.d;

         if (old && ! old->ref.deref()) {
            delete old;
         }
      }

      return *this;
   }

   QExplicitlySharedDataPointer<T> &operator=(QExplicitlySharedDataPointer<T> &&other) {
      qSwap(d, other.d);
      return *this;
   }

   QExplicitlySharedDataPointer &operator=(T *other) {
      if (other != d) {
         if (other) {
            other->ref.ref();
         }

         T *old = d;
         d = other;

         if (old && ! old->ref.deref()) {
            delete old;
         }
      }

      return *this;
   }

   T &operator*() const {
      return *d;
   }

   T *operator->() {
      return d;
   }

   T *operator->() const {
      return d;
   }

   operator bool () const {
      return d != nullptr;
   }

   bool operator==(const QExplicitlySharedDataPointer<T> &other) const {
      return d == other.d;
   }

   bool operator!=(const QExplicitlySharedDataPointer<T> &other) const {
      return d != other.d;
   }

   bool operator==(const T *ptr) const {
      return d == ptr;
   }

   bool operator!=(const T *ptr) const {
      return d != ptr;
   }

   bool operator!() const {
      return ! d;
   }

 protected:
   T *clone();

 private:
   void detach_helper();

   T *d;
};

template <class T>
inline QSharedDataPointer<T>::QSharedDataPointer(T *data)
   : d(data)
{
   if (d) {
      d->ref.ref();
   }
}

template <class T>
inline T *QSharedDataPointer<T>::clone()
{
   return new T(*d);
}

template <class T>
void QSharedDataPointer<T>::detach_helper()
{
   T *x = clone();
   x->ref.ref();

   if (! d->ref.deref()) {
      delete d;
   }

   d = x;
}

template <class T>
inline T *QExplicitlySharedDataPointer<T>::clone()
{
   return new T(*d);
}

template <class T>
void QExplicitlySharedDataPointer<T>::detach_helper()
{
   T *x = clone();
   x->ref.ref();

   if (!d->ref.deref()) {
      delete d;
   }

   d = x;
}

template <class T>
inline QExplicitlySharedDataPointer<T>::QExplicitlySharedDataPointer(T *data)
   : d(data)
{
   if (d) {
      d->ref.ref();
   }
}

template <class T>
inline void qSwap(QSharedDataPointer<T> &p1, QSharedDataPointer<T> &p2)
{
   p1.swap(p2);
}

template <class T>
inline void qSwap(QExplicitlySharedDataPointer<T> &p1, QExplicitlySharedDataPointer<T> &p2)
{
   p1.swap(p2);
}

template <class T>
inline void swap(QSharedDataPointer<T> &p1, QSharedDataPointer<T> &p2)
{
   p1.swap(p2);
}

template <class T>
inline void swap(QExplicitlySharedDataPointer<T> &p1, QExplicitlySharedDataPointer<T> &p2)
{
   p1.swap(p2);
}

#endif
