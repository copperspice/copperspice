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

#include "qitem_p.h"

#include "qabstractxmlreceiver.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

QAbstractXmlReceiver::~QAbstractXmlReceiver()
{
}

template<const QXmlNodeModelIndex::Axis axis>
void QAbstractXmlReceiver::sendFromAxis(const QXmlNodeModelIndex &node)
{
   Q_ASSERT(!node.isNull());
   const QXmlNodeModelIndex::Iterator::Ptr it(node.iterate(axis));
   QXmlNodeModelIndex next(it->next());

   while (!next.isNull()) {
      sendAsNode(next);
      next = it->next();
   }
}

void QAbstractXmlReceiver::sendAsNode(const Item &outputItem)
{
   Q_ASSERT(outputItem);
   Q_ASSERT(outputItem.isNode());
   const QXmlNodeModelIndex asNode = outputItem.asNode();

   switch (asNode.kind()) {
      case QXmlNodeModelIndex::Attribute: {
         attribute(asNode.name(), outputItem.stringValue());
         break;
      }
      case QXmlNodeModelIndex::Element: {
         startElement(asNode.name());

         /* First the namespaces, then attributes, then the children. */
         asNode.sendNamespaces(Ptr(const_cast<QAbstractXmlReceiver *>(this)));
         sendFromAxis<QXmlNodeModelIndex::AxisAttribute>(asNode);
         sendFromAxis<QXmlNodeModelIndex::AxisChild>(asNode);

         endElement();

         break;
      }
      case QXmlNodeModelIndex::Text: {
         characters(outputItem.stringValue());
         break;
      }
      case QXmlNodeModelIndex::ProcessingInstruction: {
         processingInstruction(asNode.name(), outputItem.stringValue());
         break;
      }
      case QXmlNodeModelIndex::Comment: {
         comment(outputItem.stringValue());
         break;
      }
      case QXmlNodeModelIndex::Document: {
         sendFromAxis<QXmlNodeModelIndex::AxisChild>(asNode);
         break;
      }
      case QXmlNodeModelIndex::Namespace:
         Q_ASSERT_X(false, Q_FUNC_INFO, "QXmlNodeModelIndex::Namespace was not implemented");
   }
}

void QAbstractXmlReceiver::whitespaceOnly(const QStringRef &value)
{
   Q_ASSERT_X(value.toString().trimmed().isEmpty(), Q_FUNC_INFO,
              "Only whitespace should be passed, use characters() in other cases.");
   characters(value.toString());
}

QT_END_NAMESPACE
