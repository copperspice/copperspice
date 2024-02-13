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

#include <qlockfile.h>

#include <qthread.h>
#include <qelapsedtimer.h>
#include <qdatetime.h>

#include <qlockfile_p.h>

QLockFile::QLockFile(const QString &fileName)
   : d_ptr(new QLockFilePrivate(fileName))
{
}

QLockFile::~QLockFile()
{
   unlock();
}

void QLockFile::setStaleLockTime(int staleLockTime)
{
   Q_D(QLockFile);
   d->staleLockTime = staleLockTime;
}

int QLockFile::staleLockTime() const
{
   Q_D(const QLockFile);
   return d->staleLockTime;
}

bool QLockFile::isLocked() const
{
   Q_D(const QLockFile);
   return d->isLocked;
}

bool QLockFile::lock()
{
   return tryLock(-1);
}

bool QLockFile::tryLock(int timeout)
{
   Q_D(QLockFile);

   QElapsedTimer timer;

   if (timeout > 0) {
      timer.start();
   }

   int sleepTime = 100;

   while (true) {
      d->lockError = d->tryLock_sys();

      switch (d->lockError) {
         case NoError:
            d->isLocked = true;
            return true;

         case PermissionError:
         case UnknownError:
            return false;

         case LockFailedError:
            if (! d->isLocked && d->isApparentlyStale()) {
               // Stale lock from another thread/process
               // Ensure two processes don't remove it at the same time

               QLockFile rmlock(d->fileName + ".rmlock");

               if (rmlock.tryLock()) {
                  if (d->isApparentlyStale() && d->removeStaleLock()) {
                     continue;
                  }
               }
            }

            break;
      }

      if (timeout == 0 || (timeout > 0 && timer.hasExpired(timeout))) {
         return false;
      }

      QThread::msleep(sleepTime);

      if (sleepTime < 5 * 1000) {
         sleepTime *= 2;
      }
   }

   // not reached
   return false;
}

bool QLockFile::getLockInfo(qint64 *pid, QString *hostname, QString *appname) const
{
   Q_D(const QLockFile);
   return d->getLockInfo(pid, hostname, appname);
}

bool QLockFilePrivate::getLockInfo(qint64 *pid, QString *hostname, QString *appname) const
{
   QFile reader(fileName);

   if (! reader.open(QIODevice::ReadOnly)) {
      return false;
   }

   QByteArray pidLine = reader.readLine();
   pidLine.chop(1);

   QByteArray appNameLine = reader.readLine();
   appNameLine.chop(1);

   QByteArray hostNameLine = reader.readLine();
   hostNameLine.chop(1);

   if (pidLine.isEmpty()) {
      return false;
   }

   qint64 thePid = pidLine.toLongLong();

   if (pid) {
      *pid = thePid;
   }

   if (appname) {
      *appname = QString::fromUtf8(appNameLine);
   }

   if (hostname) {
      *hostname = QString::fromUtf8(hostNameLine);
   }

   return thePid > 0;
}

bool QLockFile::removeStaleLockFile()
{
   Q_D(QLockFile);

   if (d->isLocked) {
      qWarning("QLockFile::removeStaleLockFile() File must not be locked");
      return false;
   }

   return d->removeStaleLock();
}

QLockFile::LockError QLockFile::error() const
{
   Q_D(const QLockFile);
   return d->lockError;
}
