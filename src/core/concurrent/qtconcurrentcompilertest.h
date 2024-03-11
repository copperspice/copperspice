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

#ifndef QTCONCURRENT_COMPILERTEST_H
#define QTCONCURRENT_COMPILERTEST_H

#include <qglobal.h>

namespace QtPrivate {

template <class T>
class HasResultType
{
   template <typename U>
   static char test(int, const typename U::result_type * = nullptr);

   template <typename U>
   static void *test(double);

 public:
   static constexpr bool Value = (sizeof(test<T>(0)) == sizeof(char));
};

}  // namespace

#endif
