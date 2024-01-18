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

#ifndef QDECLARATIVEJSENGINE_P_H
#define QDECLARATIVEJSENGINE_P_H

#include <qdeclarativejsglobal_p.h>
#include <qdeclarativejsastfwd_p.h>
#include <QString>
#include <QSet>

QT_QML_BEGIN_NAMESPACE

namespace QDeclarativeJS {
class QML_PARSER_EXPORT NameId
{
   QString _text;

 public:
   NameId(const QChar *u, int s)
      : _text(u, s) {
   }

   const QString asString() const {
      return _text;
   }

   bool operator == (const NameId &other) const {
      return _text == other._text;
   }

   bool operator != (const NameId &other) const {
      return _text != other._text;
   }

   bool operator < (const NameId &other) const {
      return _text < other._text;
   }
};

uint qHash(const QDeclarativeJS::NameId &id);

} // end of namespace QDeclarativeJS

namespace QDeclarativeJS {

class Lexer;
class NodePool;

namespace Ecma {

class QML_PARSER_EXPORT RegExp
{
 public:
   enum RegExpFlag {
      Global     = 0x01,
      IgnoreCase = 0x02,
      Multiline  = 0x04
   };

 public:
   static int flagFromChar(const QChar &);
   static QString flagsToString(int flags);
};

} // end of namespace Ecma

class QML_PARSER_EXPORT DiagnosticMessage
{
 public:
   enum Kind { Warning, Error };

   DiagnosticMessage()
      : kind(Error) {}

   DiagnosticMessage(Kind kind, const AST::SourceLocation &loc, const QString &message)
      : kind(kind), loc(loc), message(message) {}

   bool isWarning() const {
      return kind == Warning;
   }

   bool isError() const {
      return kind == Error;
   }

   Kind kind;
   AST::SourceLocation loc;
   QString message;
};

class QML_PARSER_EXPORT Engine
{
   Lexer *_lexer;
   NodePool *_nodePool;
   QSet<NameId> _literals;
   QList<QDeclarativeJS::AST::SourceLocation> _comments;

 public:
   Engine();
   ~Engine();

   QSet<NameId> literals() const;

   void addComment(int pos, int len, int line, int col);
   QList<QDeclarativeJS::AST::SourceLocation> comments() const;

   NameId *intern(const QChar *u, int s);

   static QString toString(NameId *id);

   Lexer *lexer() const;
   void setLexer(Lexer *lexer);

   NodePool *nodePool() const;
   void setNodePool(NodePool *nodePool);
};

} // end of namespace QDeclarativeJS

QT_QML_END_NAMESPACE

#endif // QDECLARATIVEJSENGINE_P_H
