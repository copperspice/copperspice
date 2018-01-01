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

#ifndef QSYSTEMSEMAPHORE_H
#define QSYSTEMSEMAPHORE_H

#include <QtCore/qstring.h>
#include <QtCore/qscopedpointer.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SYSTEMSEMAPHORE

class QSystemSemaphorePrivate;

class Q_CORE_EXPORT QSystemSemaphore
{

 public:
   enum AccessMode {
      Open,
      Create
   };

   enum SystemSemaphoreError {
      NoError,
      PermissionDenied,
      KeyError,
      AlreadyExists,
      NotFound,
      OutOfResources,
      UnknownError
   };

   QSystemSemaphore(const QString &key, int initialValue = 0, AccessMode mode = Open);
   ~QSystemSemaphore();

   void setKey(const QString &key, int initialValue = 0, AccessMode mode = Open);
   QString key() const;

   bool acquire();
   bool release(int n = 1);

   SystemSemaphoreError error() const;
   QString errorString() const;

 private:
   Q_DISABLE_COPY(QSystemSemaphore)
   QScopedPointer<QSystemSemaphorePrivate> d;
};

#endif // QT_NO_SYSTEMSEMAPHORE

QT_END_NAMESPACE

#endif // QSYSTEMSEMAPHORE_H

