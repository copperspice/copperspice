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

#ifndef QLOCKFILE_P_H
#define QLOCKFILE_P_H

#include <qlockfile.h>
#include <qfile.h>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

class QLockFilePrivate
{
 public:
   QLockFilePrivate(const QString &fn)
      : fileName(fn),
#ifdef Q_OS_WIN
        fileHandle(INVALID_HANDLE_VALUE),
#else
        fileHandle(-1),
#endif
        staleLockTime(30 * 1000), // 30 seconds
        lockError(QLockFile::NoError), isLocked(false) {
   }

   QLockFile::LockError tryLock_sys();
   bool removeStaleLock();
   bool getLockInfo(qint64 *pid, QString *hostname, QString *appname) const;

   // Returns \c true if the lock belongs to dead PID, or is old.
   // The attempt to delete it will tell us if it was really stale or not, though.
   bool isApparentlyStale() const;

   // used in dbusmenu
   Q_CORE_EXPORT static QString processNameByPid(qint64 pid);

#ifdef Q_OS_UNIX
   static int checkFcntlWorksAfterFlock(const QString &fn);
#endif

   QString fileName;

#ifdef Q_OS_WIN
   Qt::HANDLE fileHandle;
#else
   int fileHandle;
#endif

   int staleLockTime; // "int milliseconds" is big enough for 24 days
   QLockFile::LockError lockError;
   bool isLocked;
};

#endif
