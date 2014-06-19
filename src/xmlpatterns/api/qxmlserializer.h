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

#ifndef QXMLSERIALIZER_H
#define QXMLSERIALIZER_H

#include <QtXmlPatterns/QAbstractXmlReceiver>

QT_BEGIN_NAMESPACE

class QIODevice;
class QTextCodec;
class QXmlQuery;
class QXmlSerializerPrivate;

class Q_XMLPATTERNS_EXPORT QXmlSerializer : public QAbstractXmlReceiver
{
 public:
   QXmlSerializer(const QXmlQuery &query, QIODevice *outputDevice);

   virtual void namespaceBinding(const QXmlName &nb);

   virtual void characters(const QStringRef &value);
   virtual void comment(const QString &value);

   virtual void startElement(const QXmlName &name);

   virtual void endElement();

   virtual void attribute(const QXmlName &name, const QStringRef &value);

   virtual void processingInstruction(const QXmlName &name, const QString &value);

   virtual void atomicValue(const QVariant &value);

   virtual void startDocument();
   virtual void endDocument();
   virtual void startOfSequence();
   virtual void endOfSequence();

   QIODevice *outputDevice() const;

   void setCodec(const QTextCodec *codec);
   const QTextCodec *codec() const;

   /* The members below are internal, not part of the public API, and
    * unsupported. Using them leads to undefined behavior. */
   virtual void item(const QPatternist::Item &item);

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

QT_END_NAMESPACE

#endif
