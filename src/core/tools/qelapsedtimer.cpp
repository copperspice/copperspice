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

#include <qelapsedtimer.h>

static constexpr const qint64 invalidData = Q_INT64_C(0x8000000000000000);

void QElapsedTimer::invalidate()
{
   t1 = t2 = invalidData;
}

bool QElapsedTimer::isValid() const
{
   return t1 != invalidData && t2 != invalidData;
}

bool QElapsedTimer::hasExpired(qint64 timeout) const
{
   // if timeout is -1, quint64(timeout) is LLINT_MAX, so this will be
   // considered as never expired
   return quint64(elapsed()) > quint64(timeout);
}


