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

#ifndef QCopyOf_P_H
#define QCopyOf_P_H

#include <qsinglecontainer_p.h>

namespace QPatternist {

class CopyOf : public SingleContainer
{
 public:
   /**
    * Creats a CopyOf where it is checked that the expression @p operand conforms
    * to the type @p reqType.
    */
   CopyOf(const Expression::Ptr &operand,
          const bool inheritNSS,
          const bool preserveNSS);

   void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const override;

   /**
    * @returns always the SequenceType passed in the constructor to this class. That is, the
    * SequenceType that the operand must conform to.
    */
   SequenceType::Ptr staticType() const override;

   /**
    * @returns a list containing one CommonSequenceTypes::ZeroOrMoreItems
    */
   SequenceType::List expectedOperandTypes() const override;

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   inline Item mapToItem(const Item &source, const DynamicContext::Ptr &context) const;

   Expression::Ptr compress(const StaticContext::Ptr &context) override;

   Properties properties() const override;
   ItemType::Ptr expectedContextItemType() const override;

 private:
   typedef QExplicitlySharedDataPointer<const CopyOf> ConstPtr;
   const bool                                      m_inheritNamespaces;
   const bool                                      m_preserveNamespaces;
   const QAbstractXmlNodeModel::NodeCopySettings   m_settings;
};

}

#endif
