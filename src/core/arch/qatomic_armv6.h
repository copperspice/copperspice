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

#ifndef QATOMIC_ARMV6_H
#define QATOMIC_ARMV6_H

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

#ifndef Q_DATA_MEMORY_BARRIER
# define Q_DATA_MEMORY_BARRIER asm volatile("":::"memory")
#endif

#ifndef Q_COMPILER_MEMORY_BARRIER
# define Q_COMPILER_MEMORY_BARRIER asm volatile("":::"memory")
#endif

inline bool QBasicAtomicInt::ref()
{
   register int newValue;
   register int result;
   asm volatile("0:\n"
                "ldrex %[newValue], [%[_q_value]]\n"
                "add %[newValue], %[newValue], #1\n"
                "strex %[result], %[newValue], [%[_q_value]]\n"
                "teq %[result], #0\n"
                "bne 0b\n"
                : [newValue] "=&r" (newValue),
                [result] "=&r" (result),
                "+m" (_q_value)
                : [_q_value] "r" (&_q_value)
                : "cc", "memory");
   return newValue != 0;
}

inline bool QBasicAtomicInt::deref()
{
   register int newValue;
   register int result;
   asm volatile("0:\n"
                "ldrex %[newValue], [%[_q_value]]\n"
                "sub %[newValue], %[newValue], #1\n"
                "strex %[result], %[newValue], [%[_q_value]]\n"
                "teq %[result], #0\n"
                "bne 0b\n"
                : [newValue] "=&r" (newValue),
                [result] "=&r" (result),
                "+m" (_q_value)
                : [_q_value] "r" (&_q_value)
                : "cc", "memory");
   return newValue != 0;
}

inline bool QBasicAtomicInt::testAndSetRelaxed(int expectedValue, int newValue)
{
   register int result;
   asm volatile("0:\n"
                "ldrex %[result], [%[_q_value]]\n"
                "eors %[result], %[result], %[expectedValue]\n"
                "itt eq\n"
                "strexeq %[result], %[newValue], [%[_q_value]]\n"
                "teqeq %[result], #1\n"
                "beq 0b\n"
                : [result] "=&r" (result),
                "+m" (_q_value)
                : [expectedValue] "r" (expectedValue),
                [newValue] "r" (newValue),
                [_q_value] "r" (&_q_value)
                : "cc");
   return result == 0;
}

inline int QBasicAtomicInt::fetchAndStoreRelaxed(int newValue)
{
   register int originalValue;
   register int result;
   asm volatile("0:\n"
                "ldrex %[originalValue], [%[_q_value]]\n"
                "strex %[result], %[newValue], [%[_q_value]]\n"
                "teq %[result], #0\n"
                "bne 0b\n"
                : [originalValue] "=&r" (originalValue),
                [result] "=&r" (result),
                "+m" (_q_value)
                : [newValue] "r" (newValue),
                [_q_value] "r" (&_q_value)
                : "cc");
   return originalValue;
}

inline int QBasicAtomicInt::fetchAndAddRelaxed(int valueToAdd)
{
   register int originalValue;
   register int newValue;
   register int result;
   asm volatile("0:\n"
                "ldrex %[originalValue], [%[_q_value]]\n"
                "add %[newValue], %[originalValue], %[valueToAdd]\n"
                "strex %[result], %[newValue], [%[_q_value]]\n"
                "teq %[result], #0\n"
                "bne 0b\n"
                : [originalValue] "=&r" (originalValue),
                [newValue] "=&r" (newValue),
                [result] "=&r" (result),
                "+m" (_q_value)
                : [valueToAdd] "r" (valueToAdd),
                [_q_value] "r" (&_q_value)
                : "cc");
   return originalValue;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelaxed(T *expectedValue, T *newValue)
{
   register T *result;
   asm volatile("0:\n"
                "ldrex %[result], [%[_q_value]]\n"
                "eors %[result], %[result], %[expectedValue]\n"
                "itt eq\n"
                "strexeq %[result], %[newValue], [%[_q_value]]\n"
                "teqeq %[result], #1\n"
                "beq 0b\n"
                : [result] "=&r" (result),
                "+m" (_q_value)
                : [expectedValue] "r" (expectedValue),
                [newValue] "r" (newValue),
                [_q_value] "r" (&_q_value)
                : "cc");
   return result == 0;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelaxed(T *newValue)
{
   register T *originalValue;
   register int result;
   asm volatile("0:\n"
                "ldrex %[originalValue], [%[_q_value]]\n"
                "strex %[result], %[newValue], [%[_q_value]]\n"
                "teq %[result], #0\n"
                "bne 0b\n"
                : [originalValue] "=&r" (originalValue),
                [result] "=&r" (result),
                "+m" (_q_value)
                : [newValue] "r" (newValue),
                [_q_value] "r" (&_q_value)
                : "cc");
   return originalValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelaxed(qptrdiff valueToAdd)
{
   register T *originalValue;
   register T *newValue;
   register int result;
   asm volatile("0:\n"
                "ldrex %[originalValue], [%[_q_value]]\n"
                "add %[newValue], %[originalValue], %[valueToAdd]\n"
                "strex %[result], %[newValue], [%[_q_value]]\n"
                "teq %[result], #0\n"
                "bne 0b\n"
                : [originalValue] "=&r" (originalValue),
                [newValue] "=&r" (newValue),
                [result] "=&r" (result),
                "+m" (_q_value)
                : [valueToAdd] "r" (valueToAdd * sizeof(T)),
                [_q_value] "r" (&_q_value)
                : "cc");
   return originalValue;
}





// common code

inline bool QBasicAtomicInt::testAndSetAcquire(int expectedValue, int newValue)
{
   bool returnValue = testAndSetRelaxed(expectedValue, newValue);
   Q_DATA_MEMORY_BARRIER;
   return returnValue;
}

inline bool QBasicAtomicInt::testAndSetRelease(int expectedValue, int newValue)
{
   Q_DATA_MEMORY_BARRIER;
   return testAndSetRelaxed(expectedValue, newValue);
}

inline bool QBasicAtomicInt::testAndSetOrdered(int expectedValue, int newValue)
{
   Q_DATA_MEMORY_BARRIER;
   bool returnValue = testAndSetRelaxed(expectedValue, newValue);
   Q_COMPILER_MEMORY_BARRIER;
   return returnValue;
}

inline int QBasicAtomicInt::fetchAndStoreAcquire(int newValue)
{
   int returnValue = fetchAndStoreRelaxed(newValue);
   Q_DATA_MEMORY_BARRIER;
   return returnValue;
}

inline int QBasicAtomicInt::fetchAndStoreRelease(int newValue)
{
   Q_DATA_MEMORY_BARRIER;
   return fetchAndStoreRelaxed(newValue);
}

inline int QBasicAtomicInt::fetchAndStoreOrdered(int newValue)
{
   Q_DATA_MEMORY_BARRIER;
   int returnValue = fetchAndStoreRelaxed(newValue);
   Q_COMPILER_MEMORY_BARRIER;
   return returnValue;
}


inline int QBasicAtomicInt::fetchAndAddAcquire(int valueToAdd)
{
   int returnValue = fetchAndAddRelaxed(valueToAdd);
   Q_DATA_MEMORY_BARRIER;
   return returnValue;
}

inline int QBasicAtomicInt::fetchAndAddRelease(int valueToAdd)
{
   Q_DATA_MEMORY_BARRIER;
   return fetchAndAddRelaxed(valueToAdd);
}

inline int QBasicAtomicInt::fetchAndAddOrdered(int valueToAdd)
{
   Q_DATA_MEMORY_BARRIER;
   int returnValue = fetchAndAddRelaxed(valueToAdd);
   Q_COMPILER_MEMORY_BARRIER;
   return returnValue;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetAcquire(T *expectedValue, T *newValue)
{
   bool returnValue = testAndSetRelaxed(expectedValue, newValue);
   Q_DATA_MEMORY_BARRIER;
   return returnValue;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelease(T *expectedValue, T *newValue)
{
   Q_DATA_MEMORY_BARRIER;
   return testAndSetRelaxed(expectedValue, newValue);
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetOrdered(T *expectedValue, T *newValue)
{
   Q_DATA_MEMORY_BARRIER;
   bool returnValue = testAndSetAcquire(expectedValue, newValue);
   Q_COMPILER_MEMORY_BARRIER;
   return returnValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreAcquire(T *newValue)
{
   T *returnValue = fetchAndStoreRelaxed(newValue);
   Q_DATA_MEMORY_BARRIER;
   return returnValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelease(T *newValue)
{
   Q_DATA_MEMORY_BARRIER;
   return fetchAndStoreRelaxed(newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreOrdered(T *newValue)
{
   Q_DATA_MEMORY_BARRIER;
   T *returnValue = fetchAndStoreRelaxed(newValue);
   Q_COMPILER_MEMORY_BARRIER;
   return returnValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddAcquire(qptrdiff valueToAdd)
{
   T *returnValue = fetchAndAddRelaxed(valueToAdd);
   Q_DATA_MEMORY_BARRIER;
   return returnValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelease(qptrdiff valueToAdd)
{
   Q_DATA_MEMORY_BARRIER;
   return fetchAndAddRelaxed(valueToAdd);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddOrdered(qptrdiff valueToAdd)
{
   Q_DATA_MEMORY_BARRIER;
   T *returnValue = fetchAndAddRelaxed(valueToAdd);
   Q_COMPILER_MEMORY_BARRIER;
   return returnValue;
}

#undef Q_DATA_MEMORY_BARRIER
#undef Q_COMPILER_MEMORY_BARRIER

QT_END_NAMESPACE


#endif // QATOMIC_ARMV6_H
