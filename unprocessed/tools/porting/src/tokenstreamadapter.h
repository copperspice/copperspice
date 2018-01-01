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

#ifndef TOKENSTREAMADAPTER_H
#define TOKENSTREAMADAPTER_H

#include "tokenengine.h"
#include "tokens.h"

#include <QVector>

QT_BEGIN_NAMESPACE

namespace TokenStreamAdapter {
struct TokenStream
{
    TokenStream(TokenEngine::TokenSectionSequence translationUnit, QVector<Type> tokenKindList)
    :m_translationUnit(translationUnit),
     m_tokenKindList(tokenKindList),
     m_cursor(0),
     m_numTokens(tokenKindList.count())
    {
        Q_ASSERT(translationUnit.count() == m_numTokens);

        // Copy out the container and containerIndex for each token so we can have
        // constant time random access to it.
        TokenEngine::TokenSectionSequenceIterator it(translationUnit);
        while(it.nextToken()) {
            m_tokenContainers.append(it.tokenContainer());
            m_containerIndices.append(it.containerIndex());
        }
    }

    bool isHidden(int index) const
    {
       if(index >= m_numTokens)
            return false;
        QT_PREPEND_NAMESPACE(Type) type = m_tokenKindList.at(index);
        return (type == Token_whitespaces || type == 10 /*newline*/ ||
                type == Token_comment || type == Token_preproc );
    }

    inline int lookAhead(int n = 0) const
    {
        if(m_cursor + n >= m_numTokens)
            return 0;
        return m_tokenKindList.at(m_cursor + n);
    }

    inline int currentToken() const
    { return lookAhead(); }

    inline QByteArray currentTokenText() const
    {
        return tokenText(m_cursor);
    }

    inline TokenEngine::TokenContainer tokenContainer(int index = 0) const
    {
        if (index < m_numTokens)
            return m_tokenContainers.at(index);
        else
            return TokenEngine::TokenContainer();
    }

    inline int containerIndex(int index = 0) const
    {
        if (index < m_numTokens)
            return m_containerIndices.at(index);
        else
            return -1;
    }

    inline QByteArray tokenText(int index = 0) const
    {
        if (index <  m_numTokens) {
            const TokenEngine::TokenContainer container = tokenContainer(index);
            const int cIndex = containerIndex(index);
            return container.text(cIndex);
        } else {
            return QByteArray();
        }
    }

    inline void rewind(int index)
    { m_cursor = index; }

    inline int cursor() const
    { return m_cursor; }

    inline void nextToken()
    { ++m_cursor; }

    inline bool tokenAtEnd()
    { return m_cursor >= m_numTokens; }

    TokenEngine::TokenSectionSequence tokenSections() const
    { return m_translationUnit;  }

private:
   TokenEngine::TokenSectionSequence m_translationUnit;
   QVector<Type> m_tokenKindList;
   QList<TokenEngine::TokenContainer> m_tokenContainers;
   QList<int> m_containerIndices;
   int m_cursor;
   int m_numTokens;
};

} //namespace TokenStreamAdapter

QT_END_NAMESPACE

#endif
