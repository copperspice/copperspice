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

#include "qcommonsequencetypes_p.h"
#include "qcurrentfn_p.h"

using namespace QPatternist;

Item CurrentFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   return context->currentItem();
}

Expression::Ptr CurrentFN::compress(const StaticContext::Ptr &context)
{
   m_itemType = context->currentItemType();
   return FunctionCall::compress(context);
}

Expression::Ptr CurrentFN::typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType)
{
   m_itemType = context->currentItemType();
   return FunctionCall::typeCheck(context, reqType);
}

SequenceType::Ptr CurrentFN::staticType() const
{
   if (m_itemType) {
      return makeGenericSequenceType(m_itemType, Cardinality::exactlyOne());
   } else {
      return CommonSequenceTypes::ExactlyOneItem;
   }
}

