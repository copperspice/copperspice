/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QSCOPEDPOINTER_H
#define QSCOPEDPOINTER_H

#include <qglobal.h>
#include <qassert.h>

template <typename T>
struct QScopedPointerDeleter {
   static inline void cleanup(T *pointer) {
      // Enforce a complete type. If you get a compile error here, read the section on
      // forward declared classes in the QScopedPointer documentation.

      typedef char IsIncompleteType[ sizeof(T) ? 1 : -1 ];
      (void) sizeof(IsIncompleteType);

      delete pointer;
   }
};

template <typename T>
struct QScopedPointerArrayDeleter {
   static inline void cleanup(T *pointer) {
      // Enforce a complete type.
      // If you get a compile error here, read the section on forward declared
      // classes in the QScopedPointer documentation.
      typedef char IsIncompleteType[ sizeof(T) ? 1 : -1 ];
      (void) sizeof(IsIncompleteType);

      delete [] pointer;
   }
};

struct QScopedPointerPodDeleter {
   static inline void cleanup(void *pointer) {
      if (pointer) {
         qFree(pointer);
      }
   }
};

template <typename T, typename Cleanup = QScopedPointerDeleter<T> >
class QScopedPointer
{
   typedef T *QScopedPointer::*RestrictedBool;

 public:
   explicit inline QScopedPointer(T *p = 0) : d(p) {
   }

   inline ~QScopedPointer() {
      T *oldD = this->d;
      Cleanup::cleanup(oldD);
   }

   inline QScopedPointer(QScopedPointer<T, Cleanup> &&other)
      : d(other.take()) {
   }

   inline QScopedPointer<T, Cleanup> &operator=(QScopedPointer<T, Cleanup> && other) {
      reset(other.take());
      return *this;
   }

   inline T &operator*() const {
      Q_ASSERT(d);
      return *d;
   }

   inline T *operator->() const {
      Q_ASSERT(d);
      return d;
   }

   inline bool operator!() const {
      return !d;
   }

   inline operator RestrictedBool() const {
      return isNull() ? 0 : &QScopedPointer::d;
   }

   inline T *data() const {
      return d;
   }

   inline bool isNull() const {
      return !d;
   }

   inline void reset(T *other = 0) {
      if (d == other) {
         return;
      }
      T *oldD = d;
      d = other;
      Cleanup::cleanup(oldD);
   }

   inline T *take() {
      T *oldD = d;
      d = 0;
      return oldD;
   }

   inline void swap(QScopedPointer<T, Cleanup> &other) {
      qSwap(d, other.d);
   }

   typedef T *pointer;

 protected:
   T *d;

 private:
   Q_DISABLE_COPY(QScopedPointer)
};

template <class T, class Cleanup>
inline bool operator==(const QScopedPointer<T, Cleanup> &lhs, const QScopedPointer<T, Cleanup> &rhs)
{
   return lhs.data() == rhs.data();
}

template <class T, class Cleanup>
inline bool operator!=(const QScopedPointer<T, Cleanup> &lhs, const QScopedPointer<T, Cleanup> &rhs)
{
   return lhs.data() != rhs.data();
}

template <class T, class Cleanup>
Q_INLINE_TEMPLATE void qSwap(QScopedPointer<T, Cleanup> &p1, QScopedPointer<T, Cleanup> &p2)
{
   p1.swap(p2);
}


QT_END_NAMESPACE
namespace std {
template <class T, class Cleanup>
Q_INLINE_TEMPLATE void swap(QT_PREPEND_NAMESPACE(QScopedPointer)<T, Cleanup> &p1,
                            QT_PREPEND_NAMESPACE(QScopedPointer)<T, Cleanup> &p2)
{
   p1.swap(p2);
}
}
QT_BEGIN_NAMESPACE


namespace QtPrivate {
template <typename X, typename Y> struct QScopedArrayEnsureSameType;
template <typename X> struct QScopedArrayEnsureSameType<X, X> {
   typedef X *Type;
};

template <typename X> struct QScopedArrayEnsureSameType<const X, X> {
   typedef X *Type;
};
}

template <typename T, typename Cleanup = QScopedPointerArrayDeleter<T> >
class QScopedArrayPointer : public QScopedPointer<T, Cleanup>
{
 public:
   inline QScopedArrayPointer() : QScopedPointer<T, Cleanup>(0) {}

   template <typename D>
   explicit inline QScopedArrayPointer(D *p, typename QtPrivate::QScopedArrayEnsureSameType<T, D>::Type = 0)
      : QScopedPointer<T, Cleanup>(p) {
   }

   inline T &operator[](int i) {
      return this->d[i];
   }

   inline const T &operator[](int i) const {
      return this->d[i];
   }

 private:
   explicit inline QScopedArrayPointer(void *) {
      // Enforce the same type.

      // If you get a compile error here, make sure you declare
      // QScopedArrayPointer with the same template type as you pass to the
      // constructor. See also the QScopedPointer documentation.

      // Storing a scalar array as a pointer to a different type is not
      // allowed and results in undefined behavior.
   }

   Q_DISABLE_COPY(QScopedArrayPointer)
};

#endif // QSCOPEDPOINTER_H
