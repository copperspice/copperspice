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

#ifndef QEvaluationCache_P_H
#define QEvaluationCache_P_H

#include <qcachingiterator_p.h>
#include <qcommonsequencetypes_p.h>
#include <qnodebuilder_p.h>
#include <qoperandsiterator_p.h>
#include <qsinglecontainer_p.h>
#include <qvariabledeclaration_p.h>

namespace QPatternist {

template<bool IsForGlobal>
class EvaluationCache : public SingleContainer
{
 public:
   EvaluationCache(const Expression::Ptr &operand, const VariableDeclaration *varDecl, const VariableSlotID slot);

   Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
   Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
   Expression::Ptr compress(const StaticContext::Ptr &context) override;

   SequenceType::Ptr staticType() const override;

   /**
    * The first operand must be exactly one @c xs:string.
    */
   SequenceType::List expectedOperandTypes() const override;

   ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
   Expression::Properties properties() const override;

   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;
   const SourceLocationReflection *actualReflection() const override;

   VariableSlotID slot() const {
      return m_varSlot;
   }

 private:
   static DynamicContext::Ptr topFocusContext(const DynamicContext::Ptr &context);
   const VariableDeclaration *m_declaration;
   bool m_declarationUsedByMany;
   /**
    * This variable must not be called m_slot. If it so, a compiler bug on
    * HP-UX-aCC-64 is triggered in the constructor initializor. See the
    * preprocessor output.
    *
    * Note that this is the cache slot, and is disjoint to any variable's
    * regular slot.
    */
   const VariableSlotID m_varSlot;
};

#include "qevaluationcache.cpp"
}

#endif
