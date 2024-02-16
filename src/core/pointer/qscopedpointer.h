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

#ifndef QSCOPEDPOINTER_H
#define QSCOPEDPOINTER_H

#include <quniquepointer.h>

#if ! defined(CS_DOXYPRESS)

template <typename T, typename Deleter = std::default_delete<T>>
class QScopedPointer : public QUniquePointer<T, Deleter>
{
 public:
   using QUniquePointer<T, Deleter>::QUniquePointer;

   QScopedPointer(QScopedPointer && other) = delete;
   QScopedPointer &operator=(QScopedPointer && other) = delete;
};

#endif

// free functions
template <typename T, typename Deleter>
void swap(QScopedPointer<T, Deleter> &ptr1, QScopedPointer<T, Deleter> &ptr2) noexcept
{
   ptr1.swap(ptr2);
}

template <typename T, typename... Args, typename = typename std::enable_if_t<! std::is_array_v<T>>>
QScopedPointer<T> QMakeScoped(Args && ... args)
{
   return CsPointer::make_unique<T>(std::forward<Args>(args)...);
}

#endif
