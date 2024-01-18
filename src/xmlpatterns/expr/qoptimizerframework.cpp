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

#include "qoptimizerblocks_p.h"

using namespace QPatternist;

ExpressionIdentifier::~ExpressionIdentifier()
{
}

ExpressionCreator::~ExpressionCreator()
{
}

OptimizationPass::OptimizationPass(const ExpressionIdentifier::Ptr &startID,
                                   const ExpressionIdentifier::List &opIDs,
                                   const ExpressionMarker &sourceExpr,
                                   const ExpressionCreator::Ptr &resultCtor,
                                   const OperandsMatchMethod mMethod) : startIdentifier(startID),
   operandIdentifiers(opIDs),
   sourceExpression(sourceExpr),
   resultCreator(resultCtor),
   operandsMatchMethod(mMethod)
{
   Q_ASSERT_X(resultCtor || !sourceExpr.isEmpty(), Q_FUNC_INFO,
              "Either resultCreator or sourceExpression must be set, otherwise there's "
              "nothing to rewrite to.");
}
