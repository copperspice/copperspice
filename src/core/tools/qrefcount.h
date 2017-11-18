/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QREFCOUNT_H
#define QREFCOUNT_H

#include <qglobal.h>
#include <qassert.h>
#include <qatomic.h>

namespace QtPrivate {

class RefCount
{
 public:
   inline bool ref() {
      int count = atomic.load();
      if (count == 0) { // !isSharable
         return false;
      }
      if (count != -1) { // !isStatic
         atomic.ref();
      }
      return true;
   }

   inline bool deref() {
      int count = atomic.load();
      if (count == 0) { // !isSharable
         return false;
      }
      if (count == -1) { // isStatic
         return true;
      }
      return atomic.deref();
   }

   bool setSharable(bool sharable) {
      Q_ASSERT(!isShared());
      if (sharable) {
         return atomic.testAndSetRelaxed(0, 1);
      } else {
         return atomic.testAndSetRelaxed(1, 0);
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

   inline bool operator==(int value) const {
      return atomic.load() == value;
   }

   inline bool operator!=(int value) const {
      return atomic.load() != value;
   }

   inline bool operator!() const {
      return !atomic.load();
   }

   inline operator int() const {
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
