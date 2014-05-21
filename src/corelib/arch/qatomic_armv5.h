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

#ifndef QATOMIC_ARMV5_H
#define QATOMIC_ARMV5_H

QT_BEGIN_NAMESPACE

#define Q_ATOMIC_INT_REFERENCE_COUNTING_IS_NOT_NATIVE

inline bool QBasicAtomicInt::isReferenceCountingNative()
{ return false; }
inline bool QBasicAtomicInt::isReferenceCountingWaitFree()
{ return false; }

#define Q_ATOMIC_INT_TEST_AND_SET_IS_NOT_NATIVE

inline bool QBasicAtomicInt::isTestAndSetNative()
{ return false; }
inline bool QBasicAtomicInt::isTestAndSetWaitFree()
{ return false; }

#define Q_ATOMIC_INT_FETCH_AND_STORE_IS_ALWAYS_NATIVE
#define Q_ATOMIC_INT_FETCH_AND_STORE_IS_WAIT_FREE

inline bool QBasicAtomicInt::isFetchAndStoreNative()
{ return true; }
inline bool QBasicAtomicInt::isFetchAndStoreWaitFree()
{ return true; }

#define Q_ATOMIC_INT_FETCH_AND_ADD_IS_NOT_NATIVE

inline bool QBasicAtomicInt::isFetchAndAddNative()
{ return false; }
inline bool QBasicAtomicInt::isFetchAndAddWaitFree()
{ return false; }

#define Q_ATOMIC_POINTER_TEST_AND_SET_IS_NOT_NATIVE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isTestAndSetNative()
{ return false; }
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isTestAndSetWaitFree()
{ return false; }

#define Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_ALWAYS_NATIVE
#define Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_WAIT_FREE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndStoreNative()
{ return true; }
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndStoreWaitFree()
{ return true; }

#define Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_NOT_NATIVE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndAddNative()
{ return false; }
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndAddWaitFree()
{ return false; }

#ifndef QT_NO_ARM_EABI

// kernel places a restartable cmpxchg implementation at a fixed address
extern "C" typedef int (qt_atomic_eabi_cmpxchg_int_t)(int oldval, int newval, volatile int *ptr);
extern "C" typedef int (qt_atomic_eabi_cmpxchg_ptr_t)(const void *oldval, const void *newval, volatile void *ptr);
#define qt_atomic_eabi_cmpxchg_int (*reinterpret_cast<qt_atomic_eabi_cmpxchg_int_t *>(0xffff0fc0))
#define qt_atomic_eabi_cmpxchg_ptr (*reinterpret_cast<qt_atomic_eabi_cmpxchg_ptr_t *>(0xffff0fc0)) 

#else

extern Q_CORE_EXPORT char q_atomic_lock;
Q_CORE_EXPORT void qt_atomic_yield(int *);

inline char q_atomic_swp(volatile char *ptr, char newval)
{
    register char ret;
    asm volatile("swpb %0,%2,[%3]"
                 : "=&r"(ret), "=m" (*ptr)
                 : "r"(newval), "r"(ptr)
                 : "cc", "memory");
    return ret;
}

#endif // QT_NO_ARM_EABI

// Reference counting

inline bool QBasicAtomicInt::ref()
{
#ifndef QT_NO_ARM_EABI
    register int originalValue;
    register int newValue;
    do {
        originalValue = _q_value;
        newValue = originalValue + 1;
    } while (qt_atomic_eabi_cmpxchg_int(originalValue, newValue, &_q_value) != 0);
    return newValue != 0;
#else
    int count = 0;
    while (q_atomic_swp(&q_atomic_lock, ~0) != 0)
        qt_atomic_yield(&count);
    int originalValue = _q_value++;
    q_atomic_swp(&q_atomic_lock, 0);
    return originalValue != -1;
#endif
}

inline bool QBasicAtomicInt::deref()
{
#ifndef QT_NO_ARM_EABI
    register int originalValue;
    register int newValue;
    do {
        originalValue = _q_value;
        newValue = originalValue - 1;
    } while (qt_atomic_eabi_cmpxchg_int(originalValue, newValue, &_q_value) != 0);
    return newValue != 0;
#else
    int count = 0;
    while (q_atomic_swp(&q_atomic_lock, ~0) != 0)
        qt_atomic_yield(&count);
    int originalValue = _q_value--;
    q_atomic_swp(&q_atomic_lock, 0);
    return originalValue != 1;
#endif
}

// Test and set for integers

inline bool QBasicAtomicInt::testAndSetOrdered(int expectedValue, int newValue)
{
#ifndef QT_NO_ARM_EABI
    register int originalValue;
    do {
        originalValue = _q_value;
        if (originalValue != expectedValue)
            return false;
    } while (qt_atomic_eabi_cmpxchg_int(expectedValue, newValue, &_q_value) != 0);
    return true;
#else
    bool returnValue = false;
    int count = 0;
    while (q_atomic_swp(&q_atomic_lock, ~0) != 0)
        qt_atomic_yield(&count);
    if (_q_value == expectedValue) {
	_q_value = newValue;
	returnValue = true;
    }
    q_atomic_swp(&q_atomic_lock, 0);
    return returnValue;
#endif
}

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

// Fetch and store for integers

inline int QBasicAtomicInt::fetchAndStoreOrdered(int newValue)
{
    int originalValue;
#ifndef QT_NO_ARM_EABI
    asm volatile("swp %0,%2,[%3]"
                 : "=&r"(originalValue), "=m" (_q_value)
                 : "r"(newValue), "r"(&_q_value)
                 : "cc", "memory");
#else
    int count = 0;
    while (q_atomic_swp(&q_atomic_lock, ~0) != 0)
        qt_atomic_yield(&count);
    originalValue=_q_value;
    _q_value = newValue;
    q_atomic_swp(&q_atomic_lock, 0);
#endif
    return originalValue;
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

// Fetch and add for integers

inline int QBasicAtomicInt::fetchAndAddOrdered(int valueToAdd)
{
#ifndef QT_NO_ARM_EABI
    register int originalValue;
    register int newValue;
    do {
        originalValue = _q_value;
        newValue = originalValue + valueToAdd;
    } while (qt_atomic_eabi_cmpxchg_int(originalValue, newValue, &_q_value) != 0);
    return originalValue;
#else
    int count = 0;
    while (q_atomic_swp(&q_atomic_lock, ~0) != 0)
        qt_atomic_yield(&count);
    int originalValue = _q_value;
    _q_value += valueToAdd;
    q_atomic_swp(&q_atomic_lock, 0);
    return originalValue;
#endif
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

// Test and set for pointers

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetOrdered(T *expectedValue, T *newValue)
{
#ifndef QT_NO_ARM_EABI
    register T *originalValue;
    do {
        originalValue = _q_value;
        if (originalValue != expectedValue)
            return false;
    } while (qt_atomic_eabi_cmpxchg_ptr(expectedValue, newValue, &_q_value) != 0);
    return true;
#else
    bool returnValue = false;
    int count = 0;
    while (q_atomic_swp(&q_atomic_lock, ~0) != 0)
        qt_atomic_yield(&count);
    if (_q_value == expectedValue) {
	_q_value = newValue;
	returnValue = true;
    }
    q_atomic_swp(&q_atomic_lock, 0);
    return returnValue;
#endif
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

// Fetch and store for pointers



template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreOrdered(T *newValue)
{
    T *originalValue;
#ifndef QT_NO_ARM_EABI
    asm volatile("swp %0,%2,[%3]"
                 : "=&r"(originalValue), "=m" (_q_value)
                 : "r"(newValue), "r"(&_q_value)
                 : "cc", "memory");
#else
    int count = 0;
    while (q_atomic_swp(&q_atomic_lock, ~0) != 0)
        qt_atomic_yield(&count);
    originalValue=_q_value;
    _q_value = newValue;
    q_atomic_swp(&q_atomic_lock, 0);
#endif
    return originalValue;
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

// Fetch and add for pointers

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddOrdered(qptrdiff valueToAdd)
{
#ifndef QT_NO_ARM_EABI
    register T *originalValue;
    register T *newValue;
    do {
        originalValue = _q_value;
        newValue = originalValue + valueToAdd;
    } while (qt_atomic_eabi_cmpxchg_ptr(originalValue, newValue, &_q_value) != 0);
    return originalValue;
#else
    int count = 0;
    while (q_atomic_swp(&q_atomic_lock, ~0) != 0)
        qt_atomic_yield(&count);
    T *originalValue = (_q_value);
    _q_value += valueToAdd;
    q_atomic_swp(&q_atomic_lock, 0);
    return originalValue;
#endif
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

#endif // QATOMIC_ARMV5_H
