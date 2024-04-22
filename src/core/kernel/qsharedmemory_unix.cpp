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

#include <qdebug.h>
#include <qfile.h>
#include <qplatformdefs.h>
#include <qsharedmemory.h>

#include <errno.h>
#include <qcore_unix_p.h>
#include <qsharedmemory_p.h>

#ifndef QT_NO_SHAREDMEMORY

#include <sys/types.h>
#include <sys/ipc.h>

#ifndef QT_POSIX_IPC
#include <sys/shm.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#endif

#include <fcntl.h>
#include <unistd.h>
#endif

#ifndef QT_NO_SHAREDMEMORY

QSharedMemoryPrivate::QSharedMemoryPrivate()
   : memory(nullptr), size(0), error(QSharedMemory::NoError),

#ifndef QT_NO_SYSTEMSEMAPHORE
     systemSemaphore(QString()), lockedByMe(false),
#endif

#ifndef QT_POSIX_IPC
     unix_key(0)
#else
     hand(0)
#endif
{
}

void QSharedMemoryPrivate::setErrorString(const QString &function)
{
   // EINVAL is handled in functions so they can give better error strings
   switch (errno) {
      case EACCES:
      case EPERM:
         errorString = QSharedMemory::tr("%1: permission denied").formatArg(function);
         error = QSharedMemory::PermissionDenied;
         break;

      case EEXIST:
         errorString = QSharedMemory::tr("%1: already exists").formatArg(function);
         error = QSharedMemory::AlreadyExists;
         break;

      case ENOENT:
         errorString = QSharedMemory::tr("%1: does not exist").formatArg(function);
         error = QSharedMemory::NotFound;
         break;

      case EAGAIN:
      case EMFILE:
      case ENFILE:
      case ENOMEM:
      case ENOSPC:
         errorString = QSharedMemory::tr("%1: out of resources").formatArg(function);
         error = QSharedMemory::OutOfResources;
         break;

      case EOVERFLOW:
         errorString = QSharedMemory::tr("%1: invalid size").formatArg(function);
         error = QSharedMemory::InvalidSize;
         break;

      default:
         errorString = QSharedMemory::tr("%1: unknown error %2").formatArgs(function, errno);
         error = QSharedMemory::UnknownError;

#if defined(CS_SHOW_DEBUG_CORE)
         qDebug() << errorString << "key" << key << "errno" << errno << EINVAL;
#endif
         break;
   }
}

#ifndef QT_POSIX_IPC

key_t QSharedMemoryPrivate::handle()
{
   // already made
   if (unix_key) {
      return unix_key;
   }

   // don't allow making handles on empty keys
   if (nativeKey.isEmpty()) {
      errorString = QSharedMemory::tr("%1: key is empty").formatArg("QSharedMemory::handle");
      error = QSharedMemory::KeyError;
      return 0;
   }

   // ftok requires that an actual file exists somewhere
   if (! QFile::exists(nativeKey)) {
      errorString = QSharedMemory::tr("%1: UNIX key file does not exist").formatArg("QSharedMemory::handle");
      error = QSharedMemory::NotFound;
      return 0;
   }

   unix_key = ftok(QFile::encodeName(nativeKey).constData(), 'Q');

   if (-1 == unix_key) {
      errorString = QSharedMemory::tr("%1: ftok failed").formatArg("QSharedMemory::handle");
      error = QSharedMemory::KeyError;
      unix_key = 0;
   }

   return unix_key;
}

#else
int QSharedMemoryPrivate::handle()
{
   // don't allow making handles on empty keys
   QString safeKey = makePlatformSafeKey(key);

   if (safeKey.isEmpty()) {
      errorString = QSharedMemory::tr("%1: key is empty").formatArg("QSharedMemory::handle");
      error = QSharedMemory::KeyError;
      return 0;
   }

   return 1;
}
#endif // QT_POSIX_IPC

#endif // QT_NO_SHAREDMEMORY

#if ! (defined(QT_NO_SHAREDMEMORY) && defined(QT_NO_SYSTEMSEMAPHORE))
int QSharedMemoryPrivate::createUnixKeyFile(const QString &fileName)
{
#ifndef QT_POSIX_IPC

   int fd = qt_safe_open(QFile::encodeName(fileName).constData(), O_EXCL | O_CREAT | O_RDWR, 0640);

   if (-1 == fd) {
      if (errno == EEXIST) {
         return 0;
      }

      return -1;
   } else {
      qt_safe_close(fd);
   }

   return 1;

#else
   (void) fileName;

   // nothing to do
   return -1;
#endif

}
#endif // QT_NO_SHAREDMEMORY && QT_NO_SYSTEMSEMAPHORE

#ifndef QT_NO_SHAREDMEMORY

void QSharedMemoryPrivate::cleanHandle()
{
#ifndef QT_POSIX_IPC
   unix_key = 0;
#else
   qt_safe_close(hand);
   hand = 0;
#endif
}

bool QSharedMemoryPrivate::create(int size)
{
#ifndef QT_POSIX_IPC
   // build file if needed
   int built = createUnixKeyFile(nativeKey);

   if (built == -1) {
      errorString = QSharedMemory::tr("%1: unable to make key").formatArg("QSharedMemory::create");
      error = QSharedMemory::KeyError;
      return false;
   }

   bool createdFile = built == 1;

   // get handle
   if (!handle()) {
      if (createdFile) {
         QFile::remove(nativeKey);
      }

      return false;
   }

   // create
   if (-1 == shmget(unix_key, size, 0600 | IPC_CREAT | IPC_EXCL)) {
      QString function = "QSharedMemory::create";

      switch (errno) {
         case EINVAL:
            errorString = QSharedMemory::tr("%1: system-imposed size restrictions").formatArg(function);
            error = QSharedMemory::InvalidSize;
            break;

         default:
            setErrorString(function);
      }

      if (createdFile && error != QSharedMemory::AlreadyExists) {
         QFile::remove(nativeKey);
      }

      return false;
   }

#else

   if (! handle()) {
      return false;
   }

   QByteArray shmName = QFile::encodeName(makePlatformSafeKey(key));

   int fd;
   EINTR_LOOP(fd, shm_open(shmName.constData(), O_RDWR | O_CREAT | O_EXCL, 0666));

   if (fd == -1) {
      QString function = QLatin1String("QSharedMemory::create");

      switch (errno) {
         case ENAMETOOLONG:
         case EINVAL:
            errorString = QSharedMemory::tr("%1: bad name").formatArg(function);
            error = QSharedMemory::KeyError;
            break;

         default:
            setErrorString(function);
      }

      return false;
   }

   // the size may only be set once; ignore errors
   int ret;
   EINTR_LOOP(ret, ftruncate(fd, size));

   if (ret == -1) {
      setErrorString(QLatin1String("QSharedMemory::create (ftruncate)"));
      qt_safe_close(fd);
      return false;
   }

   qt_safe_close(fd);
#endif // QT_POSIX_IPC

   return true;
}

bool QSharedMemoryPrivate::attach(QSharedMemory::AccessMode mode)
{
#ifndef QT_POSIX_IPC
   // grab the shared memory segment id
   int id = shmget(unix_key, 0, (mode == QSharedMemory::ReadOnly ? 0400 : 0600));

   if (-1 == id) {
      setErrorString(QLatin1String("QSharedMemory::attach (shmget)"));
      return false;
   }

   // grab the memory
   memory = shmat(id, nullptr, (mode == QSharedMemory::ReadOnly ? SHM_RDONLY : 0));

   if ((void *) - 1 == memory) {
      memory = nullptr;
      setErrorString(QLatin1String("QSharedMemory::attach (shmat)"));
      return false;
   }

   // grab the size
   shmid_ds shmid_ds;

   if (!shmctl(id, IPC_STAT, &shmid_ds)) {
      size = (int)shmid_ds.shm_segsz;
   } else {
      setErrorString(QLatin1String("QSharedMemory::attach (shmctl)"));
      return false;
   }

#else
   QByteArray shmName = QFile::encodeName(makePlatformSafeKey(key));

   int oflag = (mode == QSharedMemory::ReadOnly ? O_RDONLY : O_RDWR);
   mode_t omode = (mode == QSharedMemory::ReadOnly ? 0444 : 0660);

   EINTR_LOOP(hand, shm_open(shmName.constData(), oflag, omode));

   if (hand == -1) {
      QString function = QLatin1String("QSharedMemory::attach (shm_open)");

      switch (errno) {
         case ENAMETOOLONG:
         case EINVAL:
            errorString = QSharedMemory::tr("%1: bad name").formatArg(function);
            error = QSharedMemory::KeyError;
            break;

         default:
            setErrorString(function);
      }

      hand = 0;
      return false;
   }

   // grab the size
   QT_STATBUF st;

   if (QT_FSTAT(hand, &st) == -1) {
      setErrorString(QLatin1String("QSharedMemory::attach (fstat)"));
      cleanHandle();
      return false;
   }

   size = st.st_size;

   // grab the memory
   int mprot = (mode == QSharedMemory::ReadOnly ? PROT_READ : PROT_READ | PROT_WRITE);
   memory = mmap(0, size, mprot, MAP_SHARED, hand, 0);

   if (memory == MAP_FAILED || !memory) {
      setErrorString(QLatin1String("QSharedMemory::attach (mmap)"));
      cleanHandle();
      memory = 0;
      size = 0;
      return false;
   }

#endif // QT_POSIX_IPC

   return true;
}

bool QSharedMemoryPrivate::detach()
{
#ifndef QT_POSIX_IPC

   // detach from the memory segment
   if (-1 == shmdt(memory)) {
      QString function = QLatin1String("QSharedMemory::detach");

      switch (errno) {
         case EINVAL:
            errorString = QSharedMemory::tr("%1: not attached").formatArg(function);
            error = QSharedMemory::NotFound;
            break;

         default:
            setErrorString(function);
      }

      return false;
   }

   memory = nullptr;
   size = 0;

   // Get the number of current attachments
   int id = shmget(unix_key, 0, 0400);
   cleanHandle();

   struct shmid_ds shmid_ds;

   if (0 != shmctl(id, IPC_STAT, &shmid_ds)) {
      switch (errno) {
         case EINVAL:
            return true;

         default:
            return false;
      }
   }

   // If there are no attachments then remove it.
   if (shmid_ds.shm_nattch == 0) {
      // mark for removal
      if (-1 == shmctl(id, IPC_RMID, &shmid_ds)) {
         setErrorString(QLatin1String("QSharedMemory::detach"));

         switch (errno) {
            case EINVAL:
               return true;

            default:
               return false;
         }
      }

      // remove file
      if (!QFile::remove(nativeKey)) {
         return false;
      }
   }

#else

   // detach from the memory segment
   if (munmap(memory, size) == -1) {
      setErrorString(QLatin1String("QSharedMemory::detach (munmap)"));
      return false;
   }

   memory = 0;
   size = 0;

   // get the number of current attachments
   int shm_nattch = 0;
   QT_STATBUF st;

   if (QT_FSTAT(hand, &st) == 0) {
      // subtract 2 from linkcount: one for our own open and one for the dir entry
      shm_nattch = st.st_nlink - 2;
   }

   cleanHandle();

   // if there are no attachments then unlink the shared memory
   if (shm_nattch == 0) {
      QByteArray shmName = QFile::encodeName(makePlatformSafeKey(key));

      if (shm_unlink(shmName.constData()) == -1 && errno != ENOENT) {
         setErrorString(QLatin1String("QSharedMemory::detach (shm_unlink)"));
      }
   }

#endif // QT_POSIX_IPC

   return true;
}

#endif // QT_NO_SHAREDMEMORY
