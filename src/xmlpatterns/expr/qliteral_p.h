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

#ifndef QLiteral_P_H
#define QLiteral_P_H

#include <qemptycontainer_p.h>

namespace QPatternist {

class Literal : public EmptyContainer
{
 public:
   Literal(const Item &item);

   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
   bool evaluateEBV(const DynamicContext::Ptr &context) const override;
   void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const override;

   SequenceType::Ptr staticType() const override;
   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
   ID id() const override;
   QString description() const override;

   Properties properties() const override;
   Item item() const {
      return m_item;
   }

 private:
   const Item m_item;
};

inline Expression::Ptr wrapLiteral(const Item &item,
                                   const StaticContext::Ptr &context,
                                   const SourceLocationReflection *const r)
{
   Q_ASSERT(item);

   const Expression::Ptr retval(new Literal(item));
   context->addLocation(retval.data(), context->locationFor(r));

   return retval;
}

}

#endif
