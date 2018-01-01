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

#include <qwssharedmemory_p.h>

#if !defined(QT_NO_QWS_MULTIPROCESS)

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
#include <qcore_unix_p.h>

//#define QT_SHM_DEBUG

QT_BEGIN_NAMESPACE

#ifdef QT_POSIX_IPC
#include <QtCore/QAtomicInt>
#include <QByteArray>

static QAtomicInt localUniqueId = 1;

static inline QByteArray makeKey(int id)
{
   return "/qwsshm_" + QByteArray::number(id, 16);
}
#endif

QWSSharedMemory::QWSSharedMemory()
   : shmId(-1), shmBase(0), shmSize(0)
#ifdef QT_POSIX_IPC
   , hand(-1)
#endif
{
}

QWSSharedMemory::~QWSSharedMemory()
{
   detach();
}

bool QWSSharedMemory::create(int size)
{
   if (shmId != -1) {
      detach();
   }

#ifndef QT_POSIX_IPC
   shmId = shmget(IPC_PRIVATE, size, IPC_CREAT | 0600);
#else
   // ### generate really unique IDs
   shmId = (getpid() << 16) + (localUniqueId.fetchAndAddRelaxed(1) % ushort(-1));
   QByteArray shmName = makeKey(shmId);
   EINTR_LOOP(hand, shm_open(shmName.constData(), O_RDWR | O_CREAT, 0660));
   if (hand != -1) {
      // the size may only be set once; ignore errors
      int ret;
      EINTR_LOOP(ret, ftruncate(hand, size));
      if (ret == -1) {
         shmId = -1;
      }
   } else {
      shmId = -1;
   }
#endif
   if (shmId == -1) {
#ifdef QT_SHM_DEBUG
      perror("QWSSharedMemory::create():");
      qWarning("Error allocating shared memory of size %d", size);
#endif
      detach();
      return false;
   }

#ifndef QT_POSIX_IPC
   shmBase = shmat(shmId, 0, 0);
   // On Linux, it is possible to attach a shared memory segment even if it
   // is already marked to be deleted. However, POSIX.1-2001 does not specify
   // this behaviour and many other implementations do not support it.
   shmctl(shmId, IPC_RMID, 0);
#else
   // grab the size
   QT_STATBUF st;
   if (QT_FSTAT(hand, &st) != -1) {
      shmSize = st.st_size;
      // grab the memory
      shmBase = mmap(0, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, hand, 0);
   }
#endif
   if (shmBase == (void *) - 1 || !shmBase) {
#ifdef QT_SHM_DEBUG
      perror("QWSSharedMemory::create():");
      qWarning("Error attaching to shared memory id %d", shmId);
#endif
      detach();
      return false;
   }

   return true;
}

bool QWSSharedMemory::attach(int id)
{
   if (shmId == id) {
      return id != -1;
   }

   detach();

   if (id == -1) {
      return false;
   }

   shmId = id;
#ifndef QT_POSIX_IPC
   shmBase = shmat(shmId, 0, 0);
#else
   QByteArray shmName = makeKey(shmId);
   EINTR_LOOP(hand, shm_open(shmName.constData(), O_RDWR, 0660));
   if (hand != -1) {
      // grab the size
      QT_STATBUF st;
      if (QT_FSTAT(hand, &st) != -1) {
         shmSize = st.st_size;
         // grab the memory
         shmBase = mmap(0, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, hand, 0);
      }
   }
#endif
   if (shmBase == (void *) - 1 || !shmBase) {
#ifdef QT_SHM_DEBUG
      perror("QWSSharedMemory::attach():");
      qWarning("Error attaching to shared memory id %d", shmId);
#endif
      detach();
      return false;
   }

   return true;
}

void QWSSharedMemory::detach()
{
#ifndef QT_POSIX_IPC
   if (shmBase && shmBase != (void *) - 1) {
      shmdt(shmBase);
   }
#else
   if (shmBase && shmBase != (void *) - 1) {
      munmap(shmBase, shmSize);
   }
   if (hand > 0) {
      // get the number of current attachments
      int shm_nattch = 0;
      QT_STATBUF st;
      if (QT_FSTAT(hand, &st) == 0) {
         // subtract 2 from linkcount: one for our own open and one for the dir entry
         shm_nattch = st.st_nlink - 2;
      }
      qt_safe_close(hand);
      // if there are no attachments then unlink the shared memory
      if (shm_nattch == 0) {
         QByteArray shmName = makeKey(shmId);
         shm_unlink(shmName.constData());
      }
   }
#endif
   shmBase = 0;
   shmSize = 0;
   shmId = -1;
}

int QWSSharedMemory::size() const
{
   if (shmId == -1) {
      return 0;
   }

#ifndef QT_POSIX_IPC
   if (!shmSize) {
      struct shmid_ds shm;
      shmctl(shmId, IPC_STAT, &shm);
      shmSize = shm.shm_segsz;
   }
#endif

   return shmSize;
}

QT_END_NAMESPACE

#endif // QT_NO_QWS_MULTIPROCESS
