/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qsharedmemory.h>
#include <qsharedmemory_p.h>

#include <qdebug.h>

#ifndef QT_NO_SHAREDMEMORY

//#define QSHAREDMEMORY_DEBUG

QT_BEGIN_NAMESPACE

QSharedMemoryPrivate::QSharedMemoryPrivate()
   : memory(0), size(0), error(QSharedMemory::NoError),
#ifndef QT_NO_SYSTEMSEMAPHORE
     systemSemaphore(QString()), lockedByMe(false),
#endif
     hand(0)
{
}

void QSharedMemoryPrivate::setErrorString(const QString &function)
{
   DWORD windowsError = GetLastError();
   if (windowsError == 0) {
      return;
   }

   switch (windowsError) {
      case ERROR_ALREADY_EXISTS:
         error = QSharedMemory::AlreadyExists;
         errorString = QSharedMemory::tr("%1: already exists").formatArg(function);
         break;
      case ERROR_FILE_NOT_FOUND:
         error = QSharedMemory::NotFound;
         errorString = QSharedMemory::tr("%1: doesn't exist").formatArg(function);
         break;
      case ERROR_COMMITMENT_LIMIT:
         error = QSharedMemory::InvalidSize;
         errorString = QSharedMemory::tr("%1: invalid size").formatArg(function);
         break;
      case ERROR_NO_SYSTEM_RESOURCES:
      case ERROR_NOT_ENOUGH_MEMORY:
         error = QSharedMemory::OutOfResources;
         errorString = QSharedMemory::tr("%1: out of resources").formatArg(function);
         break;
      case ERROR_ACCESS_DENIED:
         error = QSharedMemory::PermissionDenied;
         errorString = QSharedMemory::tr("%1: permission denied").formatArg(function);
         break;
      default:
         errorString = QSharedMemory::tr("%1: unknown error %2").formatArg(function).formatArg(windowsError);
         error = QSharedMemory::UnknownError;
#ifdef QSHAREDMEMORY_DEBUG
         qDebug() << errorString << "key" << key;
#endif
         break;
   }
}

HANDLE QSharedMemoryPrivate::handle()
{
   if (! hand) {
      // don't allow making handles on empty keys
      if (nativeKey.isEmpty()) {
         error       = QSharedMemory::KeyError;
         errorString = QSharedMemory::tr("%1: key is empty").formatArg("QSharedMemory::handle");
         return 0;
      }

      // This works for opening a mapping too, but always opens it with read/write access in
      // attach as it seems.
      hand = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, 0, &nativeKey.toStdWString()[0]);

      if (! hand) {
         setErrorString(QLatin1String("QSharedMemory::handle"));
      }
   }

   return hand;
}

void QSharedMemoryPrivate::cleanHandle()
{
   if (hand != 0 && !CloseHandle(hand)) {
      setErrorString(QLatin1String("QSharedMemory::cleanHandle"));
   }
   hand = 0;
}

bool QSharedMemoryPrivate::create(int size)
{
   if (nativeKey.isEmpty()) {
      error = QSharedMemory::KeyError;
      errorString = QSharedMemory::tr("%1: key is empty").formatArg("QSharedMemory::create");
      return false;
   }

   // Create the file mapping.
   hand = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, size, &nativeKey.toStdWString()[0]);
   setErrorString(QLatin1String("QSharedMemory::create"));

   // hand is valid when it already exists unlike unix so explicitly check
   return !(error == QSharedMemory::AlreadyExists || !hand);
}

bool QSharedMemoryPrivate::attach(QSharedMemory::AccessMode mode)
{
   // Grab a pointer to the memory block
   int permissions = (mode == QSharedMemory::ReadOnly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS);
   memory = (void *)MapViewOfFile(handle(), permissions, 0, 0, 0);

   if (memory == nullptr) {
      setErrorString(QLatin1String("QSharedMemory::attach"));
      cleanHandle();
      return false;
   }

   // Grab the size of the memory we have been given (a multiple of 4K on windows)
   MEMORY_BASIC_INFORMATION info;
   if (! VirtualQuery(memory, &info, sizeof(info))) {
      // Windows doesn't set an error code on this one,
      // it should only be a kernel memory error.
      error = QSharedMemory::UnknownError;
      errorString = QSharedMemory::tr("%1: size query failed").formatArg("QSharedMemory::attach");
      return false;
   }

   size = info.RegionSize;

   return true;
}

bool QSharedMemoryPrivate::detach()
{
   // umap memory
   if (!UnmapViewOfFile(memory)) {
      setErrorString(QLatin1String("QSharedMemory::detach"));
      return false;
   }
   memory = 0;
   size = 0;

   // close handle
   cleanHandle();

   return true;
}

QT_END_NAMESPACE

#endif // QT_NO_SHAREDMEMORY
