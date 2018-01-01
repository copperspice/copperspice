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

#ifndef QABSTRACTXMLRECEIVER_H
#define QABSTRACTXMLRECEIVER_H

#include <QtCore/QVariant>
#include <QtCore/QScopedPointer>
#include <QtXmlPatterns/QXmlNodeModelIndex>

QT_BEGIN_NAMESPACE

class QAbstractXmlReceiverPrivate;
class QXmlName;

namespace QPatternist {
class Item;
}

class Q_XMLPATTERNS_EXPORT QAbstractXmlReceiver
{
 public:
   QAbstractXmlReceiver();

   virtual ~QAbstractXmlReceiver();

   virtual void startElement(const QXmlName &name) = 0;
   virtual void endElement() = 0;
   virtual void attribute(const QXmlName &name,
                          const QStringRef &value) = 0;
   virtual void comment(const QString &value) = 0;
   virtual void characters(const QStringRef &value) = 0;
   virtual void startDocument() = 0;
   virtual void endDocument() = 0;

   virtual void processingInstruction(const QXmlName &target,
                                      const QString &value) = 0;

   virtual void atomicValue(const QVariant &value) = 0;
   virtual void namespaceBinding(const QXmlName &name) = 0;
   virtual void startOfSequence() = 0;
   virtual void endOfSequence() = 0;

   /* The members below are internal, not part of the public API, and
    * unsupported. Using them leads to undefined behavior. */
   virtual void whitespaceOnly(const QStringRef &value);
   virtual void item(const QPatternist::Item &item);

 protected:
   QAbstractXmlReceiver(QAbstractXmlReceiverPrivate *d);
   QScopedPointer<QAbstractXmlReceiverPrivate> d_ptr;

   void sendAsNode(const QPatternist::Item &outputItem);
 private:
   template<const QXmlNodeModelIndex::Axis axis>
   void sendFromAxis(const QXmlNodeModelIndex &node);
   Q_DISABLE_COPY(QAbstractXmlReceiver)
};

QT_END_NAMESPACE

#endif
