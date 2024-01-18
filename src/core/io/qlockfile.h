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

/*****************************************************************
** Copyright (c) 2013 David Faure <faure+bluesystems@kde.org>
*****************************************************************/

#ifndef QLOCKFILE_H
#define QLOCKFILE_H

#include <qstring.h>
#include <qscopedpointer.h>

class QLockFilePrivate;

class Q_CORE_EXPORT QLockFile
{
 public:
   enum LockError {
      NoError         = 0,
      LockFailedError = 1,
      PermissionError = 2,
      UnknownError    = 3
   };

   QLockFile(const QString &fileName);

   QLockFile(const QLockFile &) = delete;
   QLockFile &operator=(const QLockFile &) = delete;

   ~QLockFile();

   bool lock();
   bool tryLock(int timeout = 0);
   void unlock();

   void setStaleLockTime(int staleLockTime);
   int staleLockTime() const;

   bool isLocked() const;
   bool getLockInfo(qint64 *pid, QString *hostname, QString *appname) const;
   bool removeStaleLockFile();

   LockError error() const;

 protected:
   QScopedPointer<QLockFilePrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QLockFile)

};

#endif
