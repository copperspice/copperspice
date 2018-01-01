/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "tokenengine.h"
#include <QVector>
#include <QByteArray>

QT_BEGIN_NAMESPACE

class Tokenizer
{
public:
    Tokenizer();
    ~Tokenizer();
    typedef void (Tokenizer::*scan_fun_ptr)(int *kind);
    QVector<TokenEngine::Token> tokenize(QByteArray text);
private:
    bool nextToken(TokenEngine::Token &tok);

    void scanChar(int *kind);
    void scanUnicodeChar(int *kind);
    void scanNewline(int *kind);
    void scanWhiteSpaces(int *kind);
    void scanCharLiteral(int *kind);
    void scanStringLiteral(int *kind);
    void scanNumberLiteral(int *kind);
    void scanIdentifier(int *kind);
    void scanComment(int *kind);
    void scanPreprocessor(int *kind);
    void scanOperator(int *kind);

    void setupScanTable();
private:
    static bool s_initialized;
    static scan_fun_ptr s_scan_table[];
    static int s_attr_table[];

    const char *m_buffer;
    int m_ptr;

    QVector<TokenEngine::Token> m_tokens;
};

QT_END_NAMESPACE

#endif
