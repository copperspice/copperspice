/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
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

#ifndef QATOMIC_H
#define QATOMIC_H

#include <atomic>
#include <cstddef>

#ifndef ATOMIC_INT_LOCK_FREE
#define ATOMIC_INT_LOCK_FREE 0
#endif

#ifndef ATOMIC_POINTER_LOCK_FREE
#define ATOMIC_POINTER_LOCK_FREE 0
#endif


class QAtomicInt
{
   public:

      inline QAtomicInt() : m_data(0) {
      }

      inline QAtomicInt(int value) : m_data(value) {
      }

      inline QAtomicInt(const QAtomicInt &other) {
         int data = other.load();
         store(data);
      }

      inline QAtomicInt &operator=(const QAtomicInt &other) {
         int data = other.load();
         store(data);

         return *this;
      }

      inline QAtomicInt &operator=(int data) {
         store(data);
         return *this;
      }

      int load() const {
         return m_data.load();
      }

      int loadAcquire() {
         return m_data.load(std::memory_order_acquire);
      }

      void store(int newValue) {
         m_data.store(newValue);
      }

      void storeRelease(int newValue) {
         m_data.store(newValue, std::memory_order_release);
      }

      //
      static bool isReferenceCountingNative()  {
         return ATOMIC_INT_LOCK_FREE == 2;
      }

      static bool isReferenceCountingWaitFree()  {
         return ATOMIC_INT_LOCK_FREE == 2;
      }

      bool ref()  {
         int newValue = ++m_data;
         return newValue != 0;
      }

      bool deref()  {
         int newValue = --m_data;
         return newValue != 0;
      }

      //
      static bool isTestAndSetNative()  {
         return ATOMIC_INT_LOCK_FREE == 2;
      }

      static bool isTestAndSetWaitFree()  {
         return ATOMIC_INT_LOCK_FREE == 2;
      }

      bool testAndSetRelaxed(int expectedValue, int newValue)  {
         return m_data.compare_exchange_strong(expectedValue, newValue, std::memory_order_relaxed);
      }

      bool testAndSetAcquire(int expectedValue, int newValue) {
         return m_data.compare_exchange_strong(expectedValue, newValue, std::memory_order_acquire);
      }

      bool testAndSetRelease(int expectedValue, int newValue) {
         return m_data.compare_exchange_strong(expectedValue, newValue, std::memory_order_release);
      }

      bool testAndSetOrdered(int expectedValue, int newValue) {
         return m_data.compare_exchange_strong(expectedValue, newValue, std::memory_order_seq_cst);
      }

      //
      static bool isFetchAndStoreNative() {
         return ATOMIC_INT_LOCK_FREE == 2;
      }

      static bool isFetchAndStoreWaitFree() {
         return ATOMIC_INT_LOCK_FREE == 2;
      }

      int fetchAndStoreRelaxed(int newValue) {
         return m_data.exchange(newValue, std::memory_order_relaxed);
      }

      int fetchAndStoreAcquire(int newValue) {
         return m_data.exchange(newValue, std::memory_order_acquire);
      }

      int fetchAndStoreRelease(int newValue) {
         return m_data.exchange(newValue, std::memory_order_release);
      }

      int fetchAndStoreOrdered(int newValue) {
         return m_data.exchange(newValue, std::memory_order_seq_cst);
      }

      //
      static bool isFetchAndAddNative() {
         return ATOMIC_INT_LOCK_FREE == 2;
      }

      static bool isFetchAndAddWaitFree()  {
         return ATOMIC_INT_LOCK_FREE == 2;
      }

      int fetchAndAddRelaxed(int valueToAdd) {
         return m_data.fetch_add(valueToAdd, std::memory_order_relaxed);
      }

      int fetchAndAddAcquire(int valueToAdd) {
         return m_data.fetch_add(valueToAdd, std::memory_order_acquire);
      }

      int fetchAndAddRelease(int valueToAdd) {
         return m_data.fetch_add(valueToAdd, std::memory_order_release);
      }

      int fetchAndAddOrdered(int valueToAdd) {
         return m_data.fetch_add(valueToAdd, std::memory_order_seq_cst);
      }

   private:
      std::atomic<int> m_data;
};


template <typename T>
class QAtomicPointer
{
   public:
      QAtomicPointer(T *value = 0) : m_data(value) {
      }

      QAtomicPointer(const QAtomicPointer<T> &other) {
         T *data = other.load();
         store(data);
      }

      QAtomicPointer<T> &operator=(const QAtomicPointer<T> &other) {
         T *data = other.load();
         store(data);

         return *this;
      }

      QAtomicPointer<T> &operator=(T *data) {
         store(data);
         return *this;
      }

      T *load() const {
         return m_data.load();
      }

      T *loadAcquire() {
         return m_data.load(std::memory_order_acquire);
      }

      void store(T *newValue) {
         m_data.store(newValue);
      }

      void storeRelease(T *newValue) {
         m_data.store(newValue, std::memory_order_release);
      }

      //
      static bool isTestAndSetNative() {
         return ATOMIC_POINTER_LOCK_FREE == 2;
      }

      static bool isTestAndSetWaitFree() {
         return ATOMIC_POINTER_LOCK_FREE == 2;
      }

      bool testAndSetRelaxed(T *expectedValue, T *newValue) {
         return m_data.compare_exchange_strong(expectedValue, newValue, std::memory_order_relaxed);
      }

      bool testAndSetAcquire(T *expectedValue, T *newValue) {
         return m_data.compare_exchange_strong(expectedValue, newValue, std::memory_order_acquire);
      }

      bool testAndSetRelease(T *expectedValue, T *newValue) {
         return m_data.compare_exchange_strong(expectedValue, newValue, std::memory_order_release);
      }

      bool testAndSetOrdered(T *expectedValue, T *newValue) {
         return m_data.compare_exchange_strong(expectedValue, newValue, std::memory_order_seq_cst);
      }

      //
      static bool isFetchAndStoreNative() {
         return ATOMIC_POINTER_LOCK_FREE == 2;
      }

      static bool isFetchAndStoreWaitFree() {
         return ATOMIC_POINTER_LOCK_FREE == 2;
      }

      T *fetchAndStoreRelaxed(T *newValue)  {
         return m_data.exchange(newValue, std::memory_order_relaxed);
      }

      T *fetchAndStoreAcquire(T *newValue) {
         return m_data.exchange(newValue, std::memory_order_acquire);
      }

      T *fetchAndStoreRelease(T *newValue) {
         return m_data.exchange(newValue, std::memory_order_release);
      }

      T *fetchAndStoreOrdered(T *newValue) {
         return m_data.exchange(newValue, std::memory_order_seq_cst);
      }

      //
      static bool isFetchAndAddNative() {
         return ATOMIC_POINTER_LOCK_FREE == 2;
      }

      static bool isFetchAndAddWaitFree() {
         return ATOMIC_POINTER_LOCK_FREE == 2;
      }

      T *fetchAndAddRelaxed(std::ptrdiff_t valueToAdd) {
         return m_data.fetch_add(valueToAdd, std::memory_order_relaxed);
      }

      T *fetchAndAddAcquire(std::ptrdiff_t valueToAdd) {
         return m_data.fetch_add(valueToAdd, std::memory_order_acquire);
      }

      T *fetchAndAddRelease(std::ptrdiff_t valueToAdd){
         return m_data.fetch_add(valueToAdd, std::memory_order_release);
      }

      T *fetchAndAddOrdered(std::ptrdiff_t valueToAdd){
         return m_data.fetch_add(valueToAdd, std::memory_order_seq_cst);
      }

   private:
      std::atomic<T *> m_data;
};

template <typename T>
inline void qAtomicAssign(T *&d, T *x)
{
   if (d == x) {
      return;
   }

   x->ref.ref();
   if (! d->ref.deref()) {
      delete d;
   }

   d = x;
}

template <typename T>
inline void qAtomicDetach(T *&d)
{
   if (d->ref.load() == 1) {
      return;
   }

   T *x = d;
   d = new T(*d);

   if (! x->ref.deref()) {
      delete x;
   }
}

#endif
