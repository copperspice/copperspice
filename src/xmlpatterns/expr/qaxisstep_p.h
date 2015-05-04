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

#ifndef QAxisStep_P_H
#define QAxisStep_P_H

#include <qemptycontainer_p.h>
#include <qitem_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class AxisStep : public EmptyContainer
{
 public:
   AxisStep(const QXmlNodeModelIndex::Axis axis, const ItemType::Ptr &nodeTest);

   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const;
   virtual Item evaluateSingleton(const DynamicContext::Ptr &) const;

   /**
    * Returns @p node if it matches the node test this step is using, otherwise @c null.
    */
   inline Item mapToItem(const QXmlNodeModelIndex &node,
                         const DynamicContext::Ptr &context) const;

   virtual SequenceType::List expectedOperandTypes() const;
   virtual SequenceType::Ptr staticType() const;

   /**
    * Rewrites to ParentNodeAxis, if possible.
    */
   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType);

   /**
    * @returns always BuiltinTypes::node;
    */
   virtual ItemType::Ptr expectedContextItemType() const;

   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;

   virtual Properties properties() const;

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

   virtual ID id() const;
   virtual PatternPriority patternPriority() const;

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

QT_END_NAMESPACE

#endif
