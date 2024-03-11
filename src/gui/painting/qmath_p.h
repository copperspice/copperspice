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

#ifndef QMATH_P_H
#define QMATH_P_H

#include <math.h>
#include <qmath.h>

static constexpr const qreal Q_PI   = qreal(M_PI);
static constexpr const qreal Q_2PI  = qreal(M_PI * 2);
static constexpr const qreal Q_PI2  = qreal(M_PI / 2);

static constexpr const qreal Q_MM_PER_INCH = 25.4;

inline int qIntSqrtInt(int v)
{
   return static_cast<int>(qSqrt(static_cast<qreal>(v)));
}

#endif
