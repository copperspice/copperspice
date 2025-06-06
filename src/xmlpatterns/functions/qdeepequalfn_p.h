/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QDeepEqualFN_P_H
#define QDeepEqualFN_P_H

#include <qatomiccomparator_p.h>
#include <qcomparisonplatform_p.h>
#include <qfunctioncall_p.h>

namespace QPatternist {

class DeepEqualFN : public FunctionCall, public ComparisonPlatform<DeepEqualFN, false>
{
 public:
   DeepEqualFN()
      : ComparisonPlatform<DeepEqualFN, false>()
   { }

   bool evaluateEBV(const DynamicContext::Ptr &context) const override;
   Expression::Ptr typeCheck(const StaticContext::Ptr &context, const SequenceType::Ptr &reqType) override;

   AtomicComparator::Operator operatorID() const {
      return AtomicComparator::OperatorEqual;
   }
};

}

#endif
