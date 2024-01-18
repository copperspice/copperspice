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

#include <qnumeric.h>
#include <qnumeric_p.h>

Q_CORE_EXPORT bool qIsInf(double d)
{
   return qt_is_inf(d);
}

Q_CORE_EXPORT bool qIsNaN(double d)
{
   return qt_is_nan(d);
}

Q_CORE_EXPORT bool qIsFinite(double d)
{
   return qt_is_finite(d);
}

Q_CORE_EXPORT bool qIsInf(float f)
{
   return qt_is_inf(f);
}

Q_CORE_EXPORT bool qIsNaN(float f)
{
   return qt_is_nan(f);
}

Q_CORE_EXPORT bool qIsFinite(float f)
{
   return qt_is_finite(f);
}

/*!
    Returns the bit pattern of a signalling NaN as a double.
*/
Q_CORE_EXPORT double qSNaN()
{
   return qt_snan();
}

/*!
    Returns the bit pattern of a quiet NaN as a double.
*/
Q_CORE_EXPORT double qQNaN()
{
   return qt_qnan();
}

/*!
    Returns the bit pattern for an infinite number as a double.
*/
Q_CORE_EXPORT double qInf()
{
   return qt_inf();
}

