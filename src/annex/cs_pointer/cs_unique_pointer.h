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

#ifndef LIB_CS_UNIQUE_POINTER_H
#define LIB_CS_UNIQUE_POINTER_H

#include <compare>
#include <memory>

namespace CsPointer {

template <typename T, typename Deleter = std::default_delete<T>>
class CsUniquePointer
{
 public:
   using pointer      = typename std::unique_ptr<T, Deleter>::pointer;
   using element_type = typename std::unique_ptr<T, Deleter>::element_type;
   using deleter_type = typename std::unique_ptr<T, Deleter>::deleter_type;

   using Pointer      = pointer;
   using ElementType  = element_type;
   using DeleterType  = deleter_type;

   constexpr CsUniquePointer(std::nullptr_t) noexcept
   {
   }

   explicit CsUniquePointer(Pointer p = nullptr) noexcept
      : m_ptr(p)
   {
   }

   explicit CsUniquePointer(Pointer p, Deleter d) noexcept
      : m_ptr(p, std::move(d))
   {
   }

   CsUniquePointer(std::unique_ptr<T, Deleter> &&p) noexcept
      : m_ptr(std::move(p))
   {
   }

   ~CsUniquePointer() = default;

   CsUniquePointer(const CsUniquePointer &) = delete;
   CsUniquePointer &operator=(const CsUniquePointer &) = delete;

   CsUniquePointer(CsUniquePointer &&other) = default;
   CsUniquePointer &operator=(CsUniquePointer && other) = default;

   ElementType &operator*() const noexcept(noexcept(* std::declval<Pointer>())) {
      return *m_ptr;
   }

   Pointer operator->() const noexcept {
      return m_ptr.get();
   }

   bool operator!() const noexcept {
      return m_ptr == nullptr;
   }

   explicit operator bool() const noexcept {
      return ! is_null();
   }

   operator std::unique_ptr<T, Deleter>() && noexcept
   {
      return std::move(m_ptr);
   }

   Pointer data() const noexcept {
      return m_ptr.get();
   }

   Pointer get() const noexcept {
      return m_ptr.get();
   }

   Deleter &get_deleter() noexcept {
      return m_ptr.get_deleter();
   }

   const Deleter &get_deleter() const noexcept {
      return m_ptr.get_deleter();
   }

   bool is_null() const noexcept {
      return m_ptr == nullptr;
   }

   Pointer release() noexcept {
      return m_ptr.release();
   }

   void reset(Pointer other = nullptr) noexcept {
      if (m_ptr.get() == other) {
         return;
      }

      m_ptr.reset(other);
   }

   void swap(CsUniquePointer &other) noexcept {
      std::swap(m_ptr, other.m_ptr);
   }

   Pointer take() noexcept {
      return m_ptr.release();
   }

   template <typename U, typename Deleter_U>
   auto operator<=>(const CsUniquePointer<U, Deleter_U> &ptr) const noexcept {
      return this->m_ptr <=> ptr.m_ptr;
   }

   template <typename U, typename Deleter_U>
   bool operator==(const CsUniquePointer<U, Deleter_U> &ptr) const noexcept {
      return this->m_ptr == ptr.m_ptr;
   }

   template <typename U>
   auto operator<=>(const U *ptr) const noexcept {
      return this->get() <=> ptr;
   }

   template <typename U>
   bool operator==(const U *ptr) const noexcept {
      return this->get() == ptr;
   }

   auto operator<=>(std::nullptr_t) const noexcept {
      return this->m_ptr <=> nullptr;
   }

   bool operator==(std::nullptr_t) const noexcept {
      return this->m_ptr == nullptr;
   }

 private:
   std::unique_ptr<T, Deleter> m_ptr;

   template <typename U, typename D>
   friend class CsUniqueArrayPointer;
};

template <typename T, typename... Args, typename = typename std::enable_if_t<! std::is_array_v<T>>>
CsUniquePointer<T> make_unique(Args &&... args)
{
   return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T, typename Deleter>
void swap(CsUniquePointer<T, Deleter> &ptr1, CsUniquePointer<T, Deleter> &ptr2) noexcept
{
   ptr1.swap(ptr2);
}

}   // end namespace

#endif
