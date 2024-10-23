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

#ifndef QExpression_P_H
#define QExpression_P_H

#include <QFlags>
#include <QSharedData>
#include <qcontainerfwd.h>

#include <qcppcastinghelper_p.h>
#include <qxmlpatterns_debug_p.h>
#include <qdynamiccontext_p.h>
#include <qexpressiondispatch_p.h>
#include <qitem_p.h>
#include <qsequencetype_p.h>
#include <qsourcelocationreflection_p.h>
#include <qstaticcontext_p.h>

namespace QPatternist {

template<typename T, typename ListType> class ListIterator;
class OptimizationPass;

class Expression : public QSharedData, public CppCastingHelper<Expression>, public SourceLocationReflection
{
 public:
   typedef QExplicitlySharedDataPointer<Expression> Ptr;

   typedef QExplicitlySharedDataPointer<const Expression> ConstPtr;

   typedef QList<Expression::Ptr> List;

   typedef QVector<Expression::Ptr> Vector;

   typedef QT_PREPEND_NAMESPACE(QAbstractXmlForwardIterator<Expression::Ptr>)
   QAbstractXmlForwardIterator;

   enum Property {
      UseContextItem              = 1,

      DisableElimination          = 1 << 1,

      IsEvaluated                 = 1 << 2,

      DisableTypingDeduction      = 1 << 3,
      EmptynessFollowsChild       = 1 << 4,

      RewriteToEmptyOnEmpty       = 1 << 5,
      RequiresFocus               = 1 << 6,
      AffectsOrderOnly            = 1 << 7,

      RequiresContextItem         = (1 << 8) | RequiresFocus,

      CreatesFocusForLast         = 1 << 9,

      LastOperandIsCollation      = 1 << 10,
      DependsOnLocalVariable      = (1 << 11) | DisableElimination,

      EvaluationCacheRedundant       = (1 << 12),

      IsNodeConstructor           = 1 << 13,

      RequiresCurrentItem         = 1 << 14
   };

   typedef QFlags<Property> Properties;

   enum ID {
      IDBooleanValue = 1,

      IDCountFN,

      IDEmptyFN,

      IDExistsFN,
      IDExpressionSequence,

      IDGeneralComparison,

      IDIfThenClause,

      IDIgnorableExpression,

      IDIntegerValue,

      IDPositionFN,
      IDStringValue,

      IDValueComparison,
      IDRangeVariableReference,
      IDContextItem,
      IDUserFunctionCallsite,
      IDExpressionVariableReference,
      IDAttributeConstructor,
      IDUpperCaseFN,
      IDLowerCaseFN,
      IDFirstItemPredicate,
      IDEmptySequence,
      IDReturnOrderBy,
      IDLetClause,
      IDForClause,
      IDPath,
      IDNamespaceConstructor,
      IDArgumentReference,
      IDGenericPredicate,
      IDAxisStep,

      IDFloat,
      IDCombineNodes,
      IDUnresolvedVariableReference,
      IDCardinalityVerifier
   };

   Expression() {
   }
   virtual ~Expression();

   virtual Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const;

   virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;

   virtual bool evaluateEBV(const DynamicContext::Ptr &context) const;

   virtual void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const;

   virtual Expression::List operands() const = 0;

   virtual void setOperands(const Expression::List &operands) = 0;

   virtual SequenceType::Ptr staticType() const = 0;

   virtual SequenceType::List expectedOperandTypes() const = 0;

   virtual Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType);

   virtual Expression::Ptr compress(const StaticContext::Ptr &context);

   virtual Properties properties() const;

   virtual Properties dependencies() const;

   Properties deepProperties() const;

   inline bool isEvaluated() const;

   inline bool is(const ID id) const;

   inline bool has(const Property prop) const;

   inline bool hasDependency(const Property prop) const;

   virtual ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const = 0;

   virtual ID id() const;

   virtual QList<QExplicitlySharedDataPointer<OptimizationPass> > optimizationPasses() const;

   virtual ItemType::Ptr expectedContextItemType() const;
   virtual ItemType::Ptr newFocusType() const;

   const SourceLocationReflection *actualReflection() const override;

   QString description() const override;

   virtual void announceFocusType(const ItemType::Ptr &itemType);

   static inline void rewrite(Expression::Ptr &old,
                              const Expression::Ptr &New,
                              const StaticContext::Ptr &context);

   inline const Expression::Ptr &rewrite(const Expression::Ptr &to,
                                         const StaticContext::Ptr &context) const;

   virtual PatternPriority patternPriority() const;

 protected:
   virtual bool compressOperands(const StaticContext::Ptr &) = 0;

   void typeCheckOperands(const StaticContext::Ptr &context);

 private:
   static Expression::Ptr invokeOptimizers(const Expression::Ptr &expr,
                                           const StaticContext::Ptr &context);
   inline StaticContext::Ptr finalizeStaticContext(const StaticContext::Ptr &context) const;

   Expression::Ptr constantPropagate(const StaticContext::Ptr &context) const;

   Expression(const Expression &) = delete;
   Expression &operator=(const Expression &) = delete;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Expression::Properties)

inline bool Expression::is(const Expression::ID i) const
{
   return id() == i;
}

inline bool Expression::isEvaluated() const
{
   return has(IsEvaluated);
}

inline bool Expression::has(const Expression::Property prop) const
{
   return properties().testFlag(prop);
}

inline bool Expression::hasDependency(const Expression::Property prop) const
{
   return dependencies().testFlag(prop);
}

inline void Expression::rewrite(Expression::Ptr &old,
                                const Expression::Ptr &New,
                                const StaticContext::Ptr &context)
{
   Q_ASSERT(old);
   Q_ASSERT(New);

   if (old != New) {
      pDebug() << "AST REWRITE:" << old.data() << "to" << New.data()
               << '(' << old->actualReflection() << "to" << New->actualReflection() << ", "
               << old->description() << "to" << New->description() << ')';

      // The order of these two lines is significant
      context->addLocation(New.data(), context->locationFor(old->actualReflection()));
      old = New;
   }
}

inline const Expression::Ptr &Expression::rewrite(const Expression::Ptr &to,
      const StaticContext::Ptr &context) const
{
   context->addLocation(to.data(), context->locationFor(this));
   return to;
}
}

#endif
