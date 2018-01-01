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

#include <cstdlib>
#include <cstdio>

enum Token {
    EOF_SYMBOL,
    LEFT_ANGLE,
    RIGHT_ANGLE,
    ANY,
};

static int current_char;
static int yytoken;
static bool in_tag = false;

bool parseXmlStream();
bool parseTagOrWord();
bool parseTagName();

inline int nextToken()
{
    current_char = fgetc(stdin);
    if (current_char == EOF) {
        return (yytoken = EOF_SYMBOL);
    } else if (current_char == '<') {
        in_tag = true;
        return (yytoken = LEFT_ANGLE);
    } else if (in_tag && current_char == '>') {
        in_tag = false;
        return (yytoken = RIGHT_ANGLE);
    }
    return (yytoken = ANY);
}

bool parse()
{
    nextToken();
    return parseXmlStream();
}

bool parseXmlStream()
{
    while (parseTagOrWord())
        ;

    return true;
}

bool parseTagOrWord()
{
    if (yytoken == LEFT_ANGLE) {
        nextToken();
        if (! parseTagName())
            return false;
        if (yytoken != RIGHT_ANGLE)
            return false;
        nextToken();

        fprintf (stderr, "*** found a tag\n");

    } else if (yytoken == ANY) {
        nextToken();
    } else {
        return false;
    }
    return true;
}

bool parseTagName()
{
    while (yytoken == ANY)
        nextToken();

    return true;
}

int main()
{
    if (parse())
        printf("OK\n");
    else
        printf("KO\n");
}
