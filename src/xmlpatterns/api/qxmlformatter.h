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

#ifndef QXMLFORMATTER_H
#define QXMLFORMATTER_H

#include <qstringfwd.h>
#include <qxmlserializer.h>

class QIODevice;
class QTextCodec;
class QXmlQuery;
class QXmlFormatterPrivate;

class Q_XMLPATTERNS_EXPORT QXmlFormatter : public QXmlSerializer
{
 public:
   QXmlFormatter(const QXmlQuery &query, QIODevice *outputDevice);

   void characters(QStringView value) override;
   void comment(const QString &value) override;
   void startElement(const QXmlName &name) override;
   void endElement() override;

   void attribute(const QXmlName &name, QStringView value) override;
   void processingInstruction(const QXmlName &name, const QString &value) override;

   void atomicValue(const QVariant &value) override;
   void startDocument() override;
   void endDocument() override;
   void startOfSequence() override;
   void endOfSequence() override;

   int indentationDepth() const;
   void setIndentationDepth(int depth);

   /* The members below are internal, not part of the public API, and
    * unsupported. Using them leads to undefined behavior. */
   void item(const QPatternist::Item &item) override;

 private:
   inline void startFormattingContent();
   Q_DECLARE_PRIVATE(QXmlFormatter)
};

#endif
