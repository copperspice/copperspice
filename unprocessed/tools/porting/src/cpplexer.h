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

#ifndef CPPLEXER_H
#define CPPLEXER_H

#include "tokenengine.h"
#include "tokens.h"
#include <QVector>

QT_BEGIN_NAMESPACE

class CppLexer
{
public:
    CppLexer();
    typedef void (CppLexer::*scan_fun_ptr)(int *kind);
    QVector<Type> lex(TokenEngine::TokenSectionSequence tokenContainer);
private:
    Type identify(const TokenEngine::TokenTempRef &tokenTempRef);
    void setupScanTable();

    void scanChar(int *kind);
    void scanUnicodeChar(int *kind);
    void scanNewline(int *kind);
    void scanWhiteSpaces(int *kind);
    void scanCharLiteral(int *kind);
    void scanStringLiteral(int *kind);
    void scanNumberLiteral(int *kind);
    void scanIdentifier(int *kind);
    void scanPreprocessor(int *kind);
    void scanOperator(int *kind);

    void scanKeyword0(int *kind);
    void scanKeyword2(int *kind);
    void scanKeyword3(int *kind);
    void scanKeyword4(int *kind);
    void scanKeyword5(int *kind);
    void scanKeyword6(int *kind);
    void scanKeyword7(int *kind);
    void scanKeyword8(int *kind);
    void scanKeyword9(int *kind);
    void scanKeyword10(int *kind);
    void scanKeyword11(int *kind);
    void scanKeyword12(int *kind);
    void scanKeyword14(int *kind);
    void scanKeyword16(int *kind);

    CppLexer::scan_fun_ptr s_scan_table[128+1];
    int s_attr_table[256];
    CppLexer::scan_fun_ptr s_scan_keyword_table[17];

    enum
    {
        A_Alpha = 0x01,
        A_Digit = 0x02,
        A_Alphanum = A_Alpha | A_Digit,
        A_Whitespace = 0x04
    };

    const char *m_buffer;
    int m_ptr;
    int m_len;
};

QT_END_NAMESPACE

#endif
