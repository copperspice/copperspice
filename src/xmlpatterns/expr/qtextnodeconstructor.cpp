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

#include <QUrl>

#include "qcommonsequencetypes_p.h"
#include "qnodebuilder_p.h"

#include "qtextnodeconstructor_p.h"

using namespace QPatternist;

TextNodeConstructor::TextNodeConstructor(const Expression::Ptr &op) : SingleContainer(op)
{
}

Item TextNodeConstructor::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item chars(m_operand->evaluateSingleton(context));

   if (!chars) {
      return Item();
   }

   const NodeBuilder::Ptr nodeBuilder(context->nodeBuilder(QUrl()));
   const QString &v = chars.stringValue();
   nodeBuilder->characters(QStringView(v));

   const QAbstractXmlNodeModel::Ptr nm(nodeBuilder->builtDocument());
   context->addNodeModel(nm);

   return nm->root(QXmlNodeModelIndex());
}

void TextNodeConstructor::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
   const Item item(m_operand->evaluateSingleton(context));

   QAbstractXmlReceiver *const receiver = context->outputReceiver();

   if (item) {
      const QString &v = item.stringValue();
      receiver->characters(QStringView(v));

   } else {
      receiver->characters(QStringView());
   }
}

SequenceType::Ptr TextNodeConstructor::staticType() const
{
   if (m_operand->staticType()->cardinality().allowsEmpty()) {
      return CommonSequenceTypes::ZeroOrOneTextNode;
   } else {
      return CommonSequenceTypes::ExactlyOneTextNode;
   }
}

SequenceType::List TextNodeConstructor::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrOneString);
   return result;
}

Expression::Properties TextNodeConstructor::properties() const
{
   return DisableElimination | IsNodeConstructor;
}

ExpressionVisitorResult::Ptr
TextNodeConstructor::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}
