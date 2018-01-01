/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef RPPEXPRESSIONBUILDER_H
#define RPPEXPRESSIONBUILDER_H

#include "tokens.h"
#include "tokenengine.h"
#include "smallobject.h"
#include "rpp.h"
#include <QByteArray>

QT_BEGIN_NAMESPACE

namespace Rpp {

class ExpressionBuilder
{
public:
    ExpressionBuilder(const TokenEngine::TokenList &tokenList, const QVector<Type> &typeList, TypedPool<Item> *memoryPool);
    Rpp::Expression *parse();
private:

    inline bool hasNext() const { return (i < m_tokenList.count()); }
    Type next();
    bool test(int);
    bool moreTokens(int delta);
    inline void prev() {--i;}
    Type lookup(int k = 1);
    inline Type token() { return typeAt(i-1);}
    inline QByteArray lexem() { return m_tokenList.text(i-1);}
    inline Type typeAt(int t) { return m_typeList.at(m_tokenList.containerIndex(t));}

    Expression *conditional_expression();
    Expression *logical_OR_expression();
    Expression *logical_AND_expression();
    Expression *inclusive_OR_expression();
    Expression *exclusive_OR_expression();
    Expression *AND_expression();
    Expression *equality_expression();
    Expression *relational_expression();
    Expression *shift_expression();
    Expression *additive_expression();
    Expression *multiplicative_expression();
    Expression *unary_expression();
    Expression *primary_expression();

    bool unary_expression_lookup();
    bool primary_expression_lookup();

    UnaryExpression *createUnaryExpression(int op, Expression *expression);
    BinaryExpression *createBinaryExpression(int op, Expression *leftExpresson, Expression *rightExpression);
    ConditionalExpression *createConditionalExpression(Expression *condition, Expression *leftExpression, Expression *rightExpression);
    MacroReference *createMacroReference(MacroReference::Type type, TokenEngine::TokenList token);
    IntLiteral *createIntLiteral(const int arg);

    TokenEngine::TokenList createTokenList(int tokenIndex) const;

    int i;
    TokenEngine::TokenList m_tokenList;
    QVector<Type> m_typeList;
    TypedPool<Item> *m_memoryPool;
};

}

QT_END_NAMESPACE

#endif
