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

#ifndef LIB_CS_SHARED_ARRAY_POINTER_H
#define LIB_CS_SHARED_ARRAY_POINTER_H

#include <cs_pointer_traits.h>
#include <cs_shared_pointer.h>
#include <memory>

namespace CsPointer {

template <typename T>
class CsSharedArrayPointer : public CsSharedPointer<cs_add_missing_extent_t<T>>
{
 public:
   using element_type = typename std::shared_ptr<cs_add_missing_extent_t<T>>::element_type;
   using weak_type    = CsWeakPointer<cs_add_missing_extent_t<T>>;
   using pointer      = element_type *;

   using ElementType  = element_type;
   using WeakType     = weak_type;
   using Pointer      = pointer;

   using CsSharedPointer<cs_add_missing_extent_t<T>>::CsSharedPointer;

   CsSharedArrayPointer(const CsSharedArrayPointer<element_type> &other)
      : CsSharedPointer<cs_add_missing_extent_t<T>>(other.m_ptr)
   {
   }

   CsSharedArrayPointer(const CsSharedArrayPointer<element_type[]> &other)
      : CsSharedPointer<cs_add_missing_extent_t<T>>(other.m_ptr)
   {
   }

   CsSharedArrayPointer &operator=(const CsSharedArrayPointer<element_type> &other)
   {
      this->m_ptr = other.m_ptr;
      return *this;
   }

   CsSharedArrayPointer &operator=(const CsSharedArrayPointer<element_type[]> &other)
   {
      this->m_ptr = other.m_ptr;
      return *this;
   }

   CsSharedArrayPointer(CsSharedArrayPointer<element_type> &&other)
      : CsSharedPointer<cs_add_missing_extent_t<T>>(std::move(other.m_ptr))
   {
   }

   CsSharedArrayPointer(CsSharedArrayPointer<element_type[]> &&other)
      : CsSharedPointer<cs_add_missing_extent_t<T>>(std::move(other.m_ptr))
   {
   }

   CsSharedArrayPointer &operator=(CsSharedArrayPointer<element_type> &&other)
   {
      this->m_ptr = std::move(other.m_ptr);
      return *this;
   }

   CsSharedArrayPointer &operator=(CsSharedArrayPointer<element_type[]> &&other)
   {
      this->m_ptr = std::move(other.m_ptr);
      return *this;
   }

   element_type &operator*() const noexcept {
      return this->get()[0];
   }

   element_type & operator[](std::size_t index) const noexcept {
      return this->get()[index];
   }
};

template <typename T, typename = typename std::enable_if_t<std::is_array_v<T>>>
CsSharedArrayPointer<T> make_shared(std::size_t size)
{
   using Type = std::remove_extent_t<T>;

   return std::shared_ptr<T>(new Type[size]);
}

}   // end namespace

#endif
