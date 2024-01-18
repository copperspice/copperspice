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

#include "qatomicmathematician_p.h"

using namespace QPatternist;

AtomicMathematician::~AtomicMathematician()
{
}

QString AtomicMathematician::displayName(const AtomicMathematician::Operator op)
{
   switch (op) {
      case AtomicMathematician::Div:
         return QLatin1String("div");
      case AtomicMathematician::IDiv:
         return QLatin1String("idiv");
      case AtomicMathematician::Substract:
         return QLatin1String("-");
      case AtomicMathematician::Mod:
         return QLatin1String("mod");
      case AtomicMathematician::Multiply:
         return QLatin1String("*");
      case AtomicMathematician::Add:
         return QLatin1String("+");
   }

   return QString(); /* Silence GCC warning. */
}
