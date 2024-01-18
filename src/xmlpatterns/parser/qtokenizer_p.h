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

#ifndef QTokenizer_P_H
#define QTokenizer_P_H

#include <qpair.h>
#include <qshareddata.h>
#include <qstring.h>
#include <qurl.h>

#include <qparsercontext_p.h>
#include <qtokensource_p.h>

namespace QPatternist {

typedef QPair<QString, Expression::Ptr> AttributeHolder;
typedef QVector<AttributeHolder> AttributeHolderVector;

class OrderSpecTransfer
{
 public:
   typedef QList<OrderSpecTransfer> List;
   inline OrderSpecTransfer() {
   }

   inline OrderSpecTransfer(const Expression::Ptr &aExpr,
                            const OrderBy::OrderSpec aOrderSpec) : expression(aExpr),
      orderSpec(aOrderSpec) {
      Q_ASSERT(expression);
   }

   Expression::Ptr     expression;
   OrderBy::OrderSpec  orderSpec;
};

/**
 * @short The value the parser, but not the tokenizers, uses for tokens and
 * non-terminals.
 *
 * It is inefficient but ensures nothing leaks, by invoking C++
 * destructors even in the cases the code throws exceptions. This might be
 * able to be done in a more efficient way -- suggestions are welcome.
 */
class TokenValue
{
 public:
   QString                         sval;

   Expression::Ptr                 expr;
   Expression::List                expressionList;

   Cardinality                     cardinality;
   ItemType::Ptr                   itemType;
   SequenceType::Ptr               sequenceType;
   FunctionArgument::List          functionArguments;
   FunctionArgument::Ptr           functionArgument;
   QVector<QXmlName>               qNameVector;
   QXmlName                        qName;
   /**
    * Holds enum values.
    */
   EnumUnion                       enums;

   AttributeHolder                 attributeHolder;
   AttributeHolderVector           attributeHolders;
   OrderSpecTransfer::List         orderSpecs;
   OrderSpecTransfer               orderSpec;
};
}

/**
 * Macro for the data type of semantic values; int by default.
 * See section Data Types of Semantic Values.
 */
#define YYSTYPE QPatternist::TokenValue

#include "qquerytransformparser_p.h" /* This inclusion must be after TokenValue. */

namespace QPatternist {

class Tokenizer : public TokenSource
{
 public:
   inline Tokenizer(const QUrl &queryU) : m_queryURI(queryU) {
      Q_ASSERT(queryU.isValid());
   }

   typedef QExplicitlySharedDataPointer<Tokenizer> Ptr;

   /**
    * Switches the Tokenizer to only do scanning, and returns complete
    * strings for attribute value templates as opposed to the tokens for
    * the contained expressions.
    *
    * The current position in the stream is returned. It can be used to
    * later resume regular tokenization.
    */
   virtual int commenceScanOnly() = 0;

   /**
    * Resumes regular parsing from @p position. The tokenizer must be in
    * the scan-only state, which the commenceScanOnly() call transists to.
    *
    * The tokenizer will return the token POSITION_SET once after this
    * function has been called.
    */
   virtual void resumeTokenizationFrom(const int position) = 0;

   /**
    * @returns the URI of the resource being tokenized.
    */
   inline const QUrl &queryURI() const {
      return m_queryURI;
   }

   virtual void setParserContext(const ParserContext::Ptr &parseInfo) = 0;

 protected:
   /**
    * Returns a string representation of @p token.
    *
    * This function is used for debugging purposes. The implementation of
    * this function is in querytransformparser.ypp.
    */
   static QString tokenToString(const Token &token);

 private:
   Tokenizer(const Tokenizer &) = delete;
   Tokenizer &operator=(const Tokenizer &) = delete;

   const QUrl m_queryURI;
};

}

#undef Patternist_DEBUG_PARSER // disable it for now

#endif
