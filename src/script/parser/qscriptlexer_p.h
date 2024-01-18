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

#ifndef QSCRIPTLEXER_P_H
#define QSCRIPTLEXER_P_H

#include <qstring.h>

class QScriptEnginePrivate;
class QScriptNameIdImpl;

namespace QScript {

class Lexer
{
 public:
   Lexer(QScriptEnginePrivate *eng);
   ~Lexer();

   void setCode(const QString &str, int lineno);
   int lex();

   int currentLineNo() const {
      return yylineno;
   }
   int currentColumnNo() const {
      return yycolumn;
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

   QScriptNameIdImpl *pattern;
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

   static unsigned char convertHex(char32_t c1);
   static unsigned char convertHex(char32_t c1, char32_t c2);
   static QChar convertUnicode(char32_t c1, char32_t c2, char32_t c3, char32_t c4);
   static bool isIdentLetter(char32_t c);
   static bool isDecimalDigit(char32_t c);

   inline int ival() const {
      return qsyylval.ival;
   }
   inline double dval() const {
      return qsyylval.dval;
   }
   inline QScriptNameIdImpl *ustr() const {
      return qsyylval.ustr;
   }

   const QChar *characterBuffer() const {
      return buffer16;
   }
   int characterCount() const {
      return pos16;
   }

 private:

   QScriptEnginePrivate *driver;
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
   bool isHexDigit(char32_t c) const;
   bool isOctalDigit(char32_t c) const;

   int matchPunctuator(char32_t c1, char32_t c2, char32_t c3, char32_t c4);
   char32_t singleEscape(char32_t c) const;
   char32_t convertOctal(char32_t c1, char32_t c2, char32_t c3) const;

   void record8(char32_t c);
   void record16(QChar c);
   void recordStartPos();

   int findReservedWord(const QChar *buffer, int size) const;

   void syncProhibitAutomaticSemicolon();

   QString::const_iterator m_iter;

   uint length;
   int yycolumn;
   int startlineno;
   int startcolumn;
   int bol;                 // begin of line

   union {
      int ival;
      double dval;
      QScriptNameIdImpl *ustr;
   } qsyylval;

   // current and following unicode characters
   char32_t current, next1, next2, next3;

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
};

} // namespace QScript

#endif
