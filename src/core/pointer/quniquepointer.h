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

#ifndef QUNIQUEPOINTER_H
#define QUNIQUEPOINTER_H

#include <qglobal.h>

#include <cs_unique_pointer.h>
#include <cs_unique_array_pointer.h>

struct QMallocDeleter {
   void operator()(void *ptr) const {
      if (ptr != nullptr) {
         qFree(ptr);
      }
   }
};

using QScopedPointerPodDeleter [[deprecated("Use QMallocDeleter instead")]] = QMallocDeleter;

#if ! defined(CS_DOXYPRESS)

template <typename T, typename Deleter = std::default_delete<T>>
class QUniquePointer : public CsPointer::CsUniquePointer<T, Deleter>
{
 public:
   using CsPointer::CsUniquePointer<T, Deleter>::CsUniquePointer;

   QUniquePointer(CsPointer::CsUniquePointer<T, Deleter> other) noexcept
      : CsPointer::CsUniquePointer<T, Deleter>(std::move(other))
   {
   }

   bool isNull() const {
      return CsPointer::CsUniquePointer<T, Deleter>::is_null();
   }
};

#endif

#if ! defined(CS_DOXYPRESS)

template <typename T, typename Deleter = std::default_delete<CsPointer::cs_add_missing_extent_t<T>>>
class QUniqueArrayPointer : public CsPointer::CsUniqueArrayPointer<T, Deleter>
{
public:
   using CsPointer::CsUniqueArrayPointer<T, Deleter>::CsUniqueArrayPointer;

   QUniqueArrayPointer(CsPointer::CsUniqueArrayPointer<T, Deleter> other) noexcept
      : CsPointer::CsUniqueArrayPointer<T, Deleter>(std::move(other))
   {
   }

   bool isNull() const {
      return CsPointer::CsUniqueArrayPointer<T, Deleter>::is_null();
   }
};

#endif

// free functions

// QScopedPointer
template < typename T, typename... Args, typename = typename std::enable_if_t < ! std::is_array_v<T >>>
QUniquePointer<T> QMakeUnique(Args && ... args)
{
   return CsPointer::make_unique<T>(std::forward<Args>(args)...);
}

// QScopedArrayPointer
template <typename T, typename = typename std::enable_if_t<std::is_array_v<T>>>
QUniquePointer<T> QMakeUnique(std::size_t size)
{
   return CsPointer::make_unique<T>(size);
}

#endif
