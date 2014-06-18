/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <QtCore/qglobal.h>
#include <QtCore/qhash.h>

QT_BEGIN_NAMESPACE

QT_USE_NAMESPACE

#define UNLOCKED    {-1,-1,-1,-1}
#define UNLOCKED2      UNLOCKED,UNLOCKED
#define UNLOCKED4     UNLOCKED2,UNLOCKED2
#define UNLOCKED8     UNLOCKED4,UNLOCKED4
#define UNLOCKED16    UNLOCKED8,UNLOCKED8
#define UNLOCKED32   UNLOCKED16,UNLOCKED16
#define UNLOCKED64   UNLOCKED32,UNLOCKED32
#define UNLOCKED128  UNLOCKED64,UNLOCKED64
#define UNLOCKED256 UNLOCKED128,UNLOCKED128

// use a 4k page for locks
static int locks[256][4] = { UNLOCKED256 };

int *getLock(volatile void *addr)
{
   return locks[qHash(const_cast<void *>(addr)) % 256];
}

static int *align16(int *lock)
{
   ulong off = (((ulong) lock) % 16);
   return off ? (int *)(ulong(lock) + 16 - off) : lock;
}

extern "C" {

   int q_ldcw(volatile int *addr);

   void q_atomic_lock(int *lock)
   {
      // ldcw requires a 16-byte aligned address
      volatile int *x = align16(lock);
      while (q_ldcw(x) == 0)
         ;
   }

   void q_atomic_unlock(int *lock)
   {
      lock[0] = lock[1] = lock[2] = lock[3] = -1;
   }
}


QT_END_NAMESPACE
