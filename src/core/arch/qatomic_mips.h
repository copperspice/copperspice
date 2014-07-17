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

#ifndef QATOMIC_MIPS_H
#define QATOMIC_MIPS_H

QT_BEGIN_NAMESPACE

#define Q_ATOMIC_INT_REFERENCE_COUNTING_IS_ALWAYS_NATIVE

inline bool QBasicAtomicInt::isReferenceCountingNative()
{
   return true;
}
inline bool QBasicAtomicInt::isReferenceCountingWaitFree()
{
   return false;
}

#define Q_ATOMIC_INT_TEST_AND_SET_IS_ALWAYS_NATIVE

inline bool QBasicAtomicInt::isTestAndSetNative()
{
   return true;
}
inline bool QBasicAtomicInt::isTestAndSetWaitFree()
{
   return false;
}

#define Q_ATOMIC_INT_FETCH_AND_STORE_IS_ALWAYS_NATIVE

inline bool QBasicAtomicInt::isFetchAndStoreNative()
{
   return true;
}
inline bool QBasicAtomicInt::isFetchAndStoreWaitFree()
{
   return false;
}

#define Q_ATOMIC_INT_FETCH_AND_ADD_IS_ALWAYS_NATIVE

inline bool QBasicAtomicInt::isFetchAndAddNative()
{
   return true;
}
inline bool QBasicAtomicInt::isFetchAndAddWaitFree()
{
   return false;
}

#define Q_ATOMIC_POINTER_TEST_AND_SET_IS_ALWAYS_NATIVE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isTestAndSetNative()
{
   return true;
}
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isTestAndSetWaitFree()
{
   return false;
}

#define Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_ALWAYS_NATIVE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndStoreNative()
{
   return true;
}
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndStoreWaitFree()
{
   return false;
}

#define Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_ALWAYS_NATIVE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndAddNative()
{
   return true;
}
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndAddWaitFree()
{
   return false;
}

#if defined(Q_CC_GNU)

#if _MIPS_SIM == _ABIO32
#define SET_MIPS2 ".set mips2\n\t"
#else
#define SET_MIPS2
#endif

inline bool QBasicAtomicInt::ref()
{
   register int originalValue;
   register int newValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "0:\n"
                "ll %[originalValue], %[_q_value]\n"
                "addiu %[newValue], %[originalValue], %[one]\n"
                "sc %[newValue], %[_q_value]\n"
                "beqz %[newValue], 0b\n"
                "nop\n"
                ".set pop\n"
                : [originalValue] "=&r" (originalValue),
                [_q_value] "+m" (_q_value),
                [newValue] "=&r" (newValue)
                : [one] "i" (1)
                : "cc", "memory");
   return originalValue != -1;
}

inline bool QBasicAtomicInt::deref()
{
   register int originalValue;
   register int newValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "0:\n"
                "ll %[originalValue], %[_q_value]\n"
                "addiu %[newValue], %[originalValue], %[minusOne]\n"
                "sc %[newValue], %[_q_value]\n"
                "beqz %[newValue], 0b\n"
                "nop\n"
                ".set pop\n"
                : [originalValue] "=&r" (originalValue),
                [_q_value] "+m" (_q_value),
                [newValue] "=&r" (newValue)
                : [minusOne] "i" (-1)
                : "cc", "memory");
   return originalValue != 1;
}

inline bool QBasicAtomicInt::testAndSetRelaxed(int expectedValue, int newValue)
{
   register int result;
   register int tempValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "0:\n"
                "ll %[result], %[_q_value]\n"
                "xor %[result], %[result], %[expectedValue]\n"
                "bnez %[result], 0f\n"
                "nop\n"
                "move %[tempValue], %[newValue]\n"
                "sc %[tempValue], %[_q_value]\n"
                "beqz %[tempValue], 0b\n"
                "nop\n"
                "0:\n"
                ".set pop\n"
                : [result] "=&r" (result),
                [tempValue] "=&r" (tempValue),
                [_q_value] "+m" (_q_value)
                : [expectedValue] "r" (expectedValue),
                [newValue] "r" (newValue)
                : "cc", "memory");
   return result == 0;
}

inline bool QBasicAtomicInt::testAndSetAcquire(int expectedValue, int newValue)
{
   register int result;
   register int tempValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "0:\n"
                "ll %[result], %[_q_value]\n"
                "xor %[result], %[result], %[expectedValue]\n"
                "bnez %[result], 0f\n"
                "nop\n"
                "move %[tempValue], %[newValue]\n"
                "sc %[tempValue], %[_q_value]\n"
                "beqz %[tempValue], 0b\n"
                "nop\n"
                "sync\n"
                "0:\n"
                ".set pop\n"
                : [result] "=&r" (result),
                [tempValue] "=&r" (tempValue),
                [_q_value] "+m" (_q_value)
                : [expectedValue] "r" (expectedValue),
                [newValue] "r" (newValue)
                : "cc", "memory");
   return result == 0;
}

inline bool QBasicAtomicInt::testAndSetRelease(int expectedValue, int newValue)
{
   register int result;
   register int tempValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "sync\n"
                "0:\n"
                "ll %[result], %[_q_value]\n"
                "xor %[result], %[result], %[expectedValue]\n"
                "bnez %[result], 0f\n"
                "nop\n"
                "move %[tempValue], %[newValue]\n"
                "sc %[tempValue], %[_q_value]\n"
                "beqz %[tempValue], 0b\n"
                "nop\n"
                "0:\n"
                ".set pop\n"
                : [result] "=&r" (result),
                [tempValue] "=&r" (tempValue),
                [_q_value] "+m" (_q_value)
                : [expectedValue] "r" (expectedValue),
                [newValue] "r" (newValue)
                : "cc", "memory");
   return result == 0;
}

inline bool QBasicAtomicInt::testAndSetOrdered(int expectedValue, int newValue)
{
   return testAndSetAcquire(expectedValue, newValue);
}

inline int QBasicAtomicInt::fetchAndStoreRelaxed(int newValue)
{
   register int originalValue;
   register int tempValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "0:\n"
                "ll %[originalValue], %[_q_value]\n"
                "move %[tempValue], %[newValue]\n"
                "sc %[tempValue], %[_q_value]\n"
                "beqz %[tempValue], 0b\n"
                "nop\n"
                ".set pop\n"
                : [originalValue] "=&r" (originalValue),
                [tempValue] "=&r" (tempValue),
                [_q_value] "+m" (_q_value)
                : [newValue] "r" (newValue)
                : "cc", "memory");
   return originalValue;
}

inline int QBasicAtomicInt::fetchAndStoreAcquire(int newValue)
{
   register int originalValue;
   register int tempValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "0:\n"
                "ll %[originalValue], %[_q_value]\n"
                "move %[tempValue], %[newValue]\n"
                "sc %[tempValue], %[_q_value]\n"
                "beqz %[tempValue], 0b\n"
                "nop\n"
                "sync\n"
                ".set pop\n"
                : [originalValue] "=&r" (originalValue),
                [tempValue] "=&r" (tempValue),
                [_q_value] "+m" (_q_value)
                : [newValue] "r" (newValue)
                : "cc", "memory");
   return originalValue;
}

inline int QBasicAtomicInt::fetchAndStoreRelease(int newValue)
{
   register int originalValue;
   register int tempValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "sync\n"
                "0:\n"
                "ll %[originalValue], %[_q_value]\n"
                "move %[tempValue], %[newValue]\n"
                "sc %[tempValue], %[_q_value]\n"
                "beqz %[tempValue], 0b\n"
                "nop\n"
                ".set pop\n"
                : [originalValue] "=&r" (originalValue),
                [tempValue] "=&r" (tempValue),
                [_q_value] "+m" (_q_value)
                : [newValue] "r" (newValue)
                : "cc", "memory");
   return originalValue;
}

inline int QBasicAtomicInt::fetchAndStoreOrdered(int newValue)
{
   return fetchAndStoreAcquire(newValue);
}

inline int QBasicAtomicInt::fetchAndAddRelaxed(int valueToAdd)
{
   register int originalValue;
   register int newValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "0:\n"
                "ll %[originalValue], %[_q_value]\n"
                "addu %[newValue], %[originalValue], %[valueToAdd]\n"
                "sc %[newValue], %[_q_value]\n"
                "beqz %[newValue], 0b\n"
                "nop\n"
                ".set pop\n"
                : [originalValue] "=&r" (originalValue),
                [_q_value] "+m" (_q_value),
                [newValue] "=&r" (newValue)
                : [valueToAdd] "r" (valueToAdd)
                : "cc", "memory");
   return originalValue;
}

inline int QBasicAtomicInt::fetchAndAddAcquire(int valueToAdd)
{
   register int originalValue;
   register int newValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "0:\n"
                "ll %[originalValue], %[_q_value]\n"
                "addu %[newValue], %[originalValue], %[valueToAdd]\n"
                "sc %[newValue], %[_q_value]\n"
                "beqz %[newValue], 0b\n"
                "nop\n"
                "sync\n"
                ".set pop\n"
                : [originalValue] "=&r" (originalValue),
                [_q_value] "+m" (_q_value),
                [newValue] "=&r" (newValue)
                : [valueToAdd] "r" (valueToAdd)
                : "cc", "memory");
   return originalValue;
}

inline int QBasicAtomicInt::fetchAndAddRelease(int valueToAdd)
{
   register int originalValue;
   register int newValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "sync\n"
                "0:\n"
                "ll %[originalValue], %[_q_value]\n"
                "addu %[newValue], %[originalValue], %[valueToAdd]\n"
                "sc %[newValue], %[_q_value]\n"
                "beqz %[newValue], 0b\n"
                "nop\n"
                ".set pop\n"
                : [originalValue] "=&r" (originalValue),
                [_q_value] "+m" (_q_value),
                [newValue] "=&r" (newValue)
                : [valueToAdd] "r" (valueToAdd)
                : "cc", "memory");
   return originalValue;
}

inline int QBasicAtomicInt::fetchAndAddOrdered(int valueToAdd)
{
   return fetchAndAddAcquire(valueToAdd);
}

#if defined(__LP64__)
#  define LLP "lld"
#  define SCP "scd"
#else
#  define LLP "ll"
#  define SCP "sc"
#endif

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelaxed(T *expectedValue, T *newValue)
{
   register T *result;
   register T *tempValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "0:\n"
                LLP" %[result], %[_q_value]\n"
                "xor %[result], %[result], %[expectedValue]\n"
                "bnez %[result], 0f\n"
                "nop\n"
                "move %[tempValue], %[newValue]\n"
                SCP" %[tempValue], %[_q_value]\n"
                "beqz %[tempValue], 0b\n"
                "nop\n"
                "0:\n"
                ".set pop\n"
                : [result] "=&r" (result),
                [tempValue] "=&r" (tempValue),
                [_q_value] "+m" (_q_value)
                : [expectedValue] "r" (expectedValue),
                [newValue] "r" (newValue)
                : "cc", "memory");
   return result == 0;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetAcquire(T *expectedValue, T *newValue)
{
   register T *result;
   register T *tempValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "0:\n"
                LLP" %[result], %[_q_value]\n"
                "xor %[result], %[result], %[expectedValue]\n"
                "bnez %[result], 0f\n"
                "nop\n"
                "move %[tempValue], %[newValue]\n"
                SCP" %[tempValue], %[_q_value]\n"
                "beqz %[tempValue], 0b\n"
                "nop\n"
                "sync\n"
                "0:\n"
                ".set pop\n"
                : [result] "=&r" (result),
                [tempValue] "=&r" (tempValue),
                [_q_value] "+m" (_q_value)
                : [expectedValue] "r" (expectedValue),
                [newValue] "r" (newValue)
                : "cc", "memory");
   return result == 0;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelease(T *expectedValue, T *newValue)
{
   register T *result;
   register T *tempValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "sync\n"
                "0:\n"
                LLP" %[result], %[_q_value]\n"
                "xor %[result], %[result], %[expectedValue]\n"
                "bnez %[result], 0f\n"
                "nop\n"
                "move %[tempValue], %[newValue]\n"
                SCP" %[tempValue], %[_q_value]\n"
                "beqz %[tempValue], 0b\n"
                "nop\n"
                "0:\n"
                ".set pop\n"
                : [result] "=&r" (result),
                [tempValue] "=&r" (tempValue),
                [_q_value] "+m" (_q_value)
                : [expectedValue] "r" (expectedValue),
                [newValue] "r" (newValue)
                : "cc", "memory");
   return result == 0;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetOrdered(T *expectedValue, T *newValue)
{
   return testAndSetAcquire(expectedValue, newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelaxed(T *newValue)
{
   register T *originalValue;
   register T *tempValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "0:\n"
                LLP" %[originalValue], %[_q_value]\n"
                "move %[tempValue], %[newValue]\n"
                SCP" %[tempValue], %[_q_value]\n"
                "beqz %[tempValue], 0b\n"
                "nop\n"
                ".set pop\n"
                : [originalValue] "=&r" (originalValue),
                [tempValue] "=&r" (tempValue),
                [_q_value] "+m" (_q_value)
                : [newValue] "r" (newValue)
                : "cc", "memory");
   return originalValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreAcquire(T *newValue)
{
   register T *originalValue;
   register T *tempValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "0:\n"
                LLP" %[originalValue], %[_q_value]\n"
                "move %[tempValue], %[newValue]\n"
                SCP" %[tempValue], %[_q_value]\n"
                "beqz %[tempValue], 0b\n"
                "nop\n"
                "sync\n"
                ".set pop\n"
                : [originalValue] "=&r" (originalValue),
                [tempValue] "=&r" (tempValue),
                [_q_value] "+m" (_q_value)
                : [newValue] "r" (newValue)
                : "cc", "memory");
   return originalValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelease(T *newValue)
{
   register T *originalValue;
   register T *tempValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "sync\n"
                "0:\n"
                LLP" %[originalValue], %[_q_value]\n"
                "move %[tempValue], %[newValue]\n"
                SCP" %[tempValue], %[_q_value]\n"
                "beqz %[tempValue], 0b\n"
                "nop\n"
                ".set pop\n"
                : [originalValue] "=&r" (originalValue),
                [tempValue] "=&r" (tempValue),
                [_q_value] "+m" (_q_value)
                : [newValue] "r" (newValue)
                : "cc", "memory");
   return originalValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreOrdered(T *newValue)
{
   return fetchAndStoreAcquire(newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelaxed(qptrdiff valueToAdd)
{
   register T *originalValue;
   register T *newValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "0:\n"
                LLP" %[originalValue], %[_q_value]\n"
                "addu %[newValue], %[originalValue], %[valueToAdd]\n"
                SCP" %[newValue], %[_q_value]\n"
                "beqz %[newValue], 0b\n"
                "nop\n"
                ".set pop\n"
                : [originalValue] "=&r" (originalValue),
                [_q_value] "+m" (_q_value),
                [newValue] "=&r" (newValue)
                : [valueToAdd] "r" (valueToAdd * sizeof(T))
                : "cc", "memory");
   return originalValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddAcquire(qptrdiff valueToAdd)
{
   register T *originalValue;
   register T *newValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "0:\n"
                LLP" %[originalValue], %[_q_value]\n"
                "addu %[newValue], %[originalValue], %[valueToAdd]\n"
                SCP" %[newValue], %[_q_value]\n"
                "beqz %[newValue], 0b\n"
                "nop\n"
                "sync\n"
                ".set pop\n"
                : [originalValue] "=&r" (originalValue),
                [_q_value] "+m" (_q_value),
                [newValue] "=&r" (newValue)
                : [valueToAdd] "r" (valueToAdd * sizeof(T))
                : "cc", "memory");
   return originalValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelease(qptrdiff valueToAdd)
{
   register T *originalValue;
   register T *newValue;
   asm volatile(".set push\n"
                SET_MIPS2
                "sync\n"
                "0:\n"
                LLP" %[originalValue], %[_q_value]\n"
                "addu %[newValue], %[originalValue], %[valueToAdd]\n"
                SCP" %[newValue], %[_q_value]\n"
                "beqz %[newValue], 0b\n"
                "nop\n"
                ".set pop\n"
                : [originalValue] "=&r" (originalValue),
                [_q_value] "+m" (_q_value),
                [newValue] "=&r" (newValue)
                : [valueToAdd] "r" (valueToAdd * sizeof(T))
                : "cc", "memory");
   return originalValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddOrdered(qptrdiff valueToAdd)
{
   return fetchAndAddAcquire(valueToAdd);
}

#else // !Q_CC_GNU

extern "C" {
   Q_CORE_EXPORT int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval);
   Q_CORE_EXPORT int q_atomic_test_and_set_acquire_int(volatile int *ptr, int expected, int newval);
   Q_CORE_EXPORT int q_atomic_test_and_set_release_int(volatile int *ptr, int expected, int newval);
   Q_CORE_EXPORT int q_atomic_test_and_set_ptr(volatile void *ptr, void *expected, void *newval);
   Q_CORE_EXPORT int q_atomic_test_and_set_acquire_ptr(volatile void *ptr, void *expected, void *newval);
   Q_CORE_EXPORT int q_atomic_test_and_set_release_ptr(volatile void *ptr, void *expected, void *newval);
} // extern "C"

inline bool QBasicAtomicInt::ref()
{
   register int expected;
   for (;;) {
      expected = _q_value;
      if (q_atomic_test_and_set_int(&_q_value, expected, expected + 1)) {
         break;
      }
   }
   return expected != -1;
}

inline bool QBasicAtomicInt::deref()
{
   register int expected;
   for (;;) {
      expected = _q_value;
      if (q_atomic_test_and_set_int(&_q_value, expected, expected - 1)) {
         break;
      }
   }
   return expected != 1;
}

inline bool QBasicAtomicInt::testAndSetRelaxed(int expectedValue, int newValue)
{
   return q_atomic_test_and_set_int(&_q_value, expectedValue, newValue) != 0;
}

inline bool QBasicAtomicInt::testAndSetAcquire(int expectedValue, int newValue)
{
   return q_atomic_test_and_set_acquire_int(&_q_value, expectedValue, newValue) != 0;
}

inline bool QBasicAtomicInt::testAndSetRelease(int expectedValue, int newValue)
{
   return q_atomic_test_and_set_release_int(&_q_value, expectedValue, newValue) != 0;
}

inline bool QBasicAtomicInt::testAndSetOrdered(int expectedValue, int newValue)
{
   return q_atomic_test_and_set_acquire_int(&_q_value, expectedValue, newValue) != 0;
}

inline int QBasicAtomicInt::fetchAndStoreRelaxed(int newValue)
{
   int returnValue;
   for (;;) {
      returnValue = _q_value;
      if (testAndSetRelaxed(returnValue, newValue)) {
         break;
      }
   }
   return returnValue;
}

inline int QBasicAtomicInt::fetchAndStoreAcquire(int newValue)
{
   int returnValue;
   for (;;) {
      returnValue = _q_value;
      if (testAndSetAcquire(returnValue, newValue)) {
         break;
      }
   }
   return returnValue;
}

inline int QBasicAtomicInt::fetchAndStoreRelease(int newValue)
{
   int returnValue;
   for (;;) {
      returnValue = _q_value;
      if (testAndSetRelease(returnValue, newValue)) {
         break;
      }
   }
   return returnValue;
}

inline int QBasicAtomicInt::fetchAndStoreOrdered(int newValue)
{
   int returnValue;
   for (;;) {
      returnValue = _q_value;
      if (testAndSetOrdered(returnValue, newValue)) {
         break;
      }
   }
   return returnValue;
}

inline int QBasicAtomicInt::fetchAndAddRelaxed(int valueToAdd)
{
   int returnValue;
   for (;;) {
      returnValue = _q_value;
      if (testAndSetRelaxed(returnValue, returnValue + valueToAdd)) {
         break;
      }
   }
   return returnValue;
}

inline int QBasicAtomicInt::fetchAndAddAcquire(int valueToAdd)
{
   int returnValue;
   for (;;) {
      returnValue = _q_value;
      if (testAndSetAcquire(returnValue, returnValue + valueToAdd)) {
         break;
      }
   }
   return returnValue;
}

inline int QBasicAtomicInt::fetchAndAddRelease(int valueToAdd)
{
   int returnValue;
   for (;;) {
      returnValue = _q_value;
      if (testAndSetRelease(returnValue, returnValue + valueToAdd)) {
         break;
      }
   }
   return returnValue;
}

inline int QBasicAtomicInt::fetchAndAddOrdered(int valueToAdd)
{
   int returnValue;
   for (;;) {
      returnValue = _q_value;
      if (testAndSetOrdered(returnValue, returnValue + valueToAdd)) {
         break;
      }
   }
   return returnValue;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelaxed(T *expectedValue, T *newValue)
{
   return q_atomic_test_and_set_ptr(&_q_value, expectedValue, newValue) != 0;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetAcquire(T *expectedValue, T *newValue)
{
   return q_atomic_test_and_set_acquire_ptr(&_q_value, expectedValue, newValue) != 0;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelease(T *expectedValue, T *newValue)
{
   return q_atomic_test_and_set_release_ptr(&_q_value, expectedValue, newValue) != 0;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetOrdered(T *expectedValue, T *newValue)
{
   return q_atomic_test_and_set_acquire_ptr(&_q_value, expectedValue, newValue) != 0;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelaxed(T *newValue)
{
   T *returnValue;
   for (;;) {
      returnValue = (_q_value);
      if (testAndSetRelaxed(returnValue, newValue)) {
         break;
      }
   }
   return returnValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreAcquire(T *newValue)
{
   T *returnValue;
   for (;;) {
      returnValue = (_q_value);
      if (testAndSetAcquire(returnValue, newValue)) {
         break;
      }
   }
   return returnValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelease(T *newValue)
{
   T *returnValue;
   for (;;) {
      returnValue = (_q_value);
      if (testAndSetRelease(returnValue, newValue)) {
         break;
      }
   }
   return returnValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreOrdered(T *newValue)
{
   T *returnValue;
   for (;;) {
      returnValue = (_q_value);
      if (testAndSetOrdered(returnValue, newValue)) {
         break;
      }
   }
   return returnValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelaxed(qptrdiff valueToAdd)
{
   T *returnValue;
   for (;;) {
      returnValue = (_q_value);
      if (testAndSetRelaxed(returnValue, returnValue + valueToAdd)) {
         break;
      }
   }
   return returnValue;
}

template <typename T>
Q_INLINE_TEMPLATE
T *QBasicAtomicPointer<T>::fetchAndAddAcquire(qptrdiff valueToAdd)
{
   T *returnValue;
   for (;;) {
      returnValue = (_q_value);
      if (testAndSetAcquire(returnValue, returnValue + valueToAdd)) {
         break;
      }
   }
   return returnValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelease(qptrdiff valueToAdd)
{
   T *returnValue;
   for (;;) {
      returnValue = (_q_value);
      if (testAndSetRelease(returnValue, returnValue + valueToAdd)) {
         break;
      }
   }
   return returnValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddOrdered(qptrdiff valueToAdd)
{
   T *returnValue;
   for (;;) {
      returnValue = (_q_value);
      if (testAndSetOrdered(returnValue, returnValue + valueToAdd)) {
         break;
      }
   }
   return returnValue;
}

#endif // Q_CC_GNU

QT_END_NAMESPACE


#endif // QATOMIC_MIPS_H
