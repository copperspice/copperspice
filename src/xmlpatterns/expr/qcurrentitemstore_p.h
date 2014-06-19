/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef Patternist_CurrentItemStore_H
#define Patternist_CurrentItemStore_H

#include "qsinglecontainer_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist {

class CurrentItemStore : public SingleContainer
{
 public:
   CurrentItemStore(const Expression::Ptr &operand);

   virtual bool evaluateEBV(const DynamicContext::Ptr &) const;
   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const;
   virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;

   virtual SequenceType::List expectedOperandTypes() const;
   virtual Expression::Ptr compress(const StaticContext::Ptr &context);
   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);

   virtual Properties properties() const;

   /**
    * @returns the staticType() of its operand.
    */
   virtual SequenceType::Ptr staticType() const;
   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
   virtual const SourceLocationReflection *actualReflection() const;

 private:
   static inline StaticContext::Ptr newStaticContext(const StaticContext::Ptr &context);
   inline DynamicContext::Ptr createContext(const DynamicContext::Ptr &old) const;
};
}

QT_END_NAMESPACE

#endif
