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

#ifndef QTOOLS_P_H
#define QTOOLS_P_H

#include <qglobal.h>

#include <limits>

namespace QtMiscUtils {

constexpr inline char toHexUpper(uint value)
{
   return "0123456789ABCDEF"[value & 0xF];
}

constexpr inline char toHexLower(uint value)
{
   return "0123456789abcdef"[value & 0xF];
}

}   // end namespace

// typically need an extra bit for qNextPowerOfTwo when determining the next allocation size.
constexpr int MaxAllocSize = (1 << (std::numeric_limits<int>::digits - 1)) - 1;

// implemented in qbytearray.cpp
int Q_CORE_EXPORT qAllocMore(int alloc, int extra);

#endif
