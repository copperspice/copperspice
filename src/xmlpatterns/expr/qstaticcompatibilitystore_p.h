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

#ifndef QStaticCompatibilityStore_P_H
#define QStaticCompatibilityStore_P_H

#include <qsinglecontainer_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class StaticCompatibilityStore : public SingleContainer
{
 public:
   /**
    * @p baseURI must be valid, not empty, and either relative or
    * absolute.
    */
   StaticCompatibilityStore(const Expression::Ptr &operand);

   Expression::Ptr typeCheck(const StaticContext::Ptr &context,const SequenceType::Ptr &reqType) override;
   SequenceType::Ptr staticType() const override;
   SequenceType::List expectedOperandTypes() const override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
};
}

QT_END_NAMESPACE

#endif
