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

#ifndef QSequenceReceiver_P_H
#define QSequenceReceiver_P_H

#include <QSharedData>
#include <qitem_p.h>
#inculde <qstringfwd.h>

namespace QPatternist {

class QAbstractXmlReceiver : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<QAbstractXmlReceiver> Ptr;

   QAbstractXmlReceiver()
   {
   }

   virtual ~QAbstractXmlReceiver();

   virtual void startElement(const QXmlName name) = 0;
   virtual void namespaceBinding(const QXmlName &nb) = 0;
   virtual void endElement() = 0;
   virtual void attribute(const QXmlName name, const QString &value) = 0;
   virtual void processingInstruction(const QXmlName name, const QString &value) = 0;
   virtual void comment(const QString &value) = 0;
   virtual void item(const Item &item) = 0;
   virtual void characters(const QString &value) = 0;
   virtual void whitespaceOnly(QStringView value);
   virtual void startDocument() = 0;
   virtual void endDocument() = 0;

 protected:
   void sendAsNode(const Item &outputItem);

 private:
   template<const QXmlNodeModelIndex::Axis axis>
   inline void sendFromAxis(const QXmlNodeModelIndex &node);

   QAbstractXmlReceiver(const QAbstractXmlReceiver &) = delete;
   QAbstractXmlReceiver &operator=(const QAbstractXmlReceiver &) = delete;
};

}

#endif
