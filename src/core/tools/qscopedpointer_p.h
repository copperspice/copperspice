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

#ifndef QSCOPEDPOINTER_P_H
#define QSCOPEDPOINTER_P_H

#include <QtCore/qscopedpointer.h>

QT_BEGIN_NAMESPACE

/* Internal helper class - exposes the data through data_ptr (legacy from QShared)
   Required for some internal classes, do not use otherwise. */

template <typename T, typename Cleanup = QScopedPointerDeleter<T> >
class QCustomScopedPointer : public QScopedPointer<T, Cleanup>
{
 public:
   explicit inline QCustomScopedPointer(T *p = 0)
      : QScopedPointer<T, Cleanup>(p) {
   }

   inline T *&data_ptr() {
      return this->d;
   }

   inline bool operator==(const QCustomScopedPointer<T, Cleanup> &other) const {
      return this->d == other.d;
   }

   inline bool operator!=(const QCustomScopedPointer<T, Cleanup> &other) const {
      return this->d != other.d;
   }

 private:
   Q_DISABLE_COPY(QCustomScopedPointer)
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

/* Internal
   This class is basically a scoped pointer pointing to a ref-counted object
 */
template <typename T>
class QScopedSharedPointer : public QCustomScopedPointer<T, QScopedPointerSharedDeleter<T> >
{
 public:
   explicit inline QScopedSharedPointer(T *p = 0)
      : QCustomScopedPointer<T, QScopedPointerSharedDeleter<T> >(p) {
   }

   inline void detach() {
      qAtomicDetach(this->d);
   }

   inline void assign(T *other) {
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

   inline bool operator==(const QScopedSharedPointer<T> &other) const {
      return this->d == other.d;
   }

   inline bool operator!=(const QScopedSharedPointer<T> &other) const {
      return this->d != other.d;
   }

 private:
   Q_DISABLE_COPY(QScopedSharedPointer)
};

QT_END_NAMESPACE

#endif
