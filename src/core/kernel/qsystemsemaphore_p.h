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

#ifndef QSYSTEMSEMAPHORE_P_H
#define QSYSTEMSEMAPHORE_P_H

#include <qsystemsemaphore.h>

#ifndef QT_NO_SYSTEMSEMAPHORE

#include <qsharedmemory_p.h>
#include <sys/types.h>

#ifdef QT_POSIX_IPC
#  include <semaphore.h>
#endif

class QSystemSemaphorePrivate
{
 public:
   QSystemSemaphorePrivate();

   QString makeKeyFileName() const {
      return QSharedMemoryPrivate::makePlatformSafeKey(key, QString("qipc_systemsem_"));
   }

#ifdef Q_OS_WIN
   HANDLE handle(QSystemSemaphore::AccessMode mode = QSystemSemaphore::Open);
   void setErrorString(const QString &function);

#elif defined(QT_POSIX_IPC)
   bool handle(QSystemSemaphore::AccessMode mode = QSystemSemaphore::Open);
   void setErrorString(const QString &function);

#else
   key_t handle(QSystemSemaphore::AccessMode mode = QSystemSemaphore::Open);
   void setErrorString(const QString &function);

#endif

   void cleanHandle();
   bool modifySemaphore(int count);

   QString key;
   QString fileName;
   int initialValue;

#ifdef Q_OS_WIN
   HANDLE semaphore;
   HANDLE semaphoreLock;

#elif defined(QT_POSIX_IPC)
   sem_t *semaphore;
   bool createdSemaphore;

#else
   key_t unix_key;
   int semaphore;
   bool createdFile;
   bool createdSemaphore;

#endif

   QString errorString;
   QSystemSemaphore::SystemSemaphoreError error;
};

#endif // QT_NO_SYSTEMSEMAPHORE

#endif