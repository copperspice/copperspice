/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qplatformdefs.h>
#include <qbytearray.h>
#include <qstring.h>
#include <string.h>
#include <stdio.h>

QT_BEGIN_NAMESPACE

int qvsnprintf(char *str, size_t n, const char *fmt, va_list ap)
{
   return QT_VSNPRINTF(str, n, fmt, ap);
}

int qsnprintf(char *str, size_t n, const char *fmt, ...)
{
   va_list ap;
   va_start(ap, fmt);

   int ret = qvsnprintf(str, n, fmt, ap);
   va_end(ap);

   return ret;
}

QT_END_NAMESPACE
