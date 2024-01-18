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

#include <qcardinalityverifier_p.h>

#include <qcommonsequencetypes_p.h>
#include <qcommonvalues_p.h>
#include <qgenericpredicate_p.h>
#include <qgenericsequencetype_p.h>
#include <qinsertioniterator_p.h>
#include <qpatternistlocale_p.h>

using namespace QPatternist;

QString CardinalityVerifier::wrongCardinality(const Cardinality &req, const Cardinality &got)
{
   return QtXmlPatterns::tr("Required cardinality is %1, received cardinality %2.")
          .formatArgs(formatType(req), formatType(got));
}

Expression::Ptr CardinalityVerifier::verifyCardinality(const Expression::Ptr &operand,
      const Cardinality &requiredCard, const StaticContext::Ptr &context, const ReportContext::ErrorCode code)
{
   const Cardinality opCard(operand->staticType()->cardinality());

   if (requiredCard.isMatch(opCard)) {
      return operand;

   } else if (requiredCard.canMatch(opCard)) {
      return Expression::Ptr(new CardinalityVerifier(operand, requiredCard, code));

   } else if (context->compatModeEnabled() && !opCard.isEmpty()) {
      return GenericPredicate::createFirstItem(operand);

   } else {
      /* Sequences within this cardinality can never match. */
      context->error(wrongCardinality(requiredCard, opCard), code, operand.data());
      return operand;
   }
}

CardinalityVerifier::CardinalityVerifier(const Expression::Ptr &operand, const Cardinality &card, const ReportContext::ErrorCode code)
   : SingleContainer(operand), m_reqCard(card), m_allowsMany(operand->staticType()->cardinality().allowsMany()), m_errorCode(code)
{
   Q_ASSERT_X(m_reqCard != Cardinality::zeroOrMore(), Q_FUNC_INFO,
              "It makes no sense to use CardinalityVerifier for cardinality zero-or-more.");
}

Item::Iterator::Ptr CardinalityVerifier::evaluateSequence(const DynamicContext::Ptr &context) const
{
   const Item::Iterator::Ptr it(m_operand->evaluateSequence(context));
   const Item next(it->next());

   if (next) {
      const Item next2(it->next());

      if (next2) {
         if (m_reqCard.allowsMany()) {
            Item::List start;
            start.append(next);
            start.append(next2);

            return Item::Iterator::Ptr(new InsertionIterator(it, 1, makeListIterator(start)));
         } else {
            context->error(wrongCardinality(m_reqCard, Cardinality::twoOrMore()), m_errorCode, this);
            return CommonValues::emptyIterator;
         }
      } else {
         /* We might be instantiated for the empty sequence. */
         if (m_reqCard.isEmpty()) {
            context->error(wrongCardinality(m_reqCard, Cardinality::twoOrMore()), m_errorCode, this);
            return CommonValues::emptyIterator;
         } else {
            return makeSingletonIterator(next);
         }
      }
   } else {
      if (m_reqCard.allowsEmpty()) {
         return CommonValues::emptyIterator;
      } else {
         context->error(wrongCardinality(m_reqCard, Cardinality::twoOrMore()), m_errorCode, this);
         return CommonValues::emptyIterator;
      }
   }
}

Item CardinalityVerifier::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   if (m_allowsMany) {
      const Item::Iterator::Ptr it(m_operand->evaluateSequence(context));
      const Item item(it->next());

      if (item) {
         if (it->next()) {
            context->error(wrongCardinality(m_reqCard, Cardinality::twoOrMore()),
                           m_errorCode, this);
            return Item();
         } else {
            return item;
         }
      } else if (m_reqCard.allowsEmpty()) {
         return Item();
      } else {
         context->error(wrongCardinality(m_reqCard), m_errorCode, this);
         return Item();
      }
   } else {
      const Item item(m_operand->evaluateSingleton(context));

      if (item) {
         return item;
      } else if (m_reqCard.allowsEmpty()) {
         return Item();
      } else {
         context->error(wrongCardinality(m_reqCard), m_errorCode, this);
         return Item();
      }
   }
}

const SourceLocationReflection *CardinalityVerifier::actualReflection() const
{
   return m_operand->actualReflection();
}

Expression::Ptr CardinalityVerifier::compress(const StaticContext::Ptr &context)
{
   if (m_reqCard.isMatch(m_operand->staticType()->cardinality())) {
      return m_operand->compress(context);
   } else {
      return SingleContainer::compress(context);
   }
}

SequenceType::List CardinalityVerifier::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

SequenceType::Ptr CardinalityVerifier::staticType() const
{
   return makeGenericSequenceType(m_operand->staticType()->itemType(), m_reqCard);
}

ExpressionVisitorResult::Ptr CardinalityVerifier::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

Expression::ID CardinalityVerifier::id() const
{
   return IDCardinalityVerifier;
}

