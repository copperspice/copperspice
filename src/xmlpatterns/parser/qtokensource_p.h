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

#ifndef QTokenSource_P_H
#define QTokenSource_P_H

#include <qatomiccomparator_p.h>
#include <qatomicmathematician_p.h>
#include <qcombinenodes_p.h>
#include <qfunctionargument_p.h>
#include <qitem_p.h>
#include <qitemtype_p.h>
#include <qorderby_p.h>
#include <qpath_p.h>
#include <qquerytransformparser_p.h>
#include <qvalidate_p.h>

template<typename T> class QQueue;

namespace QPatternist {

union EnumUnion {
   AtomicComparator::Operator              valueOperator;
   AtomicMathematician::Operator           mathOperator;
   CombineNodes::Operator                  combinedNodeOp;
   QXmlNodeModelIndex::Axis                axis;
   QXmlNodeModelIndex::DocumentOrder       nodeOperator;
   StaticContext::BoundarySpacePolicy      boundarySpacePolicy;
   StaticContext::ConstructionMode         constructionMode;
   StaticContext::OrderingEmptySequence    orderingEmptySequence;
   StaticContext::OrderingMode             orderingMode;
   OrderBy::OrderSpec::Direction           sortDirection;
   Validate::Mode                          validationMode;
   VariableSlotID                          slot;
   int                                     tokenizerPosition;
   qint16                                  zeroer;
   bool                                    Bool;
   xsDouble                                Double;
   Path::Kind                              pathKind;
};

class TokenSource : public QSharedData
{
 public:
   /**
    * typedef for the enum Bison generates that contains the token symbols.
    */
   typedef yytokentype TokenType;

   /**
    * Represents a token by carrying its name and value.
    */
   class Token
   {
    public:
      /**
       * Constructs an invalid Token. This default constructor is need in the container classes.
       */
      inline Token() {}
      inline Token(const TokenType t) : type(t) {}
      inline Token(const TokenType t, const QString &val) : type(t), value(val) {}

      bool hasError() const {
         return type == ERROR;
      }

      TokenType type;
      QString value;
   };

   typedef QExplicitlySharedDataPointer<TokenSource> Ptr;
   typedef QQueue<Ptr> Queue;

   TokenSource() = default;

   virtual ~TokenSource();

   virtual Token nextToken(YYLTYPE *const sourceLocator) = 0;

 private:
   TokenSource(const TokenSource &) = delete;
   TokenSource &operator=(const TokenSource &) = delete;
};
}

#endif
