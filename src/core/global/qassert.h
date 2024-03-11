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

#ifndef QASSERT_H
#define QASSERT_H

#include <qexport.h>

static constexpr  inline void cs_do_nothing(void)
{
}

Q_CORE_EXPORT void qt_assert(const char *assertion, const char *file, int line);

#if ! defined(Q_ASSERT)
#  ifdef CS_DISABLE_ASSERT
#    define Q_ASSERT(cond) cs_do_nothing()
#  else
#    define Q_ASSERT(cond) ((!(cond)) ? qt_assert(#cond,__FILE__,__LINE__) : cs_do_nothing())
#  endif
#endif

Q_CORE_EXPORT void qt_assert_x(const char *where, const char *what, const char *file, int line);

#if ! defined(Q_ASSERT_X)
#  ifdef CS_DISABLE_ASSERT
#    define Q_ASSERT_X(cond, where, what) cs_do_nothing()
#  else
#    define Q_ASSERT_X(cond, where, what) ((!(cond)) ? qt_assert_x(where, what,__FILE__,__LINE__) : cs_do_nothing())
#  endif
#endif

#endif
