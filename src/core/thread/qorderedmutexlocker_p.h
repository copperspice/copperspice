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

#ifndef QORDEREDMUTEXLOCKER_P_H
#define QORDEREDMUTEXLOCKER_P_H

#include <qmutex.h>

/*
  Locks 2 mutexes in a defined order, avoiding a recursive lock if
  we are trying to lock the same mutex twice.
*/
class QOrderedMutexLocker
{
 public:
   QOrderedMutexLocker(QMutex *m1, QMutex *m2)
      : mtx1((m1 == m2) ? m1 : (m1 < m2 ? m1 : m2)),
        mtx2((m1 == m2) ? nullptr : (m1 < m2 ? m2 : m1)),
        locked(false) {
      relock();
   }
   ~QOrderedMutexLocker() {
      unlock();
   }

   void relock() {
      if (!locked) {
         if (mtx1) {
            mtx1->lock();
         }

         if (mtx2) {
            mtx2->lock();
         }

         locked = true;
      }
   }

   void unlock() {
      if (locked) {
         if (mtx1) {
            mtx1->unlock();
         }

         if (mtx2) {
            mtx2->unlock();
         }

         locked = false;
      }
   }

   static bool relock(QMutex *mtx1, QMutex *mtx2) {
      // mtx1 is already locked, mtx2 not... do we need to unlock and relock?
      if (mtx1 == mtx2) {
         return false;
      }

      if (mtx1 < mtx2) {
         mtx2->lock();
         return true;
      }

      if (!mtx2->tryLock()) {
         mtx1->unlock();
         mtx2->lock();
         mtx1->lock();
      }

      return true;
   }

 private:
   QMutex *mtx1, *mtx2;
   bool locked;
};

#endif
