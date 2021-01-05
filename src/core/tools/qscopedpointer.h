/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#ifndef QSCOPEDPOINTER_H
#define QSCOPEDPOINTER_H

#include <qglobal.h>
#include <qassert.h>

template <typename T>
struct QScopedPointerDeleter {
   static inline void cleanup(T *pointer) {
      // enforce a complete type

      typedef char IsIncompleteType[ sizeof(T) ? 1 : -1 ];
      (void) sizeof(IsIncompleteType);

      delete pointer;
   }
};

template <typename T>
struct QScopedPointerArrayDeleter {
   static inline void cleanup(T *pointer) {
      // enforce a complete type

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

template <typename T, typename Cleanup = QScopedPointerDeleter<T>>
class QScopedPointer
{
   using RestrictedBool = T *QScopedPointer::*;

 public:
   explicit inline QScopedPointer(T *p = nullptr)
      : d(p)
   {
   }

   QScopedPointer(const QScopedPointer &) = delete;
   QScopedPointer &operator=(const QScopedPointer &) = delete;

   ~QScopedPointer()
   {
      T *oldD = this->d;
      Cleanup::cleanup(oldD);
   }

   QScopedPointer(QScopedPointer<T, Cleanup> &&other)
      : d(other.take()) {
   }

   QScopedPointer<T, Cleanup> &operator=(QScopedPointer<T, Cleanup> && other) {
      reset(other.take());
      return *this;
   }

   T &operator*() const {
      Q_ASSERT(d);
      return *d;
   }

   T *operator->() const {
      Q_ASSERT(d);
      return d;
   }

   bool operator!() const {
      return !d;
   }

   operator RestrictedBool() const {
      return isNull() ? nullptr : &QScopedPointer::d;
   }

   T *data() const {
      return d;
   }

   bool isNull() const {
      return !d;
   }

   void reset(T *other = nullptr) {
      if (d == other) {
         return;
      }

      T *oldD = d;
      d = other;
      Cleanup::cleanup(oldD);
   }

   T *take() {
      T *oldD = d;
      d = nullptr;

      return oldD;
   }

   void swap(QScopedPointer<T, Cleanup> &other) {
      qSwap(d, other.d);
   }

   typedef T *pointer;

 protected:
   T *d;
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
inline void qSwap(QScopedPointer<T, Cleanup> &p1, QScopedPointer<T, Cleanup> &p2)
{
   p1.swap(p2);
}

namespace std {

template <class T, class Cleanup>
inline void swap(QT_PREPEND_NAMESPACE(QScopedPointer)<T, Cleanup> &p1,
                            QT_PREPEND_NAMESPACE(QScopedPointer)<T, Cleanup> &p2)
{
   p1.swap(p2);
}

} // namespce


namespace QtPrivate {

template <typename X, typename Y> struct QScopedArrayEnsureSameType;
template <typename X> struct QScopedArrayEnsureSameType<X, X> {
   typedef X *Type;
};

template <typename X> struct QScopedArrayEnsureSameType<const X, X> {
   typedef X *Type;
};

} // namespace

template <typename T, typename Cleanup = QScopedPointerArrayDeleter<T> >
class QScopedArrayPointer : public QScopedPointer<T, Cleanup>
{
 public:
   inline QScopedArrayPointer()
      : QScopedPointer<T, Cleanup>(nullptr)
   {
   }

   template <typename D>
   explicit inline QScopedArrayPointer(D *p, typename QtPrivate::QScopedArrayEnsureSameType<T, D>::Type = nullptr)
      : QScopedPointer<T, Cleanup>(p) {
   }

   QScopedArrayPointer(const QScopedArrayPointer &) = delete;
   QScopedArrayPointer &operator=(const QScopedArrayPointer &) = delete;

   T &operator[](int i) {
      return this->d[i];
   }

   const T &operator[](int i) const {
      return this->d[i];
   }

 private:
   explicit inline QScopedArrayPointer(void *) {
      // Enforce the same type
      // Storing a scalar array as a pointer to a different type is not
      // allowed and results in undefined behavior.
   }
};

#endif
