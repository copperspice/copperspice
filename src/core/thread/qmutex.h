/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#ifndef QMUTEX_H
#define QMUTEX_H

#include <qglobal.h>
#include <qassert.h>
#include <qatomic.h>

#include <mutex>
#include <new>

class QMutexData;

class Q_CORE_EXPORT QBasicMutex
{
 public:
   void lock() {
      if (! fastTryLock()) {
         lockInternal();
      }
   }

   void unlock() {
      Q_ASSERT(d_ptr.load()); //mutex must be locked

      QMutexData *expected = dummyLocked();

      if (! d_ptr.compareExchange(expected, nullptr, std::memory_order_release)) {
         unlockInternal();
      }
   }

   bool tryLock(int timeout = 0) {
      return fastTryLock() || lockInternal(timeout);
   }

   bool isRecursive();

 private:
   bool fastTryLock() {
      QMutexData *expected = nullptr;
      return d_ptr.compareExchange(expected, dummyLocked(), std::memory_order_acquire);
   }

   bool lockInternal(int timeout = -1);
   void unlockInternal();

   QAtomicPointer<QMutexData> d_ptr;

   static inline QMutexData *dummyLocked() {
      return reinterpret_cast<QMutexData *>(quintptr(1));
   }

   friend class QMutex;
   friend class QMutexData;
};

class Q_CORE_EXPORT QMutex : public QBasicMutex
{
 public:
   enum RecursionMode { NonRecursive, Recursive };
   explicit QMutex(RecursionMode mode = NonRecursive);

   QMutex(const QMutex &) = delete;
   QMutex &operator=(const QMutex &) = delete;

   ~QMutex();
};

class Q_CORE_EXPORT QRecursiveMutex
{
 public:
   QRecursiveMutex() = default;

   QRecursiveMutex(const QRecursiveMutex &) = delete;
   QRecursiveMutex(QRecursiveMutex &&) = delete;

   QRecursiveMutex &operator=(const QRecursiveMutex &) = delete;
   QRecursiveMutex &operator=(QRecursiveMutex &&) = delete;

   ~QRecursiveMutex() = default;

   void lock() {
      m_data.lock();
   }

   bool tryLock(int timeout = 0) {
      return m_data.try_lock_for(std::chrono::milliseconds(timeout));
   }

   bool try_lock() {
      return m_data.try_lock();
   }

   template <typename T1, typename T2>
   bool try_lock_for(std::chrono::duration<T1, T2> duration) {
      return m_data.try_lock_for(duration);
   }

   template <typename T1, typename T2>
   bool try_lock_until(std::chrono::time_point<T1, T2> timePoint) {
      return m_data.try_lock_until(timePoint);
   }

   void unlock() {
      m_data.unlock();
   }

 private:
   std::recursive_timed_mutex m_data;
};
class Q_CORE_EXPORT QMutexLocker
{
 public:
   explicit QMutexLocker(QBasicMutex *mutex) {
      Q_ASSERT_X((reinterpret_cast<quintptr>(mutex) & quintptr(1u)) == quintptr(0),
                 "QMutexLocker", "QMutex pointer is misaligned");

      if (mutex) {
         mutex->lock();
         val = reinterpret_cast<quintptr>(mutex) | quintptr(1u);
      } else {
         val = 0;
      }
   }

   QMutexLocker(const QMutexLocker &) = delete;
   QMutexLocker &operator=(const QMutexLocker &) = delete;

   ~QMutexLocker() {
      unlock();
   }

   void unlock() {
      if ((val & quintptr(1u)) == quintptr(1u)) {
         val &= ~quintptr(1u);
         mutex()->unlock();
      }
   }

   void relock() {
      if (val) {
         if ((val & quintptr(1u)) == quintptr(0u)) {
            mutex()->lock();
            val |= quintptr(1u);
         }
      }
   }

   QMutex *mutex() const {
      return reinterpret_cast<QMutex *>(val & ~quintptr(1u));
   }

 private:
   quintptr val;
};

class Q_CORE_EXPORT QRecursiveMutexLocker
{
 public:
   explicit QRecursiveMutexLocker(QRecursiveMutex *mutex)
   {
      if (mutex == nullptr) {
         // nothing

      } else {
         m_data = std::unique_lock<QRecursiveMutex>(*mutex);

      }
   }

   QRecursiveMutexLocker(const QRecursiveMutexLocker &) = delete;
   QRecursiveMutexLocker &operator=(const QRecursiveMutexLocker &) = delete;

   ~QRecursiveMutexLocker() = default;

   void lock() {
      m_data.lock();
   }

   QRecursiveMutex * mutex() const {
      return m_data.mutex();
   }

   void relock() {
      m_data.lock();
   }

   void unlock() {
      m_data.unlock();
   }

 private:
   std::unique_lock<QRecursiveMutex> m_data;
};

#endif
