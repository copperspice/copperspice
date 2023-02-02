/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#ifndef QSCOPEDPOINTER_P_H
#define QSCOPEDPOINTER_P_H

#include <qscopedpointer.h>

/* Internal helper class - exposes the data through data_ptr (legacy from QShared)
   Required for some internal classes, do not use otherwise. */

template <typename T, typename Cleanup = QScopedPointerDeleter<T> >
class QCustomScopedPointer : public QScopedPointer<T, Cleanup>
{
 public:
   explicit inline QCustomScopedPointer(T *p = nullptr)
      : QScopedPointer<T, Cleanup>(p) {
   }

   QCustomScopedPointer(const QCustomScopedPointer &) = delete;
   QCustomScopedPointer &operator=(const QCustomScopedPointer &) = delete;

   T *&data_ptr() {
      return this->d;
   }

   bool operator==(const QCustomScopedPointer<T, Cleanup> &other) const {
      return this->d == other.d;
   }

   bool operator!=(const QCustomScopedPointer<T, Cleanup> &other) const {
      return this->d != other.d;
   }
};

/* Internal helper class - a handler for QShared* classes, to be used in QCustomScopedPointer */
template <typename T>
class QScopedPointerSharedDeleter
{
 public:
   static inline void cleanup(T *d) {
      if (d && !d->ref.deref()) {
         delete d;
      }
   }
};

/* Internal - scoped pointer which points to a ref-counted object
 */
template <typename T>
class QScopedSharedPointer : public QCustomScopedPointer<T, QScopedPointerSharedDeleter<T> >
{
 public:
   explicit inline QScopedSharedPointer(T *p = nullptr)
      : QCustomScopedPointer<T, QScopedPointerSharedDeleter<T> >(p) {
   }

   QScopedSharedPointer(const QScopedSharedPointer &) = delete;
   QScopedSharedPointer &operator=(const QScopedSharedPointer &) = delete;

   void detach() {
      qAtomicDetach(this->d);
   }

   void assign(T *other) {
      if (this->d == other) {
         return;
      }

      if (other) {
         other->ref.ref();
      }

      T *oldD = this->d;
      this->d = other;
      QScopedPointerSharedDeleter<T>::cleanup(oldD);
   }

   bool operator==(const QScopedSharedPointer<T> &other) const {
      return this->d == other.d;
   }

   bool operator!=(const QScopedSharedPointer<T> &other) const {
      return this->d != other.d;
   }
};

#endif
