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

#ifndef QDECLARATIVEJSLEXER_P_H
#define QDECLARATIVEJSLEXER_P_H

#include <qdeclarativejsglobal_p.h>
#include <QtCore/QString>

QT_QML_BEGIN_NAMESPACE

namespace QDeclarativeJS {

class Engine;
class NameId;

class QML_PARSER_EXPORT Lexer
{
 public:
   Lexer(Engine *eng, bool tokenizeComments = false);
   ~Lexer();

   void setCode(const QString &c, int lineno);
   int lex();

   int currentLineNo() const {
      return yylineno;
   }
   int currentColumnNo() const {
      return yycolumn;
   }

   int tokenOffset() const {
      return startpos;
   }
   int tokenLength() const {
      return pos - startpos;
   }

   int startLineNo() const {
      return startlineno;
   }
   int startColumnNo() const {
      return startcolumn;
   }

   int endLineNo() const {
      return currentLineNo();
   }
   int endColumnNo() const {
      int col = currentColumnNo();
      return (col > 0) ? col - 1 : col;
   }

   bool prevTerminator() const {
      return terminator;
   }

   enum State { Start,
                Identifier,
                InIdentifier,
                InSingleLineComment,
                InMultiLineComment,
                InNum,
                InNum0,
                InHex,
                InOctal,
                InDecimal,
                InExponentIndicator,
                InExponent,
                Hex,
                Octal,
                Number,
                String,
                Eof,
                InString,
                InEscapeSequence,
                InHexEscape,
                InUnicodeEscape,
                Other,
                Bad
              };

   enum Error {
      NoError,
      IllegalCharacter,
      UnclosedStringLiteral,
      IllegalEscapeSequence,
      IllegalUnicodeEscapeSequence,
      UnclosedComment,
      IllegalExponentIndicator,
      IllegalIdentifier
   };

   enum ParenthesesState {
      IgnoreParentheses,
      CountParentheses,
      BalancedParentheses
   };

   enum RegExpBodyPrefix {
      NoPrefix,
      EqualPrefix
   };

   bool scanRegExp(RegExpBodyPrefix prefix = NoPrefix);

   NameId *pattern;
   int flags;

   State lexerState() const {
      return state;
   }

   QString errorMessage() const {
      return errmsg;
   }
   void setErrorMessage(const QString &err) {
      errmsg = err;
   }
   void setErrorMessage(const char *err) {
      setErrorMessage(QString::fromLatin1(err));
   }

   Error error() const {
      return err;
   }
   void clearError() {
      err = NoError;
   }

 private:
   Engine *driver;
   int yylineno;
   bool done;
   char *buffer8;
   QChar *buffer16;
   uint size8, size16;
   uint pos8, pos16;
   bool terminator;
   bool restrKeyword;
   // encountered delimiter like "'" and "}" on last run
   bool delimited;
   int stackToken;

   State state;
   void setDone(State s);
   uint pos;
   void shift(uint p);
   int lookupKeyword(const char *);

   bool isWhiteSpace() const;
   bool isLineTerminator() const;
   bool isHexDigit(ushort c) const;
   bool isOctalDigit(ushort c) const;

   int matchPunctuator(ushort c1, ushort c2,
                       ushort c3, ushort c4);
   ushort singleEscape(ushort c) const;
   ushort convertOctal(ushort c1, ushort c2,
                       ushort c3) const;
 public:
   static unsigned char convertHex(ushort c1);
   static unsigned char convertHex(ushort c1, ushort c2);
   static QChar convertUnicode(ushort c1, ushort c2,
                               ushort c3, ushort c4);
   static bool isIdentLetter(ushort c);
   static bool isDecimalDigit(ushort c);

   inline int ival() const {
      return qsyylval.ival;
   }
   inline double dval() const {
      return qsyylval.dval;
   }
   inline NameId *ustr() const {
      return qsyylval.ustr;
   }

   const QChar *characterBuffer() const {
      return buffer16;
   }
   int characterCount() const {
      return pos16;
   }

 private:
   void record8(ushort c);
   void record16(QChar c);
   void recordStartPos();

   int findReservedWord(const QChar *buffer, int size) const;

   void syncProhibitAutomaticSemicolon();

   const QChar *code;
   uint length;
   int yycolumn;
   int startpos;
   int startlineno;
   int startcolumn;
   int bol;     // begin of line

   union {
      int ival;
      double dval;
      NameId *ustr;
   } qsyylval;

   // current and following unicode characters
   ushort current, next1, next2, next3;

   struct keyword {
      const char *name;
      int token;
   };

   QString errmsg;
   Error err;

   bool wantRx;
   bool check_reserved;

   ParenthesesState parenthesesState;
   int parenthesesCount;
   bool prohibitAutomaticSemicolon;
   bool tokenizeComments;
};

} // namespace QDeclarativeJS

QT_QML_END_NAMESPACE

#endif
