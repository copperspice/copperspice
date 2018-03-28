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

#ifndef QXMLSTREAM_H
#define QXMLSTREAM_H

#include <qiodevice.h>
#include <qstring.h>
#include <qscopedpointer.h>
#include <qvector.h>

class QXmlStreamAttributes;
class QXmlStreamReaderPrivate;
class QXmlStreamWriterPrivate;

class Q_CORE_EXPORT QXmlStringView
{
 public:
   QXmlStringView()
   {}

   QXmlStringView(const QString &str)
      : m_string(str), m_begin(m_string.begin()), m_end(m_string.end())
   {}

   QXmlStringView(const QStringView8 &view)
      : m_string(view), m_begin(m_string.begin()), m_end(m_string.end())
   {}

   ~QXmlStringView()
   {}

   void clear() {
      m_string.clear();
      m_begin = m_string.begin();
      m_end   = m_string.end();
   }

   operator QStringView8() const {
      return QStringView8(m_begin, m_end);
   }

   QString::const_iterator begin() const {
      return m_begin;
   }

   QString::const_iterator end() const {
      return m_end;
   }

 private:
   QString m_string;

   QString::const_iterator m_begin;
   QString::const_iterator m_end;
};

class Q_CORE_EXPORT QXmlStreamAttribute
{
 public:
   QXmlStreamAttribute();
   QXmlStreamAttribute(const QString &qualifiedName, const QString &value);
   QXmlStreamAttribute(const QString &namespaceUri, const QString &name, const QString &value);
   QXmlStreamAttribute(const QXmlStreamAttribute &);
   QXmlStreamAttribute &operator=(const QXmlStreamAttribute &);

   ~QXmlStreamAttribute();

   inline QStringView8 namespaceUri() const {
      return m_namespaceUri;
   }

   inline QStringView8 name() const {
      return m_name;
   }

   inline QStringView8 qualifiedName() const {
      return m_qualifiedName;
   }

   inline QStringView8 prefix() const {
      auto iter_qualName = m_qualifiedName.end();
      auto iter_name     = m_name.begin();

      if (iter_name != m_name.end()) {
         ++iter_name;

         while (iter_name != m_name.end()) {

            if (iter_qualName == m_qualifiedName.begin()) {
               break;
            }

            --iter_qualName;
            ++iter_name;
         }
      }

      return QStringView8(m_qualifiedName.begin(), iter_qualName);
   }

   inline QStringView8 value() const {
      return m_value;
   }

   inline bool isDefault() const {
      return m_isDefault;
   }

   inline bool operator==(const QXmlStreamAttribute &other) const {
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

   inline bool operator!=(const QXmlStreamAttribute &other) const {
      return ! operator==(other);
   }

  private:
   QXmlStringView m_name;
   QXmlStringView m_namespaceUri;
   QXmlStringView m_qualifiedName;
   QXmlStringView m_value;

   uint m_isDefault : 1;

   friend class QXmlStreamReaderPrivate;
   friend class QXmlStreamAttributes;
};

Q_DECLARE_TYPEINFO(QXmlStreamAttribute, Q_MOVABLE_TYPE);

class Q_CORE_EXPORT QXmlStreamAttributes : public QVector<QXmlStreamAttribute>
{
 public:
   inline QXmlStreamAttributes() {}

   QStringView8 value(const QString &namespaceUri, const QString &name) const;
   QStringView8 value(const QString &qualifiedName) const;

   void append(const QString &namespaceUri, const QString &name, const QString &value);
   void append(const QString &qualifiedName, const QString &value);

   inline bool hasAttribute(const QString &qualifiedName) const {
      return ! value(qualifiedName).isEmpty();
   }

   inline bool hasAttribute(const QString &namespaceUri, const QString &name) const {
      return ! value(namespaceUri, name).isEmpty();
   }

   using QVector<QXmlStreamAttribute>::append;
};

class Q_CORE_EXPORT QXmlStreamNamespaceDeclaration
{
 public:
   QXmlStreamNamespaceDeclaration();
   QXmlStreamNamespaceDeclaration(const QXmlStreamNamespaceDeclaration &);
   QXmlStreamNamespaceDeclaration(const QString &prefix, const QString &namespaceUri);
   ~QXmlStreamNamespaceDeclaration();

   QXmlStreamNamespaceDeclaration &operator=(const QXmlStreamNamespaceDeclaration &);
   inline QStringView8 prefix() const {
      return m_prefix;
   }

   inline QStringView8 namespaceUri() const {
      return m_namespaceUri;
   }

   inline bool operator==(const QXmlStreamNamespaceDeclaration &other) const {
      return (prefix() == other.prefix() && namespaceUri() == other.namespaceUri());
   }   inline bool operator!=(const QXmlStreamNamespaceDeclaration &other) const {
      return !operator==(other);
   }

 private:
   QXmlStringView m_prefix;
   QXmlStringView m_namespaceUri;

   friend class QXmlStreamReaderPrivate;
};

Q_DECLARE_TYPEINFO(QXmlStreamNamespaceDeclaration, Q_MOVABLE_TYPE);
typedef QVector<QXmlStreamNamespaceDeclaration> QXmlStreamNamespaceDeclarations;

class Q_CORE_EXPORT QXmlStreamNotationDeclaration
{
 public:
   QXmlStreamNotationDeclaration();
   QXmlStreamNotationDeclaration(const QXmlStreamNotationDeclaration &);
   QXmlStreamNotationDeclaration &operator=(const QXmlStreamNotationDeclaration &);

   ~QXmlStreamNotationDeclaration();

   inline QStringView8 name() const {
      return m_name;
   }

   inline QStringView8 systemId() const {
      return m_systemId;
   }

   inline QStringView8 publicId() const {
      return m_publicId;
   }

   inline bool operator==(const QXmlStreamNotationDeclaration &other) const {
      return (name() == other.name() && systemId() == other.systemId()
              && publicId() == other.publicId());
   }

   inline bool operator!=(const QXmlStreamNotationDeclaration &other) const {
      return !operator==(other);
   }

 private:
   QXmlStringView m_name, m_systemId, m_publicId;

   friend class QXmlStreamReaderPrivate;
};

Q_DECLARE_TYPEINFO(QXmlStreamNotationDeclaration, Q_MOVABLE_TYPE);
typedef QVector<QXmlStreamNotationDeclaration> QXmlStreamNotationDeclarations;

class Q_CORE_EXPORT QXmlStreamEntityDeclaration
{
 public:
   QXmlStreamEntityDeclaration();
   QXmlStreamEntityDeclaration(const QXmlStreamEntityDeclaration &);
   QXmlStreamEntityDeclaration &operator=(const QXmlStreamEntityDeclaration &);

   ~QXmlStreamEntityDeclaration();

   inline QStringView8 name() const {
      return m_name;
   }

   inline QStringView8 notationName() const {
      return m_notationName;
   }

   inline QStringView8 systemId() const {
      return m_systemId;
   }

   inline QStringView8 publicId() const {
      return m_publicId;
   }

   inline QStringView8 value() const {
      return m_value;
   }
   inline bool operator==(const QXmlStreamEntityDeclaration &other) const {
      return (name() == other.name()
              && notationName() == other.notationName()
              && systemId() == other.systemId()
              && publicId() == other.publicId()
              && value() == other.value());
   }

   inline bool operator!=(const QXmlStreamEntityDeclaration &other) const {
      return !operator==(other);
   }

 private:
   QXmlStringView m_name, m_notationName, m_systemId, m_publicId, m_value;

   friend class QXmlStreamReaderPrivate;
};

Q_DECLARE_TYPEINFO(QXmlStreamEntityDeclaration, Q_MOVABLE_TYPE);
typedef QVector<QXmlStreamEntityDeclaration> QXmlStreamEntityDeclarations;

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

   void setNamespaceProcessing(bool);
   bool namespaceProcessing() const;

   inline bool isStartDocument() const {
      return tokenType() == StartDocument;
   }

   inline bool isEndDocument() const {
      return tokenType() == EndDocument;
   }

   inline bool isStartElement() const {
      return tokenType() == StartElement;
   }

   inline bool isEndElement() const {
      return tokenType() == EndElement;
   }

   inline bool isCharacters() const {
      return tokenType() == Characters;
   }

   bool isWhitespace() const;

   bool isCDATA() const;

   inline bool isComment() const {
      return tokenType() == Comment;
   }

   inline bool isDTD() const {
      return tokenType() == DTD;
   }
   inline bool isEntityReference() const {
      return tokenType() == EntityReference;
   }

   inline bool isProcessingInstruction() const {
      return tokenType() == ProcessingInstruction;
   }

   bool isStandaloneDocument() const;
   QStringView8 documentVersion() const;
   QStringView8 documentEncoding() const;

   qint64 lineNumber() const;
   qint64 columnNumber() const;
   qint64 characterOffset() const;

   QXmlStreamAttributes attributes() const;

   QString readElementText(ReadElementTextBehaviour behaviour);
   QString readElementText();

   QStringView8 name() const;
   QStringView8 namespaceUri() const;
   QStringView8 qualifiedName() const;
   QStringView8 prefix() const;

   QStringView8 processingInstructionTarget() const;
   QStringView8 processingInstructionData() const;

   QStringView8 text() const;

   QXmlStreamNamespaceDeclarations namespaceDeclarations() const;
   void addExtraNamespaceDeclaration(const QXmlStreamNamespaceDeclaration &extraNamespaceDeclaraction);
   void addExtraNamespaceDeclarations(const QXmlStreamNamespaceDeclarations &extraNamespaceDeclaractions);
   QXmlStreamNotationDeclarations notationDeclarations() const;
   QXmlStreamEntityDeclarations entityDeclarations() const;
   QStringView8 dtdName() const;
   QStringView8 dtdPublicId() const;
   QStringView8 dtdSystemId() const;

   void raiseError(const QString &message = QString());
   QString errorString() const;
   Error error() const;

   inline bool hasError() const {
      return error() != NoError;
   }

   void setEntityResolver(QXmlStreamEntityResolver *resolver);
   QXmlStreamEntityResolver *entityResolver() const;

 private:
   Q_DISABLE_COPY(QXmlStreamReader)
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
   ~QXmlStreamWriter();

   void setDevice(QIODevice *device);
   QIODevice *device() const;

   void setCodec(QTextCodec *codec);
   void setCodec(const char *codecName);
   QTextCodec *codec() const;

   void setAutoFormatting(bool);
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
   Q_DISABLE_COPY(QXmlStreamWriter)
   Q_DECLARE_PRIVATE(QXmlStreamWriter)
   QScopedPointer<QXmlStreamWriterPrivate> d_ptr;
};

#endif
