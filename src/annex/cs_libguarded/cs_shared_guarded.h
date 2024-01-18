/***********************************************************************
*
* Copyright (c) 2016-2024 Ansel Sermersheim
*
* This file is part of CsLibGuarded.
*
* CsLibGuarded is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
* CsLibGuarded is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://opensource.org/licenses/BSD-2-Clause
*
***********************************************************************/

#ifndef CSLIBGUARDED_SHARED_GUARDED_H
#define CSLIBGUARDED_SHARED_GUARDED_H

#include <memory>
#include <mutex>
#include <shared_mutex>

namespace libguarded
{

/**
   \headerfile cs_shared_guarded.h <CsLibGuarded/cs_shared_guarded.h>

   This templated class wraps an object and allows only one thread at
   a time to modify the protected object.

   This class will use std::shared_timed_mutex for the internal
   locking mechanism by default. In C++17 the std::shared_mutex class
   is also available.

   The handle returned by the various lock methods is moveable but not
   copyable.
*/
template <typename T, typename M = std::shared_timed_mutex, typename L = std::shared_lock<M>>
class shared_guarded
{
   private:
      class deleter;
      class shared_deleter;

   public:
      using handle        = std::unique_ptr<T, deleter>;
      using shared_handle = std::unique_ptr<const T, shared_deleter>;

      template <typename... Us>
      shared_guarded(Us &&... data);

      // exclusive access
      [[nodiscard]] handle lock();
      [[nodiscard]] handle try_lock();

      template <class Duration>
      [[nodiscard]] handle try_lock_for(const Duration &duration);

      template <class TimePoint>
      [[nodiscard]] handle try_lock_until(const TimePoint &timepoint);

      // shared access, note "shared" in method names
      [[nodiscard]] shared_handle lock_shared() const;
      [[nodiscard]] shared_handle try_lock_shared() const;

      template <class Duration>
      [[nodiscard]] shared_handle try_lock_shared_for(const Duration &duration) const;

      template <class TimePoint>
      [[nodiscard]] shared_handle try_lock_shared_until(const TimePoint &timepoint) const;

   private:
      T m_obj;
      mutable M m_mutex;
};

template <typename T, typename M, typename L>
class shared_guarded<T, M, L>::deleter
{
   public:
      using pointer = T *;

      deleter() = default;
      deleter(std::unique_lock<M> lock);

      void operator()(T *ptr);

   private:
      std::unique_lock<M> m_lock;
};

template <typename T, typename M, typename L>
shared_guarded<T, M, L>::deleter::deleter(std::unique_lock<M> lock)
   : m_lock(std::move(lock))
{
}

template <typename T, typename M, typename L>
void shared_guarded<T, M, L>::deleter::operator()(T *)
{
   if (m_lock.owns_lock()) {
      m_lock.unlock();
   }
}

template <typename T, typename M, typename L>
class shared_guarded<T, M, L>::shared_deleter
{
public:
   using pointer = const T *;

   shared_deleter() = default;
   shared_deleter(L lock);

   void operator()(const T *ptr);

private:
   L m_lock;
};

template <typename T, typename M, typename L>
shared_guarded<T, M, L>::shared_deleter::shared_deleter(L lock)
   : m_lock(std::move(lock))
{
}

template <typename T, typename M, typename L>
void shared_guarded<T, M, L>::shared_deleter::operator()(const T *)
{
   if (m_lock.owns_lock()) {
      m_lock.unlock();
   }
}

template <typename T, typename M, typename L>
template <typename... Us>
shared_guarded<T, M, L>::shared_guarded(Us &&... data)
   : m_obj(std::forward<Us>(data)...)
{
}

template <typename T, typename M, typename L>
auto shared_guarded<T, M, L>::lock() -> handle
{
   std::unique_lock<M> lock(m_mutex);
   return handle(&m_obj, deleter(std::move(lock)));
}

template <typename T, typename M, typename L>
auto shared_guarded<T, M, L>::try_lock() -> handle
{
   std::unique_lock<M> lock(m_mutex, std::try_to_lock);

   if (lock.owns_lock()) {
      return handle(&m_obj, deleter(std::move(lock)));
   } else {
      return handle(nullptr, deleter(std::move(lock)));
   }
}

template <typename T, typename M, typename L>
template <typename Duration>
auto shared_guarded<T, M, L>::try_lock_for(const Duration &duration) -> handle
{
   std::unique_lock<M> lock(m_mutex, duration);

   if (lock.owns_lock()) {
      return handle(&m_obj, deleter(std::move(lock)));
   } else {
      return handle(nullptr, deleter(std::move(lock)));
   }
}

template <typename T, typename M, typename L>
template <typename TimePoint>
auto shared_guarded<T, M, L>::try_lock_until(const TimePoint &timepoint) -> handle
{
   std::unique_lock<M> lock(m_mutex, timepoint);

   if (lock.owns_lock()) {
      return handle(&m_obj, deleter(std::move(lock)));
   } else {
      return handle(nullptr, deleter(std::move(lock)));
   }
}

template <typename T, typename M, typename L>
auto shared_guarded<T, M, L>::lock_shared() const -> shared_handle
{
   L lock(m_mutex);
   return shared_handle(&m_obj, shared_deleter(std::move(lock)));
}

template <typename T, typename M, typename L>
auto shared_guarded<T, M, L>::try_lock_shared() const -> shared_handle
{
   L lock(m_mutex, std::try_to_lock);

   if (lock.owns_lock()) {
      return shared_handle(&m_obj, shared_deleter(std::move(lock)));
   } else {
      return shared_handle(nullptr, shared_deleter(std::move(lock)));
   }
}

template <typename T, typename M, typename L>
template <typename Duration>
auto shared_guarded<T, M, L>::try_lock_shared_for(const Duration &d) const -> shared_handle
{
   L lock(m_mutex, d);

   if (lock.owns_lock()) {
      return shared_handle(&m_obj, shared_deleter(std::move(lock)));
   } else {
      return shared_handle(nullptr, shared_deleter(std::move(lock)));
   }
}

template <typename T, typename M, typename L>
template <typename TimePoint>
auto shared_guarded<T, M, L>::try_lock_shared_until(const TimePoint &tp) const -> shared_handle
{
   L lock(m_mutex, tp);

   if (lock.owns_lock()) {
      return shared_handle(&m_obj, shared_deleter(std::move(lock)));
   } else {
      return shared_handle(nullptr, shared_deleter(std::move(lock)));
   }
}

}  // namespace libguarded

#endif
