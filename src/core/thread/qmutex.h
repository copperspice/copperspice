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

#ifndef QMUTEX_H
#define QMUTEX_H

#include <qglobal.h>
#include <qassert.h>

#include <mutex>

class Q_CORE_EXPORT QMutex
{
 public:
   QMutex() = default;

   template <typename T>
   explicit QMutex(T) {
      static_assert(! std::is_same_v<T, T>, "Use QRecursiveMutex for recursive mutex operations");
   }

   QMutex(const QMutex &) = delete;
   QMutex(QMutex &&) = delete;

   QMutex &operator=(const QMutex &) = delete;
   QMutex &operator=(QMutex &&) = delete;

   ~QMutex() = default;

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

   // produces a clean compile error when obsolete enum values are used
   class RemovedEnum
   {
   };

   static constexpr RemovedEnum NonRecursive = RemovedEnum();
   static constexpr RemovedEnum Recursive    = RemovedEnum();

 private:
   std::timed_mutex m_data;
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
   explicit QMutexLocker(QMutex *mutex) {
      if (mutex == nullptr) {
         // nothing

      } else {
         m_data = std::unique_lock<QMutex>(*mutex);

      }
   }

   QMutexLocker(const QMutexLocker &) = delete;
   QMutexLocker &operator=(const QMutexLocker &) = delete;

   ~QMutexLocker() = default;

   void lock() {
      m_data.lock();
   }

   QMutex *mutex() const {
      return m_data.mutex();
   }

   void relock() {
      m_data.lock();
   }

   void unlock() {
      m_data.unlock();
   }

 private:
   std::unique_lock<QMutex> m_data;
};

class Q_CORE_EXPORT QRecursiveMutexLocker
{
 public:
   explicit QRecursiveMutexLocker(QRecursiveMutex *mutex) {
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

   QRecursiveMutex *mutex() const {
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
