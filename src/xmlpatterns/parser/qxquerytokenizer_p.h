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

#ifndef QXQueryTokenizer_P_H
#define QXQueryTokenizer_P_H

#include <qhash.h>
#include <qset.h>
#include <qstack.h>
#include <qstring.h>
#include <qurl.h>

#include <qtokenizer_p.h>

namespace QPatternist {
struct TokenMap;

class XQueryTokenizer : public Tokenizer
{
 public:
   enum State {
      AfterAxisSeparator,
      AposAttributeContent,
      Axis,
      Default,
      ElementContent,
      EndTag,
      ItemType,
      KindTest,
      KindTestForPI,
      NamespaceDecl,
      NamespaceKeyword,
      OccurrenceIndicator,
      Operator,
      Pragma,
      PragmaContent,
      ProcessingInstructionContent,
      ProcessingInstructionName,
      QuotAttributeContent,
      StartTag,
      VarName,
      XMLComment,
      XMLSpaceDecl,
      XQueryVersion
   };

   XQueryTokenizer(const QString &query, const QUrl &location, const State startingState = Default);

   Token nextToken(YYLTYPE *const sourceLocator) override;
   int commenceScanOnly() override;
   void resumeTokenizationFrom(const int position) override;

   void setParserContext(const ParserContext::Ptr &parseInfo) override;

 private:
   QChar charForReference(const QString &reference);

   inline Token tokenAndChangeState(const TokenType code, const State state, const int advance = 1);
   inline Token tokenAndChangeState(const TokenType code, const QString &value, const State state);
   inline Token tokenAndAdvance(const TokenType code, const int advance = 1);

   QString tokenizeCharacterReference();

   inline Token tokenizeStringLiteral();
   inline Token tokenizeNumberLiteral();

   inline QChar peekAhead(const int length = 1) const;
   inline bool aheadEquals(const char *const chs, const int len, const int offset = 1) const;

   inline Token tokenizeNCName();
   static inline bool isOperatorKeyword(const TokenType);

   static inline bool isDigit(const char ch);
   static inline Token error();
   inline TokenType consumeWhitespace();

   inline QChar peekCurrent() const;
   inline const QChar current() const;

   int peekForColonColon() const;

   static inline bool isNCNameStart(const QChar ch);
   static inline bool isNCNameBody(const QChar ch);
   static inline const TokenMap *lookupKeyword(const QString &keyword);
   inline void popState();
   inline void pushState(const State state);
   inline State state() const;
   inline void setState(const State s);
   static bool isTypeToken(const TokenType t);

   inline Token tokenizeNCNameOrQName();
   int scanUntil(const char *const content);

   inline void pushState();

   inline bool consumeRawWhitespace();
   Tokenizer::TokenType consumeComment();

   static inline bool isPhraseKeyword(const TokenType code);

   typedef QSet<int> CharacterSkips;

   static QString normalizeEOL(const QString &input, const CharacterSkips &characterSkips);

   bool atEnd() const {
      return m_pos == m_length;
   }

   Token nextToken();

   Token attributeAsRaw(const QChar separator, int &stack, const int startPos, const bool inLiteral, QString &result);

   const QString           m_data;
   const int               m_length;
   State                   m_state;
   QStack<State>           m_stateStack;

   int                     m_pos;
   int                     m_line;
   int                     m_columnOffset;

   const NamePool::Ptr     m_namePool;
   QStack<Token>           m_tokenStack;
   QHash<QString, QChar>   m_charRefs;
   bool                    m_scanOnly;

   XQueryTokenizer(const XQueryTokenizer &) = delete;
   XQueryTokenizer &operator=(const XQueryTokenizer &) = delete;
};

}

#endif
