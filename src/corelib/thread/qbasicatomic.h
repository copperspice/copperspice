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

#ifndef QBASICATOMIC_H
#define QBASICATOMIC_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class Q_CORE_EXPORT QBasicAtomicInt
{
 public:
#ifdef QT_ARCH_PARISC
   int _q_lock[4];
#endif
#if defined(QT_ARCH_WINDOWS)
   union { // needed for Q_BASIC_ATOMIC_INITIALIZER
      volatile long _q_value;
   };
#else
   volatile int _q_value;
#endif

   QBasicAtomicInt() = default;
   explicit QBasicAtomicInt(int value) : _q_value(value) {}

   // Atomic API, implemented in qatomic_XXX.h

   int load() const {
      return _q_value;
   }
   int loadAcquire() {
      return _q_value;
   }
   void store(int newValue) {
      _q_value = newValue;
   }
   void storeRelease(int newValue) {
      _q_value = newValue;
   }

   static bool isReferenceCountingNative();
   static bool isReferenceCountingWaitFree();

   bool ref();
   bool deref();

   static bool isTestAndSetNative();
   static bool isTestAndSetWaitFree();

   bool testAndSetRelaxed(int expectedValue, int newValue);
   bool testAndSetAcquire(int expectedValue, int newValue);
   bool testAndSetRelease(int expectedValue, int newValue);
   bool testAndSetOrdered(int expectedValue, int newValue);

   static bool isFetchAndStoreNative();
   static bool isFetchAndStoreWaitFree();

   int fetchAndStoreRelaxed(int newValue);
   int fetchAndStoreAcquire(int newValue);
   int fetchAndStoreRelease(int newValue);
   int fetchAndStoreOrdered(int newValue);

   static bool isFetchAndAddNative();
   static bool isFetchAndAddWaitFree();

   int fetchAndAddRelaxed(int valueToAdd);
   int fetchAndAddAcquire(int valueToAdd);
   int fetchAndAddRelease(int valueToAdd);
   int fetchAndAddOrdered(int valueToAdd);
};

template <typename T>
class QBasicAtomicPointer
{
 public:
#ifdef QT_ARCH_PARISC
   int _q_lock[4];
#endif
#if defined(QT_ARCH_WINDOWS)
   union {
      T *volatile _q_value;
#  if !defined(__i386__) && !defined(_M_IX86)
      qint64
#  else
      long
#  endif
      volatile _q_value_integral;
   };
#else
   T *volatile _q_value;
#endif
   QBasicAtomicPointer() = default;
   explicit QBasicAtomicPointer(T *value) : _q_value(value) {}

   // Atomic API, implemented in qatomic_XXX.h

   T *load() const {
      return _q_value;
   }
   T *loadAcquire() {
      return _q_value;
   }
   void store(T *newValue) {
      _q_value = newValue;
   }
   void storeRelease(T *newValue) {
      _q_value = newValue;
   }

   static bool isTestAndSetNative();
   static bool isTestAndSetWaitFree();

   bool testAndSetRelaxed(T *expectedValue, T *newValue);
   bool testAndSetAcquire(T *expectedValue, T *newValue);
   bool testAndSetRelease(T *expectedValue, T *newValue);
   bool testAndSetOrdered(T *expectedValue, T *newValue);

   static bool isFetchAndStoreNative();
   static bool isFetchAndStoreWaitFree();

   T *fetchAndStoreRelaxed(T *newValue);
   T *fetchAndStoreAcquire(T *newValue);
   T *fetchAndStoreRelease(T *newValue);
   T *fetchAndStoreOrdered(T *newValue);

   static bool isFetchAndAddNative();
   static bool isFetchAndAddWaitFree();

   T *fetchAndAddRelaxed(qptrdiff valueToAdd);
   T *fetchAndAddAcquire(qptrdiff valueToAdd);
   T *fetchAndAddRelease(qptrdiff valueToAdd);
   T *fetchAndAddOrdered(qptrdiff valueToAdd);
};

#define Q_BASIC_ATOMIC_INITIALIZER(a) QBasicAtomicInt{a}

QT_END_NAMESPACE

#include <QtCore/qatomic_arch.h>

#endif // QBASIC_ATOMIC
