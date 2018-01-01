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

#include <qlock_p.h>

#ifdef QT_NO_QWS_MULTIPROCESS

QT_BEGIN_NAMESPACE

/* no multiprocess - use a dummy */

QLock::QLock(const QString & /*filename*/, char /*id*/, bool /*create*/)
   : type(Read), data(0)
{
}

QLock::~QLock()
{
}

bool QLock::isValid() const
{
   return true;
}

void QLock::lock(Type t)
{
   data = (QLockData *) - 1;
   type = t;
}

void QLock::unlock()
{
   data = 0;
}

bool QLock::locked() const
{
   return data;
}

QT_END_NAMESPACE

#else // QT_NO_QWS_MULTIPROCESS

#if defined(Q_OS_DARWIN)
#  define QT_NO_SEMAPHORE
#endif

#include <qwssignalhandler_p.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>

#if defined(QT_NO_SEMAPHORE)
#  include <sys/stat.h>
#  include <sys/file.h>
#elif !defined(QT_POSIX_IPC)
#  include <sys/sem.h>
#else
#  include <semaphore.h>
#endif

#include <string.h>
#include <errno.h>

#include <qcore_unix_p.h> // overrides QT_OPEN

QT_BEGIN_NAMESPACE

#define MAX_LOCKS   200            // maximum simultaneous read locks

class QLockData
{
 public:
#if defined(QT_NO_SEMAPHORE) || defined(QT_POSIX_IPC)
   QByteArray file;
#endif
#if !defined(QT_POSIX_IPC)
   int id;
#else
   sem_t *id;    // Read mode resource counter
   sem_t *rsem;  // Read mode lock
   sem_t *wsem;  // Write mode lock
#endif
   int count;
   bool owned;
};


/*!
    \class QLock
    \brief The QLock class is a wrapper for a system shared semaphore.

    \ingroup qws

    \internal

    It is used by \l{Qt for Embedded Linux} for synchronizing access to the graphics
    card and shared memory region between processes.
*/

/*!
    \enum QLock::Type

    \value Read
    \value Write
*/

/*!
    Creates a lock. \a filename is the file path of the Unix-domain
    socket the \l{Qt for Embedded Linux} client is using. \a id is the name of the
    particular lock to be created on that socket. If \a create is true
    the lock is to be created (as the Qt for Embedded Linux server does); if \a
    create is false the lock should exist already (as the Qt for Embedded Linux
    client expects).
*/
QLock::QLock(const QString &filename, char id, bool create)
{
   data = new QLockData;
   data->count = 0;
#if defined(QT_NO_SEMAPHORE)
   data->file = filename.toLocal8Bit() + id;
   for (int x = 0; x < 2; ++x) {
      data->id = QT_OPEN(data->file.constData(), O_RDWR | (x ? O_CREAT : 0), S_IRWXU);
      if (data->id != -1 || !create) {
         data->owned = x;
         break;
      }
   }
#elif !defined(QT_POSIX_IPC)
   key_t semkey = ftok(filename.toLocal8Bit().constData(), id);
   data->id = semget(semkey, 0, 0);
   data->owned = create;
   if (create) {
      qt_semun arg;
      arg.val = 0;
      if (data->id != -1) {
         semctl(data->id, 0, IPC_RMID, arg);
      }
      data->id = semget(semkey, 1, IPC_CREAT | 0600);
      arg.val = MAX_LOCKS;
      semctl(data->id, 0, SETVAL, arg);
   }
#else
   data->file = filename.toLocal8Bit() + id;
   data->owned = create;

   char ids[3] = { 'c', 'r', 'w' };
   sem_t **sems[3] = { &data->id, &data->rsem, &data->wsem };
   unsigned short initialValues[3] = { MAX_LOCKS, 1, 1 };
   for (int i = 0; i < 3; ++i) {
      QByteArray file = data->file + ids[i];
      do {
         *sems[i] = sem_open(file.constData(), 0, 0666, 0);
      } while (*sems[i] == SEM_FAILED && errno == EINTR);
      if (create) {
         if (*sems[i] != SEM_FAILED) {
            sem_close(*sems[i]);
            sem_unlink(file.constData());
         }
         do {
            *sems[i] = sem_open(file.constData(), O_CREAT, 0666, initialValues[i]);
         } while (*sems[i] == SEM_FAILED && errno == EINTR);
      }
   }
#endif
   if (!isValid()) {
      qWarning("QLock::QLock: Cannot %s semaphore %s '%c' (%d, %s)",
               (create ? "create" : "get"), qPrintable(filename), id,
               errno, strerror(errno));
   }

#ifndef QT_NO_QWS_SIGNALHANDLER
   QWSSignalHandler::instance()->addLock(this);
#endif
}

/*!
    Destroys a lock
*/
QLock::~QLock()
{
#ifndef QT_NO_QWS_SIGNALHANDLER
   QWSSignalHandler::instance()->removeLock(this);
#endif

   while (locked()) {
      unlock();
   }

#if defined(QT_NO_SEMAPHORE)
   if (isValid()) {
      QT_CLOSE(data->id);
   }
#elif defined(QT_POSIX_IPC)
   if (data->id != SEM_FAILED) {
      sem_close(data->id);
   }
   if (data->rsem != SEM_FAILED) {
      sem_close(data->rsem);
   }
   if (data->wsem != SEM_FAILED) {
      sem_close(data->wsem);
   }
#endif

   if (data->owned) {
#if defined(QT_NO_SEMAPHORE)
      unlink(data->file.constData());
#elif !defined(QT_POSIX_IPC)
      qt_semun semval;
      semval.val = 0;
      semctl(data->id, 0, IPC_RMID, semval);
#else
      char ids[3] = { 'c', 'r', 'w' };
      for (int i = 0; i < 3; ++i) {
         QByteArray file = data->file + ids[i];
         sem_unlink(file.constData());
      }
#endif
   }
   delete data;
   data = 0;
}

/*!
    Returns true if the lock constructor was successful; returns false if
    the lock could not be created or was not available to connect to.
*/
bool QLock::isValid() const
{
#if !defined(QT_POSIX_IPC)
   return data && data->id != -1;
#else
   return data && data->id != SEM_FAILED && data->rsem != SEM_FAILED && data->wsem != SEM_FAILED;
#endif
}

/*!
    Locks the semaphore with a lock of type \a t. Locks can either be
    \c Read or \c Write. If a lock is \c Read, attempts by other
    processes to obtain \c Read locks will succeed, and \c Write
    attempts will block until the lock is unlocked. If locked as \c
    Write, all attempts to lock by other processes will block until
    the lock is unlocked. Locks are stacked: i.e. a given QLock can be
    locked multiple times by the same process without blocking, and
    will only be unlocked after a corresponding number of unlock()
    calls.
*/
void QLock::lock(Type t)
{
   if (!isValid()) {
      return;
   }

   if (!data->count) {
      type = t;

      int rv;
#if defined(QT_NO_SEMAPHORE)
      int op = type == Write ? LOCK_EX : LOCK_SH;

      EINTR_LOOP(rv, flock(data->id, op));
#elif !defined(QT_POSIX_IPC)
      sembuf sops;
      sops.sem_num = 0;
      sops.sem_op = type == Write ? -MAX_LOCKS : -1;
      sops.sem_flg = SEM_UNDO;

      EINTR_LOOP(rv, semop(data->id, &sops, 1));
#else
      if (type == Write) {
         EINTR_LOOP(rv, sem_wait(data->rsem));
         if (rv != -1) {
            EINTR_LOOP(rv, sem_wait(data->wsem));
            if (rv == -1) {
               sem_post(data->rsem);
            }
         }
      } else {
         EINTR_LOOP(rv, sem_wait(data->wsem));
         if (rv != -1) {
            EINTR_LOOP(rv, sem_trywait(data->rsem));
            if (rv != -1 || errno == EAGAIN) {
               EINTR_LOOP(rv, sem_wait(data->id));
               if (rv == -1) {
                  int semval;
                  sem_getvalue(data->id, &semval);
                  if (semval == MAX_LOCKS) {
                     sem_post(data->rsem);
                  }
               }
            }
            rv = sem_post(data->wsem);
         }
      }
#endif
      if (rv == -1) {
         qDebug("QLock::lock(): %s", strerror(errno));
         return;
      }
   } else if (type == Read && t == Write) {
      qDebug("QLock::lock(): Attempt to lock for write while locked for read");
   }
   data->count++;
}

/*!
    Unlocks the semaphore. If other processes were blocking waiting to
    lock() the semaphore, one of them will wake up and succeed in
    locking.
*/
void QLock::unlock()
{
   if (!isValid()) {
      return;
   }

   if (data->count > 0) {
      data->count--;
      if (!data->count) {
         int rv;
#if defined(QT_NO_SEMAPHORE)
         EINTR_LOOP(rv, flock(data->id, LOCK_UN));
#elif !defined(QT_POSIX_IPC)
         sembuf sops;
         sops.sem_num = 0;
         sops.sem_op = type == Write ? MAX_LOCKS : 1;
         sops.sem_flg = SEM_UNDO;

         EINTR_LOOP(rv, semop(data->id, &sops, 1));
#else
         if (type == Write) {
            sem_post(data->wsem);
            rv = sem_post(data->rsem);
         } else {
            EINTR_LOOP(rv, sem_wait(data->wsem));
            if (rv != -1) {
               sem_post(data->id);
               int semval;
               sem_getvalue(data->id, &semval);
               if (semval == MAX_LOCKS) {
                  sem_post(data->rsem);
               }
               rv = sem_post(data->wsem);
            }
         }
#endif
         if (rv == -1) {
            qDebug("QLock::unlock(): %s", strerror(errno));
         }
      }
   } else {
      qDebug("QLock::unlock(): Unlock without corresponding lock");
   }
}

/*!
    Returns true if the lock is currently held by the current process;
    otherwise returns false.
*/
bool QLock::locked() const
{
   return isValid() && data->count > 0;
}

QT_END_NAMESPACE

#endif // QT_NO_QWS_MULTIPROCESS
