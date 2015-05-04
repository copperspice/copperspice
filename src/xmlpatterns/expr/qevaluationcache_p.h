/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QEvaluationCache_P_H
#define QEvaluationCache_P_H

#include <qcachingiterator_p.h>
#include <qcommonsequencetypes_p.h>
#include <qnodebuilder_p.h>
#include <qoperandsiterator_p.h>
#include <qsinglecontainer_p.h>
#include <qvariabledeclaration_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
template<bool IsForGlobal>
class EvaluationCache : public SingleContainer
{
 public:
   EvaluationCache(const Expression::Ptr &operand,
                   const VariableDeclaration *varDecl,
                   const VariableSlotID slot);

   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;
   virtual Expression::Ptr compress(const StaticContext::Ptr &context);

   virtual SequenceType::Ptr staticType() const;

   /**
    * The first operand must be exactly one @c xs:string.
    */
   virtual SequenceType::List expectedOperandTypes() const;

   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const;
   virtual Properties properties() const;
   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);
   virtual const SourceLocationReflection *actualReflection() const;

   inline VariableSlotID slot() const {
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
   const VariableSlotID            m_varSlot;
};

#include "qevaluationcache.cpp"
}

QT_END_NAMESPACE

#endif
