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

#ifndef QSCOPEDARRAYPOINTER_H
#define QSCOPEDARRAYPOINTER_H

#include <quniquepointer.h>

template <typename T, typename Deleter = std::default_delete<CsPointer::cs_add_missing_extent_t<T>>>
class QScopedArrayPointer : public QUniqueArrayPointer<T, Deleter>
{
public:
   using QUniqueArrayPointer<T, Deleter>::QUniqueArrayPointer;

   QScopedArrayPointer(QScopedArrayPointer && other) = delete;
   QScopedArrayPointer &operator=(QScopedArrayPointer && other) = delete;
};

// free functions
template <typename T, typename Deleter>
void swap(QScopedArrayPointer<T, Deleter> &ptr1, QScopedArrayPointer<T, Deleter> &ptr2) noexcept
{
   ptr1.swap(ptr2);
}

template <typename T, typename = typename std::enable_if_t<std::is_array_v<T>>>
QScopedArrayPointer<T> QMakeScoped(std::size_t size)
{
   return CsPointer::make_unique<T>(size);
}

#endif
