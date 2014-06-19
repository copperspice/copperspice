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

#ifndef QXMLFORMATTER_H
#define QXMLFORMATTER_H

#include <QtXmlPatterns/QXmlSerializer>

QT_BEGIN_NAMESPACE
class QIODevice;
class QTextCodec;
class QXmlQuery;
class QXmlFormatterPrivate;

class Q_XMLPATTERNS_EXPORT QXmlFormatter : public QXmlSerializer
{
 public:
   QXmlFormatter(const QXmlQuery &query, QIODevice *outputDevice);

   virtual void characters(const QStringRef &value);
   virtual void comment(const QString &value);
   virtual void startElement(const QXmlName &name);
   virtual void endElement();

   virtual void attribute(const QXmlName &name,
                          const QStringRef &value);
   virtual void processingInstruction(const QXmlName &name,
                                      const QString &value);
   virtual void atomicValue(const QVariant &value);
   virtual void startDocument();
   virtual void endDocument();
   virtual void startOfSequence();
   virtual void endOfSequence();

   int indentationDepth() const;
   void setIndentationDepth(int depth);

   /* The members below are internal, not part of the public API, and
    * unsupported. Using them leads to undefined behavior. */
   virtual void item(const QPatternist::Item &item);

 private:
   inline void startFormattingContent();
   Q_DECLARE_PRIVATE(QXmlFormatter)
};

QT_END_NAMESPACE


#endif
