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

#ifndef XML_QXML_H
#define XML_QXML_H

#include <qtextstream.h>
#include <qfile.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qscopedpointer.h>

class QXmlNamespaceSupport;
class QXmlAttributes;
class QXmlContentHandler;
class QXmlDefaultHandler;
class QXmlDTDHandler;
class QXmlEntityResolver;
class QXmlErrorHandler;
class QXmlLexicalHandler;
class QXmlDeclHandler;
class QXmlInputSource;
class QXmlLocator;
class QXmlNamespaceSupport;
class QXmlParseException;

class QXmlReader;
class QXmlSimpleReader;

class QXmlSimpleReaderPrivate;
class QXmlNamespaceSupportPrivate;

class QXmlInputSourcePrivate;
class QXmlParseExceptionPrivate;
class QXmlLocatorPrivate;

// SAX Namespace Support
//

class Q_XML_EXPORT QXmlNamespaceSupport
{
 public:
   QXmlNamespaceSupport();

   QXmlNamespaceSupport(const QXmlNamespaceSupport &) = delete;
   QXmlNamespaceSupport &operator=(const QXmlNamespaceSupport &) = delete;

   ~QXmlNamespaceSupport();

   void setPrefix(const QString &prefix, const QString &uri);

   QString prefix(const QString &uri) const;
   QString uri(const QString &prefix) const;

   void splitName(const QString &qname, QString &prefix, QString &localname) const;
   void processName(const QString &qname, bool isAttribute, QString &nsuri, QString &localname) const;
   QStringList prefixes() const;
   QStringList prefixes(const QString &uri) const;

   void pushContext();
   void popContext();
   void reset();

 private:
   QXmlNamespaceSupportPrivate *d;

   friend class QXmlSimpleReaderPrivate;
};

//
// SAX Attributes
//

class Q_XML_EXPORT QXmlAttributes
{
 public:
   QXmlAttributes() {}
   virtual ~QXmlAttributes() {}

   int index(const QString &qName) const;
   int index(const QString &uri, const QString &localPart) const;
   int length() const;
   int count() const;
   QString localName(int index) const;
   QString qName(int index) const;
   QString uri(int index) const;
   QString type(int index) const;
   QString type(const QString &qName) const;
   QString type(const QString &uri, const QString &localName) const;
   QString value(int index) const;
   QString value(const QString &qName) const;
   QString value(const QString &uri, const QString &localName) const;

   void clear();
   void append(const QString &qName, const QString &uri, const QString &localPart, const QString &value);

 private:
   struct Attribute {
      QString qname, uri, localname, value;
   };

   typedef QList<Attribute> AttributeList;
   AttributeList attList;
};

class Q_XML_EXPORT QXmlInputSource
{
 public:
   QXmlInputSource();
   QXmlInputSource(QIODevice *device);
   virtual ~QXmlInputSource();

   virtual void setData(const QString &data);
   virtual void setData(const QByteArray &data);
   virtual void fetchData();
   virtual QString data() const;
   virtual QChar next();
   virtual void reset();

   static const ushort EndOfData;
   static const ushort EndOfDocument;

 protected:
   virtual QString fromRawData(const QByteArray &data, bool beginning = false);

 private:
   void init();
   QXmlInputSourcePrivate *d;
};

class Q_XML_EXPORT QXmlParseException
{
 public:
   explicit QXmlParseException(const QString &name = QString(), int c = -1, int l = -1,
                               const QString &p = QString(), const QString &s = QString());
   QXmlParseException(const QXmlParseException &other);
   ~QXmlParseException();

   int columnNumber() const;
   int lineNumber() const;
   QString publicId() const;
   QString systemId() const;
   QString message() const;

 private:
   QScopedPointer<QXmlParseExceptionPrivate> d;
};

class Q_XML_EXPORT QXmlReader
{
 public:
   virtual ~QXmlReader() {}
   virtual bool feature(const QString &name, bool *ok = nullptr) const = 0;
   virtual void setFeature(const QString &name, bool value) = 0;
   virtual bool hasFeature(const QString &name) const = 0;
   virtual void *property(const QString &name, bool *ok = nullptr) const = 0;
   virtual void setProperty(const QString &name, void *value) = 0;
   virtual bool hasProperty(const QString &name) const = 0;
   virtual void setEntityResolver(QXmlEntityResolver *handler) = 0;
   virtual QXmlEntityResolver *entityResolver() const = 0;
   virtual void setDTDHandler(QXmlDTDHandler *handler) = 0;
   virtual QXmlDTDHandler *DTDHandler() const = 0;
   virtual void setContentHandler(QXmlContentHandler *handler) = 0;
   virtual QXmlContentHandler *contentHandler() const = 0;
   virtual void setErrorHandler(QXmlErrorHandler *handler) = 0;
   virtual QXmlErrorHandler *errorHandler() const = 0;
   virtual void setLexicalHandler(QXmlLexicalHandler *handler) = 0;
   virtual QXmlLexicalHandler *lexicalHandler() const = 0;
   virtual void setDeclHandler(QXmlDeclHandler *handler) = 0;
   virtual QXmlDeclHandler *declHandler() const = 0;
   virtual bool parse(const QXmlInputSource &input) = 0;
   virtual bool parse(const QXmlInputSource *input) = 0;
};

class Q_XML_EXPORT QXmlSimpleReader : public QXmlReader
{
 public:
   QXmlSimpleReader();

   QXmlSimpleReader(const QXmlSimpleReader &) = delete;
   QXmlSimpleReader &operator=(const QXmlSimpleReader &) = delete;

   virtual ~QXmlSimpleReader();

   bool feature(const QString &name, bool *ok = nullptr) const override;
   void setFeature(const QString &name, bool enable) override;
   bool hasFeature(const QString &name) const override;

   void *property(const QString &name, bool *ok = nullptr) const override;
   void setProperty(const QString &name, void *value) override;
   bool hasProperty(const QString &name) const override;

   void setEntityResolver(QXmlEntityResolver *handler) override;
   QXmlEntityResolver *entityResolver() const override;
   void setDTDHandler(QXmlDTDHandler *handler) override;
   QXmlDTDHandler *DTDHandler() const override;
   void setContentHandler(QXmlContentHandler *handler) override;
   QXmlContentHandler *contentHandler() const override;
   void setErrorHandler(QXmlErrorHandler *handler) override;
   QXmlErrorHandler *errorHandler() const override;
   void setLexicalHandler(QXmlLexicalHandler *handler) override;
   QXmlLexicalHandler *lexicalHandler() const override;
   void setDeclHandler(QXmlDeclHandler *handler) override;
   QXmlDeclHandler *declHandler() const override;

   bool parse(const QXmlInputSource &input) override;
   bool parse(const QXmlInputSource *input) override;
   virtual bool parse(const QXmlInputSource *input, bool incremental);
   virtual bool parseContinue();

 private:
   Q_DECLARE_PRIVATE(QXmlSimpleReader)
   QScopedPointer<QXmlSimpleReaderPrivate> d_ptr;

   friend class QXmlSimpleReaderLocator;
};

//
// SAX Locator
//

class Q_XML_EXPORT QXmlLocator
{
 public:
   QXmlLocator();
   virtual ~QXmlLocator();

   virtual int columnNumber() const = 0;
   virtual int lineNumber() const = 0;
   //    QString getPublicId() const
   //    QString getSystemId() const
};

//
// SAX handler classes
//

class Q_XML_EXPORT QXmlContentHandler
{
 public:
   virtual ~QXmlContentHandler() {}
   virtual void setDocumentLocator(QXmlLocator *locator) = 0;
   virtual bool startDocument() = 0;
   virtual bool endDocument() = 0;
   virtual bool startPrefixMapping(const QString &prefix, const QString &uri) = 0;
   virtual bool endPrefixMapping(const QString &prefix) = 0;
   virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName,
                             const QXmlAttributes &atts) = 0;
   virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) = 0;
   virtual bool characters(const QString &ch) = 0;
   virtual bool ignorableWhitespace(const QString &ch) = 0;
   virtual bool processingInstruction(const QString &target, const QString &data) = 0;
   virtual bool skippedEntity(const QString &name) = 0;
   virtual QString errorString() const = 0;
};

class Q_XML_EXPORT QXmlErrorHandler
{
 public:
   virtual ~QXmlErrorHandler() {}
   virtual bool warning(const QXmlParseException &exception) = 0;
   virtual bool error(const QXmlParseException &exception) = 0;
   virtual bool fatalError(const QXmlParseException &exception) = 0;
   virtual QString errorString() const = 0;
};

class Q_XML_EXPORT QXmlDTDHandler
{
 public:
   virtual ~QXmlDTDHandler() {}
   virtual bool notationDecl(const QString &name, const QString &publicId, const QString &systemId) = 0;
   virtual bool unparsedEntityDecl(const QString &name, const QString &publicId, const QString &systemId,
                                   const QString &notationName) = 0;
   virtual QString errorString() const = 0;
};

class Q_XML_EXPORT QXmlEntityResolver
{
 public:
   virtual ~QXmlEntityResolver() {}
   virtual bool resolveEntity(const QString &publicId, const QString &systemId, QXmlInputSource *&inputSource) = 0;
   virtual QString errorString() const = 0;
};

class Q_XML_EXPORT QXmlLexicalHandler
{
 public:
   virtual ~QXmlLexicalHandler() {}
   virtual bool startDTD(const QString &name, const QString &publicId, const QString &systemId) = 0;
   virtual bool endDTD() = 0;
   virtual bool startEntity(const QString &name) = 0;
   virtual bool endEntity(const QString &name) = 0;
   virtual bool startCDATA() = 0;
   virtual bool endCDATA() = 0;
   virtual bool comment(const QString &ch) = 0;
   virtual QString errorString() const = 0;
};

class Q_XML_EXPORT QXmlDeclHandler
{
 public:
   virtual ~QXmlDeclHandler() {}
   virtual bool attributeDecl(const QString &eName, const QString &aName, const QString &type, const QString &valueDefault,
                              const QString &value) = 0;

   virtual bool internalEntityDecl(const QString &name, const QString &value) = 0;
   virtual bool externalEntityDecl(const QString &name, const QString &publicId, const QString &systemId) = 0;
   virtual QString errorString() const = 0;

   // TODO: Conform to SAX by adding elementDecl
};


class Q_XML_EXPORT QXmlDefaultHandler : public QXmlContentHandler, public QXmlErrorHandler, public QXmlDTDHandler,
   public QXmlEntityResolver, public QXmlLexicalHandler, public QXmlDeclHandler
{
 public:
   QXmlDefaultHandler()
   {
   }

   QXmlDefaultHandler(const QXmlDefaultHandler &) = delete;
   QXmlDefaultHandler &operator=(const QXmlDefaultHandler &) = delete;

   virtual ~QXmlDefaultHandler()
   {
   }

   void setDocumentLocator(QXmlLocator *locator) override;
   bool startDocument() override;
   bool endDocument() override;
   bool startPrefixMapping(const QString &prefix, const QString &uri) override;
   bool endPrefixMapping(const QString &prefix) override;
   bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName,
                     const QXmlAttributes &atts) override;
   bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override;
   bool characters(const QString &ch) override;
   bool ignorableWhitespace(const QString &ch) override;
   bool processingInstruction(const QString &target, const QString &data) override ;
   bool skippedEntity(const QString &name) override;

   bool warning(const QXmlParseException &exception) override;
   bool error(const QXmlParseException &exception) override;
   bool fatalError(const QXmlParseException &exception) override;

   bool notationDecl(const QString &name, const QString &publicId, const QString &systemId) override;
   bool unparsedEntityDecl(const QString &name, const QString &publicId, const QString &systemId,
                           const QString &notationName) override;

   bool resolveEntity(const QString &publicId, const QString &systemId, QXmlInputSource *&inputSource) override;

   bool startDTD(const QString &name, const QString &publicId, const QString &systemId) override;
   bool endDTD() override;
   bool startEntity(const QString &name) override;
   bool endEntity(const QString &name) override;
   bool startCDATA() override;
   bool endCDATA() override;
   bool comment(const QString &ch) override;

   bool attributeDecl(const QString &eName, const QString &aName, const QString &type, const QString &valueDefault,
                      const QString &value) override;

   bool internalEntityDecl(const QString &name, const QString &value) override;
   bool externalEntityDecl(const QString &name, const QString &publicId, const QString &systemId) override;

   QString errorString() const override;
};

inline int QXmlAttributes::count() const
{
   return length();
}

#endif // QXML_H
