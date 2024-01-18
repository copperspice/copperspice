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
   /**
    * Tokenizer states. Organized alphabetically.
    */
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

   /**
    * Does nothing.
    */
   void setParserContext(const ParserContext::Ptr &parseInfo) override;

 private:

   /**
    * Returns the character corresponding to the builtin reference @p
    * reference. For instance, passing @c gt will give you '>' in return.
    *
    * If @p reference is an invalid character reference, a null QChar is
    * returned.
    *
    * @see QChar::isNull()
    */
   QChar charForReference(const QString &reference);

   inline Token tokenAndChangeState(const TokenType code,
                                    const State state,
                                    const int advance = 1);
   inline Token tokenAndChangeState(const TokenType code,
                                    const QString &value,
                                    const State state);
   inline Token tokenAndAdvance(const TokenType code,
                                const int advance = 1);
   QString tokenizeCharacterReference();

   inline Token tokenizeStringLiteral();
   inline Token tokenizeNumberLiteral();

   /**
    * @returns the character @p length characters from the current
    * position.
    */
   inline QChar peekAhead(const int length = 1) const;

   /**
    * @returns whether the stream, starting from @p offset from the
    * current position, matches @p chs. The length of @p chs is @p len.
    */
   inline bool aheadEquals(const char *const chs,
                           const int len,
                           const int offset = 1) const;

   inline Token tokenizeNCName();
   static inline bool isOperatorKeyword(const TokenType);

   static inline bool isDigit(const char ch);
   static inline Token error();
   inline TokenType consumeWhitespace();

   inline QChar peekCurrent() const;

   /**
    * Disregarding encoding conversion, equivalent to calling:
    *
    * @code
    * peekAhead(0);
    * @endcode
    */
   inline const QChar current() const;

   /**
    * @p hadWhitespace is always set to a proper value.
    *
    * @returns the length of whitespace scanned before reaching "::", or
    * -1 if something else was found.
    */
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
   /**
    * Advances m_pos until content is encountered.
    *
    * Returned is the length stretching from m_pos when starting, until
    * @p content is encountered. @p content is not included in the length.
    */
   int scanUntil(const char *const content);

   /**
    * Same as calling:
    * @code
    * pushState(currentState());
    * @endcode
    */
   inline void pushState();

   /**
    * Consumes only whitespace, in the traditional sense. The function exits
    * if non-whitespace is encountered, such as the start of a comment.
    *
    * @returns @c true if the end was reached, otherwise @c false
    */
   inline bool consumeRawWhitespace();

   /**
    * @short Parses comments: <tt>(: comment content :)</tt>. It recurses for
    * parsing nested comments.
    *
    * It is assumed that the start token for the comment, "(:", has
    * already been parsed.
    *
    * Typically, don't call this function, but ignoreWhitespace().
    *
    * @see <a href="http://www.w3.org/TR/xpath20/#comments">XML Path Language (XPath)
    * 2.0, 2.6 Comments</a>
    * @returns
    * - SUCCESS if everything went ok
    * - ERROR if there was an error in parsing one or more comments
    * - END_OF_FILE if the end was reached
    */
   Tokenizer::TokenType consumeComment();

   /**
    * Determines whether @p code is a keyword
    * that is followed by a second keyword. For instance <tt>declare
    * function</tt>.
    */
   static inline bool isPhraseKeyword(const TokenType code);

   /**
    * A set of indexes into a QString, the one being passed to
    * normalizeEOL() whose characters shouldn't be normalized. */
   typedef QSet<int> CharacterSkips;

   /**
    * Returns @p input, normalized according to
    * <a href="http://www.w3.org/TR/xquery/#id-eol-handling">XQuery 1.0:
    * An XML Query Language, A.2.3 End-of-Line Handling</a>
    */
   static QString normalizeEOL(const QString &input,
                               const CharacterSkips &characterSkips);

   inline bool atEnd() const {
      return m_pos == m_length;
   }

   Token nextToken();
   /**
    * Instead of recognizing and tokenizing embedded expressions in
    * direct attriute constructors, this function is essentially a mini
    * recursive-descent parser that has the necessary logic to recognize
    * embedded expressions and their potentially interfering string literals, in
    * order to scan to the very end of the attribute value, and return the
    * whole as a string.
    *
    * There is of course syntax errors this function will not detect, but
    * that is ok since the attributes will be parsed once more.
    *
    * An inelegant solution, but which gets the job done.
    *
    * @see commenceScanOnly(), resumeTokenizationFrom()
    */
   Token attributeAsRaw(const QChar separator, int &stack, const int startPos,
                  const bool inLiteral, QString &result);

   const QString           m_data;
   const int               m_length;
   State                   m_state;
   QStack<State>           m_stateStack;
   int                     m_pos;

   /**
    * The current line number.
    *
    * The line number and column number both starts at 1.
    */
   int                     m_line;

   /**
    * The offset into m_length for where
    * the current column starts. So m_length - m_columnOffset
    * is the current column.
    *
    * The line number and column number both starts at 1.
    */
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
