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

#include <qglobal.h>
#include <qatomic.h>

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

   inline void detach() {
      if (d && d->ref.load() != 1) {
         detach_helper();
      }
   }

   inline T &operator*() {
      detach();
      return *d;
   }

   inline const T &operator*() const {
      return *d;
   }

   inline T *operator->() {
      detach();
      return d;
   }

   inline const T *operator->() const {
      return d;
   }

   inline operator T *() {
      detach();
      return d;
   }

   inline operator const T *() const {
      return d;
   }

   inline T *data() {
      detach();
      return d;
   }

   inline const T *data() const {
      return d;
   }

   inline const T *constData() const {
      return d;
   }

   inline bool operator==(const QSharedDataPointer<T> &other) const {
      return d == other.d;
   }

   inline bool operator!=(const QSharedDataPointer<T> &other) const {
      return d != other.d;
   }

   inline QSharedDataPointer() {
      d = nullptr;
   }

   inline ~QSharedDataPointer() {
      if (d && ! d->ref.deref()) {
         delete d;
      }
   }

   explicit QSharedDataPointer(T *data);

   inline QSharedDataPointer(const QSharedDataPointer<T> &other)
      : d(other.d)
   {
      if (d) {
         d->ref.ref();
      }
   }

   inline QSharedDataPointer<T> &operator=(const QSharedDataPointer<T> &other) {
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

   inline QSharedDataPointer &operator=(T *other) {
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

   QSharedDataPointer(QSharedDataPointer && other)
      : d(other.d) {
      other.d = nullptr;
   }

   inline QSharedDataPointer<T> &operator=(QSharedDataPointer<T> && other) {
      qSwap(d, other.d);
      return *this;
   }

   inline bool operator!() const {
      return ! d;
   }

   inline void swap(QSharedDataPointer &other) {
      qSwap(d, other.d);
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
   typedef T Type;
   typedef T *pointer;

   inline T &operator*() const {
      return *d;
   }

   inline T *operator->() {
      return d;
   }

   inline T *operator->() const {
      return d;
   }

   inline T *data() const {
      return d;
   }

   inline const T *constData() const {
      return d;
   }

   inline void detach() {
      if (d && d->ref.load() != 1) {
         detach_helper();
      }
   }

   inline void reset() {
      if (d && !d->ref.deref()) {
         delete d;
      }

      d = nullptr;
   }

   inline operator bool () const {
      return d != nullptr;
   }

   inline bool operator==(const QExplicitlySharedDataPointer<T> &other) const {
      return d == other.d;
   }

   inline bool operator!=(const QExplicitlySharedDataPointer<T> &other) const {
      return d != other.d;
   }

   inline bool operator==(const T *ptr) const {
      return d == ptr;
   }

   inline bool operator!=(const T *ptr) const {
      return d != ptr;
   }

   inline QExplicitlySharedDataPointer() {
      d = nullptr;
   }

   inline ~QExplicitlySharedDataPointer() {
      if (d && ! d->ref.deref()) {
         delete d;
      }
   }

   explicit QExplicitlySharedDataPointer(T *data);

   inline QExplicitlySharedDataPointer(const QExplicitlySharedDataPointer<T> &other)
      : d(other.d)
   {
      if (d) {
         d->ref.ref();
      }
   }

   template <class X>
   inline QExplicitlySharedDataPointer(const QExplicitlySharedDataPointer<X> &other)
      : d(static_cast<T *>(other.data()))
   {
      if (d) {
         d->ref.ref();
      }
   }

   inline QExplicitlySharedDataPointer<T> &operator=(const QExplicitlySharedDataPointer<T> &other) {
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

   inline QExplicitlySharedDataPointer &operator=(T *other) {
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

   inline QExplicitlySharedDataPointer(QExplicitlySharedDataPointer &&other)
      : d(other.d)
   {
      other.d = nullptr;
   }

   inline QExplicitlySharedDataPointer<T> &operator=(QExplicitlySharedDataPointer<T> &&other) {
      qSwap(d, other.d);
      return *this;
   }

   inline bool operator!() const {
      return ! d;
   }

   inline void swap(QExplicitlySharedDataPointer &other) {
      qSwap(d, other.d);
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
