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

#include <qstringview.h>
#include <qunicodetables_p.h>

std::pair<int32_t,  const ushort *> cs_internal_convertCaseTrait(int trait, const uint32_t value)
{
   int32_t caseDiff          = 0;
   const ushort *specialCase = nullptr;


   const QUnicodeTables::Properties *prop = QUnicodeTables::properties(value);

   if (trait == 1)  {
      caseDiff = QUnicodeTables::CasefoldTraits::caseDiff(prop);

      if (QUnicodeTables::CasefoldTraits::caseSpecial(prop)) {
         specialCase = QUnicodeTables::specialCaseMap + caseDiff;
      }

   } else if (trait == 2) {
      caseDiff = QUnicodeTables::LowercaseTraits::caseDiff(prop);

      if (QUnicodeTables::LowercaseTraits::caseSpecial(prop)) {
         specialCase = QUnicodeTables::specialCaseMap + caseDiff;
      }

   } else if (trait == 3) {
      caseDiff = QUnicodeTables::UppercaseTraits::caseDiff(prop);

      if (QUnicodeTables::UppercaseTraits::caseSpecial(prop)) {
         specialCase = QUnicodeTables::specialCaseMap + caseDiff;
      }
   }

   return std::make_pair(caseDiff, specialCase);
}
