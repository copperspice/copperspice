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

#ifndef QWSLOCK_P_H
#define QWSLOCK_P_H

#include <qglobal.h>

#ifndef QT_NO_QWS_MULTIPROCESS

#ifdef QT_POSIX_IPC
#  include <semaphore.h>
#endif

QT_BEGIN_NAMESPACE

class QWSLock
{
 public:
   enum LockType { BackingStore, Communication, RegionEvent };

   QWSLock(int lockId = -1);
   ~QWSLock();

   bool lock(LockType type, int timeout = -1);
   void unlock(LockType type);
   bool wait(LockType type, int timeout = -1);
   bool hasLock(LockType type);
   int id() const {
      return semId;
   }

 private:
   bool up(unsigned short semNum);
   bool down(unsigned short semNum, int timeout);
   int getValue(unsigned short semNum) const;

   int semId;
   int lockCount[2];

#ifdef QT_POSIX_IPC
   sem_t *sems[3];
   bool owned;
#endif
};

QT_END_NAMESPACE

#endif // QT_NO_QWS_MULTIPROCESS

#endif // QWSLOCK_P_H
