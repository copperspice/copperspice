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

#include <qsystemsemaphore.h>
#include <qsystemsemaphore_p.h>
#include <qcoreapplication.h>
#include <qdebug.h>

#ifndef QT_NO_SYSTEMSEMAPHORE

//#define QSYSTEMSEMAPHORE_DEBUG

QT_BEGIN_NAMESPACE

QSystemSemaphorePrivate::QSystemSemaphorePrivate() :
   semaphore(0), error(QSystemSemaphore::NoError)
{
}

void QSystemSemaphorePrivate::setErrorString(const QString &function)
{
   DWORD windowsError = GetLastError();
   if (windowsError == 0) {
      return;
   }

   switch (windowsError) {
      case ERROR_NO_SYSTEM_RESOURCES:
      case ERROR_NOT_ENOUGH_MEMORY:
         error = QSystemSemaphore::OutOfResources;
         errorString = QCoreApplication::translate("QSystemSemaphore", "%1: out of resources").arg(function);
         break;
      case ERROR_ACCESS_DENIED:
         error = QSystemSemaphore::PermissionDenied;
         errorString = QCoreApplication::translate("QSystemSemaphore", "%1: permission denied").arg(function);
         break;
      default:
         errorString = QCoreApplication::translate("QSystemSemaphore", "%1: unknown error %2").arg(function).arg(windowsError);
         error = QSystemSemaphore::UnknownError;
#ifdef QSYSTEMSEMAPHORE_DEBUG
         qDebug() << errorString << "key" << key;
#endif
         break;
   }
}

HANDLE QSystemSemaphorePrivate::handle(QSystemSemaphore::AccessMode)
{
   // don't allow making handles on empty keys
   if (key.isEmpty()) {
      return 0;
   }

   // Create it if it doesn't already exists.
   if (semaphore == 0) {
      semaphore = CreateSemaphore(0, initialValue, MAXLONG, (wchar_t *)fileName.utf16());
      if (semaphore == NULL) {
         setErrorString(QLatin1String("QSystemSemaphore::handle"));
      }
   }

   return semaphore;
}

void QSystemSemaphorePrivate::cleanHandle()
{
   if (semaphore && !CloseHandle(semaphore)) {
#if defined QSYSTEMSEMAPHORE_DEBUG
      qDebug() << QLatin1String("QSystemSemaphorePrivate::CloseHandle: sem failed");
#endif
   }
   semaphore = 0;
}

bool QSystemSemaphorePrivate::modifySemaphore(int count)
{
   if (0 == handle()) {
      return false;
   }

   if (count > 0) {
      if (0 == ReleaseSemaphore(semaphore, count, 0)) {
         setErrorString(QLatin1String("QSystemSemaphore::modifySemaphore"));
#if defined QSYSTEMSEMAPHORE_DEBUG
         qDebug() << QLatin1String("QSystemSemaphore::modifySemaphore ReleaseSemaphore failed");
#endif
         return false;
      }
   } else {
      if (WAIT_OBJECT_0 != WaitForSingleObject(semaphore, INFINITE)) {
         setErrorString(QLatin1String("QSystemSemaphore::modifySemaphore"));
#if defined QSYSTEMSEMAPHORE_DEBUG
         qDebug() << QLatin1String("QSystemSemaphore::modifySemaphore WaitForSingleObject failed");
#endif
         return false;
      }
   }

   return true;
}

QT_END_NAMESPACE

#endif // QT_NO_SYSTEMSEMAPHORE
