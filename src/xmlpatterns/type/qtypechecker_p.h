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

#ifndef QTypeChecker_P_H
#define QTypeChecker_P_H

#include <qstaticcontext_p.h>
#include <qexpression_p.h>

namespace QPatternist {
class TypeChecker
{
 public:
   enum Option {
      AutomaticallyConvert = 1,
      CheckFocus = 2,

      GeneratePromotion
   };
   typedef QFlags<Option> Options;

   static Expression::Ptr
   applyFunctionConversion(const Expression::Ptr &operand,
                           const SequenceType::Ptr &reqType,
                           const StaticContext::Ptr &context,
                           const ReportContext::ErrorCode code = ReportContext::XPTY0004,
                           const Options = Options(AutomaticallyConvert | CheckFocus));
 private:

   static inline Expression::Ptr typeCheck(Expression *const op,
                                           const StaticContext::Ptr &context,
                                           const SequenceType::Ptr &reqType);
   static Expression::Ptr verifyType(const Expression::Ptr &operand,
                                     const SequenceType::Ptr &reqSeqType,
                                     const StaticContext::Ptr &context,
                                     const ReportContext::ErrorCode code,
                                     const Options options);

   static bool promotionPossible(const ItemType::Ptr &fromType,
                                 const ItemType::Ptr &toType,
                                 const StaticContext::Ptr &context);

   static inline QString wrongType(const NamePool::Ptr &np,
                                   const ItemType::Ptr &reqType,
                                   const ItemType::Ptr &opType);

   inline TypeChecker();

   TypeChecker(const TypeChecker &) = delete;
   TypeChecker &operator=(const TypeChecker &) = delete;
};
}

#endif
