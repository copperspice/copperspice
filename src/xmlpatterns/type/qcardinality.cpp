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

#include "qpatternistlocale_p.h"

#include "qcardinality_p.h"

using namespace QPatternist;

QString Cardinality::displayName(const CustomizeDisplayName explain) const
{
   if (explain == IncludeExplanation) {
      if (isEmpty()) {
         return QString(QtXmlPatterns::tr("empty") + QLatin1String("(\"empty-sequence()\")"));
      } else if (isZeroOrOne()) {
         return QString(QtXmlPatterns::tr("zero or one") + QLatin1String("(\"?\")"));
      } else if (isExactlyOne()) {
         return QString(QtXmlPatterns::tr("exactly one"));
      } else if (isOneOrMore()) {
         return QString(QtXmlPatterns::tr("one or more") + QLatin1String("(\"+\")"));
      } else {
         return QString(QtXmlPatterns::tr("zero or more") + QLatin1String("(\"*\")"));
      }
   } else {
      Q_ASSERT(explain == ExcludeExplanation);

      if (isEmpty() || isZeroOrOne()) {
         return QLatin1String("?");
      } else if (isExactlyOne()) {
         return QString();
      } else if (isExact()) {
         return QString(QLatin1Char('{'))    +
                QString::number(maximum())   +
                QLatin1Char('}');
      } else {
         if (m_max == -1) {
            if (isOneOrMore()) {
               return QChar::fromLatin1('+');
            } else {
               return QChar::fromLatin1('*');
            }
         } else {
            /* We have a range. We use a RegExp-like syntax. */
            return QString(QLatin1Char('{'))    +
                   QString::number(minimum())   +
                   QLatin1String(", ")          +
                   QString::number(maximum())   +
                   QLatin1Char('}');

         }
      }
   }
}

