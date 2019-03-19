/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QSCRIPTASTVISITOR_P_H
#define QSCRIPTASTVISITOR_P_H

#include "qscriptastfwd_p.h"

QT_BEGIN_NAMESPACE

namespace QScript {
namespace AST {

class Visitor
{
 public:
   Visitor();
   virtual ~Visitor();

   virtual bool preVisit(Node *) {
      return true;
   }
   virtual void postVisit(Node *) {}

   virtual bool visit(ThisExpression *) {
      return true;
   }
   virtual void endVisit(ThisExpression *) {}

   virtual bool visit(IdentifierExpression *) {
      return true;
   }
   virtual void endVisit(IdentifierExpression *) {}

   virtual bool visit(NullExpression *) {
      return true;
   }
   virtual void endVisit(NullExpression *) {}

   virtual bool visit(TrueLiteral *) {
      return true;
   }
   virtual void endVisit(TrueLiteral *) {}

   virtual bool visit(FalseLiteral *) {
      return true;
   }
   virtual void endVisit(FalseLiteral *) {}

   virtual bool visit(StringLiteral *) {
      return true;
   }
   virtual void endVisit(StringLiteral *) {}

   virtual bool visit(NumericLiteral *) {
      return true;
   }
   virtual void endVisit(NumericLiteral *) {}

   virtual bool visit(RegExpLiteral *) {
      return true;
   }
   virtual void endVisit(RegExpLiteral *) {}

   virtual bool visit(ArrayLiteral *) {
      return true;
   }
   virtual void endVisit(ArrayLiteral *) {}

   virtual bool visit(ObjectLiteral *) {
      return true;
   }
   virtual void endVisit(ObjectLiteral *) {}

   virtual bool visit(ElementList *) {
      return true;
   }
   virtual void endVisit(ElementList *) {}

   virtual bool visit(Elision *) {
      return true;
   }
   virtual void endVisit(Elision *) {}

   virtual bool visit(PropertyNameAndValueList *) {
      return true;
   }
   virtual void endVisit(PropertyNameAndValueList *) {}

   virtual bool visit(IdentifierPropertyName *) {
      return true;
   }
   virtual void endVisit(IdentifierPropertyName *) {}

   virtual bool visit(StringLiteralPropertyName *) {
      return true;
   }
   virtual void endVisit(StringLiteralPropertyName *) {}

   virtual bool visit(NumericLiteralPropertyName *) {
      return true;
   }
   virtual void endVisit(NumericLiteralPropertyName *) {}

   virtual bool visit(ArrayMemberExpression *) {
      return true;
   }
   virtual void endVisit(ArrayMemberExpression *) {}

   virtual bool visit(FieldMemberExpression *) {
      return true;
   }
   virtual void endVisit(FieldMemberExpression *) {}

   virtual bool visit(NewMemberExpression *) {
      return true;
   }
   virtual void endVisit(NewMemberExpression *) {}

   virtual bool visit(NewExpression *) {
      return true;
   }
   virtual void endVisit(NewExpression *) {}

   virtual bool visit(CallExpression *) {
      return true;
   }
   virtual void endVisit(CallExpression *) {}

   virtual bool visit(ArgumentList *) {
      return true;
   }
   virtual void endVisit(ArgumentList *) {}

   virtual bool visit(PostIncrementExpression *) {
      return true;
   }
   virtual void endVisit(PostIncrementExpression *) {}

   virtual bool visit(PostDecrementExpression *) {
      return true;
   }
   virtual void endVisit(PostDecrementExpression *) {}

   virtual bool visit(DeleteExpression *) {
      return true;
   }
   virtual void endVisit(DeleteExpression *) {}

   virtual bool visit(VoidExpression *) {
      return true;
   }
   virtual void endVisit(VoidExpression *) {}

   virtual bool visit(TypeOfExpression *) {
      return true;
   }
   virtual void endVisit(TypeOfExpression *) {}

   virtual bool visit(PreIncrementExpression *) {
      return true;
   }
   virtual void endVisit(PreIncrementExpression *) {}

   virtual bool visit(PreDecrementExpression *) {
      return true;
   }
   virtual void endVisit(PreDecrementExpression *) {}

   virtual bool visit(UnaryPlusExpression *) {
      return true;
   }
   virtual void endVisit(UnaryPlusExpression *) {}

   virtual bool visit(UnaryMinusExpression *) {
      return true;
   }
   virtual void endVisit(UnaryMinusExpression *) {}

   virtual bool visit(TildeExpression *) {
      return true;
   }
   virtual void endVisit(TildeExpression *) {}

   virtual bool visit(NotExpression *) {
      return true;
   }
   virtual void endVisit(NotExpression *) {}

   virtual bool visit(BinaryExpression *) {
      return true;
   }
   virtual void endVisit(BinaryExpression *) {}

   virtual bool visit(ConditionalExpression *) {
      return true;
   }
   virtual void endVisit(ConditionalExpression *) {}

   virtual bool visit(Expression *) {
      return true;
   }
   virtual void endVisit(Expression *) {}

   virtual bool visit(Block *) {
      return true;
   }
   virtual void endVisit(Block *) {}

   virtual bool visit(StatementList *) {
      return true;
   }
   virtual void endVisit(StatementList *) {}

   virtual bool visit(VariableStatement *) {
      return true;
   }
   virtual void endVisit(VariableStatement *) {}

   virtual bool visit(VariableDeclarationList *) {
      return true;
   }
   virtual void endVisit(VariableDeclarationList *) {}

   virtual bool visit(VariableDeclaration *) {
      return true;
   }
   virtual void endVisit(VariableDeclaration *) {}

   virtual bool visit(EmptyStatement *) {
      return true;
   }
   virtual void endVisit(EmptyStatement *) {}

   virtual bool visit(ExpressionStatement *) {
      return true;
   }
   virtual void endVisit(ExpressionStatement *) {}

   virtual bool visit(IfStatement *) {
      return true;
   }
   virtual void endVisit(IfStatement *) {}

   virtual bool visit(DoWhileStatement *) {
      return true;
   }
   virtual void endVisit(DoWhileStatement *) {}

   virtual bool visit(WhileStatement *) {
      return true;
   }
   virtual void endVisit(WhileStatement *) {}

   virtual bool visit(ForStatement *) {
      return true;
   }
   virtual void endVisit(ForStatement *) {}

   virtual bool visit(LocalForStatement *) {
      return true;
   }
   virtual void endVisit(LocalForStatement *) {}

   virtual bool visit(ForEachStatement *) {
      return true;
   }
   virtual void endVisit(ForEachStatement *) {}

   virtual bool visit(LocalForEachStatement *) {
      return true;
   }
   virtual void endVisit(LocalForEachStatement *) {}

   virtual bool visit(ContinueStatement *) {
      return true;
   }
   virtual void endVisit(ContinueStatement *) {}

   virtual bool visit(BreakStatement *) {
      return true;
   }
   virtual void endVisit(BreakStatement *) {}

   virtual bool visit(ReturnStatement *) {
      return true;
   }
   virtual void endVisit(ReturnStatement *) {}

   virtual bool visit(WithStatement *) {
      return true;
   }
   virtual void endVisit(WithStatement *) {}

   virtual bool visit(SwitchStatement *) {
      return true;
   }
   virtual void endVisit(SwitchStatement *) {}

   virtual bool visit(CaseBlock *) {
      return true;
   }
   virtual void endVisit(CaseBlock *) {}

   virtual bool visit(CaseClauses *) {
      return true;
   }
   virtual void endVisit(CaseClauses *) {}

   virtual bool visit(CaseClause *) {
      return true;
   }
   virtual void endVisit(CaseClause *) {}

   virtual bool visit(DefaultClause *) {
      return true;
   }
   virtual void endVisit(DefaultClause *) {}

   virtual bool visit(LabelledStatement *) {
      return true;
   }
   virtual void endVisit(LabelledStatement *) {}

   virtual bool visit(ThrowStatement *) {
      return true;
   }
   virtual void endVisit(ThrowStatement *) {}

   virtual bool visit(TryStatement *) {
      return true;
   }
   virtual void endVisit(TryStatement *) {}

   virtual bool visit(Catch *) {
      return true;
   }
   virtual void endVisit(Catch *) {}

   virtual bool visit(Finally *) {
      return true;
   }
   virtual void endVisit(Finally *) {}

   virtual bool visit(FunctionDeclaration *) {
      return true;
   }
   virtual void endVisit(FunctionDeclaration *) {}

   virtual bool visit(FunctionExpression *) {
      return true;
   }
   virtual void endVisit(FunctionExpression *) {}

   virtual bool visit(FormalParameterList *) {
      return true;
   }
   virtual void endVisit(FormalParameterList *) {}

   virtual bool visit(FunctionBody *) {
      return true;
   }
   virtual void endVisit(FunctionBody *) {}

   virtual bool visit(Program *) {
      return true;
   }
   virtual void endVisit(Program *) {}

   virtual bool visit(SourceElements *) {
      return true;
   }
   virtual void endVisit(SourceElements *) {}

   virtual bool visit(FunctionSourceElement *) {
      return true;
   }
   virtual void endVisit(FunctionSourceElement *) {}

   virtual bool visit(StatementSourceElement *) {
      return true;
   }
   virtual void endVisit(StatementSourceElement *) {}

   virtual bool visit(DebuggerStatement *) {
      return true;
   }
   virtual void endVisit(DebuggerStatement *) {}
};

}
} // namespace AST

QT_END_NAMESPACE

#endif // QSCRIPTASTVISITOR_P_H
