/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qassert.h>
#include <qlog.h>

void qt_assert(const char *assertion, const char *file, int line)
{
   // Q_ASSERT macro calls this function when the test fails
   qFatal("ASSERT: \"%s\" in file %s, line %d", assertion, file, line);
}

void qt_assert_x(const char *where, const char *what, const char *file, int line)
{
   // Q_ASSERT_X macro calls this function when the test fails
   qFatal("ASSERT failure in %s: \"%s\", file %s, line %d", where, what, file, line);
}