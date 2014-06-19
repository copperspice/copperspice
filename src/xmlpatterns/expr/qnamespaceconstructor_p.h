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

#ifndef Patternist_NamespaceConstructor_H
#define Patternist_NamespaceConstructor_H

#include "qemptycontainer_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist {
class NamespaceConstructor : public EmptyContainer
{
 public:
   NamespaceConstructor(const QXmlName nb);

   virtual void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const;

   /**
    * @returns a list containing one CommonSequenceTypes::ExactlyOneString instance.
    */
   virtual SequenceType::List expectedOperandTypes() const;

   /**
    * The static type is exactly one attribute node. It's unclear what
    * affects the static type has, but specifying anything else could lead
    * to complications wrt. node order, XQTY0024. Of course, it's not
    * conceptually correct, since a namespace node isn't an attribute
    * node.
    */
   virtual SequenceType::Ptr staticType() const;

   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
   virtual Expression::Properties properties() const;

   inline const QXmlName &namespaceBinding() const {
      return m_binding;
   }

   virtual ID id() const;
 private:
   const QXmlName m_binding;
};
}

QT_END_NAMESPACE

#endif
