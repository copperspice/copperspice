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

#include <math.h>

#include "qabstractfloat_p.h"
#include "qboolean_p.h"
#include "qbuiltintypes_p.h"
#include "qcommonvalues_p.h"
#include "qdecimal_p.h"
#include "qinteger_p.h"

#include "qschemanumeric_p.h"

/**
 * @file Contains class Numeric. This file was originally called qnumeric.cpp,
 * but was renamed to stay consistent with qschemanumeric_p.h
 */

using namespace QPatternist;

AtomicValue::Ptr Numeric::fromLexical(const QString &number)
{
   Q_ASSERT(!number.isEmpty());
   Q_ASSERT_X(!number.contains(QLatin1Char('e')) &&
              !number.contains(QLatin1Char('E')),
              Q_FUNC_INFO, "Should not contain any e/E");

   if (number.contains(QLatin1Char('.'))) { /* an xs:decimal. */
      return Decimal::fromLexical(number);
   } else { /* It's an integer, of some sort. E.g, -3, -2, -1, 0, 1, 2, 3 */
      return Integer::fromLexical(number);
   }
}

xsDouble Numeric::roundFloat(const xsDouble val)
{
   if (qIsInf(val) || AbstractFloat<true>::isEqual(val, 0.0)) {
      return val;
   } else if (qIsNaN(val)) {
      return val;
   } else {
      if (val >= -0.5 && val < 0) {
         return -0.0;
      } else {
         return ::floor(val + 0.5);
      }

   }
}
