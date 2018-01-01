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

#ifndef TOKENREPLACEMENTS_H
#define TOKENREPLACEMENTS_H

#include "tokenengine.h"
#include "textreplacement.h"
#include <QStringList>
#include <QByteArray>

QT_BEGIN_NAMESPACE

void addLogSourceEntry(const QString &text, const TokenEngine::TokenContainer&, const int index);
void addLogWarning(const QString &text);

class TokenReplacement
{
public:

    virtual bool doReplace(const TokenEngine::TokenContainer& ,
                           int /*tokenIndex*/,
                           TextReplacements&){return false;};
    /*
        returns the replace key for this replacement. Every time a token matches the replace key,
        doReplace() will be called for this TokenReplacement.
    */
    virtual QByteArray getReplaceKey(){return QByteArray();};
    virtual ~TokenReplacement(){};
};

/*
    A TokenReplacement that change any token
*/
class GenericTokenReplacement : public TokenReplacement
{
public:
    GenericTokenReplacement(QByteArray oldToken, QByteArray newToken);
    bool doReplace(const TokenEngine::TokenContainer &tokenContainer,
                   int tokenIndex, TextReplacements &textReplacements);
    QByteArray getReplaceKey();
private:
    QByteArray oldToken;
    QByteArray newToken;
};

/*
    A TokenReplacement that changes tokens that specify class names.
    In some cases where the class name token is a part of a qualified name
    it is not correct to rename it. ex:

    QButton::toggleState

    Here it is wrong to rename QButton -> Q3Button, since there is
    a rule that says QButton::ToggleState -> QCheckBox::ToggleState,
    but no rule for Q3Button::ToggleState.
*/
class ClassNameReplacement : public TokenReplacement
{
public:
    ClassNameReplacement(QByteArray oldToken, QByteArray newToken);
    bool doReplace(const TokenEngine::TokenContainer &tokenContainer,
                   int tokenIndex, TextReplacements &textReplacements);
    QByteArray getReplaceKey();
private:
    QByteArray oldToken;
    QByteArray newToken;
};

/*
   Changes scoped tokens:
   AA::BB -> CC::DD
   oldToken corresponds to the AA::BB part, newToken corresponds CC::DD.
   Since this is a token replacement, the AA part of oldToken is typically
   unknown. This means that we might change tokens named BB that does not belong
   to the AA scope. Ast replacemnts will fix this.

*/
class ScopedTokenReplacement : public TokenReplacement
{
public:
    ScopedTokenReplacement(const QByteArray &oldToken, const QByteArray &newToken);
    bool doReplace(const TokenEngine::TokenContainer &tokenContainer,
                   int tokenIndex, TextReplacements &textReplacements);
    QByteArray getReplaceKey();
private:
    QByteArray oldName;
    QByteArray oldScope;
    QByteArray newName;
    QByteArray newScope;
    QByteArray newScopedName;
    bool strictMode;
};

class QualifiedNameParser
{
public:
    QualifiedNameParser(const TokenEngine::TokenContainer &tokenContainer,
                        const int tokenIndex);
    enum Direction { Left=-1, Right=1 };
    bool isPartOfQualifiedName();
    bool isValidIndex(int index);
    bool isQualifier();
    bool isName();
    int peek(Direction direction);
    int move(Direction direction);
private:
    int nextScopeToken(Direction direction);
    int findScopeOperator(Direction direction);
    const TokenEngine::TokenContainer tokenContainer;
    int currentIndex;
};

QT_END_NAMESPACE

#endif
