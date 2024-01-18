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

#ifndef QCardinalityVerifier_P_H
#define QCardinalityVerifier_P_H

#include <qsinglecontainer_p.h>

namespace QPatternist {

class CardinalityVerifier : public SingleContainer
{
 public:
   CardinalityVerifier(const Expression::Ptr &operand, const Cardinality &card,
                       const ReportContext::ErrorCode code);

   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   Item evaluateSingleton(const DynamicContext::Ptr &) const override;

   SequenceType::List expectedOperandTypes() const override;
   SequenceType::Ptr staticType() const override;

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

   /**
    * If the static cardinality of the operand is within the required cardinality,
    * the operand is returned as is, since results will always be valid and hence
    * is not a CardinalityVerifier necessary.
    */
   Expression::Ptr compress(const StaticContext::Ptr &context) override;

   /**
    * A utility function for determining whether the static type of an Expression matches
    * a cardinality. More specifically, this function performs the cardinality verification
    * part of the Function Conversion Rules.
    *
    * @todo Mention the rewrite and when exactly an error is issued via @p context
    */
   static Expression::Ptr
   verifyCardinality(const Expression::Ptr &operand, const Cardinality &card,
                     const StaticContext::Ptr &context,
                     const ReportContext::ErrorCode code = ReportContext::XPTY0004);

   const SourceLocationReflection *actualReflection() const override;

   ID id() const override;

 private:
   /**
    * Centralizes a message string in order to increase consistency and
    * reduce work for translators.
    */
   static inline QString wrongCardinality(const Cardinality &req, const Cardinality &got = Cardinality::empty());

   const Cardinality m_reqCard;
   const bool m_allowsMany;
   const ReportContext::ErrorCode m_errorCode;
};

}

#endif
