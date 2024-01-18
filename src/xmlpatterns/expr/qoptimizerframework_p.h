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

#ifndef QOptimizerFramework_P_H
#define QOptimizerFramework_P_H

#include <QSharedData>
#include <qexpression_p.h>

namespace QPatternist {

class ExpressionCreator : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<ExpressionCreator> Ptr;

   /**
    * For some reason this constructor cannot be synthesized.
    */
   inline ExpressionCreator() {
   }

   virtual ~ExpressionCreator();

   virtual Expression::Ptr create(const Expression::List &operands,
                                  const StaticContext::Ptr &context,
                                  const SourceLocationReflection *const) const = 0;

 private:
   ExpressionCreator(const ExpressionCreator &) = delete;
   ExpressionCreator &operator=(const ExpressionCreator &) = delete;
};

class ExpressionIdentifier : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<ExpressionIdentifier> Ptr;
   typedef QList<ExpressionIdentifier::Ptr> List;

   /**
    * For some reason this constructor cannot be synthesized.
    */
   inline ExpressionIdentifier() {
   }

   virtual ~ExpressionIdentifier();
   virtual bool matches(const Expression::Ptr &expr) const = 0;

 private:
   ExpressionIdentifier(const ExpressionIdentifier &) = delete;
   ExpressionIdentifier &operator=(const ExpressionIdentifier &) = delete;
};

class OptimizationPass : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<OptimizationPass> Ptr;
   typedef QList<OptimizationPass::Ptr> List;

   enum OperandsMatchMethod {
      Sequential = 1,
      AnyOrder
   };


   typedef QList<qint8> ExpressionMarker;

   OptimizationPass(const ExpressionIdentifier::Ptr &startID,
                    const ExpressionIdentifier::List &operandIDs,
                    const ExpressionMarker &sourceExpr,
                    const ExpressionCreator::Ptr &resultCtor = ExpressionCreator::Ptr(),
                    const OperandsMatchMethod matchMethod = Sequential);

   const ExpressionIdentifier::Ptr startIdentifier;
   const ExpressionIdentifier::List operandIdentifiers;
   const ExpressionMarker sourceExpression;

   const ExpressionCreator::Ptr resultCreator;
   const OperandsMatchMethod operandsMatchMethod;

 private:
   OptimizationPass(const OptimizationPass &) = delete;
   OptimizationPass &operator=(const OptimizationPass &) = delete;
};

}

#endif
