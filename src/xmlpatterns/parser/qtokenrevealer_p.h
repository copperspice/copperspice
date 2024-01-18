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

#ifndef QTokenRevealer_P_H
#define QTokenRevealer_P_H

#include <qset.h>

#include <qtokenizer_p.h>

namespace QPatternist {

class TokenRevealer : public Tokenizer
{
 public:
   TokenRevealer(const QUrl &uri, const Tokenizer::Ptr &other);
   virtual ~TokenRevealer();

   Token nextToken(YYLTYPE *const sourceLocator) override;
   int commenceScanOnly() override;
   void resumeTokenizationFrom(const int position) override;
   void setParserContext(const ParserContext::Ptr &parseInfo) override;

 private:
   const Tokenizer::Ptr    m_tokenizer;
   QString                 m_result;
   QString                 m_indentationString;
};

}

#endif

