/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qpatternistlocale_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist {
QString escape(const QString &input)
{
   QString rich;
   rich.reserve(int(input.length() * 1.1));

   for (int i = 0; i < input.length(); ++i) {
      switch (input.at(i).unicode()) {
         case '<': {
            rich += QLatin1String("&lt;");
            break;
         }
         case '>': {
            rich += QLatin1String("&gt;");
            break;
         }
         case '&': {
            rich += QLatin1String("&amp;");
            break;
         }
         case '"': {
            rich += QLatin1String("&quot;");
            break;
         }
         case '\'': {
            rich += QLatin1String("&apos;");
            break;
         }
         default:
            rich += input.at(i);
      }
   }

   return rich;
}
}

QT_END_NAMESPACE
