/***********************************************************************
*
* Copyright (c) 2023-2025 Barbara Geller
* Copyright (c) 2023-2025 Ansel Sermersheim
*
* This file is part of CsPointer.
*
* CsPointer is free software which is released under the BSD 2-Clause license.
* For license details refer to the LICENSE provided with this project.
*
* CsPointer is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://opensource.org/licenses/BSD-2-Clause
*
***********************************************************************/

#ifndef LIB_CS_WEAK_POINTER_H
#define LIB_CS_WEAK_POINTER_H

#include <cs_shared_pointer.h>

#include <memory>

namespace CsPointer {

template <typename T>
class CsWeakPointer
{
 public:
   using element_type = typename std::weak_ptr<T>::element_type;
   using pointer      = element_type *;

   using Pointer      = pointer;
   using ElementType  = element_type;

   CsWeakPointer()
   {
   }

   template <typename U>
   CsWeakPointer(const CsWeakPointer<U> &p) noexcept
      : m_ptr(p.m_ptr)
   {
   }

   template <typename U>
   CsWeakPointer(const CsSharedPointer<U> &p) noexcept
      : m_ptr(p.m_ptr)
   {
   }

   ~CsWeakPointer() = default;

   CsWeakPointer(const CsWeakPointer &other) = default;
   CsWeakPointer &operator=(const CsWeakPointer &other) = default;

   CsWeakPointer(CsWeakPointer &&other) = default;
   CsWeakPointer &operator=(CsWeakPointer && other) = default;

   template <typename U>
   CsWeakPointer &operator=(const CsSharedPointer<U> &p) noexcept
   {
      m_ptr = p.m_ptr;
      return *this;
   }

   bool operator !() const {
      return is_null();
   }

   explicit operator bool() const noexcept {
      return ! is_null();
   }

   void clear() noexcept {
      m_ptr.reset();
   }

   bool expired() const noexcept {
      return m_ptr.expired();
   }

   CsSharedPointer<T> lock() const noexcept {
      return m_ptr.lock();
   }

   bool is_null() const noexcept {
      return m_ptr.expired();
   }

   template <typename U>
   bool owner_before(const CsSharedPointer<U> &p) const noexcept {
      return m_ptr.owner_before(p.m_ptr);
   }

   template <typename U>
   bool owner_before(const CsWeakPointer<U> &p) const noexcept {
      return m_ptr.owner_before(p.m_ptr);
   }

   void reset() noexcept {
      m_ptr.reset();
   }

   void swap(CsWeakPointer &other) noexcept {
      std::swap(m_ptr, other.m_ptr);
   }

   CsSharedPointer<T> toStrongRef() const noexcept {
      return m_ptr.lock();
   }

   long use_count() const noexcept {
      return m_ptr.use_count();
   }

 private:
   std::weak_ptr<T> m_ptr;

   template <typename U>
   friend class CsSharedPointer;

   template <typename U>
   friend class CsWeakPointer;
};

// equal
template <typename T1, typename T2>
bool operator==(const CsWeakPointer<T1> &ptr1, const CsWeakPointer<T2> &ptr2) noexcept
{
   return ptr1.owner_before(ptr2) == false && ptr2.owner_before(ptr1) == false;
}

template <typename T1, typename T2>
bool operator==(const CsSharedPointer<T1> &ptr1, const CsWeakPointer<T2> &ptr2) noexcept
{
   return ptr1.owner_before(ptr2) == false && ptr2.owner_before(ptr1) == false;
}

template <typename T1, typename T2>
bool operator==(const CsWeakPointer<T1> &ptr1, const CsSharedPointer<T2> &ptr2) noexcept
{
   return ptr1.owner_before(ptr2) == false && ptr2.owner_before(ptr1) == false;
}

template <typename T1>
bool operator==(const CsWeakPointer<T1> &ptr1, std::nullptr_t) noexcept
{
   return ptr1.expired();
}

template <typename T1>
bool operator==(std::nullptr_t, const CsWeakPointer<T1> &ptr2) noexcept
{
   return ptr2.expired();
}

// not equal
template <typename T1, typename T2>
bool operator!=(const CsWeakPointer<T1> &ptr1, const CsWeakPointer<T2> &ptr2) noexcept
{
   return ptr1.owner_before(ptr2) == true || ptr2.owner_before(ptr1) == true;
}

template <typename T1, typename T2>
bool operator!=(const CsSharedPointer<T1> &ptr1, const CsWeakPointer<T2> &ptr2) noexcept
{
   return ptr1.owner_before(ptr2) == true || ptr2.owner_before(ptr1) == true;
}

template <typename T1, typename T2>
bool operator!=(const CsWeakPointer<T1> &ptr1, const CsSharedPointer<T2> &ptr2) noexcept
{
   return ptr1.owner_before(ptr2) == true || ptr2.owner_before(ptr1) == true;
}

template <typename T1>
bool operator!=(const CsWeakPointer<T1> &ptr1, std::nullptr_t) noexcept
{
   return ! ptr1.expired();
}

template <typename T1>
bool operator!=(std::nullptr_t, const CsWeakPointer<T1> &ptr2) noexcept
{
   return ! ptr2.expired();
}

template <typename T>
void swap(CsWeakPointer<T> &ptr1, CsWeakPointer<T> &ptr2) noexcept
{
   ptr1.swap(ptr2);
}


}   // end namespace

#endif
