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

#include <QVariant>

#include "qabstractxmlnodemodel_p.h"
#include "qitemmappingiterator_p.h"
#include "qitem_p.h"
#include "qxmlname.h"
#include "qxmlquery_p.h"

#include "qpullbridge_p.h"

using namespace QPatternist;

/*!
  \brief Bridges a QPatternist::SequenceIterator to QAbstractXmlPullProvider.
  \class QPatternist::PullBridge
  \internal
  \reentrant
  \ingroup xml-tools

  The approach of this class is rather straight forward since QPatternist::SequenceIterator
  and QAbstractXmlPullProvider are conceptually similar. While QPatternist::SequenceIterator only
  delivers top level items(since it's not an event stream, it's a list of items), PullBridge
  needs to recursively iterate the children of nodes too, which is achieved through the
  stack m_iterators.
 */

AbstractXmlPullProvider::Event PullBridge::next()
{
   m_index = m_iterators.top().second->next();

   if (!m_index.isNull()) {
      Item item(m_index);

      if (item && item.isAtomicValue()) {
         m_current = AtomicValue;
      } else {
         Q_ASSERT(item.isNode());

         switch (m_index.kind()) {
            case QXmlNodeModelIndex::Attribute: {
               m_current = Attribute;
               break;
            }
            case QXmlNodeModelIndex::Comment: {
               m_current = Comment;
               break;
            }
            case QXmlNodeModelIndex::Element: {
               m_iterators.push(qMakePair(StartElement, m_index.iterate(QXmlNodeModelIndex::AxisChild)));
               m_current = StartElement;
               break;
            }
            case QXmlNodeModelIndex::Document: {
               m_iterators.push(qMakePair(StartDocument, m_index.iterate(QXmlNodeModelIndex::AxisChild)));
               m_current = StartDocument;
               break;
            }
            case QXmlNodeModelIndex::Namespace: {
               m_current = Namespace;
               break;
            }
            case QXmlNodeModelIndex::ProcessingInstruction: {
               m_current = ProcessingInstruction;
               break;
            }
            case QXmlNodeModelIndex::Text: {
               m_current = Text;
               break;
            }
         }
      }
   } else {
      if (m_iterators.isEmpty()) {
         m_current = EndOfInput;
      } else {
         switch (m_iterators.top().first) {
            case StartOfInput: {
               m_current = EndOfInput;
               break;
            }
            case StartElement: {
               m_current = EndElement;
               m_iterators.pop();
               break;
            }
            case StartDocument: {
               m_current = EndDocument;
               m_iterators.pop();
               break;
            }
            default: {
               Q_ASSERT_X(false, Q_FUNC_INFO, "Invalid value.");
               m_current = EndOfInput;
            }
         }
      }

   }

   return m_current;
}

AbstractXmlPullProvider::Event PullBridge::current() const
{
   return m_current;
}

QXmlNodeModelIndex PullBridge::index() const
{
   return m_index;
}

QSourceLocation PullBridge::sourceLocation() const
{
   return m_index.model()->sourceLocation(m_index);
}

QXmlName PullBridge::name() const
{
   return m_index.name();
}

QVariant PullBridge::atomicValue() const
{
   return QVariant();
}

QString PullBridge::stringValue() const
{
   return QString();
}

QHash<QXmlName, QString> PullBridge::attributes()
{
   Q_ASSERT(m_current == StartElement);

   QHash<QXmlName, QString> attributes;

   QXmlNodeModelIndex::Iterator::Ptr it = m_index.iterate(QXmlNodeModelIndex::AxisAttribute);
   QXmlNodeModelIndex index = it->next();
   while (!index.isNull()) {
      const Item attribute(index);
      attributes.insert(index.name(), index.stringValue());

      index = it->next();
   }

   return attributes;
}

QHash<QXmlName, QXmlItem> PullBridge::attributeItems()
{
   Q_ASSERT(m_current == StartElement);

   QHash<QXmlName, QXmlItem> attributes;

   QXmlNodeModelIndex::Iterator::Ptr it = m_index.iterate(QXmlNodeModelIndex::AxisAttribute);
   QXmlNodeModelIndex index = it->next();
   while (!index.isNull()) {
      const Item attribute(index);
      attributes.insert(index.name(), QXmlItem(index));

      index = it->next();
   }

   return attributes;
}
