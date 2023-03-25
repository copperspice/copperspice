/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

template <typename T, typename Deleter = std::default_delete<T>>
using QUniquePointer = CsPointer::CsUniquePointer<T, Deleter>;

template <typename T, typename Deleter = std::default_delete<CsPointer::cs_add_missing_extent_t<T>>>
using QUniqueArrayPointer = CsPointer::CsUniqueArrayPointer<T, Deleter>;

template <typename T, typename Deleter = std::default_delete<T>>
using QScopedPointer = CsPointer::CsUniquePointer<T, Deleter>;

template <typename T, typename Deleter = std::default_delete<CsPointer::cs_add_missing_extent_t<T>>>
using QScopedArrayPointer = CsPointer::CsUniqueArrayPointer<T, Deleter>;


// free functions
template <class T, class... Args, class = typename std::enable_if_t<! std::is_array_v<T>>>
QScopedPointer<T> QMakeScoped(Args &&... args) {
   return CsPointer::make_unique<T>(std::forward<Args>(args)...);
}

template <class T, class... Args, class = typename std::enable_if_t<! std::is_array_v<T>>>
QScopedPointer<T> QMakeUnique(Args &&... args) {
   return CsPointer::make_unique<T>(std::forward<Args>(args)...);
}

template <class T, class = typename std::enable_if_t<std::is_array_v<T>>>
QScopedArrayPointer<T> QMakeScoped(std::size_t size) {
   return CsPointer::make_unique<T>(size);
}

template <class T, class = typename std::enable_if_t<std::is_array_v<T>>>
QScopedArrayPointer<T> QMakeUnique(std::size_t size) {
   return CsPointer::make_unique<T>(size);
}

#endif
