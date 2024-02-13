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
   QAtomicInt()
      : m_data(0) {
   }

   QAtomicInt(int value)
      : m_data(value) {
   }

   QAtomicInt(const QAtomicInt &other) {
      int data = other.load();
      store(data);
   }

   QAtomicInt &operator=(const QAtomicInt &other) {
      int data = other.load();
      store(data);

      return *this;
   }

   QAtomicInt &operator=(int value) {
      store(value);
      return *this;
   }

   int load() const {
      return m_data.load(std::memory_order_seq_cst);
   }

   int load(std::memory_order order) const {
      return m_data.load(order);
   }

   int loadAcquire() const {
      return m_data.load(std::memory_order_acquire);
   }

   void store(int newValue) {
      m_data.store(newValue, std::memory_order_seq_cst);
   }

   void store(int newValue, std::memory_order order) {
      m_data.store(newValue, order);
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

   bool compareExchange(int &expectedValue, int newValue,
         std::memory_order order1 = std::memory_order_seq_cst,
         std::memory_order order2 = std::memory_order_seq_cst)  {
      return m_data.compare_exchange_strong(expectedValue, newValue, order1, order2);
   }

   bool compareExchangeWeak(int &expectedValue, int newValue,
         std::memory_order order1 = std::memory_order_seq_cst,
         std::memory_order order2 = std::memory_order_seq_cst)  {
      return m_data.compare_exchange_weak(expectedValue, newValue, order1, order2);
   }

   //
   [[deprecated]] bool testAndSetRelaxed(int expectedValue, int newValue)  {
      return m_data.compare_exchange_strong(expectedValue, newValue, std::memory_order_relaxed);
   }

   [[deprecated]] bool testAndSetAcquire(int expectedValue, int newValue) {
      return m_data.compare_exchange_strong(expectedValue, newValue, std::memory_order_acquire);
   }

   [[deprecated]] bool testAndSetRelease(int expectedValue, int newValue) {
      return m_data.compare_exchange_strong(expectedValue, newValue, std::memory_order_release);
   }

   [[deprecated]] bool testAndSetOrdered(int expectedValue, int newValue) {
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

   int operator++() {
      return ++m_data;
   }

   int operator++(int) {
      return m_data++;
   }

   int operator--() {
      return --m_data;
   }

   int operator--(int) {
      return m_data--;
   }

   int operator+=(int value) {
      return m_data += value;
   }

   int operator-=(int value) {
      return m_data -= value;
   }

 private:
   std::atomic<int> m_data;
};

template <typename T>
class QAtomicPointer
{
 public:
   QAtomicPointer(T *value = nullptr)
      : m_data(value) {
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

   QAtomicPointer<T> &operator=(T *value) {
      store(value);
      return *this;
   }

   T *load() const {
      return m_data.load(std::memory_order_seq_cst);
   }

   T *load(std::memory_order order) const {
      return m_data.load(order);
   }

   T *loadAcquire() const {
      return m_data.load(std::memory_order_acquire);
   }

   void store(T *newValue) {
      m_data.store(newValue, std::memory_order_seq_cst);
   }

   void store(T *newValue, std::memory_order order) {
      m_data.store(newValue, order);
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

   bool compareExchange(T*&expectedValue, T *newValue,
         std::memory_order order1 = std::memory_order_seq_cst,
         std::memory_order order2 = std::memory_order_seq_cst)  {
      return m_data.compare_exchange_strong(expectedValue, newValue, order1, order2);
   }

   bool compareExchangeWeak(T*&expectedValue, T *newValue,
         std::memory_order order1 = std::memory_order_seq_cst,
         std::memory_order order2 = std::memory_order_seq_cst)  {
      return m_data.compare_exchange_weak(expectedValue, newValue, order1, order2);
   }

   [[deprecated]] bool testAndSetRelaxed(T *expectedValue, T *newValue) {
      return m_data.compare_exchange_strong(expectedValue, newValue, std::memory_order_relaxed);
   }

   [[deprecated]] bool testAndSetAcquire(T *expectedValue, T *newValue) {
      return m_data.compare_exchange_strong(expectedValue, newValue, std::memory_order_acquire);
   }

   [[deprecated]] bool testAndSetRelease(T *expectedValue, T *newValue) {
      return m_data.compare_exchange_strong(expectedValue, newValue, std::memory_order_release);
   }

   [[deprecated]] bool testAndSetOrdered(T *expectedValue, T *newValue) {
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

   T *fetchAndAddRelease(std::ptrdiff_t valueToAdd) {
      return m_data.fetch_add(valueToAdd, std::memory_order_release);
   }

   T *fetchAndAddOrdered(std::ptrdiff_t valueToAdd) {
      return m_data.fetch_add(valueToAdd, std::memory_order_seq_cst);
   }

   T *operator++() {
      return ++m_data;
   }

   T *operator++(int) {
      return m_data++;
   }

   T *operator--() {
      return --m_data;
   }

   T *operator--(int) {
      return m_data--;
   }

   T *operator+=(std::ptrdiff_t value) {
      return m_data += value;
   }

   T *operator-=(std::ptrdiff_t value) {
      return m_data -= value;
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
