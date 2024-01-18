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

#include "qcommonvalues_p.h"
#include "qcommonsequencetypes_p.h"
#include "qemptysequencetype_p.h"

#include "qemptysequence_p.h"

using namespace QPatternist;

Expression::Ptr EmptySequence::create(const Expression *const replacementFor,
                                      const StaticContext::Ptr &context)
{
   Q_ASSERT(replacementFor);
   Q_ASSERT(context);

   const Expression::Ptr retval(new EmptySequence());
   context->wrapExpressionWith(replacementFor, retval);
   return retval;
}

Item::Iterator::Ptr EmptySequence::evaluateSequence(const DynamicContext::Ptr &) const
{
   return CommonValues::emptyIterator;
}

Item EmptySequence::evaluateSingleton(const DynamicContext::Ptr &) const
{
   return Item();
}

void EmptySequence::evaluateToSequenceReceiver(const DynamicContext::Ptr &) const
{
}

ItemType::Ptr EmptySequence::type() const
{
   return CommonSequenceTypes::Empty;
}

SequenceType::Ptr EmptySequence::staticType() const
{
   return CommonSequenceTypes::Empty;
}

bool EmptySequence::evaluateEBV(const DynamicContext::Ptr &) const
{
   return false;
}

QString EmptySequence::stringValue() const
{
   return QString();
}

ExpressionVisitorResult::Ptr EmptySequence::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

Expression::ID EmptySequence::id() const
{
   return IDEmptySequence;
}

Expression::Properties EmptySequence::properties() const
{
   return IsEvaluated;
}
