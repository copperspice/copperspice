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

#include "private/qdeclarativerewrite_p.h"

#include "private/qdeclarativeglobal_p.h"

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

DEFINE_BOOL_CONFIG_OPTION(rewriteDump, QML_REWRITE_DUMP);

namespace QDeclarativeRewrite {

bool SharedBindingTester::isSharable(const QString &code)
{
   Engine engine;
   NodePool pool(QString(), &engine);
   Lexer lexer(&engine);
   Parser parser(&engine);
   lexer.setCode(code, 0);
   parser.parseStatement();
   if (!parser.statement()) {
      return false;
   }

   return isSharable(parser.statement());
}

bool SharedBindingTester::isSharable(AST::Node *node)
{
   _sharable = true;
   AST::Node::acceptChild(node, this);
   return _sharable;
}

QString RewriteBinding::operator()(const QString &code, bool *ok, bool *sharable)
{
   Engine engine;
   NodePool pool(QString(), &engine);
   Lexer lexer(&engine);
   Parser parser(&engine);
   lexer.setCode(code, 0);
   parser.parseStatement();
   if (!parser.statement()) {
      if (ok) {
         *ok = false;
      }
      return QString();
   } else {
      if (ok) {
         *ok = true;
      }
      if (sharable) {
         SharedBindingTester tester;
         *sharable = tester.isSharable(parser.statement());
      }
   }
   return rewrite(code, 0, parser.statement());
}

QString RewriteBinding::operator()(QDeclarativeJS::AST::Node *node, const QString &code, bool *sharable)
{
   if (!node) {
      return code;
   }

   if (sharable) {
      SharedBindingTester tester;
      *sharable = tester.isSharable(node);
   }

   QDeclarativeJS::AST::ExpressionNode *expression = node->expressionCast();
   QDeclarativeJS::AST::Statement *statement = node->statementCast();
   if (!expression && !statement) {
      return code;
   }

   TextWriter w;
   _writer = &w;
   _position = expression ? expression->firstSourceLocation().begin() : statement->firstSourceLocation().begin();
   _inLoop = 0;

   accept(node);

   unsigned startOfStatement = 0;
   unsigned endOfStatement = (expression ? expression->lastSourceLocation().end() : statement->lastSourceLocation().end())
                             - _position;

   QString startString = QLatin1String("(function ") + QString::fromUtf8(_name) + QLatin1String("() { ");
   if (expression) {
      startString += QLatin1String("return ");
   }
   _writer->replace(startOfStatement, 0, startString);
   _writer->replace(endOfStatement, 0, QLatin1String(" })"));

   if (rewriteDump()) {
      qWarning() << "=============================================================";
      qWarning() << "Rewrote:";
      qWarning() << qPrintable(code);
   }

   QString codeCopy = code;
   w.write(&codeCopy);

   if (rewriteDump()) {
      qWarning() << "To:";
      qWarning() << qPrintable(code);
      qWarning() << "=============================================================";
   }

   return codeCopy;
}

void RewriteBinding::accept(AST::Node *node)
{
   AST::Node::acceptChild(node, this);
}

QString RewriteBinding::rewrite(QString code, unsigned position,
                                AST::Statement *node)
{
   TextWriter w;
   _writer = &w;
   _position = position;
   _inLoop = 0;

   accept(node);

   unsigned startOfStatement = node->firstSourceLocation().begin() - _position;
   unsigned endOfStatement = node->lastSourceLocation().end() - _position;

   _writer->replace(startOfStatement, 0, QLatin1String("(function ") + QString::fromUtf8(_name) + QLatin1String("() { "));
   _writer->replace(endOfStatement, 0, QLatin1String(" })"));

   if (rewriteDump()) {
      qWarning() << "=============================================================";
      qWarning() << "Rewrote:";
      qWarning() << qPrintable(code);
   }

   w.write(&code);

   if (rewriteDump()) {
      qWarning() << "To:";
      qWarning() << qPrintable(code);
      qWarning() << "=============================================================";
   }

   return code;
}

bool RewriteBinding::visit(AST::Block *ast)
{
   for (AST::StatementList *it = ast->statements; it; it = it->next) {
      if (! it->next) {
         // we need to rewrite only the last statement of a block.
         accept(it->statement);
      }
   }

   return false;
}

bool RewriteBinding::visit(AST::ExpressionStatement *ast)
{
   if (! _inLoop) {
      unsigned startOfExpressionStatement = ast->firstSourceLocation().begin() - _position;
      _writer->replace(startOfExpressionStatement, 0, QLatin1String("return "));
   }

   return false;
}

bool RewriteBinding::visit(AST::DoWhileStatement *)
{
   ++_inLoop;
   return true;
}

void RewriteBinding::endVisit(AST::DoWhileStatement *)
{
   --_inLoop;
}

bool RewriteBinding::visit(AST::WhileStatement *)
{
   ++_inLoop;
   return true;
}

void RewriteBinding::endVisit(AST::WhileStatement *)
{
   --_inLoop;
}

bool RewriteBinding::visit(AST::ForStatement *)
{
   ++_inLoop;
   return true;
}

void RewriteBinding::endVisit(AST::ForStatement *)
{
   --_inLoop;
}

bool RewriteBinding::visit(AST::LocalForStatement *)
{
   ++_inLoop;
   return true;
}

void RewriteBinding::endVisit(AST::LocalForStatement *)
{
   --_inLoop;
}

bool RewriteBinding::visit(AST::ForEachStatement *)
{
   ++_inLoop;
   return true;
}

void RewriteBinding::endVisit(AST::ForEachStatement *)
{
   --_inLoop;
}

bool RewriteBinding::visit(AST::LocalForEachStatement *)
{
   ++_inLoop;
   return true;
}

void RewriteBinding::endVisit(AST::LocalForEachStatement *)
{
   --_inLoop;
}

} // namespace QDeclarativeRewrite

QT_END_NAMESPACE
