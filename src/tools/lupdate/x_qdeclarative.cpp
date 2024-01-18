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

#include "lupdate.h"

#include <translator.h>

#include "parser/qdeclarativejsengine_p.h"
#include "parser/qdeclarativejsparser_p.h"
#include "parser/qdeclarativejslexer_p.h"
#include "parser/qdeclarativejsnodepool_p.h"
#include "parser/qdeclarativejsastvisitor_p.h"
#include "parser/qdeclarativejsast_p.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QString>
#include <QStringList>

#include <iostream>
#include <cstdlib>
#include <cctype>

class LU
{
   Q_DECLARE_TR_FUNCTIONS(LUpdate)
};

using namespace QDeclarativeJS;

class Comment
{
 public:
   Comment() : lastLine(-1) {}
   QString extracomment;
   QString msgid;
   TranslatorMessage::ExtraData extra;
   QString sourcetext;
   int lastLine;

   bool isValid() const {
      return !extracomment.isEmpty() || !msgid.isEmpty() || !sourcetext.isEmpty() || !extra.isEmpty();
   }
};

class FindTrCalls: protected AST::Visitor
{
 public:
   void operator()(Translator *translator, const QString &fileName, AST::Node *node) {
      m_translator = translator;
      m_fileName = fileName;
      m_component = QFileInfo(fileName).baseName();   //matches qsTr usage in QScriptEngine
      accept(node);
   }

   QList<Comment> comments;

 protected:
   using AST::Visitor::visit;
   using AST::Visitor::endVisit;

   void accept(AST::Node *node) {
      AST::Node::acceptChild(node, this);
   }

   virtual void endVisit(AST::CallExpression *node) {
      m_bSource.clear();
      if (AST::IdentifierExpression *idExpr = AST::cast<AST::IdentifierExpression *>(node->base)) {
         if (idExpr->name->asString() == QLatin1String("qsTr") ||
               idExpr->name->asString() == QLatin1String("QT_TR_NOOP")) {
            if (!node->arguments) {
               return;
            }
            AST::BinaryExpression *binary = AST::cast<AST::BinaryExpression *>(node->arguments->expression);
            if (binary) {
               if (!createString(binary)) {
                  m_bSource.clear();
               }
            }
            AST::StringLiteral *literal = AST::cast<AST::StringLiteral *>(node->arguments->expression);
            if (literal || !m_bSource.isEmpty()) {
               const QString source = literal ? literal->value->asString() : m_bSource;

               QString comment;
               bool plural = false;
               AST::ArgumentList *commentNode = node->arguments->next;
               if (commentNode && AST::cast<AST::StringLiteral *>(commentNode->expression)) {
                  literal = AST::cast<AST::StringLiteral *>(commentNode->expression);
                  comment = literal->value->asString();

                  AST::ArgumentList *nNode = commentNode->next;
                  if (nNode) {
                     plural = true;
                  }
               }

               QString id;
               QString extracomment;
               TranslatorMessage::ExtraData extra;
               Comment scomment = findComment(node->firstSourceLocation().startLine);
               if (scomment.isValid()) {
                  extracomment = scomment.extracomment;
                  extra = scomment.extra;
                  id = scomment.msgid;
               }

               TranslatorMessage msg(m_component, source,
                                     comment, QString(), m_fileName,
                                     node->firstSourceLocation().startLine, QStringList(),
                                     TranslatorMessage::Unfinished, plural);
               msg.setExtraComment(extracomment.simplified());
               msg.setId(id);
               msg.setExtras(extra);
               m_translator->extend(msg);
            }
         } else if (idExpr->name->asString() == QLatin1String("qsTranslate") ||
                    idExpr->name->asString() == QLatin1String("QT_TRANSLATE_NOOP")) {
            if (node->arguments && AST::cast<AST::StringLiteral *>(node->arguments->expression)) {
               AST::StringLiteral *literal = AST::cast<AST::StringLiteral *>(node->arguments->expression);
               const QString context = literal->value->asString();

               QString source;
               QString comment;
               bool plural = false;
               AST::ArgumentList *sourceNode = node->arguments->next;
               if (!sourceNode) {
                  return;
               }
               literal = AST::cast<AST::StringLiteral *>(sourceNode->expression);
               AST::BinaryExpression *binary = AST::cast<AST::BinaryExpression *>(sourceNode->expression);
               if (binary) {
                  if (!createString(binary)) {
                     m_bSource.clear();
                  }
               }
               if (!literal && m_bSource.isEmpty()) {
                  return;
               }

               QString id;
               QString extracomment;
               TranslatorMessage::ExtraData extra;
               Comment scomment = findComment(node->firstSourceLocation().startLine);
               if (scomment.isValid()) {
                  extracomment = scomment.extracomment;
                  extra = scomment.extra;
                  id = scomment.msgid;
               }

               source = literal ? literal->value->asString() : m_bSource;
               AST::ArgumentList *commentNode = sourceNode->next;
               if (commentNode && AST::cast<AST::StringLiteral *>(commentNode->expression)) {
                  literal = AST::cast<AST::StringLiteral *>(commentNode->expression);
                  comment = literal->value->asString();

                  AST::ArgumentList *nNode = commentNode->next;
                  if (nNode) {
                     plural = true;
                  }
               }

               TranslatorMessage msg(context, source,
                                     comment, QString(), m_fileName,
                                     node->firstSourceLocation().startLine, QStringList(),
                                     TranslatorMessage::Unfinished, plural);
               msg.setExtraComment(extracomment.simplified());
               msg.setId(id);
               msg.setExtras(extra);
               m_translator->extend(msg);
            }
         } else if (idExpr->name->asString() == QLatin1String("qsTrId") ||
                    idExpr->name->asString() == QLatin1String("QT_TRID_NOOP")) {
            if (!node->arguments) {
               return;
            }

            AST::StringLiteral *literal = AST::cast<AST::StringLiteral *>(node->arguments->expression);
            if (literal) {

               QString extracomment;
               QString sourcetext;
               TranslatorMessage::ExtraData extra;
               Comment comment = findComment(node->firstSourceLocation().startLine);
               if (comment.isValid()) {
                  extracomment = comment.extracomment;
                  sourcetext = comment.sourcetext;
                  extra = comment.extra;
               }

               const QString id = literal->value->asString();
               bool plural = node->arguments->next;

               TranslatorMessage msg(QString(), sourcetext,
                                     QString(), QString(), m_fileName,
                                     node->firstSourceLocation().startLine, QStringList(),
                                     TranslatorMessage::Unfinished, plural);
               msg.setExtraComment(extracomment.simplified());
               msg.setId(id);
               msg.setExtras(extra);
               m_translator->extend(msg);
            }
         }
      }
   }

 private:
   bool createString(AST::BinaryExpression *b) {
      if (!b || b->op != 0) {
         return false;
      }
      AST::BinaryExpression *l = AST::cast<AST::BinaryExpression *>(b->left);
      AST::BinaryExpression *r = AST::cast<AST::BinaryExpression *>(b->right);
      AST::StringLiteral *ls = AST::cast<AST::StringLiteral *>(b->left);
      AST::StringLiteral *rs = AST::cast<AST::StringLiteral *>(b->right);
      if ((!l && !ls) || (!r && !rs)) {
         return false;
      }
      if (l) {
         if (!createString(l)) {
            return false;
         }
      } else {
         m_bSource.prepend(ls->value->asString());
      }

      if (r) {
         if (!createString(r)) {
            return false;
         }
      } else {
         m_bSource.append(rs->value->asString());
      }

      return true;
   }

   Comment findComment(int loc) {
      if (comments.isEmpty()) {
         return Comment();
      }

      int i = 0;
      int commentLoc = comments.at(i).lastLine;
      while (commentLoc <= loc) {
         if (commentLoc == loc) {
            return comments.at(i);
         }
         if (i == comments.count() - 1) {
            break;
         }
         commentLoc = comments.at(++i).lastLine;
      }
      return Comment();
   }

   Translator *m_translator;
   QString m_fileName;
   QString m_component;
   QString m_bSource;
};

QString createErrorString(const QString &filename, const QString &code, Parser &parser)
{
   // print out error
   QStringList lines = code.split('\n');
   lines.append("\n");

   QString errorString;

   for (const DiagnosticMessage &m : parser.diagnosticMessages()) {

      if (m.isWarning()) {
         continue;
      }

      QString error = filename + QLatin1Char(':') + QString::number(m.loc.startLine)
                      + QLatin1Char(':') + QString::number(m.loc.startColumn) + QLatin1String(": error: ")
                      + m.message + QLatin1Char('\n');

      int line = 0;
      if (m.loc.startLine > 0) {
         line = m.loc.startLine - 1;
      }

      const QString textLine = lines.at(line);

      error += textLine + QLatin1Char('\n');

      int column = m.loc.startColumn - 1;
      if (column < 0) {
         column = 0;
      }

      column = qMin(column, textLine.length());

      for (int i = 0; i < column; ++i) {
         const QChar ch = textLine.at(i);
         if (ch.isSpace()) {
            error += ch.unicode();
         } else {
            error += QLatin1Char(' ');
         }
      }
      error += QLatin1String("^\n");
      errorString += error;
   }
   return errorString;
}

bool processComment(const QChar *chars, int length, Comment &comment)
{
   // Try to match the logic of the QtScript parser.
   if (!length) {
      return comment.isValid();
   }
   if (*chars == QLatin1Char(':') && chars[1].isSpace()) {
      comment.extracomment += QString(chars + 1, length - 1);
   } else if (*chars == QLatin1Char('=') && chars[1].isSpace()) {
      comment.msgid = QString(chars + 2, length - 2).simplified();
   } else if (*chars == QLatin1Char('~') && chars[1].isSpace()) {
      QString text = QString(chars + 2, length - 2).trimmed();
      int k = text.indexOf(QLatin1Char(' '));
      if (k > -1) {
         comment.extra.insert(text.left(k), text.mid(k + 1).trimmed());
      }
   } else if (*chars == QLatin1Char('%') && chars[1].isSpace()) {
      comment.sourcetext.reserve(comment.sourcetext.length() + length - 2);
      ushort *ptr = (ushort *)comment.sourcetext.data() + comment.sourcetext.length();
      int p = 2, c;

      while (true) {
         if (p >= length) {
            break;
         }
         c = chars[p++].unicode();
         if (isspace(c)) {
            continue;
         }
         if (c != '"') {
            break;
         }

         while (true) {
            if (p >= length) {
               break;
            }
            c = chars[p++].unicode();
            if (c == '"') {
               break;
            }
            if (c == '\\') {
               if (p >= length) {
                  break;
               }
               c = chars[p++].unicode();
               if (c == '\n') {
                  break;
               }
               *ptr++ = '\\';
            }
            *ptr++ = c;
         }
      }
      comment.sourcetext.resize(ptr - (ushort *)comment.sourcetext.data());
   }
   return comment.isValid();
}

bool loadQml(Translator &translator, const QString &filename, ConversionData &cd)
{
   cd.m_sourceFileName = filename;
   QFile file(filename);
   if (!file.open(QIODevice::ReadOnly)) {
      cd.appendError(LU::tr("Cannot open %1: %2").arg(filename, file.errorString()));
      return false;
   }

   QTextStream ts(&file);
   ts.setCodec("UTF-8");
   ts.setAutoDetectUnicode(true);
   const QString code = ts.readAll();

   Engine driver;
   Parser parser(&driver);

   NodePool nodePool(filename, &driver);
   driver.setNodePool(&nodePool);

   Lexer lexer(&driver);
   lexer.setCode(code, /*line = */ 1);
   driver.setLexer(&lexer);

   if (parser.parse()) {
      FindTrCalls trCalls;

      // build up a list of comments that contain translation information.
      for (int i = 0; i < driver.comments().size(); ++i) {
         AST::SourceLocation loc = driver.comments().at(i);
         QString commentStr = code.mid(loc.offset, loc.length);

         if (trCalls.comments.isEmpty() || trCalls.comments.last().lastLine != int(loc.startLine)) {
            Comment comment;
            comment.lastLine = loc.startLine + 1;
            if (processComment(commentStr.constData(), commentStr.length(), comment)) {
               trCalls.comments.append(comment);
            }
         } else {
            Comment &lastComment = trCalls.comments.last();
            lastComment.lastLine += 1;
            processComment(commentStr.constData(), commentStr.length(), lastComment);
         }
      }

      //find all tr calls in the code
      trCalls(&translator, filename, parser.ast());
   } else {
      QString error = createErrorString(filename, code, parser);
      cd.appendError(error);
      return false;
   }
   return true;
}

