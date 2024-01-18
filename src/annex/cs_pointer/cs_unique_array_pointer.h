/***********************************************************************
*
* Copyright (c) 2023-2024 Barbara Geller
* Copyright (c) 2023-2024 Ansel Sermersheim
*
* This file is part of CsPointer.
*
* CsPointer is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
* CsPointer is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://opensource.org/licenses/BSD-2-Clause
*
***********************************************************************/

#ifndef LIB_CS_UNIQUE_ARRAY_POINTER_H
#define LIB_CS_UNIQUE_ARRAY_POINTER_H

#include <cs_unique_pointer.h>
#include <memory>

namespace CsPointer {

template <typename T>
struct cs_add_missing_extent {
   using type = T[];
};

template <typename T>
struct cs_add_missing_extent<T[]> {
   using type = T[];
};

template <typename T>
using cs_add_missing_extent_t = typename cs_add_missing_extent<T>::type;

template <typename T, typename Deleter = std::default_delete<cs_add_missing_extent_t<T>>>
class CsUniqueArrayPointer : public CsUniquePointer<cs_add_missing_extent_t<T>, Deleter>
{
 public:
   using pointer      = typename std::unique_ptr<cs_add_missing_extent_t<T>, Deleter>::pointer;
   using element_type = typename std::unique_ptr<cs_add_missing_extent_t<T>, Deleter>::element_type;
   using deleter_type = typename std::unique_ptr<cs_add_missing_extent_t<T>, Deleter>::deleter_type;

   using Pointer      = pointer;
   using ElementType  = element_type;
   using DeleterType  = deleter_type;

   using CsUniquePointer<cs_add_missing_extent_t<T>, Deleter>::CsUniquePointer;

   CsUniqueArrayPointer(CsUniqueArrayPointer<ElementType> &&other) noexcept
      : CsUniquePointer<cs_add_missing_extent_t<T>, Deleter>(std::move(other.m_ptr))
   {
   }

   CsUniqueArrayPointer &operator=(CsUniqueArrayPointer<ElementType> &&other) noexcept
   {
      this->m_ptr = std::move(other.m_ptr);
      return *this;
   }

   CsUniqueArrayPointer(CsUniqueArrayPointer<ElementType[]> &&other) noexcept
      : CsUniquePointer<cs_add_missing_extent_t<T>, Deleter>(std::move(other.m_ptr))
   {
   }

   CsUniqueArrayPointer &operator=(CsUniqueArrayPointer<ElementType[]> &&other) noexcept {
      this->m_ptr = std::move(other.m_ptr);
      return *this;
   }

   ElementType &operator*() const noexcept(noexcept(* std::declval<pointer>())) {
      return this->get()[0];
   }

   ElementType &operator[](std::size_t index) const noexcept {
      return this->get()[index];
   }
};

template <typename T, typename = typename std::enable_if_t<std::is_array_v<T>>>
CsUniqueArrayPointer<T> make_unique(std::size_t size)
{
   return std::make_unique<T>(size);
}

}   // end namespace

#endif
