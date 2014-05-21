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

#ifndef PATTERNIST_PULLBRIDGE_P_H
#define PATTERNIST_PULLBRIDGE_P_H

#include <QtCore/QPair>
#include <QtCore/QStack>

#include "qabstractxmlforwarditerator_p.h"
#include "qabstractxmlpullprovider_p.h"
#include "qitem_p.h"

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    class PullBridge : public AbstractXmlPullProvider
    {
    public:
        inline PullBridge(const QXmlNodeModelIndex::Iterator::Ptr &it) : m_current(StartOfInput)
        {
            Q_ASSERT(it);
            m_iterators.push(qMakePair(StartOfInput, it));
        }

        virtual Event next();
        virtual Event current() const;
        virtual QXmlName name() const;
        /**
         * Returns always an empty QVariant.
         */
        virtual QVariant atomicValue() const;
        virtual QString stringValue() const;
        virtual QHash<QXmlName, QString> attributes();
        virtual QHash<QXmlName, QXmlItem> attributeItems();

        QXmlNodeModelIndex index() const;
        QSourceLocation sourceLocation() const;

    private:
        typedef QStack<QPair<Event, QXmlNodeModelIndex::Iterator::Ptr> > IteratorStack;
        IteratorStack      m_iterators;
        QXmlNodeModelIndex m_index;
        Event              m_current;
    };
}

QT_END_NAMESPACE

#endif
