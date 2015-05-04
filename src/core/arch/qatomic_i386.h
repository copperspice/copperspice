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

#ifndef QATOMIC_I386_H
#define QATOMIC_I386_H

QT_BEGIN_NAMESPACE

#define Q_ATOMIC_INT_REFERENCE_COUNTING_IS_ALWAYS_NATIVE
#define Q_ATOMIC_INT_REFERENCE_COUNTING_IS_WAIT_FREE

inline bool QBasicAtomicInt::isReferenceCountingNative()
{
   return true;
}
inline bool QBasicAtomicInt::isReferenceCountingWaitFree()
{
   return true;
}

#define Q_ATOMIC_INT_TEST_AND_SET_IS_ALWAYS_NATIVE
#define Q_ATOMIC_INT_TEST_AND_SET_IS_WAIT_FREE

inline bool QBasicAtomicInt::isTestAndSetNative()
{
   return true;
}
inline bool QBasicAtomicInt::isTestAndSetWaitFree()
{
   return true;
}

#define Q_ATOMIC_INT_FETCH_AND_STORE_IS_ALWAYS_NATIVE
#define Q_ATOMIC_INT_FETCH_AND_STORE_IS_WAIT_FREE

inline bool QBasicAtomicInt::isFetchAndStoreNative()
{
   return true;
}
inline bool QBasicAtomicInt::isFetchAndStoreWaitFree()
{
   return true;
}

#define Q_ATOMIC_INT_FETCH_AND_ADD_IS_ALWAYS_NATIVE
#define Q_ATOMIC_INT_FETCH_AND_ADD_IS_WAIT_FREE

inline bool QBasicAtomicInt::isFetchAndAddNative()
{
   return true;
}
inline bool QBasicAtomicInt::isFetchAndAddWaitFree()
{
   return true;
}

#define Q_ATOMIC_POINTER_TEST_AND_SET_IS_ALWAYS_NATIVE
#define Q_ATOMIC_POINTER_TEST_AND_SET_IS_WAIT_FREE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isTestAndSetNative()
{
   return true;
}
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isTestAndSetWaitFree()
{
   return true;
}

#define Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_ALWAYS_NATIVE
#define Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_WAIT_FREE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndStoreNative()
{
   return true;
}
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndStoreWaitFree()
{
   return true;
}

#define Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_ALWAYS_NATIVE
#define Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_WAIT_FREE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndAddNative()
{
   return true;
}
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndAddWaitFree()
{
   return true;
}

#if defined(Q_CC_GNU) || defined(Q_CC_INTEL)

inline bool QBasicAtomicInt::ref()
{
   unsigned char ret;
   asm volatile("lock\n"
                "incl %0\n"
                "setne %1"
                : "=m" (_q_value), "=qm" (ret)
                : "m" (_q_value)
                : "memory");
   return ret != 0;
}

inline bool QBasicAtomicInt::deref()
{
   unsigned char ret;
   asm volatile("lock\n"
                "decl %0\n"
                "setne %1"
                : "=m" (_q_value), "=qm" (ret)
                : "m" (_q_value)
                : "memory");
   return ret != 0;
}

inline bool QBasicAtomicInt::testAndSetOrdered(int expectedValue, int newValue)
{
   unsigned char ret;
   asm volatile("lock\n"
                "cmpxchgl %3,%2\n"
                "sete %1\n"
                : "=a" (newValue), "=qm" (ret), "+m" (_q_value)
                : "r" (newValue), "0" (expectedValue)
                : "memory");
   return ret != 0;
}

inline int QBasicAtomicInt::fetchAndStoreOrdered(int newValue)
{
   asm volatile("xchgl %0,%1"
                : "=r" (newValue), "+m" (_q_value)
                : "0" (newValue)
                : "memory");
   return newValue;
}

inline int QBasicAtomicInt::fetchAndAddOrdered(int valueToAdd)
{
   asm volatile("lock\n"
                "xaddl %0,%1"
                : "=r" (valueToAdd), "+m" (_q_value)
                : "0" (valueToAdd)
                : "memory");
   return valueToAdd;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetOrdered(T *expectedValue, T *newValue)
{
   unsigned char ret;
   asm volatile("lock\n"
                "cmpxchgl %3,%2\n"
                "sete %1\n"
                : "=a" (newValue), "=qm" (ret), "+m" (_q_value)
                : "r" (newValue), "0" (expectedValue)
                : "memory");
   return ret != 0;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreOrdered(T *newValue)
{
   asm volatile("xchgl %0,%1"
                : "=r" (newValue), "+m" (_q_value)
                : "0" (newValue)
                : "memory");
   return newValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddOrdered(qptrdiff valueToAdd)
{
   asm volatile("lock\n"
                "xaddl %0,%1"
                : "=r" (valueToAdd), "+m" (_q_value)
                : "0" (valueToAdd * sizeof(T))
                : "memory");
   return reinterpret_cast<T *>(valueToAdd);
}

#else

extern "C" {
   Q_CORE_EXPORT int q_atomic_test_and_set_int(volatile int *ptr, int expected, int newval);
   Q_CORE_EXPORT int q_atomic_test_and_set_ptr(volatile void *ptr, const void *expected, const void *newval);
   Q_CORE_EXPORT int q_atomic_increment(volatile int *ptr);
   Q_CORE_EXPORT int q_atomic_decrement(volatile int *ptr);
   Q_CORE_EXPORT int q_atomic_set_int(volatile int *ptr, int newval);
   Q_CORE_EXPORT void *q_atomic_set_ptr(volatile void *ptr, void *newval);
   Q_CORE_EXPORT int q_atomic_fetch_and_add_int(volatile int *ptr, int value);
   Q_CORE_EXPORT void *q_atomic_fetch_and_add_ptr(volatile void *ptr, int value);
} // extern "C"

inline bool QBasicAtomicInt::ref()
{
   return q_atomic_increment(&_q_value) != 0;
}

inline bool QBasicAtomicInt::deref()
{
   return q_atomic_decrement(&_q_value) != 0;
}

inline bool QBasicAtomicInt::testAndSetOrdered(int expectedValue, int newValue)
{
   return q_atomic_test_and_set_int(&_q_value, expectedValue, newValue) != 0;
}

inline int QBasicAtomicInt::fetchAndStoreOrdered(int newValue)
{
   return q_atomic_set_int(&_q_value, newValue);
}

inline int QBasicAtomicInt::fetchAndAddOrdered(int valueToAdd)
{
   return q_atomic_fetch_and_add_int(&_q_value, valueToAdd);
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetOrdered(T *expectedValue, T *newValue)
{
   return q_atomic_test_and_set_ptr(&_q_value, expectedValue, newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreOrdered(T *newValue)
{
   return reinterpret_cast<T *>(q_atomic_set_ptr(&_q_value, newValue));
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddOrdered(qptrdiff valueToAdd)
{
   return reinterpret_cast<T *>(q_atomic_fetch_and_add_ptr(&_q_value, valueToAdd * sizeof(T)));
}

#endif

inline bool QBasicAtomicInt::testAndSetRelaxed(int expectedValue, int newValue)
{
   return testAndSetOrdered(expectedValue, newValue);
}

inline bool QBasicAtomicInt::testAndSetAcquire(int expectedValue, int newValue)
{
   return testAndSetOrdered(expectedValue, newValue);
}

inline bool QBasicAtomicInt::testAndSetRelease(int expectedValue, int newValue)
{
   return testAndSetOrdered(expectedValue, newValue);
}

inline int QBasicAtomicInt::fetchAndStoreRelaxed(int newValue)
{
   return fetchAndStoreOrdered(newValue);
}

inline int QBasicAtomicInt::fetchAndStoreAcquire(int newValue)
{
   return fetchAndStoreOrdered(newValue);
}

inline int QBasicAtomicInt::fetchAndStoreRelease(int newValue)
{
   return fetchAndStoreOrdered(newValue);
}

inline int QBasicAtomicInt::fetchAndAddRelaxed(int valueToAdd)
{
   return fetchAndAddOrdered(valueToAdd);
}

inline int QBasicAtomicInt::fetchAndAddAcquire(int valueToAdd)
{
   return fetchAndAddOrdered(valueToAdd);
}

inline int QBasicAtomicInt::fetchAndAddRelease(int valueToAdd)
{
   return fetchAndAddOrdered(valueToAdd);
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelaxed(T *expectedValue, T *newValue)
{
   return testAndSetOrdered(expectedValue, newValue);
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetAcquire(T *expectedValue, T *newValue)
{
   return testAndSetOrdered(expectedValue, newValue);
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelease(T *expectedValue, T *newValue)
{
   return testAndSetOrdered(expectedValue, newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelaxed(T *newValue)
{
   return fetchAndStoreOrdered(newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreAcquire(T *newValue)
{
   return fetchAndStoreOrdered(newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelease(T *newValue)
{
   return fetchAndStoreOrdered(newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelaxed(qptrdiff valueToAdd)
{
   return fetchAndAddOrdered(valueToAdd);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddAcquire(qptrdiff valueToAdd)
{
   return fetchAndAddOrdered(valueToAdd);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelease(qptrdiff valueToAdd)
{
   return fetchAndAddOrdered(valueToAdd);
}

QT_END_NAMESPACE

#endif // QATOMIC_I386_H
