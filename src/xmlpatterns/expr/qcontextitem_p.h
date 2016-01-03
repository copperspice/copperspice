/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

#ifndef QContextItem_P_H
#define QContextItem_P_H

#include <qemptycontainer_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {

class ContextItem : public EmptyContainer
{
 public:
   /**
    * @p expr is possibly used for error reporting. If this context item has been
    * created implicitly, such as for the expression <tt>fn:string()</tt>, @p expr
    * should be passed a valid pointer to the Expression that this context
    * item is generated for.
    */
   inline ContextItem(const Expression::Ptr &expr = Expression::Ptr()) : m_expr(expr) {
   }

   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
   virtual SequenceType::Ptr staticType() const;

   /**
    * @returns always DisableElimination and RequiresContextItem
    */
   virtual Expression::Properties properties() const;

   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

   /**
    * Overridden to store a pointer to StaticContext::contextItemType().
    */
   virtual Expression::Ptr compress(const StaticContext::Ptr &context);

   /**
    * Overridden to store a pointer to StaticContext::contextItemType().
    */
   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);

   /**
    * @returns always IDContextItem
    */
   virtual ID id() const;

   /**
    * @returns always BuiltinTypes::item;
    */
   virtual ItemType::Ptr expectedContextItemType() const;

   virtual const SourceLocationReflection *actualReflection() const;
   virtual void announceFocusType(const ItemType::Ptr &type);

 private:
   ItemType::Ptr           m_itemType;
   const Expression::Ptr   m_expr;
};
}

QT_END_NAMESPACE

#endif
