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

#ifndef QXMLSERIALIZER_H
#define QXMLSERIALIZER_H

#include <qabstractxmlreceiver.h>

class QIODevice;
class QTextCodec;
class QXmlQuery;
class QXmlSerializerPrivate;

class Q_XMLPATTERNS_EXPORT QXmlSerializer : public QAbstractXmlReceiver
{
 public:
   QXmlSerializer(const QXmlQuery &query, QIODevice *outputDevice);

   void namespaceBinding(const QXmlName &nb) override;

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

   QIODevice *outputDevice() const;

   void setCodec(const QTextCodec *codec);
   const QTextCodec *codec() const;

   /* The members below are internal, not part of the public API, and
    * unsupported. Using them leads to undefined behavior. */
   void item(const QPatternist::Item &item) override;

 protected:
   QXmlSerializer(QAbstractXmlReceiverPrivate *d);

 private:
   inline bool isBindingInScope(const QXmlName nb) const;

   /**
    * Where in the document the QXmlSerializer is currently working.
    */
   enum State {
      /**
       * Before the document element. This is the XML prolog where the
       * XML declaration, and possibly comments and processing
       * instructions are found.
       */
      BeforeDocumentElement,

      /**
       * This is inside the document element, at any level.
       */
      InsideDocumentElement
   };

   /**
    * If the current state is neither BeforeDocumentElement or
    * AfterDocumentElement.
    */
   inline bool atDocumentRoot() const;

   /**
    * Closes any open element start tag. Must be called before outputting
    * any element content.
    */
   inline void startContent();

   /**
    * Escapes content intended as text nodes for elements.
    */
   void writeEscaped(const QString &toEscape);

   /**
    * Identical to writeEscaped(), but also escapes quotes.
    */
   inline void writeEscapedAttribute(const QString &toEscape);

   /**
    * Writes out @p name.
    */
   inline void write(const QXmlName &name);

   inline void write(const char *const chars);
   /**
    * Encodes and writes out @p content.
    */
   inline void write(const QString &content);

   Q_DECLARE_PRIVATE(QXmlSerializer)
};

#endif
