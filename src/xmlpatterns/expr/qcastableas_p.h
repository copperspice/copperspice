/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QCastableAs_P_H
#define QCastableAs_P_H

#include <qsinglecontainer_p.h>
#include <qcastingplatform_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class CastableAs : public SingleContainer, public CastingPlatform<CastableAs, false>
{
 public:
   CastableAs(const Expression::Ptr &operand,
              const SequenceType::Ptr &targetType);

   virtual bool evaluateEBV(const DynamicContext::Ptr &) const;

   /**
    * Overridden to const fold to @c true when the target type
    * is a type which casting to always succeeds. This is
    * the type identical to the target type, <tt>xs:string</tt>,
    * and <tt>xs:untypedAtomic</tt>.
    */
   virtual Expression::Ptr compress(const StaticContext::Ptr &context);
   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);

   virtual SequenceType::List expectedOperandTypes() const;
   virtual SequenceType::Ptr staticType() const;

   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

   inline ItemType::Ptr targetType() const {
      return m_targetType->itemType();
   }

 private:
   const SequenceType::Ptr m_targetType;
};
}

QT_END_NAMESPACE

#endif
