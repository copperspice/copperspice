/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef Patternist_TokenRevealer_h
#define Patternist_TokenRevealer_h

#include <QSet>
#include "qtokenizer_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist {
class TokenRevealer : public Tokenizer
{
 public:
   TokenRevealer(const QUrl &uri,
                 const Tokenizer::Ptr &other);

   virtual ~TokenRevealer();

   virtual Token nextToken(YYLTYPE *const sourceLocator);
   virtual int commenceScanOnly();
   virtual void resumeTokenizationFrom(const int position);
   virtual void setParserContext(const ParserContext::Ptr &parseInfo);

 private:
   const Tokenizer::Ptr    m_tokenizer;
   QString                 m_result;
   QString                 m_indentationString;
};
}

QT_END_NAMESPACE

#endif

