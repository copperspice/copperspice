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

#ifndef QDECLARATIVEREWRITE_P_H
#define QDECLARATIVEREWRITE_P_H

#include <textwriter_p.h>
#include <qdeclarativejslexer_p.h>
#include <qdeclarativejsparser_p.h>
#include <qdeclarativejsnodepool_p.h>

QT_BEGIN_NAMESPACE

namespace QDeclarativeRewrite {
using namespace QDeclarativeJS;

class SharedBindingTester : protected AST::Visitor
{
   bool _sharable;
 public:
   bool isSharable(const QString &code);
   bool isSharable(AST::Node *Node);

   virtual bool visit(AST::FunctionDeclaration *) {
      _sharable = false;
      return false;
   }
   virtual bool visit(AST::FunctionExpression *) {
      _sharable = false;
      return false;
   }
   virtual bool visit(AST::CallExpression *) {
      _sharable = false;
      return false;
   }
};

class RewriteBinding: protected AST::Visitor
{
   unsigned _position;
   TextWriter *_writer;
   QByteArray _name;

 public:
   QString operator()(const QString &code, bool *ok = 0, bool *sharable = 0);
   QString operator()(QDeclarativeJS::AST::Node *node, const QString &code, bool *sharable = 0);

   //name of the function:  used for the debugger
   void setName(const QByteArray &name) {
      _name = name;
   }

 protected:
   using AST::Visitor::visit;

   void accept(AST::Node *node);
   QString rewrite(QString code, unsigned position, AST::Statement *node);

   virtual bool visit(AST::Block *ast);
   virtual bool visit(AST::ExpressionStatement *ast);

   virtual bool visit(AST::DoWhileStatement *ast);
   virtual void endVisit(AST::DoWhileStatement *ast);

   virtual bool visit(AST::WhileStatement *ast);
   virtual void endVisit(AST::WhileStatement *ast);

   virtual bool visit(AST::ForStatement *ast);
   virtual void endVisit(AST::ForStatement *ast);

   virtual bool visit(AST::LocalForStatement *ast);
   virtual void endVisit(AST::LocalForStatement *ast);

   virtual bool visit(AST::ForEachStatement *ast);
   virtual void endVisit(AST::ForEachStatement *ast);

   virtual bool visit(AST::LocalForEachStatement *ast);
   virtual void endVisit(AST::LocalForEachStatement *ast);

 private:
   int _inLoop;
};

} // namespace QDeclarativeRewrite

QT_END_NAMESPACE

#endif // QDECLARATIVEREWRITE_P_H

