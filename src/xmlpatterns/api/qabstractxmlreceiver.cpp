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

#include <qstring.h>
#include <qitem_p.h>
#include <qabstractxmlreceiver_p.h>
#include <qabstractxmlreceiver.h>

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

/*!
 \internal
 */
QAbstractXmlReceiver::QAbstractXmlReceiver(QAbstractXmlReceiverPrivate *d)
   : d_ptr(d)
{
}

/*!
  Constructs an abstract xml receiver.
 */
QAbstractXmlReceiver::QAbstractXmlReceiver()
   : d_ptr(nullptr)
{
}

/*!
  Destroys the xml receiver.
 */
QAbstractXmlReceiver::~QAbstractXmlReceiver()
{
}

void QAbstractXmlReceiver::sendAsNode(const QPatternist::Item &outputItem)
{
   Q_ASSERT(outputItem);
   Q_ASSERT(outputItem.isNode());
   const QXmlNodeModelIndex asNode = outputItem.asNode();

   switch (asNode.kind()) {
      case QXmlNodeModelIndex::Attribute: {
         const QString &v = outputItem.stringValue();
         attribute(asNode.name(), QStringView(v));
         return;
      }
      case QXmlNodeModelIndex::Element: {
         startElement(asNode.name());

         /* First the namespaces, then attributes, then the children. */
         asNode.sendNamespaces(this);
         sendFromAxis<QXmlNodeModelIndex::AxisAttribute>(asNode);
         sendFromAxis<QXmlNodeModelIndex::AxisChild>(asNode);

         endElement();

         return;
      }
      case QXmlNodeModelIndex::Text: {
         const QString &v = asNode.stringValue();
         characters(QStringView(v));
         return;
      }
      case QXmlNodeModelIndex::ProcessingInstruction: {
         processingInstruction(asNode.name(), outputItem.stringValue());
         return;
      }
      case QXmlNodeModelIndex::Comment: {
         comment(outputItem.stringValue());
         return;
      }
      case QXmlNodeModelIndex::Document: {
         startDocument();
         sendFromAxis<QXmlNodeModelIndex::AxisChild>(asNode);
         endDocument();
         return;
      }
      case QXmlNodeModelIndex::Namespace:
         Q_ASSERT_X(false, Q_FUNC_INFO, "QXmlNodeModelIndex::Namespace was not implemented");
   }

   Q_ASSERT_X(false, Q_FUNC_INFO, QString("Unknown node type: %1").formatArg(asNode.kind()).toUtf8().constData());
}

void QAbstractXmlReceiver::whitespaceOnly(QStringView value)
{
   Q_ASSERT_X(value.toString().trimmed().isEmpty(), Q_FUNC_INFO,
              "The caller must guarantee only whitespace is passed. Use characters() in other cases.");
   const QString &v = QString(value);
   characters(QStringView(v));
}

/*!
  \internal
 */
void QAbstractXmlReceiver::item(const QPatternist::Item &item)
{
   if (item.isNode()) {
      return sendAsNode(item);
   } else {
      atomicValue(QPatternist::AtomicValue::toQt(item.asAtomicValue()));
   }
}
