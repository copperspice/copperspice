/***********************************************************************
*
* Copyright (c) 2015-2018 Barbara Geller
* Copyright (c) 2015-2018 Ansel Sermersheim
* All rights reserved.
*
* This file is part of libguarded
*
* libguarded is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
***********************************************************************/

#ifndef LIBGUARDED_RCU_GUARDED_HPP
#define LIBGUARDED_RCU_GUARDED_HPP

#include <memory>

namespace libguarded
{
/**
   \headerfile rcu_guarded.hpp <libguarded/rcu_guarded.hpp>

   This templated class implements a mechanism which controls access
   to an RCU data structure. The only way to access the underlying
   data structure is to use either the lock_read or lock_write methods
   to receive a read-only or writable handle to the data structure,
   respectively.
*/
template <typename T>
class rcu_guarded
{
  public:
    class write_handle;
    class read_handle;

    template <typename... Us>
    rcu_guarded(Us &&... data);

    // write access
    write_handle lock_write();

    // read access
    read_handle lock_read() const;

    class write_handle
    {
      public:
        using pointer      = T *;
        using element_type = T;

        write_handle(T *ptr);

        ~write_handle()
        {
            if (m_accessed) {
                m_guard.rcu_write_unlock(*m_ptr);
            }
        }

        T & operator*() const
        {
            access();
            return *m_ptr;
        }

        T * operator->() const
        {
            access();
            return m_ptr;
        }

      private:
        void access() const
        {
            if (!m_accessed) {
                m_guard.rcu_write_lock(*m_ptr);
                m_accessed = true;
            }
        }

        T * m_ptr;
        mutable typename T::rcu_write_guard m_guard;
        mutable bool m_accessed;
    };

    class read_handle
    {
      public:
        using pointer      = const T *;
        using element_type = const T;

        read_handle(const T * ptr)
            : m_ptr(ptr), m_accessed(false)
        {
        }

        ~read_handle()
        {
            if (m_accessed) {
                m_guard.rcu_read_unlock(*m_ptr);
            }
        }

        const T & operator*() const
        {
            access();
            return *m_ptr;
        }

        const T * operator->() const
        {
            access();
            return m_ptr;
        }

      private:
        void access() const
        {
            if (! m_accessed) {
                m_guard.rcu_read_lock(*m_ptr);
                m_accessed = true;
            }
        }

        const T * m_ptr;
        mutable typename T::rcu_read_guard m_guard;
        mutable bool m_accessed;
    };

  private:
    T m_obj;
};

template <typename T>
template <typename... Us>
rcu_guarded<T>::rcu_guarded(Us &&... data)
    : m_obj(std::forward<Us>(data)...)
{
}

template <typename T>
auto rcu_guarded<T>::lock_write() -> write_handle
{
    return write_handle(&m_obj);
}

template <typename T>
auto rcu_guarded<T>::lock_read() const -> read_handle
{
    return read_handle(&m_obj);
}

template <typename T>
rcu_guarded<T>::write_handle::write_handle(T *ptr)
    : m_ptr(ptr), m_accessed(false)
{
}
}

#endif
