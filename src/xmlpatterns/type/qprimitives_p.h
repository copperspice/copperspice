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

/**
 * This is the native C++ scalar type holding the value space
 * for atomic values of type xs:integer. Taking this type, xsInteger,
 * as parameter, is the most efficient way to integrate with xs:integer.
 *
 * @ingroup Patternist_cppWXSTypes
 */
typedef qint64 xsInteger;

/**
 * This is the native C++ scalar type holding the value space
 * for atomic values of type xs:integer. Taking this type, xsInteger,
 * as parameter, is the most efficient way to integrate with xs:integer.
 *
 * @ingroup Patternist_cppWXSTypes
 */
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

/**
 * Milliseconds. 1 equals 0.001 SecondProperty.
 */
typedef qint16  MSecondProperty;

/**
 * The hour property of a zone offset. For example, -13 in the
 * zone offset "-13:08".
 */
typedef qint8   ZOHourProperty;

/**
 * The minute property of a zone offset. For example, -08 in the
 * zone offset "-13:08".
 */
typedef qint8   ZOMinuteProperty;

/**
 * The full zone offset in minutes.
 */
typedef qint32  ZOTotal;

typedef xsDouble PatternPriority;

/**
 * Signifies the import precedence of a template. For instance, the first
 * stylesheet module has 1, the first import 2, and so forth. Smaller means
 * higher import precedence. 0 is reserved for builtin templates.
 */
typedef int ImportPrecedence;

QString escape(const QString &input);
}

#endif
