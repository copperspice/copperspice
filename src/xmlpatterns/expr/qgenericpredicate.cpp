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

#include "qabstractfloat_p.h"
#include "qboolean_p.h"
#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qemptysequence_p.h"
#include "qfirstitempredicate_p.h"
#include "qgenericsequencetype_p.h"
#include "qitemmappingiterator_p.h"
#include "qliteral_p.h"
#include "qpatternistlocale_p.h"
#include "qtruthpredicate_p.h"

#include "qgenericpredicate_p.h"

using namespace QPatternist;

GenericPredicate::GenericPredicate(const Expression::Ptr &sourceExpression,
                                   const Expression::Ptr &predicate) : PairContainer(sourceExpression,
                                            predicate)
{
}

Expression::Ptr GenericPredicate::create(const Expression::Ptr &sourceExpression,
      const Expression::Ptr &predicateExpression,
      const StaticContext::Ptr &context,
      const QSourceLocation &location)
{
   Q_ASSERT(sourceExpression);
   Q_ASSERT(predicateExpression);
   Q_ASSERT(context);
   const ItemType::Ptr type(predicateExpression->staticType()->itemType());

   if (predicateExpression->is(IDIntegerValue) &&
         predicateExpression->as<Literal>()->item().as<Numeric>()->toInteger() == 1) {
      /* Handle [1] */
      return createFirstItem(sourceExpression);
   } else if (BuiltinTypes::numeric->xdtTypeMatches(type)) {
      /* A numeric predicate, other than [1]. */
      /* TODO at somepoint we'll return a specialized expr here, NumericPredicate or so.
       * Dependency analysis is a bit tricky, since the contained expression can depend on
       * some loop component. */
      return Expression::Ptr(new GenericPredicate(sourceExpression, predicateExpression));
   } else if (*CommonSequenceTypes::Empty == *type) {
      return EmptySequence::create(predicateExpression.data(), context);
   } else if (*BuiltinTypes::item == *type ||
              *BuiltinTypes::xsAnyAtomicType == *type) {
      /* The type couldn't be narrowed at compile time, so we use
       * a generic predicate. This check is before the CommonSequenceTypes::EBV check,
       * because the latter matches these types as well. */
      return Expression::Ptr(new GenericPredicate(sourceExpression, predicateExpression));
   } else if (CommonSequenceTypes::EBV->itemType()->xdtTypeMatches(type)) {
      return Expression::Ptr(new TruthPredicate(sourceExpression, predicateExpression));
   } else {
      context->error(QtXmlPatterns::tr("A value of type %1 cannot be a "
                                       "predicate. A predicate must have "
                                       "either a numeric type or an "
                                       "Effective Boolean Value type.")
                     .formatArg(formatType(context->namePool(),
                                     sourceExpression->staticType())),
                     ReportContext::FORG0006, location);
      return Expression::Ptr(); /* Silence compiler warning. */
   }
}

Expression::Ptr GenericPredicate::createFirstItem(const Expression::Ptr &sourceExpression)

{
   return Expression::Ptr(new FirstItemPredicate(sourceExpression));
}

Item GenericPredicate::mapToItem(const Item &item,
                                 const DynamicContext::Ptr &context) const
{
   const Item::Iterator::Ptr it(m_operand2->evaluateSequence(context));
   const Item pcateItem(it->next());

   if (!pcateItem) {
      return Item();   /* The predicate evaluated to the empty sequence */
   } else if (pcateItem.isNode()) {
      return item;
   }
   /* Ok, now it must be an AtomicValue */
   else if (BuiltinTypes::numeric->xdtTypeMatches(pcateItem.type())) {
      /* It's a positional predicate. */
      if (it->next()) {
         context->error(QtXmlPatterns::tr("A positional predicate must "
                                          "evaluate to a single numeric "
                                          "value."),
                        ReportContext::FORG0006, this);
         return Item();
      }

      if (Double::isEqual(static_cast<xsDouble>(context->contextPosition()),
                          pcateItem.as<Numeric>()->toDouble())) {
         return item;
      } else {
         return Item();
      }
   } else if (Boolean::evaluateEBV(pcateItem, it, context)) { /* It's a truth predicate. */
      return item;
   } else {
      return Item();
   }
}

Item::Iterator::Ptr GenericPredicate::evaluateSequence(const DynamicContext::Ptr &context) const
{
   const Item::Iterator::Ptr focus(m_operand1->evaluateSequence(context));
   const DynamicContext::Ptr newContext(context->createFocus());
   newContext->setFocusIterator(focus);

   return makeItemMappingIterator<Item>(ConstPtr(this),
                                        focus,
                                        newContext);
}

Item GenericPredicate::evaluateSingleton(const DynamicContext::Ptr &context) const
{
   const Item::Iterator::Ptr focus(m_operand1->evaluateSequence(context));
   const DynamicContext::Ptr newContext(context->createFocus());
   newContext->setFocusIterator(focus);
   return mapToItem(focus->next(), newContext);
}

SequenceType::List GenericPredicate::expectedOperandTypes() const
{
   SequenceType::List result;
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   result.append(CommonSequenceTypes::ZeroOrMoreItems);
   return result;
}

SequenceType::Ptr GenericPredicate::staticType() const
{
   const SequenceType::Ptr type(m_operand1->staticType());
   return makeGenericSequenceType(type->itemType(),
                                  type->cardinality() | Cardinality::zeroOrOne());
}

ExpressionVisitorResult::Ptr GenericPredicate::accept(const ExpressionVisitor::Ptr &visitor) const
{
   return visitor->visit(this);
}

ItemType::Ptr GenericPredicate::newFocusType() const
{
   return m_operand1->staticType()->itemType();
}

Expression::Properties GenericPredicate::properties() const
{
   return CreatesFocusForLast;
}

QString GenericPredicate::description() const
{
   return QLatin1String("predicate");
}

Expression::ID GenericPredicate::id() const
{
   return IDGenericPredicate;
}
