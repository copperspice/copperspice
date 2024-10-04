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

#ifndef QPrimitives_P_H
#define QPrimitives_P_H

#include <qglobal.h>
#include <qhash.h>
#include <qurl.h>
#include <qstringfwd.h>

namespace QPatternist {
typedef qreal xsDouble;
typedef xsDouble xsFloat;
typedef xsDouble xsDecimal;

typedef qint64 xsInteger;
typedef qint32 VariableSlotID;

typedef qint32  DayCountProperty;
typedef qint32  HourCountProperty;
typedef qint32  MinuteCountProperty;
typedef qint32  MonthCountProperty;
typedef qint32  SecondCountProperty;
typedef qint64  MSecondCountProperty;
typedef qint32  SecondProperty;
typedef qint32  YearProperty;
typedef qint8   DayProperty;
typedef qint8   HourProperty;
typedef qint8   MinuteProperty;
typedef qint8   MonthProperty;

typedef qint16  MSecondProperty;
typedef qint8   ZOHourProperty;
typedef qint8   ZOMinuteProperty;
typedef qint32  ZOTotal;

typedef xsDouble PatternPriority;
typedef int ImportPrecedence;

QString escape(const QString &input);
}

#endif
