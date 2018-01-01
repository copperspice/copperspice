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

#ifndef QPULLBRIDGE_P_H
#define QPULLBRIDGE_P_H

#include <QtCore/QPair>
#include <QtCore/QStack>

#include <qabstractxmlforwarditerator_p.h>
#include <qabstractxmlpullprovider_p.h>
#include <qitem_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist {
class PullBridge : public AbstractXmlPullProvider
{
 public:
   inline PullBridge(const QXmlNodeModelIndex::Iterator::Ptr &it) : m_current(StartOfInput) {
      Q_ASSERT(it);
      m_iterators.push(qMakePair(StartOfInput, it));
   }

   Event next() override;
   Event current() const override;
   QXmlName name() const override;
   /**
    * Returns always an empty QVariant.
    */
   QVariant atomicValue() const override;
   QString stringValue() const override;
   QHash<QXmlName, QString> attributes() override;
   QHash<QXmlName, QXmlItem> attributeItems() override;

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
