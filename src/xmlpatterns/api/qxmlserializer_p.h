/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QXMLSERIALIZER_P_H
#define QXMLSERIALIZER_P_H

#include <QtCore/QIODevice>
#include <QtCore/QStack>
#include <QtCore/QTextCodec>
#include <QtXmlPatterns/QXmlQuery>
#include <QtXmlPatterns/QXmlNamePool>
#include <QtXmlPatterns/QXmlSerializer>
#include "qnamepool_p.h"
#include "qabstractxmlreceiver_p.h"

QT_BEGIN_NAMESPACE

class QXmlSerializerPrivate : public QAbstractXmlReceiverPrivate
{
 public:
   QXmlSerializerPrivate(const QXmlQuery &q,
                         QIODevice *outputDevice);

   QStack<QPair<QXmlName, bool> >      hasClosedElement;
   bool                                isPreviousAtomic;
   QXmlSerializer::State               state;
   const QPatternist::NamePool::Ptr    np;

   /**
    * This member worries me a bit. We never use it but nevertheless
    * it is pushed and pops linear to startElement() and endElement().
    * An optimization would be to at least merge it with hasClosedElement,
    * but even better to push it on demand. That is, namespaceBinding()
    * pushes it up to the tree depth first when it is needed.
    */
   QStack<QVector<QXmlName> >          namespaces;

   QIODevice                          *device;
   const QTextCodec                   *codec;
   QTextCodec::ConverterState          converterState;
   /**
    * Name cache. Since encoding QStrings are rather expensive
    * operations to do, and we on top of that would have to do
    * it each time a name appears, we here map names to their
    * encoded equivalents.
    *
    * This means that when writing out large documents, the serialization
    * of names after a while is reduced to a hash lookup and passing an
    * existing byte array.
    *
    * We use QXmlName::Code as key as opposed to merely QName, because the
    * prefix is of significance.
    */
   QHash<QXmlName::Code, QByteArray>   nameCache;
   const QXmlQuery                     query;

   inline void write(const char c);

 private:
   enum Constants {
      EstimatedTreeDepth = 10,

      /**
       * We use a high count to avoid rehashing. We can afford it since we
       * only allocate one hash for this.
       */
      EstimatedNameCount = 60
   };
};

void QXmlSerializerPrivate::write(const char c)
{
   device->putChar(c);
}
QT_END_NAMESPACE

#endif
