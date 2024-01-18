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
#include "qpatternistlocale_p.h"
#include "qnodebuilder_p.h"

#include "qcommentconstructor_p.h"

using namespace QPatternist;

CommentConstructor::CommentConstructor(const Expression::Ptr &op) : SingleContainer(op)
{
}

QString CommentConstructor::evaluateContent(const DynamicContext::Ptr &context) const
{
   const Item item(m_operand->evaluateSingleton(context));

   if (!item) {
      return QString();
   }

   const QString content(item.stringValue());

   if (content.contains(QLatin1String("--"))) {
      context->error(QtXmlPatterns::tr("A comment cannot contain %1")
                     .formatArg(formatData("--")),
                     ReportContext::XQDY0072, this);
   } else if (content.endsWith(QLatin1Char('-'))) {
      context->error(QtXmlPatterns::tr("A comment cannot end with a %1.")
                     .formatArg(formatData(QLatin1Char('-'))),
                     ReportContext::XQDY0072, this);
   }

   return content;
}

Item CommentConstructor::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const QString content(evaluateContent(context));
   const NodeBuilder::Ptr nodeBuilder(context->nodeBuilder(QUrl()));
   nodeBuilder->comment(content);

   const QAbstractXmlNodeModel::Ptr nm(nodeBuilder->builtDocument());
   context->addNodeModel(nm);

   return nm->root(QXmlNodeModelIndex());
}

void CommentConstructor::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
   const QString content(evaluateContent(context));
   QAbstractXmlReceiver *const receiver = context->outputReceiver();

   receiver->comment(content);
}

SequenceType::Ptr CommentConstructor::staticType() const
{
   return CommonSequenceTypes::ExactlyOneComment;
}

SequenceType::List CommentConstructor::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrOneString);
   return result;
}

Expression::Properties CommentConstructor::properties() const
{
   return DisableElimination | IsNodeConstructor;
}

ExpressionVisitorResult::Ptr
CommentConstructor::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

