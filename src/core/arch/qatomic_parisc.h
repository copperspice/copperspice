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

#ifndef QATOMIC_PARISC_H
#define QATOMIC_PARISC_H

QT_BEGIN_NAMESPACE

#define Q_ATOMIC_INT_REFERENCE_COUNTING_IS_NOT_NATIVE

inline bool QBasicAtomicInt::isReferenceCountingNative()
{
   return false;
}
inline bool QBasicAtomicInt::isReferenceCountingWaitFree()
{
   return false;
}

#define Q_ATOMIC_INT_TEST_AND_SET_IS_NOT_NATIVE

inline bool QBasicAtomicInt::isTestAndSetNative()
{
   return false;
}
inline bool QBasicAtomicInt::isTestAndSetWaitFree()
{
   return false;
}

#define Q_ATOMIC_INT_FETCH_AND_STORE_IS_NOT_NATIVE

inline bool QBasicAtomicInt::isFetchAndStoreNative()
{
   return false;
}
inline bool QBasicAtomicInt::isFetchAndStoreWaitFree()
{
   return false;
}

#define Q_ATOMIC_INT_FETCH_AND_ADD_IS_NOT_NATIVE

inline bool QBasicAtomicInt::isFetchAndAddNative()
{
   return false;
}
inline bool QBasicAtomicInt::isFetchAndAddWaitFree()
{
   return false;
}

#define Q_ATOMIC_POINTER_TEST_AND_SET_IS_NOT_NATIVE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isTestAndSetNative()
{
   return false;
}
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isTestAndSetWaitFree()
{
   return false;
}

#define Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_NOT_NATIVE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndStoreNative()
{
   return false;
}
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndStoreWaitFree()
{
   return false;
}

#define Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_NOT_NATIVE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndAddNative()
{
   return false;
}
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndAddWaitFree()
{
   return false;
}

extern "C" {
   Q_CORE_EXPORT void q_atomic_lock(int *lock);
   Q_CORE_EXPORT void q_atomic_unlock(int *lock);
}

// Reference counting

inline bool QBasicAtomicInt::ref()
{
   q_atomic_lock(_q_lock);
   bool ret = (++_q_value != 0);
   q_atomic_unlock(_q_lock);
   return ret;
}

inline bool QBasicAtomicInt::deref()
{
   q_atomic_lock(_q_lock);
   bool ret = (--_q_value != 0);
   q_atomic_unlock(_q_lock);
   return ret;
}

// Test-and-set for integers

inline bool QBasicAtomicInt::testAndSetOrdered(int expectedValue, int newValue)
{
   q_atomic_lock(_q_lock);
   if (_q_value == expectedValue) {
      _q_value = newValue;
      q_atomic_unlock(_q_lock);
      return true;
   }
   q_atomic_unlock(_q_lock);
   return false;
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

// Fetch-and-store for integers

inline int QBasicAtomicInt::fetchAndStoreOrdered(int newValue)
{
   q_atomic_lock(_q_lock);
   int returnValue = _q_value;
   _q_value = newValue;
   q_atomic_unlock(_q_lock);
   return returnValue;
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

// Fetch-and-add for integers

inline int QBasicAtomicInt::fetchAndAddOrdered(int valueToAdd)
{
   q_atomic_lock(_q_lock);
   int originalValue = _q_value;
   _q_value += valueToAdd;
   q_atomic_unlock(_q_lock);
   return originalValue;
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
   q_atomic_lock(_q_lock);
   if (_q_value == expectedValue) {
      _q_value = newValue;
      q_atomic_unlock(_q_lock);
      return true;
   }
   q_atomic_unlock(_q_lock);
   return false;
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
   q_atomic_lock(_q_lock);
   T *returnValue = (_q_value);
   _q_value = newValue;
   q_atomic_unlock(_q_lock);
   return returnValue;
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
   q_atomic_lock(_q_lock);
   T *returnValue = (_q_value);
   _q_value += valueToAdd;
   q_atomic_unlock(_q_lock);
   return returnValue;
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

#endif // QATOMIC_PARISC_H
