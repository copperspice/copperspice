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

#include <qwslock_p.h>

#ifndef QT_NO_QWS_MULTIPROCESS

#include <qwssignalhandler_p.h>

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>

#ifndef QT_POSIX_IPC
#include <sys/sem.h>
#endif

#include <sys/time.h>
#include <time.h>

#ifdef Q_OS_LINUX
#include <linux/version.h>
#endif

#include <unistd.h>
#include <qcore_unix_p.h>

QT_BEGIN_NAMESPACE

#ifdef QT_NO_SEMAPHORE
#error QWSLock currently requires semaphores
#endif

#ifdef QT_POSIX_IPC
#include <QtCore/QAtomicInt>

static QAtomicInt localUniqueId = 1;
#endif

QWSLock::QWSLock(int id) : semId(id)
{
   static unsigned short initialValues[3] = { 1, 1, 0 };

#ifndef QT_NO_QWS_SIGNALHANDLER
   QWSSignalHandler::instance()->addWSLock(this);
#endif

#ifndef QT_POSIX_IPC
   if (semId == -1) {
      semId = semget(IPC_PRIVATE, 3, IPC_CREAT | 0666);
      if (semId == -1) {
         perror("QWSLock::QWSLock");
         qFatal("Unable to create semaphore");
      }

      qt_semun semval;
      semval.array = initialValues;
      if (semctl(semId, 0, SETALL, semval) == -1) {
         perror("QWSLock::QWSLock");
         qFatal("Unable to initialize semaphores");
      }
   }
#else
   sems[0] = sems[1] = sems[2] = SEM_FAILED;
   owned = false;

   if (semId == -1) {
      // ### generate really unique IDs
      semId = (getpid() << 16) + (localUniqueId.fetchAndAddRelaxed(1) % ushort(-1));
      owned = true;
   }

   QByteArray pfx = "/qwslock_" + QByteArray::number(semId, 16) + '_';
   QByteArray keys[3] = { pfx + "BackingStore", pfx + "Communication", pfx + "RegionEvent" };
   for (int i = 0; i < 3; ++i) {
      if (owned) {
         sem_unlink(keys[i].constData());
      }
      do {
         sems[i] = sem_open(keys[i].constData(), (owned ? O_CREAT : 0), 0666, initialValues[i]);
      } while (sems[i] == SEM_FAILED && errno == EINTR);
      if (sems[i] == SEM_FAILED) {
         perror("QWSLock::QWSLock");
         qFatal("Unable to %s semaphore", (owned ? "create" : "open"));
      }
   }
#endif

   lockCount[0] = lockCount[1] = 0;
}

QWSLock::~QWSLock()
{
#ifndef QT_NO_QWS_SIGNALHANDLER
   QWSSignalHandler::instance()->removeWSLock(this);
#endif

   if (semId != -1) {
#ifndef QT_POSIX_IPC
      qt_semun semval;
      semval.val = 0;
      semctl(semId, 0, IPC_RMID, semval);
      semId = -1;
#else
      // emulate the SEM_UNDO behavior for the BackingStore lock
      while (hasLock(BackingStore)) {
         unlock(BackingStore);
      }

      QByteArray pfx = "/qwslock_" + QByteArray::number(semId, 16) + '_';
      QByteArray keys[3] = { pfx + "BackingStore", pfx + "Communication", pfx + "RegionEvent" };
      for (int i = 0; i < 3; ++i) {
         if (sems[i] != SEM_FAILED) {
            sem_close(sems[i]);
            sems[i] = SEM_FAILED;
         }
         if (owned) {
            sem_unlink(keys[i].constData());
         }
      }
#endif
   }
}

bool QWSLock::up(unsigned short semNum)
{
   int ret;

#ifndef QT_POSIX_IPC
   sembuf sops = { semNum, 1, 0 };
   // As the BackingStore lock is a mutex, and only one process may own
   // the lock, it's safe to use SEM_UNDO. On the other hand, the
   // Communication lock is locked by the client but unlocked by the
   // server and therefore can't use SEM_UNDO.
   if (semNum == BackingStore) {
      sops.sem_flg |= SEM_UNDO;
   }

   EINTR_LOOP(ret, semop(semId, &sops, 1));
#else
   ret = sem_post(sems[semNum]);
#endif
   if (ret == -1) {
      qDebug("QWSLock::up(): %s", strerror(errno));
      return false;
   }

   return true;
}

bool QWSLock::down(unsigned short semNum, int)
{
   int ret;

#ifndef QT_POSIX_IPC
   sembuf sops = { semNum, -1, 0 };
   // As the BackingStore lock is a mutex, and only one process may own
   // the lock, it's safe to use SEM_UNDO. On the other hand, the
   // Communication lock is locked by the client but unlocked by the
   // server and therefore can't use SEM_UNDO.
   if (semNum == BackingStore) {
      sops.sem_flg |= SEM_UNDO;
   }

   EINTR_LOOP(ret, semop(semId, &sops, 1));
#else
   EINTR_LOOP(ret, sem_wait(sems[semNum]));
#endif
   if (ret == -1) {
      qDebug("QWSLock::down(): %s", strerror(errno));
      return false;
   }

   return true;
}

int QWSLock::getValue(unsigned short semNum) const
{
   int ret;
#ifndef QT_POSIX_IPC
   ret = semctl(semId, semNum, GETVAL, 0);
#else
   if (sem_getvalue(sems[semNum], &ret) == -1) {
      ret = -1;
   }
#endif
   if (ret == -1) {
      qDebug("QWSLock::getValue(): %s", strerror(errno));
   }
   return ret;
}

bool QWSLock::lock(LockType type, int timeout)
{
   if (type == RegionEvent) {
      return up(type);
   }

   if (lockCount[type] > 0) {
      ++lockCount[type];
      return true;
   }

   if (down(type, timeout)) {
      ++lockCount[type];
      return true;
   }

   return false;
}

bool QWSLock::hasLock(LockType type)
{
   if (type == RegionEvent) {
      return getValue(type) == 0;
   }

   return lockCount[type] > 0;
}

void QWSLock::unlock(LockType type)
{
   if (type == RegionEvent) {
      down(type, -1);
      return;
   }

   if (lockCount[type] > 0) {
      --lockCount[type];
      if (lockCount[type] > 0) {
         return;
      }
   }

   up(type);
}

bool QWSLock::wait(LockType type, int timeout)
{
   bool ok = down(type, timeout);
   if (ok) {
      unlock(type);
   }
   return ok;
}

QT_END_NAMESPACE

#endif // QT_NO_QWS_MULTIPROCESS
