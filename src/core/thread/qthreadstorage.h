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

#ifndef QTHREADSTORAGE_H
#define QTHREADSTORAGE_H

#include <qglobal.h>

class Q_CORE_EXPORT QThreadStorageData
{
 public:
   explicit QThreadStorageData(void (*func)(void *));
   ~QThreadStorageData();

   void **get() const;
   void **set(void *p);

   static void finish(void **);
   int id;
};

// pointer specialization
template <typename T>
inline T *&qThreadStorage_localData(QThreadStorageData &d, T **)
{
   void **v = d.get();

   if (! v) {
      v = d.set(nullptr);
   }

   return *(reinterpret_cast<T **>(v));
}

template <typename T>
inline T *qThreadStorage_localData_const(const QThreadStorageData &d, T **)
{
   void **v = d.get();

   return v ? *(reinterpret_cast<T **>(v)) : nullptr;
}

template <typename T>
inline void qThreadStorage_setLocalData(QThreadStorageData &d, T **t)
{
   (void) d.set(*t);
}

template <typename T>
inline void qThreadStorage_deleteData(void *d, T **)
{
   delete static_cast<T *>(d);
}

// value-based specialization
template <typename T>
inline T &qThreadStorage_localData(QThreadStorageData &d, T *)
{
   void **v = d.get();

   if (! v) {
      v = d.set(new T());
   }

   return *(reinterpret_cast<T *>(*v));
}

template <typename T>
inline T qThreadStorage_localData_const(const QThreadStorageData &d, T *)
{
   void **v = d.get();
   return v ? *(reinterpret_cast<T *>(*v)) : T();
}

template <typename T>
inline void qThreadStorage_setLocalData(QThreadStorageData &d, T *t)
{
   (void) d.set(new T(*t));
}

template <typename T>
inline void qThreadStorage_deleteData(void *d, T *)
{
   delete static_cast<T *>(d);
}

template <class T>
class QThreadStorage
{
 private:
   QThreadStorageData d;

   static void deleteData(void *x) {
      qThreadStorage_deleteData(x, static_cast<T *>(nullptr));
   }

 public:
   QThreadStorage()
      : d(deleteData)
   { }

   QThreadStorage(const QThreadStorage &) = delete;
   QThreadStorage &operator=(const QThreadStorage &) = delete;

   ~QThreadStorage()
   { }

   bool hasLocalData() const {
      return d.get() != nullptr;
   }

   T &localData() {
      return qThreadStorage_localData(d, static_cast<T *>(nullptr));
   }

   T localData() const {
      return qThreadStorage_localData_const(d, static_cast<T *>(nullptr));
   }

   void setLocalData(T data) {
      qThreadStorage_setLocalData(d, &data);
   }
};

#endif
