/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#include <QtCore/qatomic.h>

#include <limits.h>
#include <sched.h>

extern "C" {

   int q_atomic_trylock_int(volatile int *addr);
   int q_atomic_trylock_ptr(volatile void *addr);

   Q_CORE_EXPORT int q_atomic_lock_int(volatile int *addr)
   {
      int returnValue = q_atomic_trylock_int(addr);

      if (returnValue == INT_MIN) {
         do {
            // spin until we think we can succeed
            do {
               sched_yield();
               returnValue = *addr;
            } while (returnValue == INT_MIN);

            // try again
            returnValue = q_atomic_trylock_int(addr);
         } while (returnValue == INT_MIN);
      }

      return returnValue;
   }

   Q_CORE_EXPORT int q_atomic_lock_ptr(volatile void *addr)
   {
      int returnValue = q_atomic_trylock_ptr(addr);

      if (returnValue == -1) {
         do {
            // spin until we think we can succeed
            do {
               sched_yield();
               returnValue = *reinterpret_cast<volatile int *>(addr);
            } while (returnValue == -1);

            // try again
            returnValue = q_atomic_trylock_ptr(addr);
         } while (returnValue == -1);
      }

      return returnValue;
   }

} // extern "C"
