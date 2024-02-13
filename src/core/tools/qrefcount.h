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

#ifndef QREFCOUNT_H
#define QREFCOUNT_H

#include <qassert.h>
#include <qatomic.h>
#include <qglobal.h>

namespace QtPrivate {

class RefCount
{
 public:
   bool ref() {
      int count = atomic.load();

      if (count == 0) {
         // !isSharable
         return false;
      }

      if (count != -1) {
         // !isStatic
         atomic.ref();
      }

      return true;
   }

   bool deref() {
      int count = atomic.load();

      if (count == 0) {
         // !isSharable
         return false;
      }

      if (count == -1) {
         // isStatic
         return true;
      }

      return atomic.deref();
   }

   bool setSharable(bool sharable) {
      Q_ASSERT(!isShared());

      if (sharable) {
         int expected = 0;
         return atomic.compareExchange(expected, 1, std::memory_order_relaxed);
      } else {
         int expected = 1;
         return atomic.compareExchange(expected, 0, std::memory_order_relaxed);
      }
   }

   bool isStatic() const {
      // Persistent object, never deleted
      return atomic.load() == -1;
   }

   bool isSharable() const {
      // Sharable === Shared ownership.
      return atomic.load() != 0;
   }

   bool isShared() const {
      int count = atomic.load();
      return (count != 1) && (count != 0);
   }

   bool operator==(int value) const {
      return atomic.load() == value;
   }

   bool operator!=(int value) const {
      return atomic.load() != value;
   }

   bool operator!() const {
      return !atomic.load();
   }

   operator int() const {
      return atomic.load();
   }

   void initializeOwned() {
      atomic.store(1);
   }

   QAtomicInt atomic;
};

}

#define Q_REFCOUNT_INITIALIZE_STATIC { -1 }

#endif
