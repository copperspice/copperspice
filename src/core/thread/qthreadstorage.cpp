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

#include <qthreadstorage.h>

#include <qthread.h>
#include <qmutex.h>
#include <string.h>

#include <qthread_p.h>

#ifdef THREADSTORAGE_DEBUG
#define DEBUG_MSG qtsDebug

#include <stdio.h>
#include <stdarg.h>

void qtsDebug(const char *fmt, ...)
{
   va_list va;
   va_start(va, fmt);

   fprintf(stderr, "QThreadStorage: ");
   vfprintf(stderr, fmt, va);
   fprintf(stderr, "\n");

   va_end(va);
}
#else
#  define DEBUG_MSG if(false)qDebug
#endif

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

   if (!destr) {
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
      DEBUG_MSG("QThreadStorageData: Allocated id %d, destructor %p cannot be stored", id, func);
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
   DEBUG_MSG("QThreadStorageData: Allocated id %d, destructor %p", id, func);
}

QThreadStorageData::~QThreadStorageData()
{
   DEBUG_MSG("QThreadStorageData: Released id %d", id);
   QMutexLocker locker(mutex());
   if (destructors()) {
      (*destructors())[id] = nullptr;
   }
}

void **QThreadStorageData::get() const
{
   QThreadData *data = QThreadData::current();
   if (!data) {
      qWarning("QThreadStorage::get: QThreadStorage can only be used with threads started with QThread");
      return nullptr;
   }
   QVector<void *> &tls = data->tls;
   if (tls.size() <= id) {
      tls.resize(id + 1);
   }
   void **v = &tls[id];

   DEBUG_MSG("QThreadStorageData: Returning storage %d, data %p, for thread %p",
             id,
             *v,
             data->thread.load());

   return *v ? v : nullptr;
}

void **QThreadStorageData::set(void *p)
{
   QThreadData *data = QThreadData::current();
   if (! data) {
      qWarning("QThreadStorage::set: QThreadStorage can only be used with threads started with QThread");
      return nullptr;
   }

   QVector<void *> &tls = data->tls;
   if (tls.size() <= id) {
      tls.resize(id + 1);
   }

   void *&value = tls[id];
   // delete any previous data
   if (value != nullptr) {
      DEBUG_MSG("QThreadStorageData: Deleting previous storage %d, data %p, for thread %p",
                id, value, data->thread.load());

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
   DEBUG_MSG("QThreadStorageData: Set storage %d for thread %p to %p", id, data->thread.load(), p);

   return &value;
}

void QThreadStorageData::finish(void **p)
{
   QVector<void *> *tls = reinterpret_cast<QVector<void *> *>(p);
   if (!tls || tls->isEmpty() || !mutex()) {
      return;   // nothing to do
   }

   DEBUG_MSG("QThreadStorageData: Destroying storage for thread %p", QThread::currentThread());
   while (!tls->isEmpty()) {
      void *&value = tls->last();
      void *q = value;
      value = nullptr;
      int i = tls->size() - 1;
      tls->resize(i);

      if (!q) {
         // data already deleted
         continue;
      }

      QMutexLocker locker(mutex());
      void (*destructor)(void *) = destructors()->value(i);
      locker.unlock();

      if (!destructor) {
         if (QThread::currentThread())
            qWarning("QThreadStorage: Thread %p exited after QThreadStorage %d destroyed",
                     QThread::currentThread(), i);
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
