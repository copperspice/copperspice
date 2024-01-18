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

#ifndef QAxisStep_P_H
#define QAxisStep_P_H

#include <qemptycontainer_p.h>
#include <qitem_p.h>

namespace QPatternist {

class AxisStep : public EmptyContainer
{
 public:
   AxisStep(const QXmlNodeModelIndex::Axis axis, const ItemType::Ptr &nodeTest);

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const override;
   Item evaluateSingleton(const DynamicContext::Ptr &) const override;

   /**
    * Returns @p node if it matches the node test this step is using, otherwise @c null.
    */
   inline Item mapToItem(const QXmlNodeModelIndex &node,
                         const DynamicContext::Ptr &context) const;

   SequenceType::List expectedOperandTypes() const override;
   SequenceType::Ptr staticType() const override;

   /**
    * Rewrites to ParentNodeAxis, if possible.
    */
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   /**
    * @returns always BuiltinTypes::node;
    */
   ItemType::Ptr expectedContextItemType() const override;

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   Properties properties() const override;

   /**
    * @returns the axis this step is using.
    */
   QXmlNodeModelIndex::Axis axis() const;

   inline ItemType::Ptr nodeTest() const {
      return m_nodeTest;
   }

   void setNodeTest(const ItemType::Ptr &nev) {
      m_nodeTest = nev;
   }

   static QString axisName(const QXmlNodeModelIndex::Axis axis);

   ID id() const override;
   PatternPriority patternPriority() const override;

   inline void setAxis(const QXmlNodeModelIndex::Axis newAxis);

 private:
   typedef QExplicitlySharedDataPointer<const AxisStep> ConstPtr;

   static const QXmlNodeModelIndex::NodeKind s_whenAxisNodeKindEmpty[];

   static bool isAlwaysEmpty(const QXmlNodeModelIndex::Axis axis, const QXmlNodeModelIndex::NodeKind nodeKind);


   QXmlNodeModelIndex::Axis m_axis;
   ItemType::Ptr m_nodeTest;
};

void AxisStep::setAxis(const QXmlNodeModelIndex::Axis newAxis)
{
   m_axis = newAxis;
}

}

#endif
