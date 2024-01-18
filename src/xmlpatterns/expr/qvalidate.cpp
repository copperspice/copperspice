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

#include "qbuiltintypes_p.h"
#include "qgenericsequencetype_p.h"
#include "qmultiitemtype_p.h"
#include "qtypechecker_p.h"

#include "qvalidate_p.h"

using namespace QPatternist;

Expression::Ptr Validate::create(const Expression::Ptr &operandNode,
                                 const Mode validationMode,
                                 const StaticContext::Ptr &context)
{
   Q_ASSERT(operandNode);
   Q_ASSERT(validationMode == Lax || validationMode == Strict);
   Q_ASSERT(context);

   (void) validationMode;
   (void) context;

   ItemType::List tList;
   tList.append(BuiltinTypes::element);
   tList.append(BuiltinTypes::document);

   const SequenceType::Ptr elementOrDocument(makeGenericSequenceType(ItemType::Ptr(new MultiItemType(tList)),
         Cardinality::exactlyOne()));


   return TypeChecker::applyFunctionConversion(operandNode,
          elementOrDocument,
          context,
          ReportContext::XQTY0030);
}
