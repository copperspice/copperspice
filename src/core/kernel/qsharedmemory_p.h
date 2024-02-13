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

#ifndef QSHAREDMEMORY_P_H
#define QSHAREDMEMORY_P_H

#include <qsharedmemory.h>
#include <qstringparser.h>

#ifdef QT_NO_SHAREDMEMORY

#ifndef QT_NO_SYSTEMSEMAPHORE

namespace QSharedMemoryPrivate {

int createUnixKeyFile(const QString &fileName);
QString makePlatformSafeKey(const QString &key, const QString &prefix = QString("qipc_sharedmemory_"));

}

#endif

#else

#include <qsystemsemaphore.h>

#ifdef Q_OS_WIN
#  include <qt_windows.h>
#else
#  include <sys/types.h>
#endif

#ifndef QT_NO_SYSTEMSEMAPHORE

class QSharedMemoryLocker
{
 public:
   QSharedMemoryLocker(QSharedMemory *sharedMemory) : q_sm(sharedMemory) {
      Q_ASSERT(q_sm);
   }

   ~QSharedMemoryLocker() {
      if (q_sm) {
         q_sm->unlock();
      }
   }

   bool lock() {
      if (q_sm && q_sm->lock()) {
         return true;
      }

      q_sm = nullptr;
      return false;
   }

 private:
   QSharedMemory *q_sm;
};
#endif // QT_NO_SYSTEMSEMAPHORE

class QSharedMemoryPrivate
{
   Q_DECLARE_PUBLIC(QSharedMemory)

 public:
   QSharedMemoryPrivate();
   virtual ~QSharedMemoryPrivate() {}

   void *memory;
   int size;
   QString key;
   QString nativeKey;
   QSharedMemory::SharedMemoryError error;
   QString errorString;

#ifndef QT_NO_SYSTEMSEMAPHORE
   QSystemSemaphore systemSemaphore;
   bool lockedByMe;
#endif

   static int createUnixKeyFile(const QString &fileName);
   static QString makePlatformSafeKey(const QString &key, const QString &prefix = QString("qipc_sharedmemory_"));

#ifdef Q_OS_WIN
   HANDLE handle();
#elif defined(QT_POSIX_IPC)
   int handle();
#else
   key_t handle();
#endif

   bool initKey();
   void cleanHandle();
   bool create(int size);
   bool attach(QSharedMemory::AccessMode mode);
   bool detach();

   void setErrorString(const QString &function);

#ifndef QT_NO_SYSTEMSEMAPHORE
   bool tryLocker(QSharedMemoryLocker *locker, const QString &function) {

      if (! locker->lock()) {
         errorString = QSharedMemory::tr("%1: unable to lock").formatArg(function);
         error = QSharedMemory::LockError;
         return false;
      }

      return true;
   }
#endif

 protected:
   QSharedMemory *q_ptr;

 private:
#ifdef Q_OS_WIN
   HANDLE hand;
#elif defined(QT_POSIX_IPC)
   int hand;
#else
   key_t unix_key;
#endif

};

#endif // QT_NO_SHAREDMEMORY

#endif
