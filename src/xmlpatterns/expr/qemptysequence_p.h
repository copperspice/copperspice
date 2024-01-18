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

#ifndef QEmptySequence_P_H
#define QEmptySequence_P_H

#include <qemptycontainer_p.h>

namespace QPatternist {

class EmptySequence : public EmptyContainer
{
 public:

   static Expression::Ptr create(const Expression *const replacementFor, const StaticContext::Ptr &context);

   inline EmptySequence() {
   }

   virtual QString stringValue() const;

   /**
    * @returns always an empty iterator, an instance of EmptyIterator.
    */
   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const override;

   /**
    * @returns always @c null.
    */
   Item evaluateSingleton(const DynamicContext::Ptr &) const override;

   /**
    * Does nothing.
    */
   void evaluateToSequenceReceiver(const DynamicContext::Ptr &) const override;

   /**
    * @returns always @c false.
    */
   bool evaluateEBV(const DynamicContext::Ptr &context) const override;

   /**
    * @returns always CommonSequenceTypes::Empty
    */
   virtual ItemType::Ptr type() const;

   /**
    * @returns always CommonSequenceTypes::Empty
    */
   SequenceType::Ptr staticType() const override;

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
   ID id() const override;
   Properties properties() const override;
};

}

#endif
