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

#ifndef QXMLSTREAM_H
#define QXMLSTREAM_H

#include <qiodevice.h>
#include <qstring.h>
#include <qscopedpointer.h>
#include <qvector.h>

class QXmlStreamAttributes;
class QXmlStreamReaderPrivate;
class QXmlStreamWriterPrivate;

class Q_CORE_EXPORT QXmlStreamAttribute
{
 public:
   QXmlStreamAttribute();

   QXmlStreamAttribute(const QString &qualifiedName, const QString &value);
   QXmlStreamAttribute(const QString &namespaceUri, const QString &name, const QString &value);

   QXmlStreamAttribute(const QXmlStreamAttribute &other);
   QXmlStreamAttribute &operator=(const QXmlStreamAttribute &other);

   ~QXmlStreamAttribute();

   QStringView namespaceUri() const {
      return m_namespaceUri;
   }

   QStringView name() const {
      return m_name;
   }

   QStringView qualifiedName() const {
      return m_qualifiedName;
   }

   QStringView prefix() const {

      if (m_qualifiedName.isEmpty()) {
         return QStringView();
      }

      auto iter = m_qualifiedName.end() - 1;
      auto max  = m_name.size();

      for (int i = 0; i < max; ++i) {
         if (iter == m_qualifiedName.begin()) {
            break;
         }

         --iter;
      }

      return QStringView(m_qualifiedName.begin(), iter);
   }

   QStringView value() const {
      return m_value;
   }

   bool isDefault() const {
      return m_isDefault;
   }

   bool operator==(const QXmlStreamAttribute &other) const {
      bool retval = (value() == other.value());

      if (retval) {
         if (namespaceUri().isEmpty()) {
            retval = (qualifiedName() == other.qualifiedName());

         } else {
            retval = (namespaceUri() == other.namespaceUri() && name() == other.name());

         }
      }

      return retval;
   }

   bool operator!=(const QXmlStreamAttribute &other) const {
      return !operator==(other);
   }

 private:
   QString m_name;
   QString m_namespaceUri;
   QString m_qualifiedName;
   QString m_value;

   uint m_isDefault : 1;

   friend class QXmlStreamReaderPrivate;
   friend class QXmlStreamAttributes;
};

class Q_CORE_EXPORT QXmlStreamAttributes : public QVector<QXmlStreamAttribute>
{
 public:
   QXmlStreamAttributes()
   {
   }

   QStringView value(const QString &namespaceUri, const QString &name) const;
   QStringView value(const QString &qualifiedName) const;

   void append(const QString &namespaceUri, const QString &name, const QString &value);
   void append(const QString &qualifiedName, const QString &value);

   bool hasAttribute(const QString &qualifiedName) const {
      return ! value(qualifiedName).isEmpty();
   }

   bool hasAttribute(const QString &namespaceUri, const QString &name) const {
      return ! value(namespaceUri, name).isEmpty();
   }

   using QVector<QXmlStreamAttribute>::append;
};

class Q_CORE_EXPORT QXmlStreamNamespaceDeclaration
{
 public:
   QXmlStreamNamespaceDeclaration();

   QXmlStreamNamespaceDeclaration(const QString &prefix, const QString &namespaceUri);
   QXmlStreamNamespaceDeclaration(const QXmlStreamNamespaceDeclaration &other);

   ~QXmlStreamNamespaceDeclaration();

   QStringView prefix() const {
      return m_prefix;
   }

   QStringView namespaceUri() const {
      return m_namespaceUri;
   }

   QXmlStreamNamespaceDeclaration &operator=(const QXmlStreamNamespaceDeclaration &other);

   bool operator==(const QXmlStreamNamespaceDeclaration &other) const {
      return (prefix() == other.prefix() && namespaceUri() == other.namespaceUri());
   }

   bool operator!=(const QXmlStreamNamespaceDeclaration &other) const {
      return !operator==(other);
   }

 private:
   QString m_prefix;
   QString m_namespaceUri;

   friend class QXmlStreamReaderPrivate;
};

using QXmlStreamNamespaceDeclarations = QVector<QXmlStreamNamespaceDeclaration>;

class Q_CORE_EXPORT QXmlStreamNotationDeclaration
{
 public:
   QXmlStreamNotationDeclaration();

   QXmlStreamNotationDeclaration(const QXmlStreamNotationDeclaration &other);
   QXmlStreamNotationDeclaration &operator=(const QXmlStreamNotationDeclaration &other);

   ~QXmlStreamNotationDeclaration();

   QStringView name() const {
      return m_name;
   }

   QStringView systemId() const {
      return m_systemId;
   }

   QStringView publicId() const {
      return m_publicId;
   }

   bool operator==(const QXmlStreamNotationDeclaration &other) const {
      return (name() == other.name() && systemId() == other.systemId() && publicId() == other.publicId());
   }

   bool operator!=(const QXmlStreamNotationDeclaration &other) const {
      return ! operator==(other);
   }

 private:
   QString m_name;
   QString m_systemId;
   QString m_publicId;

   friend class QXmlStreamReaderPrivate;
};

using QXmlStreamNotationDeclarations = QVector<QXmlStreamNotationDeclaration>;

class Q_CORE_EXPORT QXmlStreamEntityDeclaration
{
 public:
   QXmlStreamEntityDeclaration();

   QXmlStreamEntityDeclaration(const QXmlStreamEntityDeclaration &other);
   QXmlStreamEntityDeclaration &operator=(const QXmlStreamEntityDeclaration &other);

   ~QXmlStreamEntityDeclaration();

   QStringView name() const {
      return m_name;
   }

   QStringView notationName() const {
      return m_notationName;
   }

   QStringView systemId() const {
      return m_systemId;
   }

   QStringView publicId() const {
      return m_publicId;
   }

   QStringView value() const {
      return m_value;
   }

   bool operator==(const QXmlStreamEntityDeclaration &other) const {
      return (name() == other.name() && notationName() == other.notationName() && systemId() == other.systemId()
            && publicId() == other.publicId() && value() == other.value());
   }

   bool operator!=(const QXmlStreamEntityDeclaration &other) const {
      return !operator==(other);
   }

 private:
   QString m_name;
   QString m_notationName;
   QString m_systemId;
   QString m_publicId;
   QString m_value;

   friend class QXmlStreamReaderPrivate;
};

using QXmlStreamEntityDeclarations = QVector<QXmlStreamEntityDeclaration>;

class Q_CORE_EXPORT QXmlStreamEntityResolver
{
 public:
   virtual ~QXmlStreamEntityResolver();

   virtual QString resolveEntity(const QString &publicId, const QString &systemId);
   virtual QString resolveUndeclaredEntity(const QString &name);
};

class Q_CORE_EXPORT QXmlStreamReader
{
 public:
   enum TokenType {
      NoToken = 0,
      Invalid,
      StartDocument,
      EndDocument,
      StartElement,
      EndElement,
      Characters,
      Comment,
      DTD,
      EntityReference,
      ProcessingInstruction
   };

   enum ReadElementTextBehaviour {
      ErrorOnUnexpectedElement,
      IncludeChildElements,
      SkipChildElements
   };

   enum Error {
      NoError,
      UnexpectedElementError,
      CustomError,
      NotWellFormedError,
      PrematureEndOfDocumentError
   };

   QXmlStreamReader();
   QXmlStreamReader(QIODevice *device);
   QXmlStreamReader(const QByteArray &data);
   QXmlStreamReader(const QString &data);
   QXmlStreamReader(const char *data);

   QXmlStreamReader(const QXmlStreamReader &) = delete;
   QXmlStreamReader &operator=(const QXmlStreamReader &) = delete;

   ~QXmlStreamReader();

   void setDevice(QIODevice *device);
   QIODevice *device() const;
   void addData(const QByteArray &data);
   void addData(const QString &data);
   void addData(const char *data);
   void clear();

   bool atEnd() const;
   TokenType readNext();

   bool readNextStartElement();
   void skipCurrentElement();

   TokenType tokenType() const;
   QString tokenString() const;

   void setNamespaceProcessing(bool enable);
   bool namespaceProcessing() const;

   bool isStartDocument() const {
      return tokenType() == StartDocument;
   }

   bool isEndDocument() const {
      return tokenType() == EndDocument;
   }

   bool isStartElement() const {
      return tokenType() == StartElement;
   }

   bool isEndElement() const {
      return tokenType() == EndElement;
   }

   bool isCharacters() const {
      return tokenType() == Characters;
   }

   bool isWhitespace() const;

   bool isCDATA() const;

   bool isComment() const {
      return tokenType() == Comment;
   }

   bool isDTD() const {
      return tokenType() == DTD;
   }

   bool isEntityReference() const {
      return tokenType() == EntityReference;
   }

   bool isProcessingInstruction() const {
      return tokenType() == ProcessingInstruction;
   }

   bool isStandaloneDocument() const;
   QStringView documentVersion() const;
   QStringView documentEncoding() const;

   qint64 lineNumber() const;
   qint64 columnNumber() const;
   qint64 characterOffset() const;

   QXmlStreamAttributes attributes() const;

   QString readElementText(ReadElementTextBehaviour behavior);
   QString readElementText();

   QStringView name() const;
   QStringView namespaceUri() const;
   QStringView qualifiedName() const;
   QStringView prefix() const;

   QStringView processingInstructionTarget() const;
   QStringView processingInstructionData() const;

   QStringView text() const;

   QXmlStreamNamespaceDeclarations namespaceDeclarations() const;
   void addExtraNamespaceDeclaration(const QXmlStreamNamespaceDeclaration &declaration);
   void addExtraNamespaceDeclarations(const QXmlStreamNamespaceDeclarations &declarations);
   QXmlStreamNotationDeclarations notationDeclarations() const;
   QXmlStreamEntityDeclarations entityDeclarations() const;
   QStringView dtdName() const;
   QStringView dtdPublicId() const;
   QStringView dtdSystemId() const;

   void raiseError(const QString &message = QString());
   QString errorString() const;
   Error error() const;

   bool hasError() const {
      return error() != NoError;
   }

   void setEntityResolver(QXmlStreamEntityResolver *resolver);
   QXmlStreamEntityResolver *entityResolver() const;

 private:
   Q_DECLARE_PRIVATE(QXmlStreamReader)
   QScopedPointer<QXmlStreamReaderPrivate> d_ptr;

};

class Q_CORE_EXPORT QXmlStreamWriter
{
 public:
   QXmlStreamWriter();
   QXmlStreamWriter(QIODevice *device);
   QXmlStreamWriter(QByteArray *array);
   QXmlStreamWriter(QString *string);

   QXmlStreamWriter(const QXmlStreamWriter &) = delete;
   QXmlStreamWriter &operator=(const QXmlStreamWriter &) = delete;

   ~QXmlStreamWriter();

   void setDevice(QIODevice *device);
   QIODevice *device() const;

   void setCodec(QTextCodec *codec);
   void setCodec(const char *codecName);
   QTextCodec *codec() const;

   void setAutoFormatting(bool enable);
   bool autoFormatting() const;

   void setAutoFormattingIndent(int spacesOrTabs);
   int autoFormattingIndent() const;

   void writeAttribute(const QString &qualifiedName, const QString &value);
   void writeAttribute(const QString &namespaceUri, const QString &name, const QString &value);
   void writeAttribute(const QXmlStreamAttribute &attribute);
   void writeAttributes(const QXmlStreamAttributes &attributes);

   void writeCDATA(const QString &text);
   void writeCharacters(const QString &text);
   void writeComment(const QString &text);

   void writeDTD(const QString &dtd);

   void writeEmptyElement(const QString &qualifiedName);
   void writeEmptyElement(const QString &namespaceUri, const QString &name);

   void writeTextElement(const QString &qualifiedName, const QString &text);
   void writeTextElement(const QString &namespaceUri, const QString &name, const QString &text);

   void writeEndDocument();
   void writeEndElement();

   void writeEntityReference(const QString &name);
   void writeNamespace(const QString &namespaceUri, const QString &prefix = QString());
   void writeDefaultNamespace(const QString &namespaceUri);
   void writeProcessingInstruction(const QString &target, const QString &data = QString());

   void writeStartDocument();
   void writeStartDocument(const QString &version);
   void writeStartDocument(const QString &version, bool standalone);
   void writeStartElement(const QString &qualifiedName);
   void writeStartElement(const QString &namespaceUri, const QString &name);

   void writeCurrentToken(const QXmlStreamReader &reader);
   bool hasError() const;

 private:
   Q_DECLARE_PRIVATE(QXmlStreamWriter)
   QScopedPointer<QXmlStreamWriterPrivate> d_ptr;
};

#endif
