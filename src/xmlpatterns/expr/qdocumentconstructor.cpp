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

#include "qcommonsequencetypes_p.h"
#include "qdocumentcontentvalidator_p.h"
#include "qnodebuilder_p.h"

#include "qdocumentconstructor_p.h"

using namespace QPatternist;

DocumentConstructor::DocumentConstructor(const Expression::Ptr &op) : SingleContainer(op)
{
}

Item DocumentConstructor::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   NodeBuilder::Ptr nodeBuilder(context->nodeBuilder(m_staticBaseURI));

   DocumentContentValidator validator(nodeBuilder.get(), context, ConstPtr(this));
   const DynamicContext::Ptr receiverContext(context->createReceiverContext(&validator));

   validator.startDocument();
   m_operand->evaluateToSequenceReceiver(receiverContext);
   validator.endDocument();

   const QAbstractXmlNodeModel::Ptr nm(nodeBuilder->builtDocument());
   context->addNodeModel(nm);

   return nm->root(QXmlNodeModelIndex());
}

void DocumentConstructor::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
   QAbstractXmlReceiver *const receiver = context->outputReceiver();

   DocumentContentValidator validator(receiver, context, ConstPtr(this));

   const DynamicContext::Ptr receiverContext(context->createReceiverContext(&validator));

   validator.startDocument();
   m_operand->evaluateToSequenceReceiver(receiverContext);
   validator.endDocument();
}

Expression::Ptr DocumentConstructor::typeCheck(const StaticContext::Ptr &context,
      const SequenceType::Ptr &reqType)
{
   m_staticBaseURI = context->baseURI();
   return SingleContainer::typeCheck(context, reqType);
}

SequenceType::Ptr DocumentConstructor::staticType() const
{
   return CommonSequenceTypes::ExactlyOneDocumentNode;
}

SequenceType::List DocumentConstructor::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

Expression::Properties DocumentConstructor::properties() const
{
   return DisableElimination | IsNodeConstructor;
}

ExpressionVisitorResult::Ptr
DocumentConstructor::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}
