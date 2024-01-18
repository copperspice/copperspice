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

#include <qstring.h>

#include "qatomiccomparator_p.h"

using namespace QPatternist;

AtomicComparator::AtomicComparator()
{ }

AtomicComparator::~AtomicComparator()
{ }

AtomicComparator::ComparisonResult
AtomicComparator::compare(const Item &, const AtomicComparator::Operator, const Item &) const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function should never be called.");
   return LessThan;
}

QString AtomicComparator::displayName(const AtomicComparator::Operator op, const ComparisonType type)
{
   Q_ASSERT(type == AsGeneralComparison || type == AsValueComparison);
   if (type == AsGeneralComparison) {
      switch (op) {
         case OperatorEqual:
            return QLatin1String("=");
         case OperatorGreaterOrEqual:
            return QLatin1String("<=");
         case OperatorGreaterThan:
            return QLatin1String("<");
         case OperatorLessOrEqual:
            return QLatin1String(">=");

         case OperatorLessThanNaNLeast:
         case OperatorLessThanNaNGreatest:
         case OperatorLessThan:
            return QLatin1String(">");

         case OperatorNotEqual:
            return QLatin1String("!=");
      }
   }

   switch (op) {
      case OperatorEqual:
         return QLatin1String("eq");
      case OperatorGreaterOrEqual:
         return QLatin1String("ge");
      case OperatorGreaterThan:
         return QLatin1String("gt");
      case OperatorLessOrEqual:
         return QLatin1String("le");

      case OperatorLessThanNaNLeast:
      case OperatorLessThanNaNGreatest:
      case OperatorLessThan:
         return QLatin1String("lt");

      case OperatorNotEqual:
         return QLatin1String("ne");
   }

   Q_ASSERT(false);
   return QString(); /* GCC unbarfer. */
}

