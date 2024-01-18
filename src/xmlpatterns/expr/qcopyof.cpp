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

#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qitemmappingiterator_p.h"

#include "qcopyof_p.h"

using namespace QPatternist;

CopyOf::CopyOf(const Expression::Ptr &operand,
               const bool inheritNSS,
               const bool preserveNSS) : SingleContainer(operand)
   , m_inheritNamespaces(inheritNSS)
   , m_preserveNamespaces(preserveNSS)
   , m_settings((m_inheritNamespaces ? QAbstractXmlNodeModel::InheritNamespaces :
                 QAbstractXmlNodeModel::NodeCopySettings()) |
                (m_preserveNamespaces ? QAbstractXmlNodeModel::PreserveNamespaces : QAbstractXmlNodeModel::NodeCopySettings()))
{
}

Expression::Ptr CopyOf::compress(const StaticContext::Ptr &context)
{
   /* We have zero effect if we have these settings. */
   if (m_inheritNamespaces && m_preserveNamespaces) {
      return m_operand->compress(context);
   } else {
      const ItemType::Ptr t(m_operand->staticType()->itemType());
      /* We have no effect on the empty sequence or atomic values. */
      if (BuiltinTypes::xsAnyAtomicType->xdtTypeMatches(t)
            || *t == *CommonSequenceTypes::Empty) {
         return m_operand->compress(context);
      } else {
         return SingleContainer::compress(context);
      }
   }
}

void CopyOf::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
   /* Optimization: this completely breaks streaming. We get a call to
    * evaluateToSequenceReceiver() but we require heap allocations by calling
    * evaluateSequence(). */

   const Item::Iterator::Ptr it(m_operand->evaluateSequence(context));
   QAbstractXmlReceiver *const receiver = context->outputReceiver();
   Item next(it->next());

   while (next) {
      if (next.isNode()) {
         const QXmlNodeModelIndex &asNode = next.asNode();
         asNode.model()->copyNodeTo(asNode, receiver, m_settings);
      } else {
         receiver->item(next);
      }

      next = it->next();
   }
}

ExpressionVisitorResult::Ptr CopyOf::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

SequenceType::Ptr CopyOf::staticType() const
{
   return m_operand->staticType();
}

SequenceType::List CopyOf::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

Expression::Properties CopyOf::properties() const
{
   /* We have the content of node constructors as children, but even though
    * createCopyOf() typically avoids creating us, we can still end up with an operand
    * that allows compression. We must always avoid that, because we don't have
    * implementation of evaluateSequence(), and so on. */
   return (m_operand->properties() & ~CreatesFocusForLast) | DisableElimination;
}

ItemType::Ptr CopyOf::expectedContextItemType() const
{
   return m_operand->expectedContextItemType();
}

