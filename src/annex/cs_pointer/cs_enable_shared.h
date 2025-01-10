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

#ifndef LIB_CS_ENABLE_SHARED_H
#define LIB_CS_ENABLE_SHARED_H

#include <cs_shared_pointer.h>

#include <memory>

namespace CsPointer {

template <typename T>
class CsEnableSharedFromThis : public std::enable_shared_from_this<T>
{
 public:
   CsSharedPointer<T> sharedFromThis() {
      try {
         return this->shared_from_this();

      } catch (std::bad_weak_ptr &) {
         return nullptr;

      }
   }

   CsSharedPointer<const T> sharedFromThis() const {
      try {
         return this->shared_from_this();

      } catch (std::bad_weak_ptr &) {
         return nullptr;

      }
   }

 protected:
   ~CsEnableSharedFromThis() = default;
   CsEnableSharedFromThis()  = default;

   CsEnableSharedFromThis(const CsEnableSharedFromThis &) {
   }

   CsEnableSharedFromThis &operator=(const CsEnableSharedFromThis &) {
      return *this;
   }
};

}   // end namespace

#endif

