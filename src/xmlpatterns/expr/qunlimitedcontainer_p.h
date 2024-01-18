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

#ifndef QUnlimitedContainer_P_H
#define QUnlimitedContainer_P_H

#include <qlist.h>

#include <qexpression_p.h>
#include <qgenericsequencetype_p.h>

namespace QPatternist {

class UnlimitedContainer : public Expression
{
 public:
   UnlimitedContainer(const Expression::List &operands = Expression::List());

   void setOperands(const Expression::List &list) override;
   Expression::List operands() const override;
   bool compressOperands(const StaticContext::Ptr &) override;

   enum CardinalityComputation {
      ProductOfCardinality,
      UnionOfCardinality
   };

 protected:
   template<CardinalityComputation suppliedCard>
   inline
   SequenceType::Ptr operandsUnionType() const {
      Q_ASSERT(suppliedCard ==  ProductOfCardinality || suppliedCard == UnionOfCardinality);
      const Expression::List::const_iterator end(m_operands.constEnd());
      Expression::List::const_iterator it(m_operands.constBegin());

      /* Load the first one, and jump over it in the loop. */
      SequenceType::Ptr t(m_operands.first()->staticType());
      ItemType::Ptr type(t->itemType());
      Cardinality card(t->cardinality());
      ++it;

      for (; it != end; ++it) {
         t = (*it)->staticType();
         type |= t->itemType();

         /* Since this function is a template function, it doesn't
          * hurt performance that this test is inside the loop. */
         if constexpr (suppliedCard == ProductOfCardinality) {
            card += t->cardinality();
         } else {
            card |= t->cardinality();
         }
      }

      return makeGenericSequenceType(type, card);
   }

   Expression::List m_operands;
};


}

#endif
