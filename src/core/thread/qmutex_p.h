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

#ifndef QMUTEX_P_H
#define QMUTEX_P_H

#include <QtCore/qglobal.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qmutex.h>
#include <QtCore/qatomic.h>

#if defined(Q_OS_MAC)
# include <mach/semaphore.h>
#endif

QT_BEGIN_NAMESPACE

class QMutexData
{
 public:
   bool recursive;
   QMutexData(QMutex::RecursionMode mode = QMutex::NonRecursive)
      : recursive(mode == QMutex::Recursive) {}
};

#if !defined(Q_OS_LINUX)
class QMutexPrivate : public QMutexData
{
 public:
   ~QMutexPrivate();
   QMutexPrivate();

   bool wait(int timeout = -1);
   void wakeUp();

   // Conrol the lifetime of the privates
   QAtomicInt refCount;
   int id;

   bool ref() {
      Q_ASSERT(refCount.load() >= 0);
      int c;
      do {
         c = refCount.load();
         if (c == 0) {
            return false;
         }
      } while (!refCount.testAndSetRelaxed(c, c + 1));
      Q_ASSERT(refCount.load() >= 0);
      return true;
   }

   void deref() {
      Q_ASSERT(refCount.load() >= 0);
      if (!refCount.deref()) {
         release();
      }
      Q_ASSERT(refCount.load() >= 0);
   }
   void release();
   static QMutexPrivate *allocate();

   QAtomicInt waiters; //number of thread waiting
   QAtomicInt possiblyUnlocked; //bool saying that a timed wait timed out
   enum { BigNumber = 0x100000 }; //Must be bigger than the possible number of waiters (number of threads)
   void derefWaiters(int value);

   //platform specific stuff
#if defined(Q_OS_MAC)
   semaphore_t mach_semaphore;

#elif defined(Q_OS_UNIX)
   bool wakeup;
   pthread_mutex_t mutex;
   pthread_cond_t cond;

#elif defined(Q_OS_WIN32)
   HANDLE event;
#endif

};
#endif //Q_OS_LINUX

class QRecursiveMutexPrivate : public QMutexData
{
 public:
   QRecursiveMutexPrivate()
      : QMutexData(QMutex::Recursive), owner(0), count(0) {}
   std::atomic<Qt::HANDLE> owner;
   uint count;
   QMutex mutex;

   bool lock(int timeout);
   void unlock();
};

QT_END_NAMESPACE

#endif // QMUTEX_P_H
