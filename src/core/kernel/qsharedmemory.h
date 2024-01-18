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

#ifndef QSHAREDMEMORY_H
#define QSHAREDMEMORY_H

#include <qobject.h>
#include <qscopedpointer.h>

#ifndef QT_NO_SHAREDMEMORY

class QSharedMemoryPrivate;

class Q_CORE_EXPORT QSharedMemory : public QObject
{
   CORE_CS_OBJECT(QSharedMemory)
   Q_DECLARE_PRIVATE(QSharedMemory)

 public:
   enum AccessMode {
      ReadOnly,
      ReadWrite
   };

   enum SharedMemoryError {
      NoError,
      PermissionDenied,
      InvalidSize,
      KeyError,
      AlreadyExists,
      NotFound,
      LockError,
      OutOfResources,
      UnknownError
   };

   QSharedMemory(QObject *parent = nullptr);
   QSharedMemory(const QString &key, QObject *parent = nullptr);

   QSharedMemory(const QSharedMemory &) = delete;
   QSharedMemory &operator=(const QSharedMemory &) = delete;

   ~QSharedMemory();

   void setKey(const QString &key);
   QString key() const;
   void setNativeKey(const QString &key);
   QString nativeKey() const;

   bool create(int size, AccessMode mode = ReadWrite);
   int size() const;

   bool attach(AccessMode mode = ReadWrite);
   bool isAttached() const;
   bool detach();

   void *data();
   const void *constData() const;
   const void *data() const;

#ifndef QT_NO_SYSTEMSEMAPHORE
   bool lock();
   bool unlock();
#endif

   SharedMemoryError error() const;
   QString errorString() const;

 protected:
   QScopedPointer<QSharedMemoryPrivate> d_ptr;
};

#endif // QT_NO_SHAREDMEMORY

#endif

