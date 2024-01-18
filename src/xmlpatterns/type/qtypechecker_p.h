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
      /**
       * @short When set, the function conversion rules are applied.
       *
       * For instance, this is type promotion and conversions from @c
       * xs:untypedAtomic to @c xs:date. This is done for function calls,
       * but not when binding an expression to a variable.
       */
      AutomaticallyConvert = 1,

      /**
       * @short Whether the focus should be checked or not.
       *
       * Sometimes the focus is unknown at the time
       * applyFunctionConversion() is called, and therefore it is
       * of interest to post pone the check to later on.
       */
      CheckFocus = 2,

      /**
       * When applyFunctionConversion() is passed AutomaticallyConvert
       * and promotion is required, such as from @c xs:integer to
       * @c xs:float, there will be no conversion performed, with the
       * assumption that the receiver will call Numeric::toFloat() or
       * similar.
       *
       * However, when GeneratePromotion is set, code will be generated
       * that performs this conversion regardless of what any receiver
       * do.
       *
       * This is useful in the case where one Expression only pipes the
       * result of another. The only known case of that as of this
       * writing is when UserFunctionCallsite evaluates its body.
       */
      GeneratePromotion
   };
   typedef QFlags<Option> Options;

   /**
    * @short Builds a pipeline of artificial AST nodes that ensures @p operand
    * conforms to the type @p reqType by applying the Function
    * Conversion Rules.
    *
    * This new Expression is returned, or, if no conversions were necessary,
    * @p operand as it is.
    *
    * applyFunctionConversion() also performs various checks, such as if
    * @p operand needs the focus and that the focus is defined in the
    * @p context. These checks are largely guided by @p operand's
    * Expression::properties().
    *
    * @see <a href="http://www.w3.org/TR/xpath20/\#id-function-calls">XML Path
    * Language (XPath) 2.0, 3.1.5 Function Calls</a>, more specifically the
    * Function Conversion Rules
    */
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
   /**
    * @short Implements the type checking and promotion part of the Function Conversion Rules.
    */
   static Expression::Ptr verifyType(const Expression::Ptr &operand,
                                     const SequenceType::Ptr &reqSeqType,
                                     const StaticContext::Ptr &context,
                                     const ReportContext::ErrorCode code,
                                     const Options options);

   /**
    * Determines whether type promotion is possible from one type to another. False
    * is returned when a promotion is not possible or if a promotion is not needed(as when
    * the types are identical), since that can be considered to not be type promotion.
    *
    * @returns @c true if @p fromType can be promoted to @p toType.
    * @see <a href="http://www.w3.org/TR/xpath20/#promotion">XML Path Language
    * (XPath) 2.0, B.1 Type Promotion</a>
    */
   static bool promotionPossible(const ItemType::Ptr &fromType,
                                 const ItemType::Ptr &toType,
                                 const StaticContext::Ptr &context);

   /**
    * @short Centralizes a message-string to reduce work for translators
    * and increase consistency.
    */
   static inline QString wrongType(const NamePool::Ptr &np,
                                   const ItemType::Ptr &reqType,
                                   const ItemType::Ptr &opType);

   /**
    * No implementation is provided for this constructor. This class
    * is not supposed to be instantiated.
    */
   inline TypeChecker();

   TypeChecker(const TypeChecker &) = delete;
   TypeChecker &operator=(const TypeChecker &) = delete;
};
}

#endif
