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

#ifndef LIB_CS_SHARED_POINTER_H
#define LIB_CS_SHARED_POINTER_H

#include <memory>

namespace CsPointer {

template <typename T, typename Deleter>
class CsUniquePointer;

template <typename T>
class CsWeakPointer;

template <typename T>
class CsSharedPointer
{
 public:
   using element_type = typename std::shared_ptr<T>::element_type;
   using weak_type    = CsWeakPointer<T>;
   using pointer      = element_type *;

   using ElementType  = element_type;
   using WeakType     = weak_type;
   using Pointer      = pointer;

   constexpr CsSharedPointer() noexcept
   {
   }

   constexpr CsSharedPointer(std::nullptr_t) noexcept
   {
   }

   template <typename U>
   explicit CsSharedPointer(U *p)
      : m_ptr(p)
   {
   }

   template <typename U, typename Deleter>
   CsSharedPointer(U *p, Deleter d)
      : m_ptr(p, std::move(d))
   {
   }

   template <typename Deleter>
   CsSharedPointer(std::nullptr_t, Deleter d)
      : m_ptr(nullptr, std::move(d))
   {
   }

   template <typename U, typename Deleter, typename Alloc>
   CsSharedPointer(U *p, Deleter d, Alloc a)
      : m_ptr(p, std::move(d), std::move(a))
   {
   }

   template <typename Deleter, typename Alloc>
   CsSharedPointer(std::nullptr_t, Deleter d, Alloc a)
      : m_ptr(nullptr, std::move(d), std::move(a))
   {
   }

   template <typename U, typename Deleter>
   CsSharedPointer(CsUniquePointer<U, Deleter> &&p)
      : m_ptr(std::unique_ptr<U, Deleter>(std::move(p)))
   {
   }

   template <typename U>
   CsSharedPointer(const std::shared_ptr<U> &p) noexcept
      : m_ptr(p)
   {
   }

   template <typename U>
   CsSharedPointer(std::shared_ptr<U> &&p) noexcept
      : m_ptr(std::move(p))
   {
   }

   template <typename U>
   CsSharedPointer(const CsSharedPointer<U> &p1, element_type *p2) noexcept
      : m_ptr(p1.m_ptr, p2)
   {
   }

   template <typename U>
   CsSharedPointer(CsSharedPointer<U> &&p1, element_type *p2) noexcept
      : m_ptr(std::move(p1.m_ptr), p2)
   {
      p1.m_ptr.reset();
   }

   ~CsSharedPointer() = default;

   CsSharedPointer(const CsSharedPointer &other) = default;
   CsSharedPointer &operator=(const CsSharedPointer &other) = default;

   CsSharedPointer(CsSharedPointer &&other) = default;
   CsSharedPointer &operator=(CsSharedPointer && other) = default;

   template <typename U>
   CsSharedPointer(const CsSharedPointer<U> &p) noexcept
      : m_ptr(p.m_ptr)
   {
   }

   template <typename U>
   CsSharedPointer &operator=(const CsSharedPointer<U> &p) noexcept {
      m_ptr = p.m_ptr;
      return *this;
   }

   template <typename U>
   CsSharedPointer(CsSharedPointer<U> &&p) noexcept
      : m_ptr(std::move(p.m_ptr))
   {
   }

   template <typename U>
   CsSharedPointer &operator=(CsSharedPointer<U> &&p) noexcept {
      m_ptr = std::move(p.m_ptr);
      return *this;
   }

   template <typename U>
   explicit CsSharedPointer(const CsWeakPointer<U> &p)
      : m_ptr(p.m_ptr.lock())
   {
      if (is_null()) {
         throw std::bad_weak_ptr();
      }
   }

   template <typename U>
   CsSharedPointer &operator=(const CsWeakPointer<U> &p) {
      m_ptr = p.m_ptr.lock();
      return *this;
   }

   element_type &operator*() const noexcept {
      return *m_ptr;
   }

   pointer operator->() const noexcept {
      return m_ptr.get();
   }

   bool operator !() const noexcept {
      return m_ptr == nullptr;
   }

   explicit operator bool() const noexcept {
      return m_ptr != nullptr;
   }

   template <typename U>
   operator std::shared_ptr<U>() const & noexcept
   {
      return m_ptr;
   }

   template <typename U>
   operator std::shared_ptr<U>() && noexcept
   {
      return std::move(m_ptr);
   }

   //
   void clear() noexcept {
      reset();
   }

   template <typename... Args>
   static CsSharedPointer<T> create(Args &&... args) {
      return std::make_shared<T>(std::forward<Args>(args)...);
   }

   pointer data() const noexcept {
      return m_ptr.get();
   }

   pointer get() const noexcept {
      return m_ptr.get();
   }

   bool is_null() const noexcept {
      return m_ptr == nullptr;
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

   template <typename U>
   void reset(U *p) {
      if (m_ptr.get() == p) {
         return;
      }

      m_ptr.reset(p);
   }

   template <typename U, typename Deleter>
   void reset(U *p, Deleter d) {
      if (m_ptr.get() == p) {
         return;
      }

      m_ptr.reset(p, std::move(d));
   }

   template <typename U, typename Deleter, typename Alloc>
   void reset(U *p, Deleter d, Alloc a) {
      if (m_ptr.get() == p) {
         return;
      }

      m_ptr.reset(p, std::move(d), std::move(a));
   }

   void swap(CsSharedPointer &other) noexcept {
      std::swap(m_ptr, other.m_ptr);
   }

   CsWeakPointer<T> toWeakRef() const {
      /* returns a CsWeakPointer to this CsSharedPointer

         CsSharedPointer<T> sharedPtr = new T();
         CsWeakPointer<T> weakPtr     = sharedPtr;
      */

       return *this;
   }

   bool unique() const noexcept {
      return m_ptr.use_count() == 1;
   }

   long use_count() const noexcept {
      return m_ptr.use_count();
   }

 private:
   std::shared_ptr<T> m_ptr;

   template <typename U>
   friend class CsSharedArrayPointer;

   template <typename U>
   friend class CsSharedPointer;

   template <typename U>
   friend class CsWeakPointer;
};

template <typename T, typename... Args, typename = typename std::enable_if_t<! std::is_array_v<T>>>
CsSharedPointer<T> make_shared(Args &&... args)
{
   return std::make_shared<T>(std::forward<Args>(args)...);
}

// equal
template <typename T1, typename T2>
bool operator==(const CsSharedPointer<T1> &ptr1, const CsSharedPointer<T2> &ptr2) noexcept
{
   return ptr1.get() == ptr2.get();
}

template <typename T1, typename T2>
bool operator==(const CsSharedPointer<T1> &ptr1, const T2 *ptr2) noexcept
{
   return ptr1.get() == ptr2;
}

template <typename T1, typename T2>
bool operator==(const T1 *ptr1, const CsSharedPointer<T2> &ptr2) noexcept
{
   return ptr1 == ptr2.get();
}

template <typename T>
bool operator==(const CsSharedPointer<T> &ptr1, std::nullptr_t) noexcept
{
   return ptr1.get() == nullptr;
}

template <typename T>
bool operator==(std::nullptr_t, const CsSharedPointer<T> &ptr2) noexcept
{
   return nullptr == ptr2.get();
}

// not equal
template <typename T1, typename T2>
bool operator!=(const CsSharedPointer<T1> &ptr1, const CsSharedPointer<T2> &ptr2) noexcept
{
   return ptr1.get() != ptr2.get();
}

template <typename T1, typename T2>
bool operator!=(const CsSharedPointer<T1> &ptr1, const T2 *ptr2) noexcept
{
   return ptr1.get() != ptr2;
}

template <typename T1, typename T2>
bool operator!=(const T1 *ptr1, const CsSharedPointer<T2> &ptr2) noexcept
{
   return ptr1 != ptr2.get();
}

template <typename T>
bool operator!=(const CsSharedPointer<T> &ptr1, std::nullptr_t) noexcept
{
   return ptr1.get() != nullptr;
}

template <typename T>
bool operator!=(std::nullptr_t, const CsSharedPointer<T> &ptr2) noexcept
{
   return nullptr != ptr2.get();
}

// compare
template <typename T1, typename T2>
bool operator<(const CsSharedPointer<T1> &ptr1, const CsSharedPointer<T2> &ptr2) noexcept
{
   return ptr1.get() < ptr2.get();
}

template <typename T1, typename T2>
bool operator<=(const CsSharedPointer<T1> &ptr1, const CsSharedPointer<T2> &ptr2) noexcept
{
   return ptr1.get() <= ptr2.get();
}

template <typename T1, typename T2>
bool operator>(const CsSharedPointer<T1> &ptr1, const CsSharedPointer<T2> &ptr2) noexcept
{
   return ptr1.get() > ptr2.get();
}

template <typename T1, typename T2>
bool operator>=(const CsSharedPointer<T1> &ptr1, const CsSharedPointer<T2> &ptr2) noexcept
{
   return ptr1.get() >= ptr2.get();
}

template <typename T>
void swap(CsSharedPointer<T> &ptr1, CsSharedPointer<T> &ptr2) noexcept
{
   ptr1.swap(ptr2);
}

// cast functions
template <typename T, typename U>
CsSharedPointer<T> static_pointer_cast(const CsSharedPointer<U> &ptr)
{
   return std::static_pointer_cast<T> (std::shared_ptr<U>(ptr));
}

template <typename T, typename U>
CsSharedPointer<T> dynamic_pointer_cast(const CsSharedPointer<U> &ptr)
{
   return std::dynamic_pointer_cast<T> (std::shared_ptr<U>(ptr));
}

template <typename T, typename U>
CsSharedPointer<T> const_pointer_cast(const CsSharedPointer<U> &ptr)
{
   return std::const_pointer_cast<T> (std::shared_ptr<U>(ptr));
}

template <typename T, typename U>
CsSharedPointer<T> static_pointer_cast(CsSharedPointer<U> &&ptr)
{
   return std::static_pointer_cast<T> (std::shared_ptr<U>(std::move(ptr)));
}

template <typename T, typename U>
CsSharedPointer<T> dynamic_pointer_cast(CsSharedPointer<U> &&ptr)
{
   return std::dynamic_pointer_cast<T> (std::shared_ptr<U>(std::move(ptr)));
}

template <typename T, typename U>
CsSharedPointer<T> const_pointer_cast(CsSharedPointer<U> &&ptr)
{
   return std::const_pointer_cast<T> (std::shared_ptr<U>(std::move(ptr)));
}

}   // end namespace

#endif
