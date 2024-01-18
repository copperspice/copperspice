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

#ifndef QCurrentItemStore_P_H
#define QCurrentItemStore_P_H

#include <qsinglecontainer_p.h>

namespace QPatternist {

class CurrentItemStore : public SingleContainer
{
 public:
   CurrentItemStore(const Expression::Ptr &operand);

   bool evaluateEBV(const DynamicContext::Ptr &) const override;
   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const override;
   Item evaluateSingleton(const DynamicContext::Ptr &) const override;

   SequenceType::List expectedOperandTypes() const override;
   Expression::Ptr compress(const StaticContext::Ptr &context) override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   Properties properties() const override;

   /**
    * @returns the staticType() of its operand.
    */
   SequenceType::Ptr staticType() const override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
   const SourceLocationReflection *actualReflection() const override;

 private:
   static inline StaticContext::Ptr newStaticContext(const StaticContext::Ptr &context);
   inline DynamicContext::Ptr createContext(const DynamicContext::Ptr &old) const;
};

}

#endif
