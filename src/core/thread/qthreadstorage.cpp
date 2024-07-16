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

#include <qthreadstorage.h>

#include <qmutex.h>
#include <qthread.h>

#include <qthread_p.h>

#include <string.h>

using DestructorMap = QVector<void (*)(void *)>;

static QMutex *mutex()
{
   static QMutex retval;
   return &retval;
}

static DestructorMap *destructors()
{
   static DestructorMap retval;
   return &retval;
}

QThreadStorageData::QThreadStorageData(void (*func)(void *))
{
   QMutexLocker locker(mutex());
   DestructorMap *destr = destructors();

   if (! destr) {
      /*
       the destructors vector has already been destroyed, yet a new
       QThreadStorage is being allocated. this can only happen during global
       destruction, at which point we assume that there is only one thread.
       in order to keep QThreadStorage working, we need somewhere to store
       the data, best place we have in this situation is at the tail of the
       current thread's tls vector. the destructor is ignored, since we have
       no where to store it, and no way to actually call it.
       */
      QThreadData *data = QThreadData::current();
      id = data->tls.count();

      return;
   }

   for (id = 0; id < destr->count(); id++) {
      if (destr->at(id) == nullptr) {
         break;
      }
   }

   if (id == destr->count()) {
      destr->append(func);
   } else {
      (*destr)[id] = func;
   }
}

QThreadStorageData::~QThreadStorageData()
{
   QMutexLocker locker(mutex());

   if (destructors()) {
      (*destructors())[id] = nullptr;
   }
}

void **QThreadStorageData::get() const
{
   QThreadData *data = QThreadData::current();

   if (! data) {
      qWarning("QThreadStorage::get() Only valid from threads started with QThread");
      return nullptr;
   }

   QVector<void *> &tls = data->tls;

   if (tls.size() <= id) {
      tls.resize(id + 1);
   }

   void **v = &tls[id];

   return *v ? v : nullptr;
}

void **QThreadStorageData::set(void *p)
{
   QThreadData *data = QThreadData::current();

   if (! data) {
      qWarning("QThreadStorage::set() Only valid from threads started with QThread");
      return nullptr;
   }

   QVector<void *> &tls = data->tls;

   if (tls.size() <= id) {
      tls.resize(id + 1);
   }

   void *&value = tls[id];

   // delete any previous data
   if (value != nullptr) {
      QMutexLocker locker(mutex());

      DestructorMap *destr = destructors();
      void (*destructor)(void *) = destr ? destr->value(id) : nullptr;
      locker.unlock();

      void *q = value;
      value = nullptr;

      if (destructor) {
         destructor(q);
      }
   }

   // store new data
   value = p;

   return &value;
}

void QThreadStorageData::finish(void **p)
{
   QVector<void *> *tls = reinterpret_cast<QVector<void *> *>(p);

   if (! tls || tls->isEmpty() || !mutex()) {
      return;   // nothing to do
   }

   while (! tls->isEmpty()) {
      void *&value = tls->last();
      void *q = value;
      value = nullptr;
      int i = tls->size() - 1;
      tls->resize(i);

      if (! q) {
         // data already deleted
         continue;
      }

      QMutexLocker locker(mutex());
      void (*destructor)(void *) = destructors()->value(i);
      locker.unlock();

      if (destructor == nullptr) {
         if (QThread::currentThread()) {
            qWarning("QThreadStorage::finish() Thread %p exited after QThreadStorage %d destroyed",
                  static_cast<void *>(QThread::currentThread()), i);
         }

         continue;
      }

      destructor(q); //crash here might mean the thread exited after qthreadstorage was destroyed

      if (tls->size() > i) {
         //re reset the tls in case it has been recreated by its own destructor.
         (*tls)[i] = nullptr;
      }
   }

   tls->clear();
}
