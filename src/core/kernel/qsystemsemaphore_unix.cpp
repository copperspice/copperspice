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

#include <qsystemsemaphore.h>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qfile.h>

#include <qsystemsemaphore_p.h>

#ifndef QT_NO_SYSTEMSEMAPHORE

#include <sys/types.h>
#include <sys/ipc.h>

#ifndef QT_POSIX_IPC
#include <sys/sem.h>
#endif

#include <fcntl.h>
#include <errno.h>

#include <qcore_unix_p.h>

// OpenBSD 4.2 does not define EIDRM, refer to BUGS section
// http://www.openbsd.org/cgi-bin/man.cgi?query=semop&manpath=OpenBSD+4.2
#if defined(Q_OS_OPENBSD) && ! defined(EIDRM)
#define EIDRM EINVAL
#endif

#ifndef QT_POSIX_IPC
   QSystemSemaphorePrivate::QSystemSemaphorePrivate()
      : unix_key(-1), semaphore(-1), createdFile(false), createdSemaphore(false),
        error(QSystemSemaphore::NoError)
   { }
#else
   QSystemSemaphorePrivate::QSystemSemaphorePrivate()
      : semaphore(SEM_FAILED), createdSemaphore(false), error(QSystemSemaphore::NoError)
   { }
#endif

void QSystemSemaphorePrivate::setErrorString(const QString &function)
{
   // EINVAL is handled in functions so they can give better error strings
   switch (errno) {
      case EPERM:
      case EACCES:
         errorString = QCoreApplication::translate("QSystemSemaphore", "%1: permission denied").formatArg(function);
         error = QSystemSemaphore::PermissionDenied;
         break;

      case EEXIST:
         errorString = QCoreApplication::translate("QSystemSemaphore", "%1: already exists").formatArg(function);
         error = QSystemSemaphore::AlreadyExists;
         break;

      case ENOENT:
         errorString = QCoreApplication::translate("QSystemSemaphore", "%1: does not exist").formatArg(function);
         error = QSystemSemaphore::NotFound;
         break;

      case ERANGE:
      case ENOMEM:
      case ENOSPC:
      case EMFILE:
      case ENFILE:
      case EOVERFLOW:
         errorString = QCoreApplication::translate("QSystemSemaphore", "%1: out of resources").formatArg(function);
         error = QSystemSemaphore::OutOfResources;
         break;

      case ENAMETOOLONG:
         errorString = QCoreApplication::translate("QSystemSemaphore", "%1: name error").formatArg(function);
         error = QSystemSemaphore::KeyError;
         break;

      default:
         errorString = QCoreApplication::translate("QSystemSemaphore", "%1: unknown error %2").formatArg(function).formatArg(errno);
         error = QSystemSemaphore::UnknownError;

#if defined(CS_SHOW_DEBUG_CORE_SEMAPHORE)
         qDebug() << errorString << "key" << key << "errno" << errno << EINVAL;
#endif
         break;
   }
}

#ifndef QT_POSIX_IPC

key_t QSystemSemaphorePrivate::handle(QSystemSemaphore::AccessMode mode)
{
   if (-1 != unix_key) {
      return unix_key;
   }

   if (key.isEmpty()) {
      errorString = QCoreApplication::tr("%1: key is empty", "QSystemSemaphore").formatArg("QSystemSemaphore::handle");
      error = QSystemSemaphore::KeyError;
      return -1;
   }

   // ftok requires that an actual file exists somewhere
   int built = QSharedMemoryPrivate::createUnixKeyFile(fileName);

   if (-1 == built) {
      errorString = QCoreApplication::tr("%1: unable to make key", "QSystemSemaphore").formatArg("QSystemSemaphore::handle");

      error = QSystemSemaphore::KeyError;
      return -1;
   }

   createdFile = (1 == built);

   // Get the unix key for the created file
   unix_key = ftok(QFile::encodeName(fileName).constData(), 'Q');

   if (-1 == unix_key) {
      errorString = QCoreApplication::tr("%1: ftok failed", "QSystemSemaphore").formatArg("QSystemSemaphore::handle");
      error = QSystemSemaphore::KeyError;
      return -1;
   }

   // Get semaphore
   semaphore = semget(unix_key, 1, 0600 | IPC_CREAT | IPC_EXCL);

   if (-1 == semaphore) {
      if (errno == EEXIST) {
         semaphore = semget(unix_key, 1, 0600 | IPC_CREAT);
      }

      if (-1 == semaphore) {
         setErrorString("QSystemSemaphore::handle");
         cleanHandle();
         return -1;
      }

      if (mode == QSystemSemaphore::Create) {
         createdSemaphore = true;
         createdFile = true;
      }
   } else {
      createdSemaphore = true;
      // Force cleanup of file, it is possible that it can be left over from a crash
      createdFile = true;
   }

   // Created semaphore so initialize its value.
   if (createdSemaphore && initialValue >= 0) {
      qt_semun init_op;
      init_op.val = initialValue;

      if (-1 == semctl(semaphore, 0, SETVAL, init_op)) {
         setErrorString("QSystemSemaphore::handle");
         cleanHandle();
         return -1;
      }
   }

   return unix_key;
}

#else

bool QSystemSemaphorePrivate::handle(QSystemSemaphore::AccessMode mode)
{
   if (semaphore != SEM_FAILED) {
      return true;   // we already have a semaphore
   }

   if (fileName.isEmpty()) {
      errorString = QCoreApplication::tr("%1: key is empty", "QSystemSemaphore").formatArg("QSystemSemaphore::handle");
      error = QSystemSemaphore::KeyError;
      return false;
   }

   QByteArray semName = QFile::encodeName(fileName);

   // Always try with O_EXCL so we know whether we created the semaphore.
   int oflag = O_CREAT | O_EXCL;

   for (int tryNum = 0, maxTries = 1; tryNum < maxTries; ++tryNum) {
      do {
         semaphore = sem_open(semName.constData(), oflag, 0666, initialValue);
      } while (semaphore == SEM_FAILED && errno == EINTR);

      if (semaphore == SEM_FAILED && errno == EEXIST) {
         if (mode == QSystemSemaphore::Create) {
            if (sem_unlink(semName.constData()) == -1 && errno != ENOENT) {
               setErrorString("QSystemSemaphore::handle (sem_unlink)");
               return false;
            }

            // Race condition: the semaphore might be recreated before
            // we call sem_open again, so we'll retry several times.
            maxTries = 3;

         } else {
            // Race condition: if it no longer exists at the next sem_open
            // call, we won't realize we created it, so we'll leak it later.
            oflag &= ~O_EXCL;
            maxTries = 2;
         }
      } else {
         break;
      }
   }

   if (semaphore == SEM_FAILED) {
      setErrorString("QSystemSemaphore::handle");
      return false;
   }

   createdSemaphore = (oflag & O_EXCL) != 0;
   return true;
}
#endif // QT_POSIX_IPC

void QSystemSemaphorePrivate::cleanHandle()
{
#ifndef QT_POSIX_IPC
   unix_key = -1;

   // remove the file if we made it
   if (createdFile) {
      QFile::remove(fileName);
      createdFile = false;
   }

   if (createdSemaphore) {
      if (-1 != semaphore) {
         if (-1 == semctl(semaphore, 0, IPC_RMID, 0)) {
            setErrorString("QSystemSemaphore::cleanHandle");

#if defined(CS_SHOW_DEBUG_CORE_SEMAPHORE)
            qDebug("QSystemSemaphore::cleanHandle semctl failed.");
#endif
         }

         semaphore = -1;
      }

      createdSemaphore = false;
   }

#else

   if (semaphore != SEM_FAILED) {
      if (sem_close(semaphore) == -1) {
         setErrorString("QSystemSemaphore::cleanHandle (sem_close)");

#if defined(CS_SHOW_DEBUG_CORE_SEMAPHORE)
         qDebug() << QString("QSystemSemaphore::cleanHandle sem_close failed.");
#endif
      }

      semaphore = SEM_FAILED;
   }

   if (createdSemaphore) {
      if (sem_unlink(QFile::encodeName(fileName).constData()) == -1 && errno != ENOENT) {
         setErrorString("QSystemSemaphore::cleanHandle (sem_unlink)");

#if defined(CS_SHOW_DEBUG_CORE_SEMAPHORE)
         qDebug() << QString("QSystemSemaphore::cleanHandle sem_unlink failed.");
#endif
      }

      createdSemaphore = false;
   }

#endif // QT_POSIX_IPC
}

bool QSystemSemaphorePrivate::modifySemaphore(int count)
{
#ifndef QT_POSIX_IPC

   if (-1 == handle()) {
      return false;
   }

   struct sembuf operation;

   operation.sem_num = 0;
   operation.sem_op = count;
   operation.sem_flg = SEM_UNDO;

   int res;
   EINTR_LOOP(res, semop(semaphore, &operation, 1));

   if (-1 == res) {
      // If the semaphore was removed be nice and create it and then modifySemaphore again
      if (errno == EINVAL || errno == EIDRM) {
         semaphore = -1;
         cleanHandle();
         handle();

         return modifySemaphore(count);
      }

      setErrorString("QSystemSemaphore::modifySemaphore");

#if defined(CS_SHOW_DEBUG_CORE_SEMAPHORE)
      qDebug() << QString("QSystemSemaphore::modify failed") << count << semctl(semaphore, 0, GETVAL)
            << errno << EIDRM << EINVAL;
#endif
      return false;
   }

#else

   if (! handle()) {
      return false;
   }

   if (count > 0) {
      int cnt = count;

      do {
         if (sem_post(semaphore) == -1) {
            setErrorString("QSystemSemaphore::modifySemaphore (sem_post)");

#if defined(CS_SHOW_DEBUG_CORE_SEMAPHORE)
            qDebug() << QString("QSystemSemaphore::modify sem_post failed") << count << errno;
#endif

            // rollback changes to preserve the SysV semaphore behavior
            for ( ; cnt < count; ++cnt) {
               int res;
               EINTR_LOOP(res, sem_wait(semaphore));
            }

            return false;
         }

         --cnt;
      } while (cnt > 0);

   } else {
      int res;
      EINTR_LOOP(res, sem_wait(semaphore));

      if (res == -1) {
         // If the semaphore was removed be nice and create it and then modifySemaphore again
         if (errno == EINVAL || errno == EIDRM) {
            semaphore = SEM_FAILED;
            return modifySemaphore(count);
         }

         setErrorString("QSystemSemaphore::modifySemaphore (sem_wait)");

#if defined(CS_SHOW_DEBUG_CORE_SEMAPHORE)
         qDebug() << QString("QSystemSemaphore::modify sem_wait failed") << count << errno;
#endif
         return false;
      }
   }

#endif // QT_POSIX_IPC

   return true;
}

#endif // QT_NO_SYSTEMSEMAPHORE
