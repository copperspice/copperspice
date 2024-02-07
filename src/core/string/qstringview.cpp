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

#include <qstringview.h>

#include <qstring8.h>
#include <qstring16.h>

#include <qunicodetables_p.h>

std::pair<char32_t, const char32_t *> cs_internal_convertCaseTrait(int trait, char32_t value)
{
   char32_t caseValue = value;
   const char32_t *caseSpecial = nullptr;

   if (trait == 1)  {
      caseValue = QUnicodeTables::CaseFoldTraits::caseValue(value);

      if (caseValue == 0 && value != 0) {
         // special char
         caseSpecial = QUnicodeTables::CaseFoldTraits::caseSpecial(value);
      }

   } else if (trait == 2) {
      caseValue = QUnicodeTables::LowerCaseTraits::caseValue(value);

      if (caseValue == 0 && value != 0) {
         // special char
         caseSpecial = QUnicodeTables::LowerCaseTraits::caseSpecial(value);
      }

   } else if (trait == 3) {
      caseValue = QUnicodeTables::UpperCaseTraits::caseValue(value);

      if (caseValue == 0 && value != 0) {
         // special char
         caseSpecial = QUnicodeTables::UpperCaseTraits::caseSpecial(value);
      }
   }

   return { caseValue, caseSpecial };
}

QStringView8 make_view(QString8 &&str)
{
   return QStringView8(str.cbegin(), str.cend());
}

QStringView16 make_view(QString16 &&str)
{
   return QStringView16(str.cbegin(), str.cend());
}
