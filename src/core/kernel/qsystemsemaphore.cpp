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

#include <qglobal.h>

#include <qsystemsemaphore_p.h>

#ifndef QT_NO_SYSTEMSEMAPHORE

QSystemSemaphore::QSystemSemaphore(const QString &key, int initialValue, AccessMode mode)
   : d(new QSystemSemaphorePrivate)
{
   setKey(key, initialValue, mode);
}

QSystemSemaphore::~QSystemSemaphore()
{
   d->cleanHandle();
}

void QSystemSemaphore::setKey(const QString &key, int initialValue, AccessMode mode)
{
   if (key == d->key && mode == Open) {
      return;
   }

   d->error = NoError;
   d->errorString = QString();

#if ! defined(Q_OS_WIN) && !defined(QT_POSIX_IPC)

   // optimization to not destroy/create the file & semaphore
   if (key == d->key && mode == Create && d->createdSemaphore && d->createdFile) {
      d->initialValue = initialValue;
      d->unix_key = -1;
      d->handle(mode);
      return;
   }

#endif

   d->cleanHandle();
   d->key = key;
   d->initialValue = initialValue;

   // cache the file name so it doesn't have to be generated all the time.
   d->fileName = d->makeKeyFileName();
   d->handle(mode);
}

QString QSystemSemaphore::key() const
{
   return d->key;
}

bool QSystemSemaphore::acquire()
{
   return d->modifySemaphore(-1);
}

bool QSystemSemaphore::release(int n)
{
   if (n == 0) {
      return true;
   }

   if (n < 0) {
      qWarning("QSystemSemaphore::release() Value can not be negative.");
      return false;
   }

   return d->modifySemaphore(n);
}

QSystemSemaphore::SystemSemaphoreError QSystemSemaphore::error() const
{
   return d->error;
}

QString QSystemSemaphore::errorString() const
{
   return d->errorString;
}

#endif
