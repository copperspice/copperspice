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

#include <qtokenrevealer_p.h>

using namespace QPatternist;

TokenRevealer::TokenRevealer(const QUrl &uri,
                             const Tokenizer::Ptr &other) : Tokenizer(uri)
   , m_tokenizer(other)
{
   Q_ASSERT(other);
}

TokenRevealer::~TokenRevealer()
{
   qDebug() << "Tokens Revealed:" << m_result;
}

void TokenRevealer::setParserContext(const ParserContext::Ptr &parseInfo)
{
   m_tokenizer->setParserContext(parseInfo);
}

Tokenizer::Token TokenRevealer::nextToken(YYLTYPE *const sourceLocator)
{
   const Token token(m_tokenizer->nextToken(sourceLocator));
   const QString asString(tokenToString(token));
   const TokenType type = token.type;

   /* Indent. */
   switch (type) {
      case CURLY_LBRACE: {
         m_result += QLatin1Char('\n') + m_indentationString + asString + QLatin1Char('\n');
         m_indentationString.append(QLatin1String("    "));
         m_result += m_indentationString;
         break;
      }

      case CURLY_RBRACE: {
         m_indentationString.chop(4);
         m_result += QLatin1Char('\n') + m_indentationString + asString;
         break;
      }

      case SEMI_COLON:
      case COMMA: {
         m_result += asString + QLatin1Char('\n') + m_indentationString;
         break;
      }

      default:
         m_result += asString + QLatin1Char(' ');
   }

   return token;
}

int TokenRevealer::commenceScanOnly()
{
   return m_tokenizer->commenceScanOnly();
}

void TokenRevealer::resumeTokenizationFrom(const int position)
{
   m_tokenizer->resumeTokenizationFrom(position);
}
