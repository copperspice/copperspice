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

#include <qxml.h>
#include <qtextcodec.h>
#include <qbuffer.h>
#include <qregularexpression.h>
#include <qmap.h>
#include <qhash.h>
#include <qstack.h>
#include <qdebug.h>

// Error strings for the XML reader
#define XMLERR_OK                         cs_mark_tr("QXml", "no error occurred")
#define XMLERR_ERRORBYCONSUMER            cs_mark_tr("QXml", "error triggered by consumer")
#define XMLERR_UNEXPECTEDEOF              cs_mark_tr("QXml", "unexpected end of file")
#define XMLERR_MORETHANONEDOCTYPE         cs_mark_tr("QXml", "more than one document type definition")
#define XMLERR_ERRORPARSINGELEMENT        cs_mark_tr("QXml", "error occurred while parsing element")
#define XMLERR_TAGMISMATCH                cs_mark_tr("QXml", "tag mismatch")
#define XMLERR_ERRORPARSINGCONTENT        cs_mark_tr("QXml", "error occurred while parsing content")
#define XMLERR_UNEXPECTEDCHARACTER        cs_mark_tr("QXml", "unexpected character")
#define XMLERR_INVALIDNAMEFORPI           cs_mark_tr("QXml", "invalid name for processing instruction")
#define XMLERR_VERSIONEXPECTED            cs_mark_tr("QXml", "version expected while reading the XML declaration")
#define XMLERR_WRONGVALUEFORSDECL         cs_mark_tr("QXml", "wrong value for standalone declaration")
#define XMLERR_EDECLORSDDECLEXPECTED      cs_mark_tr("QXml", "encoding declaration or standalone declaration expected while reading the XML declaration")
#define XMLERR_SDDECLEXPECTED             cs_mark_tr("QXml", "standalone declaration expected while reading the XML declaration")
#define XMLERR_ERRORPARSINGDOCTYPE        cs_mark_tr("QXml", "error occurred while parsing document type definition")
#define XMLERR_LETTEREXPECTED             cs_mark_tr("QXml", "letter is expected")
#define XMLERR_ERRORPARSINGCOMMENT        cs_mark_tr("QXml", "error occurred while parsing comment")
#define XMLERR_ERRORPARSINGREFERENCE      cs_mark_tr("QXml", "error occurred while parsing reference")
#define XMLERR_INTERNALGENERALENTITYINDTD cs_mark_tr("QXml", "internal general entity reference not allowed in DTD")
#define XMLERR_EXTERNALGENERALENTITYINAV  cs_mark_tr("QXml", "external parsed general entity reference not allowed in attribute value")
#define XMLERR_EXTERNALGENERALENTITYINDTD cs_mark_tr("QXml", "external parsed general entity reference not allowed in DTD")
#define XMLERR_UNPARSEDENTITYREFERENCE    cs_mark_tr("QXml", "unparsed entity reference in wrong context")
#define XMLERR_RECURSIVEENTITIES          cs_mark_tr("QXml", "recursive entities")
#define XMLERR_ERRORINTEXTDECL            cs_mark_tr("QXml", "error in the text declaration of an external entity")

// the constants for the lookup table
static const signed char cltWS      =  0; // white space
static const signed char cltPer     =  1; // %
static const signed char cltAmp     =  2; // &
static const signed char cltGt      =  3; // >
static const signed char cltLt      =  4; // <
static const signed char cltSlash   =  5; // /
static const signed char cltQm      =  6; // ?
static const signed char cltEm      =  7; // !
static const signed char cltDash    =  8; // -
static const signed char cltCB      =  9; // ]
static const signed char cltOB      = 10; // [
static const signed char cltEq      = 11; // =
static const signed char cltDq      = 12; // "
static const signed char cltSq      = 13; // '
static const signed char cltUnknown = 14;

// sneaky way to let QDom know where the skipped entity occurred
// this variable means the code is not reentrant
bool qt_xml_skipped_entity_in_content;

// character lookup table
static const signed char charLookupTable[256] = {
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x00 - 0x07
   cltUnknown, // 0x08
   cltWS,      // 0x09 \t
   cltWS,      // 0x0A \n
   cltUnknown, // 0x0B
   cltUnknown, // 0x0C
   cltWS,      // 0x0D \r
   cltUnknown, // 0x0E
   cltUnknown, // 0x0F
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x17 - 0x16
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x18 - 0x1F
   cltWS,      // 0x20 Space
   cltEm,      // 0x21 !
   cltDq,      // 0x22 "
   cltUnknown, // 0x23
   cltUnknown, // 0x24
   cltPer,     // 0x25 %
   cltAmp,     // 0x26 &
   cltSq,      // 0x27 '
   cltUnknown, // 0x28
   cltUnknown, // 0x29
   cltUnknown, // 0x2A
   cltUnknown, // 0x2B
   cltUnknown, // 0x2C
   cltDash,    // 0x2D -
   cltUnknown, // 0x2E
   cltSlash,   // 0x2F /
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x30 - 0x37
   cltUnknown, // 0x38
   cltUnknown, // 0x39
   cltUnknown, // 0x3A
   cltUnknown, // 0x3B
   cltLt,      // 0x3C <
   cltEq,      // 0x3D =
   cltGt,      // 0x3E >
   cltQm,      // 0x3F ?
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x40 - 0x47
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x48 - 0x4F
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x50 - 0x57
   cltUnknown, // 0x58
   cltUnknown, // 0x59
   cltUnknown, // 0x5A
   cltOB,      // 0x5B [
   cltUnknown, // 0x5C
   cltCB,      // 0x5D]
   cltUnknown, // 0x5E
   cltUnknown, // 0x5F
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x60 - 0x67
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x68 - 0x6F
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x70 - 0x77
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x78 - 0x7F
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x80 - 0x87
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x88 - 0x8F
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x90 - 0x97
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0x98 - 0x9F
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xA0 - 0xA7
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xA8 - 0xAF
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xB0 - 0xB7
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xB8 - 0xBF
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xC0 - 0xC7
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xC8 - 0xCF
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xD0 - 0xD7
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xD8 - 0xDF
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xE0 - 0xE7
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xE8 - 0xEF
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, // 0xF0 - 0xF7
   cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown, cltUnknown  // 0xF8 - 0xFF
};

static bool stripTextDecl(QString &str)
{
   QString textDeclStart("<?xml");

   if (str.startsWith(textDeclStart)) {

      QRegularExpression8 textDecl(QString::fromLatin1(
                          "^<\\?xml\\s+"
                          "(version\\s*=\\s*((['\"])[-a-zA-Z0-9_.:]+\\3))?"
                          "\\s*"
                          "(encoding\\s*=\\s*((['\"])[A-Za-z][-a-zA-Z0-9_.]*\\6))?"
                          "\\s*\\?>"));

      QString strTmp = str.replace(textDecl, "");

      if (strTmp.length() != str.length()) {
         return false;   // external entity has wrong TextDecl
      }

      str = strTmp;
   }

   return true;
}

class QXmlInputSourcePrivate
{
 public:
   QIODevice   *inputDevice;
   QTextStream *inputStream;

   QString str;

   QString::const_iterator m_position;
   QString::const_iterator m_end;

   bool nextReturnedEndOfData;

#ifndef QT_NO_TEXTCODEC
   QTextDecoder *encMapper;
#endif

   QByteArray encodingDeclBytes;
   QString encodingDeclChars;
   bool lookingForEncodingDecl;
};

class QXmlParseExceptionPrivate
{
 public:
   QXmlParseExceptionPrivate()
      : column(-1), line(-1) {
   }

   QXmlParseExceptionPrivate(const QXmlParseExceptionPrivate &other)
      : msg(other.msg), column(other.column), line(other.line),
        pub(other.pub), sys(other.sys) {
   }

   QString msg;
   int column;
   int line;
   QString pub;
   QString sys;

};

class QXmlLocatorPrivate
{
};

class QXmlSimpleReaderPrivate
{
 public:
   ~QXmlSimpleReaderPrivate();

 private:
   // used by parseReference() and parsePEReference()
   enum EntityRecognitionContext {
      InContent,
      InAttributeValue,
      InEntityValue,
      InDTD
   };

   // used for standalone declaration
   enum Standalone {
      Yes,
      No,
      Unknown
   };

   QXmlSimpleReaderPrivate(QXmlSimpleReader *reader);
   void initIncrementalParsing();

   // used to determine if elements are correctly nested
   QStack<QString> tags;

   // used for entity declarations
   struct ExternParameterEntity {
      ExternParameterEntity()
      { }

      ExternParameterEntity(const QString &p, const QString &s)
         : publicId(p), systemId(s)
      { }

      QString publicId;
      QString systemId;
   };

   struct ExternEntity {
      ExternEntity() {}

      ExternEntity(const QString &p, const QString &s, const QString &n)
         : publicId(p), systemId(s), notation(n) {}

      QString publicId;
      QString systemId;
      QString notation;
   };

   QMap<QString, ExternParameterEntity> externParameterEntities;
   QMap<QString, QString> parameterEntities;
   QMap<QString, ExternEntity> externEntities;
   QMap<QString, QString> entities;

   // used for parsing of entity references
   struct XmlRef {
      XmlRef()
         : index(0)
      {}

      XmlRef(const QString &_name, const QString &_value)
         : name(_name), value(_value), index(0)
      {}

      bool isEmpty() const {
         return index == value.length();
      }

      QChar next() {
         return value.at(index++);
      }

      QString name;
      QString value;
      int index;
   };

   QStack<XmlRef> xmlRefStack;

   QString doctype; // only used for the doctype
   QString xmlVersion; // only used to store the version information
   QString encoding; // only used to store the encoding
   Standalone standalone; // used to store the value of the standalone declaration

   QString publicId; // used by parseExternalID() to store the public ID
   QString systemId; // used by parseExternalID() to store the system ID

   // Since publicId/systemId is used as temporary variables by parseExternalID(), it
   // might overwrite the PUBLIC/SYSTEM for the document we're parsing. In effect, we would
   // possibly send off an QXmlParseException that has the PUBLIC/SYSTEM of a entity declaration
   // instead of those of the current document.
   // Hence we have these two variables for storing the document's data.
   QString thisPublicId;
   QString thisSystemId;

   QString attDeclEName; // use by parseAttlistDecl()
   QString attDeclAName; // use by parseAttlistDecl()

   // flags for some features support
   bool useNamespaces;
   bool useNamespacePrefixes;
   bool reportWhitespaceCharData;
   bool reportEntities;

   // used to build the attribute list
   QXmlAttributes attList;

   // used in QXmlSimpleReader::parseContent() to decide whether character
   // data was read
   bool contentCharDataRead;

   // helper classes
   QScopedPointer<QXmlLocator> locator;
   QXmlNamespaceSupport namespaceSupport;

   // error string
   QString error;

   // arguments for parse functions (this is needed to allow incremental
   // parsing)
   bool parsePI_xmldecl;
   bool parseName_useRef;
   bool parseReference_charDataRead;
   EntityRecognitionContext parseReference_context;
   bool parseExternalID_allowPublicID;
   EntityRecognitionContext parsePEReference_context;
   QString parseString_s;

   // for incremental parsing
   struct ParseState {
      typedef bool (QXmlSimpleReaderPrivate::*ParseFunction)();
      ParseFunction function;
      int state;
   };
   QStack<ParseState> *parseStack;

   // used in parseProlog()
   bool xmldecl_possible;
   bool doctype_read;

   // used in parseDoctype()
   bool startDTDwasReported;

   // used in parseString()
   signed char Done;


   // variables
   QXmlContentHandler *contentHnd;
   QXmlErrorHandler   *errorHnd;
   QXmlDTDHandler     *dtdHnd;
   QXmlEntityResolver *entityRes;
   QXmlLexicalHandler *lexicalHnd;
   QXmlDeclHandler    *declHnd;

   QXmlInputSource    *inputSource;

   QChar c;                    // the character at reading position
   int   lineNr;               // number of line
   int   columnNr;             // position in line

   QString nameValue;          // only used for names
   QString refValue;           // only used for references
   QString stringValue;        // used for any other strings that are parsed

   QString emptyStr;

   QHash<QString, int> literalEntitySizes;

   // The entity at (QMap<QString,) referenced the entities at (QMap<QString,) (int>) times.
   QHash<QString, QHash<QString, int> > referencesToOtherEntities;
   QHash<QString, int> expandedSizes;

   // The limit to the amount of times the DTD parsing functions can be called
   // for the DTD currently being parsed.
   static constexpr int dtdRecursionLimit = 2;

   // The maximum amount of characters an entity value may contain, after expansion.
   static constexpr const int entityCharacterLimit = 1024;

   const QString &string();
   void stringClear();
   void stringAddC(QChar);
   void stringAddC() {
      stringAddC(c);
   }

   const QString &name();
   void nameClear();
   void nameAddC(QChar);

   void nameAddC() {
      nameAddC(c);
   }

   const QString &ref();
   void refClear();
   void refAddC(QChar);

   void refAddC() {
      refAddC(c);
   }

   // private functions
   bool eat_ws();
   bool next_eat_ws();

   void QT_FASTCALL next();
   bool atEnd();

   void init(const QXmlInputSource *i);
   void initData();

   bool entityExist(const QString &) const;

   bool parseBeginOrContinue(int state, bool incremental);

   bool parseProlog();
   bool parseElement();
   bool processElementEmptyTag();
   bool processElementETagBegin2();
   bool processElementAttribute();
   bool parseMisc();
   bool parseContent();

   bool parsePI();
   bool parseDoctype();
   bool parseComment();

   bool parseName();
   bool parseNmtoken();
   bool parseAttribute();
   bool parseReference();
   bool processReference();

   bool parseExternalID();
   bool parsePEReference();
   bool parseMarkupdecl();
   bool parseAttlistDecl();
   bool parseAttType();
   bool parseAttValue();
   bool parseElementDecl();
   bool parseNotationDecl();
   bool parseChoiceSeq();
   bool parseEntityDecl();
   bool parseEntityValue();

   bool parseString();

   bool insertXmlRef(const QString &, const QString &, bool);

   bool reportEndEntities();
   void reportParseError(const QString &error);

   typedef bool (QXmlSimpleReaderPrivate::*ParseFunction) ();
   void unexpectedEof(ParseFunction where, int state);
   void parseFailed(ParseFunction where, int state);
   void pushParseState(ParseFunction function, int state);
   bool isExpandedEntityValueTooLarge(QString *errorMessage);

   Q_DECLARE_PUBLIC(QXmlSimpleReader)
   QXmlSimpleReader *q_ptr;

   friend class QXmlSimpleReaderLocator;
};

constexpr int QXmlSimpleReaderPrivate::dtdRecursionLimit;

QXmlParseException::QXmlParseException(const QString &name, int c, int l, const QString &p, const QString &s)
   : d(new QXmlParseExceptionPrivate)
{
   d->msg = name;
   d->column = c;
   d->line = l;
   d->pub = p;
   d->sys = s;
}

QXmlParseException::QXmlParseException(const QXmlParseException &other) :
   d(new QXmlParseExceptionPrivate(*other.d))
{
}

QXmlParseException::~QXmlParseException()
{
}

QString QXmlParseException::message() const
{
   return d->msg;
}

int QXmlParseException::columnNumber() const
{
   return d->column;
}

int QXmlParseException::lineNumber() const
{
   return d->line;
}

QString QXmlParseException::publicId() const
{
   return d->pub;
}

QString QXmlParseException::systemId() const
{
   return d->sys;
}

QXmlLocator::QXmlLocator()
{
}

QXmlLocator::~QXmlLocator()
{
}

class QXmlSimpleReaderLocator : public QXmlLocator
{
 public:
   QXmlSimpleReaderLocator(QXmlSimpleReader *parent) {
      reader = parent;
   }
   ~QXmlSimpleReaderLocator() {
   }

   int columnNumber() const override {
      return (reader->d_ptr->columnNr == -1 ? -1 : reader->d_ptr->columnNr + 1);
   }

   int lineNumber() const override {
      return (reader->d_ptr->lineNr == -1 ? -1 : reader->d_ptr->lineNr + 1);
   }

   //    QString getPublicId()
   //    QString getSystemId()

 private:
   QXmlSimpleReader *reader;
};

typedef QMap<QString, QString> NamespaceMap;

class QXmlNamespaceSupportPrivate
{
 public:
   QXmlNamespaceSupportPrivate() {
      // the XML namespace
      ns.insert("xml", "http://www.w3.org/XML/1998/namespace");
   }

   ~QXmlNamespaceSupportPrivate() {
   }

   QStack<NamespaceMap> nsStack;
   NamespaceMap ns;
};

QXmlNamespaceSupport::QXmlNamespaceSupport()
{
   d = new QXmlNamespaceSupportPrivate;
}

QXmlNamespaceSupport::~QXmlNamespaceSupport()
{
   delete d;
}

void QXmlNamespaceSupport::setPrefix(const QString &pre, const QString &uri)
{
   if (pre.isEmpty()) {
      d->ns.insert("", uri);
   } else {
      d->ns.insert(pre, uri);
   }
}

QString QXmlNamespaceSupport::prefix(const QString &uri) const
{
   NamespaceMap::const_iterator itc, it = d->ns.constBegin();

   while ((itc = it) != d->ns.constEnd()) {
      ++it;
      if (*itc == uri && ! itc.key().isEmpty()) {
         return itc.key();
      }
   }

   return QString();
}

QString QXmlNamespaceSupport::uri(const QString &prefix) const
{
   return d->ns[prefix];
}

void QXmlNamespaceSupport::splitName(const QString &qname, QString &prefix, QString &localname) const
{
   int pos = qname.indexOf(':');

   if (pos == -1) {
      pos = qname.size();
   }

   prefix    = qname.left(pos);
   localname = qname.mid(pos + 1);
}

void QXmlNamespaceSupport::processName(const QString &qname, bool isAttribute, QString &nsuri, QString &localname) const
{
   auto iter = qname.indexOfFast(":");

   if (iter != qname.end()) {
      nsuri     = uri(QString(qname.begin(), iter));
      localname = QString(iter + 1, qname.end());

      return;
   }

   // there was no ':'
   nsuri.clear();

   // attributes do not take default namespace
   if (! isAttribute && ! d->ns.isEmpty()) {

      NamespaceMap::const_iterator first = d->ns.constBegin();

      if (first.key().isEmpty()) {
         // get default namespace
         nsuri = first.value();
      }
   }

   localname = qname;
}

QStringList QXmlNamespaceSupport::prefixes() const
{
   QStringList list;

   NamespaceMap::const_iterator itc, it = d->ns.constBegin();
   while ((itc = it) != d->ns.constEnd()) {
      ++it;
      if (!itc.key().isEmpty()) {
         list.append(itc.key());
      }
   }
   return list;
}

QStringList QXmlNamespaceSupport::prefixes(const QString &uri) const
{
   QStringList list;

   NamespaceMap::const_iterator itc, it = d->ns.constBegin();
   while ((itc = it) != d->ns.constEnd()) {
      ++it;
      if (*itc == uri && !itc.key().isEmpty()) {
         list.append(itc.key());
      }
   }
   return list;
}

void QXmlNamespaceSupport::pushContext()
{
   d->nsStack.push(d->ns);
}

void QXmlNamespaceSupport::popContext()
{
   d->ns.clear();
   if (!d->nsStack.isEmpty()) {
      d->ns = d->nsStack.pop();
   }
}

void QXmlNamespaceSupport::reset()
{
   QXmlNamespaceSupportPrivate *newD = new QXmlNamespaceSupportPrivate;
   delete d;
   d = newD;
}

int QXmlAttributes::index(const QString &qName) const
{
   for (int i = 0; i < attList.size(); ++i) {
      if (attList.at(i).qname == qName) {
         return i;
      }
   }
   return -1;
}

int QXmlAttributes::index(const QString &uri, const QString &localPart) const
{
   for (int i = 0; i < attList.size(); ++i) {
      const Attribute &att = attList.at(i);
      if (att.uri == uri && att.localname == localPart) {
         return i;
      }
   }
   return -1;
}

int QXmlAttributes::length() const
{
   return attList.count();
}

QString QXmlAttributes::localName(int index) const
{
   return attList.at(index).localname;
}

QString QXmlAttributes::qName(int index) const
{
   return attList.at(index).qname;
}

QString QXmlAttributes::uri(int index) const
{
   return attList.at(index).uri;
}

QString QXmlAttributes::type(int) const
{
   return QString("CDATA");
}

QString QXmlAttributes::type(const QString &) const
{
   return QString("CDATA");
}

QString QXmlAttributes::type(const QString &, const QString &) const
{
   return QString("CDATA");
}

QString QXmlAttributes::value(int index) const
{
   return attList.at(index).value;
}

QString QXmlAttributes::value(const QString &qName) const
{
   int i = index(qName);
   if (i == -1) {
      return QString();
   }
   return attList.at(i).value;
}

QString QXmlAttributes::value(const QString &uri, const QString &localName) const
{
   int i = index(uri, localName);
   if (i == -1) {
      return QString();
   }
   return attList.at(i).value;
}

void QXmlAttributes::clear()
{
   attList.clear();
}

void QXmlAttributes::append(const QString &qName, const QString &uri, const QString &localPart, const QString &value)
{
   Attribute att;
   att.qname = qName;
   att.uri = uri;
   att.localname = localPart;
   att.value = value;

   attList.append(att);
}

// the following two are guaranteed not to be a character
const ushort QXmlInputSource::EndOfData = 0xfffe;
const ushort QXmlInputSource::EndOfDocument = 0xffff;

void QXmlInputSource::init()
{
   d = new QXmlInputSourcePrivate;

   try {
      d->inputDevice = nullptr;
      d->inputStream = nullptr;

      setData(QString());
#ifndef QT_NO_TEXTCODEC
      d->encMapper = nullptr;
#endif
      d->nextReturnedEndOfData = true; // first call to next() will call fetchData()

      d->encodingDeclBytes.clear();
      d->encodingDeclChars.clear();
      d->lookingForEncodingDecl = true;

   } catch(...) {
      delete(d);
      throw;
   }
}

QXmlInputSource::QXmlInputSource()
{
   init();
}

QXmlInputSource::QXmlInputSource(QIODevice *dev)
{
   init();
   d->inputDevice = dev;
   if (dev->isOpen()) {
      d->inputDevice->setTextModeEnabled(false);
   }
}

QXmlInputSource::~QXmlInputSource()
{
   // must close the input device

#ifndef QT_NO_TEXTCODEC
   delete d->encMapper;
#endif

   delete d;
}

QChar QXmlInputSource::next()
{
   if (d->m_position == d->m_end) {

      if (d->nextReturnedEndOfData) {
         d->nextReturnedEndOfData = false;
         fetchData();

         if (d->m_position == d->m_end) {
            return EndOfDocument;
         }

         return next();
      }

      d->nextReturnedEndOfData = true;
      return EndOfData;
   }

   // QXmlInputSource has no way to signal encoding errors. The best we can do
   // is return EndOfDocument. We do *not* return EndOfData, because the reader
   // will then just call this function again to get the next char.

   QChar c = *(d->m_position);
   ++(d->m_position);

   if (c.unicode() == EndOfData) {
      c = EndOfDocument;
   }

   return c;
}

void QXmlInputSource::reset()
{
   d->nextReturnedEndOfData = false;
   d->m_position = d->str.begin();
}

QString QXmlInputSource::data() const
{
   if (d->nextReturnedEndOfData) {
      QXmlInputSource *that = const_cast<QXmlInputSource *>(this);
      that->d->nextReturnedEndOfData = false;
      that->fetchData();
   }

   return d->str;
}

void QXmlInputSource::setData(const QString &inputStr)
{
   d->str        = inputStr;
   d->m_position = d->str.begin();
   d->m_end      = d->str.end();

   d->nextReturnedEndOfData = false;
}

void QXmlInputSource::setData(const QByteArray &dat)
{
   setData(fromRawData(dat));
}

void QXmlInputSource::fetchData()
{
   static constexpr const int BufferSize = 1024;

   QByteArray rawData;

   if (d->inputDevice || d->inputStream) {
      QIODevice *device = d->inputDevice ? d->inputDevice : d->inputStream->device();

      if (!device) {
         if (d->inputStream && d->inputStream->string()) {
            QString *s = d->inputStream->string();
            rawData = QByteArray((const char *) s->constData(), s->size() * sizeof(QChar));
         }

      } else if (device->isOpen() || device->open(QIODevice::ReadOnly)) {
         rawData.resize(BufferSize);
         qint64 size = device->read(rawData.data(), BufferSize);

         if (size != -1) {
            // We don't want to give fromRawData() less than four bytes if we can avoid it.
            while (size < 4) {
               if (!device->waitForReadyRead(-1)) {
                  break;
               }

               int ret = device->read(rawData.data() + size, BufferSize - size);
               if (ret <= 0) {
                  break;
               }
               size += ret;
            }
         }

         rawData.resize(qMax(qint64(0), size));
      }

      /* We do this inside the "if (d->inputDevice ..." scope
       * because if we're not using a stream or device, that is,
       * the user set a QString manually, we don't want to set
       * d->str. */
      setData(fromRawData(rawData));
   }
}

#ifndef QT_NO_TEXTCODEC
static QString extractEncodingDecl(const QString &text, bool *needMoreText)
{
   *needMoreText = false;

   int l = text.length();
   QString snip = QString::fromLatin1("<?xml").left(l);
   if (l > 0 && !text.startsWith(snip)) {
      return QString();
   }

   int endPos = text.indexOf('>');
   if (endPos == -1) {
      *needMoreText = l < 255; // we won't look forever
      return QString();
   }

   int pos = text.indexOf("encoding");
   if (pos == -1 || pos >= endPos) {
      return QString();
   }

   while (pos < endPos) {
      ushort uc = text.at(pos).unicode();
      if (uc == '\'' || uc == '"') {
         break;
      }
      ++pos;
   }

   if (pos == endPos) {
      return QString();
   }

   QString encoding;
   ++pos;
   while (pos < endPos) {
      ushort uc = text.at(pos).unicode();
      if (uc == '\'' || uc == '"') {
         break;
      }
      encoding.append(uc);
      ++pos;
   }

   return encoding;
}
#endif // QT_NO_TEXTCODEC

QString QXmlInputSource::fromRawData(const QByteArray &data, bool beginning)
{
#ifdef QT_NO_TEXTCODEC
   (void) beginning;
   return QString::fromLatin1(data.constData(), data.size());

#else

   if (data.size() == 0) {
      return QString();
   }

   if (beginning) {
      delete d->encMapper;
      d->encMapper = nullptr;
   }

   int mib = 106; // UTF-8

   // This is the initial UTF codec we will read the encoding declaration with
   if (d->encMapper == nullptr) {
      d->encodingDeclBytes.clear();
      d->encodingDeclChars.clear();
      d->lookingForEncodingDecl = true;

      // look for byte order mark and read the first 5 characters
      if (data.size() >= 4) {
         uchar ch1 = data.at(0);
         uchar ch2 = data.at(1);
         uchar ch3 = data.at(2);
         uchar ch4 = data.at(3);

         if ((ch1 == 0 && ch2 == 0 && ch3 == 0xfe && ch4 == 0xff) ||
               (ch1 == 0xff && ch2 == 0xfe && ch3 == 0 && ch4 == 0)) {
            mib = 1017;   // UTF-32 with byte order mark

         } else if (ch1 == 0x3c && ch2 == 0x00 && ch3 == 0x00 && ch4 == 0x00) {
            mib = 1019;   // UTF-32LE

         } else if (ch1 == 0x00 && ch2 == 0x00 && ch3 == 0x00 && ch4 == 0x3c) {
            mib = 1018;   // UTF-32BE
         }
      }
      if (mib == 106 && data.size() >= 2) {
         uchar ch1 = data.at(0);
         uchar ch2 = data.at(1);

         if ((ch1 == 0xfe && ch2 == 0xff) || (ch1 == 0xff && ch2 == 0xfe)) {
            mib = 1015;   // UTF-16 with byte order mark

         } else if (ch1 == 0x3c && ch2 == 0x00) {
            mib = 1014;   // UTF-16LE

         } else if (ch1 == 0x00 && ch2 == 0x3c) {
            mib = 1013;   // UTF-16BE

         }
      }

      QTextCodec *codec = QTextCodec::codecForMib(mib);
      Q_ASSERT(codec);

      d->encMapper = codec->makeDecoder();
   }

   QString input = d->encMapper->toUnicode(data.constData(), data.size());

   if (d->lookingForEncodingDecl) {
      d->encodingDeclChars += input;

      bool needMoreText;
      QString encoding = extractEncodingDecl(d->encodingDeclChars, &needMoreText);

      if (!encoding.isEmpty()) {
         if (QTextCodec *codec = QTextCodec::codecForName(encoding.toLatin1())) {

            /* If the encoding is the same, we don't have to do toUnicode() all over again. */
            if (codec->mibEnum() != mib) {
               delete d->encMapper;
               d->encMapper = codec->makeDecoder();

               /* The variable input can potentially be large, so we deallocate
                * it before calling toUnicode() in order to avoid having two
                * large QStrings in memory simultaneously. */
               input.clear();

               // prime the decoder with the data so far
               d->encMapper->toUnicode(d->encodingDeclBytes.constData(), d->encodingDeclBytes.size());
               // now feed it the new data
               input = d->encMapper->toUnicode(data.constData(), data.size());
            }
         }
      }

      d->encodingDeclBytes += data;
      d->lookingForEncodingDecl = needMoreText;
   }

   return input;
#endif
}

void QXmlDefaultHandler::setDocumentLocator(QXmlLocator *)
{
}

bool QXmlDefaultHandler::startDocument()
{
   return true;
}

bool QXmlDefaultHandler::endDocument()
{
   return true;
}

bool QXmlDefaultHandler::startPrefixMapping(const QString &, const QString &)
{
   return true;
}

bool QXmlDefaultHandler::endPrefixMapping(const QString &)
{
   return true;
}

bool QXmlDefaultHandler::startElement(const QString &, const QString &,
                                      const QString &, const QXmlAttributes &)
{
   return true;
}

bool QXmlDefaultHandler::endElement(const QString &, const QString &,
                                    const QString &)
{
   return true;
}

bool QXmlDefaultHandler::characters(const QString &)
{
   return true;
}

bool QXmlDefaultHandler::ignorableWhitespace(const QString &)
{
   return true;
}

bool QXmlDefaultHandler::processingInstruction(const QString &,
      const QString &)
{
   return true;
}

bool QXmlDefaultHandler::skippedEntity(const QString &)
{
   return true;
}

bool QXmlDefaultHandler::warning(const QXmlParseException &)
{
   return true;
}

bool QXmlDefaultHandler::error(const QXmlParseException &)
{
   return true;
}

bool QXmlDefaultHandler::fatalError(const QXmlParseException &)
{
   return true;
}

bool QXmlDefaultHandler::notationDecl(const QString &, const QString &,
                                      const QString &)
{
   return true;
}

bool QXmlDefaultHandler::unparsedEntityDecl(const QString &, const QString &,
      const QString &, const QString &)
{
   return true;
}

bool QXmlDefaultHandler::resolveEntity(const QString &, const QString &, QXmlInputSource *&ret)
{
   ret = nullptr;
   return true;
}

QString QXmlDefaultHandler::errorString() const
{
   return QString::fromLatin1(XMLERR_ERRORBYCONSUMER);
}

bool QXmlDefaultHandler::startDTD(const QString &, const QString &, const QString &)
{
   return true;
}

bool QXmlDefaultHandler::endDTD()
{
   return true;
}

bool QXmlDefaultHandler::startEntity(const QString &)
{
   return true;
}

bool QXmlDefaultHandler::endEntity(const QString &)
{
   return true;
}

bool QXmlDefaultHandler::startCDATA()
{
   return true;
}

bool QXmlDefaultHandler::endCDATA()
{
   return true;
}

bool QXmlDefaultHandler::comment(const QString &)
{
   return true;
}

bool QXmlDefaultHandler::attributeDecl(const QString &, const QString &, const QString &, const QString &, const QString &)
{
   return true;
}

bool QXmlDefaultHandler::internalEntityDecl(const QString &, const QString &)
{
   return true;
}

bool QXmlDefaultHandler::externalEntityDecl(const QString &, const QString &, const QString &)
{
   return true;
}

bool QXmlSimpleReaderPrivate::atEnd()
{
   return (c.unicode() | 0x0001) == 0xffff;
}

void QXmlSimpleReaderPrivate::stringClear()
{
   stringValue.clear();
}

void QXmlSimpleReaderPrivate::nameClear()
{
   nameValue.clear();
}

void QXmlSimpleReaderPrivate::refClear()
{
   refValue.clear();
}

QXmlSimpleReaderPrivate::QXmlSimpleReaderPrivate(QXmlSimpleReader *reader)
{
   q_ptr = reader;
   parseStack = nullptr;

   locator.reset(new QXmlSimpleReaderLocator(reader));
   entityRes  = nullptr;
   dtdHnd     = nullptr;
   contentHnd = nullptr;
   errorHnd   = nullptr;
   lexicalHnd = nullptr;
   declHnd    = nullptr;

   // default feature settings
   useNamespaces            = true;
   useNamespacePrefixes     = false;
   reportWhitespaceCharData = true;
   reportEntities           = false;
}

QXmlSimpleReaderPrivate::~QXmlSimpleReaderPrivate()
{
   delete parseStack;
}

void QXmlSimpleReaderPrivate::initIncrementalParsing()
{
   if (parseStack) {
      parseStack->clear();
   } else {
      parseStack = new QStack<ParseState>;
   }
}

static inline bool is_S(QChar ch)
{
   ushort uc = ch.unicode();
   return (uc == ' ' || uc == '\t' || uc == '\n' || uc == '\r');
}

enum NameChar {
   NameBeginning,
   NameNotBeginning,
   NotName
};

static const char Begi = (char)NameBeginning;
static const char NtBg = (char)NameNotBeginning;
static const char NotN = (char)NotName;

static const char nameCharTable[128] = {
   // 0x00
   NotN, NotN, NotN, NotN, NotN, NotN, NotN, NotN,
   NotN, NotN, NotN, NotN, NotN, NotN, NotN, NotN,
   // 0x10
   NotN, NotN, NotN, NotN, NotN, NotN, NotN, NotN,
   NotN, NotN, NotN, NotN, NotN, NotN, NotN, NotN,
   // 0x20 (0x2D is '-', 0x2E is '.')
   NotN, NotN, NotN, NotN, NotN, NotN, NotN, NotN,
   NotN, NotN, NotN, NotN, NotN, NtBg, NtBg, NotN,
   // 0x30 (0x30..0x39 are '0'..'9', 0x3A is ':')
   NtBg, NtBg, NtBg, NtBg, NtBg, NtBg, NtBg, NtBg,
   NtBg, NtBg, Begi, NotN, NotN, NotN, NotN, NotN,
   // 0x40 (0x41..0x5A are 'A'..'Z')
   NotN, Begi, Begi, Begi, Begi, Begi, Begi, Begi,
   Begi, Begi, Begi, Begi, Begi, Begi, Begi, Begi,
   // 0x50 (0x5F is '_')
   Begi, Begi, Begi, Begi, Begi, Begi, Begi, Begi,
   Begi, Begi, Begi, NotN, NotN, NotN, NotN, Begi,
   // 0x60 (0x61..0x7A are 'a'..'z')
   NotN, Begi, Begi, Begi, Begi, Begi, Begi, Begi,
   Begi, Begi, Begi, Begi, Begi, Begi, Begi, Begi,
   // 0x70
   Begi, Begi, Begi, Begi, Begi, Begi, Begi, Begi,
   Begi, Begi, Begi, NotN, NotN, NotN, NotN, NotN
};

static inline NameChar fastDetermineNameChar(QChar ch)
{
   ushort uc = ch.unicode();
   if (!(uc & ~0x7f)) { // uc < 128
      return (NameChar)nameCharTable[uc];
   }

   QChar::Category cat = ch.category();
   // ### some these categories might be slightly wrong
   if ((cat >= QChar::Letter_Uppercase && cat <= QChar::Letter_Other)
         || cat == QChar::Number_Letter) {
      return NameBeginning;
   }
   if ((cat >= QChar::Number_DecimalDigit && cat <= QChar::Number_Other)
         || (cat >= QChar::Mark_NonSpacing && cat <= QChar::Mark_Enclosing)) {
      return NameNotBeginning;
   }
   return NotName;
}

static NameChar determineNameChar(QChar ch)
{
   return fastDetermineNameChar(ch);
}

QXmlSimpleReader::QXmlSimpleReader()
   : d_ptr(new QXmlSimpleReaderPrivate(this))
{
}

QXmlSimpleReader::~QXmlSimpleReader()
{
}

bool QXmlSimpleReader::feature(const QString &name, bool *ok) const
{
   const QXmlSimpleReaderPrivate *d = d_func();

   if (ok != nullptr) {
      *ok = true;
   }

   if (name == "http://xml.org/sax/features/namespaces") {
      return d->useNamespaces;

   } else if (name == "http://xml.org/sax/features/namespace-prefixes") {
      return d->useNamespacePrefixes;

   } else if (name == "http://copperspice.com/xml/features/report-whitespace-only-CharData") {
      return d->reportWhitespaceCharData;

   } else if (name == "http://copperspice.com/xml/features/report-start-end-entity") {
      return d->reportEntities;

   } else {
      qWarning("Unknown feature %s", name.toLatin1().data());

      if (ok != nullptr) {
         *ok = false;
      }
   }

   return false;
}

void QXmlSimpleReader::setFeature(const QString &name, bool enable)
{
   Q_D(QXmlSimpleReader);

   if (name == "http://xml.org/sax/features/namespaces") {
      d->useNamespaces = enable;

   } else if (name == "http://xml.org/sax/features/namespace-prefixes") {
      d->useNamespacePrefixes = enable;

   } else if (name == "http://copperspice.com/xml/features/report-whitespace-only-CharData") {
      d->reportWhitespaceCharData = enable;

   } else if (name == "http://copperspice.com/xml/features/report-start-end-entity") {
      d->reportEntities = enable;

   } else {
      qWarning("Unknown feature %s", name.toLatin1().data());
   }
}

bool QXmlSimpleReader::hasFeature(const QString &name) const
{
   if (name == "http://xml.org/sax/features/namespaces"
         || name == "http://xml.org/sax/features/namespace-prefixes"
         || name == "http://copperspice.com/xml/features/report-whitespace-only-CharData"
         || name == "http://copperspice.com/xml/features/report-start-end-entity") {

      return true;

   } else {
      return false;
   }
}

void *QXmlSimpleReader::property(const QString &, bool *ok) const
{
   if (ok != nullptr) {
      *ok = false;
   }

   return nullptr;
}

void QXmlSimpleReader::setProperty(const QString &, void *)
{
}

bool QXmlSimpleReader::hasProperty(const QString &) const
{
   return false;
}

void QXmlSimpleReader::setEntityResolver(QXmlEntityResolver *handler)
{
   Q_D(QXmlSimpleReader);
   d->entityRes = handler;
}

QXmlEntityResolver *QXmlSimpleReader::entityResolver() const
{
   const QXmlSimpleReaderPrivate *d = d_func();
   return d->entityRes;
}

void QXmlSimpleReader::setDTDHandler(QXmlDTDHandler *handler)
{
   Q_D(QXmlSimpleReader);
   d->dtdHnd = handler;
}

QXmlDTDHandler *QXmlSimpleReader::DTDHandler() const
{
   const QXmlSimpleReaderPrivate *d = d_func();
   return d->dtdHnd;
}

void QXmlSimpleReader::setContentHandler(QXmlContentHandler *handler)
{
   Q_D(QXmlSimpleReader);
   d->contentHnd = handler;
}

QXmlContentHandler *QXmlSimpleReader::contentHandler() const
{
   const QXmlSimpleReaderPrivate *d = d_func();
   return d->contentHnd;
}

void QXmlSimpleReader::setErrorHandler(QXmlErrorHandler *handler)
{
   Q_D(QXmlSimpleReader);
   d->errorHnd = handler;
}

QXmlErrorHandler *QXmlSimpleReader::errorHandler() const
{
   const QXmlSimpleReaderPrivate *d = d_func();
   return d->errorHnd;
}

void QXmlSimpleReader::setLexicalHandler(QXmlLexicalHandler *handler)
{
   Q_D(QXmlSimpleReader);
   d->lexicalHnd = handler;
}

QXmlLexicalHandler *QXmlSimpleReader::lexicalHandler() const
{
   const QXmlSimpleReaderPrivate *d = d_func();
   return d->lexicalHnd;
}


void QXmlSimpleReader::setDeclHandler(QXmlDeclHandler *handler)
{
   Q_D(QXmlSimpleReader);
   d->declHnd = handler;
}

QXmlDeclHandler *QXmlSimpleReader::declHandler() const
{
   const QXmlSimpleReaderPrivate *d = d_func();
   return d->declHnd;
}

bool QXmlSimpleReader::parse(const QXmlInputSource &input)
{
   return parse(&input, false);
}

bool QXmlSimpleReader::parse(const QXmlInputSource *input)
{
   return parse(input, false);
}

bool QXmlSimpleReader::parse(const QXmlInputSource *input, bool incremental)
{
   Q_D(QXmlSimpleReader);

   d->literalEntitySizes.clear();
   d->referencesToOtherEntities.clear();
   d->expandedSizes.clear();

   if (incremental) {
      d->initIncrementalParsing();

   } else {
      delete d->parseStack;
      d->parseStack = nullptr;
   }

   d->init(input);

   // call the handler
   if (d->contentHnd) {
      d->contentHnd->setDocumentLocator(d->locator.data());

      if (! d->contentHnd->startDocument()) {
         d->reportParseError(d->contentHnd->errorString());
         d->tags.clear();
         return false;
      }
   }

   qt_xml_skipped_entity_in_content = false;

   return d->parseBeginOrContinue(0, incremental);
}

bool QXmlSimpleReader::parseContinue()
{
   Q_D(QXmlSimpleReader);

   if (d->parseStack == nullptr || d->parseStack->isEmpty()) {
      return false;
   }

   d->initData();

   int state = d->parseStack->pop().state;
   return d->parseBeginOrContinue(state, true);
}

bool QXmlSimpleReaderPrivate::parseBeginOrContinue(int state, bool incremental)
{
   bool atEndOrig = atEnd();

   if (state == 0) {
      if (! parseProlog()) {
         if (incremental && error.isEmpty()) {
            pushParseState(nullptr, 0);
            return true;

         } else {
            tags.clear();
            return false;
         }
      }
      state = 1;
   }

   if (state == 1) {

      if (! parseElement()) {

         if (incremental && error.isEmpty()) {
            pushParseState(nullptr, 1);
            return true;

         } else {
            tags.clear();
            return false;
         }
      }

      state = 2;
   }

   // parse Misc*
   while (!atEnd()) {
      if (!parseMisc()) {
         if (incremental && error.isEmpty()) {
            pushParseState(nullptr, 2);
            return true;

         } else {
            tags.clear();
            return false;
         }
      }
   }

   if (!atEndOrig && incremental) {
      // we parsed something at all, so be prepared to come back later
      pushParseState(nullptr, 2);
      return true;
   }

   // is stack empty?
   if (! tags.isEmpty() && !error.isEmpty()) {
      reportParseError(QString::fromLatin1(XMLERR_UNEXPECTEDEOF));
      tags.clear();
      return false;
   }

   // call the handler
   if (contentHnd) {
      delete parseStack;
      parseStack = nullptr;

      if (!contentHnd->endDocument()) {
         reportParseError(contentHnd->errorString());
         return false;
      }
   }

   return true;
}

//
// The following private parse functions have another semantics for the return
// value: They return true if parsing has finished successfully (i.e. the end
// of the XML file must be reached!). If one of these functions return false,
// there is only an error when d->error.isEmpty() is also false.
//

/*
  For the incremental parsing, it is very important that the parse...()
  functions have a certain structure. Since it might be hard to understand how
  they work, here is a description of the layout of these functions:

    bool QXmlSimpleReader::parse...()
    {
(1)        const signed char Init             = 0;
        ...

(2)        const signed char Inp...           = 0;
        ...

(3)        static const signed char table[3][2] = {
        ...
        };
        signed char state;
        signed char input;

(4)        if (d->parseStack == 0 || d->parseStack->isEmpty()) {
(4a)        ...
        } else {
(4b)        ...
        }

        for (; ;) {
(5)            switch (state) {
            ...
            }

(6)
(6a)            if (atEnd()) {
                unexpectedEof(&QXmlSimpleReader::parseNmtoken, state);
                return false;
            }
(6b)            if (determineNameChar(c) != NotName) {
            ...
            }
(7)            state = table[state][input];

(8)            switch (state) {
            ...
            }
        }
    }

  Explanation:
  ad 1: constants for the states (used in the transition table)
  ad 2: constants for the input (used in the transition table)
  ad 3: the transition table for the state machine
  ad 4: test if we are in a parseContinue() step
        a) if no, do inititalizations
        b) if yes, restore the state and call parse functions recursively
  ad 5: Do some actions according to the state; from the logical execution
        order, this code belongs after 8 (see there for an explanation)
  ad 6: Check the character that is at the actual "cursor" position:
        a) If we reached the EOF, report either error or push the state (in the
           case of incremental parsing).
        b) Otherwise, set the input character constant for the transition
           table.
  ad 7: Get the new state according to the input that was read.
  ad 8: Do some actions according to the state. The last line in every case
        statement reads new data (i.e. it move the cursor). This can also be
        done by calling another parse...() function. If you need processing for
        this state after that, you have to put it into the switch statement 5.
        This ensures that you have a well defined re-entry point, when you ran
        out of data.
*/

/*
  Parses the prolog [22].
*/

bool QXmlSimpleReaderPrivate::parseProlog()
{
   const signed char Init             = 0;
   const signed char EatWS            = 1; // eat white spaces
   const signed char Lt               = 2; // '<' read
   const signed char Em               = 3; // '!' read
   const signed char DocType          = 4; // read doctype
   const signed char Comment          = 5; // read comment
   const signed char CommentR         = 6; // same as Comment, but already reported
   const signed char PInstr           = 7; // read PI
   const signed char PInstrR          = 8; // same as PInstr, but already reported
   const signed char Done             = 9;

   const signed char InpWs            = 0;
   const signed char InpLt            = 1; // <
   const signed char InpQm            = 2; // ?
   const signed char InpEm            = 3; // !
   const signed char InpD             = 4; // D
   const signed char InpDash          = 5; // -
   const signed char InpUnknown       = 6;

   static const signed char table[9][7] = {
      /*  InpWs   InpLt  InpQm  InpEm  InpD      InpDash  InpUnknown */
      { EatWS,  Lt,    -1,    -1,    -1,       -1,       -1      }, // Init
      { -1,     Lt,    -1,    -1,    -1,       -1,       -1      }, // EatWS
      { -1,     -1,    PInstr, Em,    Done,     -1,       Done    }, // Lt
      { -1,     -1,    -1,    -1,    DocType,  Comment,  -1      }, // Em
      { EatWS,  Lt,    -1,    -1,    -1,       -1,       -1      }, // DocType
      { EatWS,  Lt,    -1,    -1,    -1,       -1,       -1      }, // Comment
      { EatWS,  Lt,    -1,    -1,    -1,       -1,       -1      }, // CommentR
      { EatWS,  Lt,    -1,    -1,    -1,       -1,       -1      }, // PInstr
      { EatWS,  Lt,    -1,    -1,    -1,       -1,       -1      }  // PInstrR
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      xmldecl_possible = true;
      doctype_read = false;
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseProlog (cont) in state %d", state);
#endif

      if (! parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();

#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }

         if (! (this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseProlog, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case DocType:
            if (doctype_read) {
               reportParseError(QString::fromLatin1(XMLERR_MORETHANONEDOCTYPE));
               return false;
            } else {
               doctype_read = false;
            }
            break;

         case Comment:
            if (lexicalHnd) {
               if (! lexicalHnd->comment(string())) {
                  reportParseError(lexicalHnd->errorString());
                  return false;
               }
            }
            state = CommentR;
            break;

         case PInstr:
            // call the handler
            if (contentHnd) {
               if (xmldecl_possible && !xmlVersion.isEmpty()) {
                  QString value("version='");
                  value += xmlVersion;
                  value += '\'';

                  if (!encoding.isEmpty()) {
                     value += " encoding='";
                     value += encoding;
                     value += '\'';
                  }

                  if (standalone == QXmlSimpleReaderPrivate::Yes) {
                     value += " standalone='yes'";

                  } else if (standalone == QXmlSimpleReaderPrivate::No) {
                     value += " standalone='no'";
                  }

                  if (!contentHnd->processingInstruction("xml", value)) {
                     reportParseError(contentHnd->errorString());
                     return false;
                  }
               } else {
                  if (!contentHnd->processingInstruction(name(), string())) {
                     reportParseError(contentHnd->errorString());
                     return false;
                  }
               }
            }
            // XML declaration only on first position possible
            xmldecl_possible = false;
            state = PInstrR;
            break;

         case Done:
            return true;

         case -1:
            reportParseError(QString::fromLatin1(XMLERR_ERRORPARSINGELEMENT));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseProlog, state);
         return false;
      }

      if (is_S(c)) {
         input = InpWs;

      } else if (c == '<') {
         input = InpLt;

      } else if (c == '?') {
         input = InpQm;

      } else if (c == '!') {
         input = InpEm;

      } else if (c == 'D') {
         input = InpD;

      } else if (c == '-') {
         input = InpDash;

      } else {
         input = InpUnknown;
      }
      state = table[state][input];

      switch (state) {
         case EatWS:
            // XML declaration only on first position possible
            xmldecl_possible = false;
            if (! eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseProlog, state);
               return false;
            }
            break;

         case Lt:
            next();
            break;

         case Em:
            // XML declaration only on first position possible
            xmldecl_possible = false;
            next();
            break;

         case DocType:
            if (! parseDoctype()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseProlog, state);
               return false;
            }
            break;

         case Comment:
         case CommentR:
            if (! parseComment()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseProlog, state);
               return false;
            }
            break;

         case PInstr:
         case PInstrR:
            parsePI_xmldecl = xmldecl_possible;
            if (!parsePI()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseProlog, state);
               return false;
            }
            break;
      }
   }

   return false;
}

/*
  Parse an element [39].

  Precondition: the opening '<' is already read.
*/
bool QXmlSimpleReaderPrivate::parseElement()
{
   const int Init             =  0;
   const int ReadName         =  1;
   const int Ws1              =  2;
   const int STagEnd          =  3;
   const int STagEnd2         =  4;
   const int ETagBegin        =  5;
   const int ETagBegin2       =  6;
   const int Ws2              =  7;
   const int EmptyTag         =  8;
   const int Attrib           =  9;
   const int AttribPro        = 10; // like Attrib, but processAttribute was already called
   const int Ws3              = 11;
   const int Done             = 12;

   const int InpWs            = 0; // whitespace
   const int InpNameBe        = 1; // NameBeginning
   const int InpGt            = 2; // >
   const int InpSlash         = 3; // /
   const int InpUnknown       = 4;

   static const int table[12][5] = {
      /*  InpWs      InpNameBe    InpGt        InpSlash     InpUnknown */
      { -1,        ReadName,    -1,          -1,          -1        }, // Init
      { Ws1,       Attrib,      STagEnd,     EmptyTag,    -1        }, // ReadName
      { -1,        Attrib,      STagEnd,     EmptyTag,    -1        }, // Ws1
      { STagEnd2,  STagEnd2,    STagEnd2,    STagEnd2,    STagEnd2  }, // STagEnd
      { -1,        -1,          -1,          ETagBegin,   -1        }, // STagEnd2
      { -1,        ETagBegin2,  -1,          -1,          -1        }, // ETagBegin
      { Ws2,       -1,          Done,        -1,          -1        }, // ETagBegin2
      { -1,        -1,          Done,        -1,          -1        }, // Ws2
      { -1,        -1,          Done,        -1,          -1        }, // EmptyTag
      { Ws3,       Attrib,      STagEnd,     EmptyTag,    -1        }, // Attrib
      { Ws3,       Attrib,      STagEnd,     EmptyTag,    -1        }, // AttribPro
      { -1,        Attrib,      STagEnd,     EmptyTag,    -1        }  // Ws3
   };
   int state;
   int input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseElement (cont) in state %d", state);
#endif

      if (! parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();

#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }

         if (! (this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseElement, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case ReadName:
            // store it on the stack
            tags.push(name());

            // empty the attributes
            attList.clear();

            if (useNamespaces) {
               namespaceSupport.pushContext();
            }
            break;

         case ETagBegin2:
            if (!processElementETagBegin2()) {
               return false;
            }
            break;

         case Attrib:
            if (!processElementAttribute()) {
               return false;
            }
            state = AttribPro;
            break;

         case Done:
            return true;

         case -1:
            reportParseError(QString::fromLatin1(XMLERR_ERRORPARSINGELEMENT));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseElement, state);
         return false;
      }

      if (fastDetermineNameChar(c) == NameBeginning) {
         input = InpNameBe;

      } else if (c == '>') {
         input = InpGt;

      } else if (is_S(c)) {
         input = InpWs;

      } else if (c == '/') {
         input = InpSlash;

      } else {
         input = InpUnknown;
      }

      state = table[state][input];

      switch (state) {
         case ReadName:
            parseName_useRef = false;

            if (!parseName()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElement, state);
               return false;
            }
            break;

         case Ws1:
         case Ws2:
         case Ws3:
            if (! eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElement, state);
               return false;
            }
            break;

         case STagEnd:
            // call the handler
            if (contentHnd) {
               const QString &tagsTop = tags.top();
               if (useNamespaces) {
                  QString uri, lname;
                  namespaceSupport.processName(tagsTop, false, uri, lname);
                  if (!contentHnd->startElement(uri, lname, tagsTop, attList)) {
                     reportParseError(contentHnd->errorString());
                     return false;
                  }
               } else {
                  if (!contentHnd->startElement(QString(), QString(), tagsTop, attList)) {
                     reportParseError(contentHnd->errorString());
                     return false;
                  }
               }
            }
            next();
            break;

         case STagEnd2:
            if (! parseContent()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElement, state);
               return false;
            }
            break;

         case ETagBegin:
            next();
            break;

         case ETagBegin2:
            // get the name of the tag
            parseName_useRef = false;
            if (!parseName()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElement, state);
               return false;
            }
            break;

         case EmptyTag:
            if  (tags.isEmpty()) {
               reportParseError(QString::fromLatin1(XMLERR_TAGMISMATCH));
               return false;
            }
            if (!processElementEmptyTag()) {
               return false;
            }
            next();
            break;

         case Attrib:
         case AttribPro:
            // get name and value of attribute
            if (!parseAttribute()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElement, state);
               return false;
            }
            break;

         case Done:
            next();
            break;
      }
   }

   return false;
}

bool QXmlSimpleReaderPrivate::processElementEmptyTag()
{
   QString uri, lname;
   // pop the stack and call the handler
   if (contentHnd) {
      if (useNamespaces) {
         // report startElement first...
         namespaceSupport.processName(tags.top(), false, uri, lname);
         if (! contentHnd->startElement(uri, lname, tags.top(), attList)) {
            reportParseError(contentHnd->errorString());
            return false;
         }

         // ... followed by endElement...
         if (! contentHnd->endElement(uri, lname, tags.pop())) {
            reportParseError(contentHnd->errorString());
            return false;
         }

         // ... followed by endPrefixMapping
         QStringList prefixesBefore, prefixesAfter;
         if (contentHnd) {
            prefixesBefore = namespaceSupport.prefixes();
         }

         namespaceSupport.popContext();

         // call the handler for prefix mapping
         prefixesAfter = namespaceSupport.prefixes();

         for (QStringList::iterator it = prefixesBefore.begin(); it != prefixesBefore.end(); ++it) {
            if (!prefixesAfter.contains(*it)) {
               if (!contentHnd->endPrefixMapping(*it)) {
                  reportParseError(contentHnd->errorString());
                  return false;
               }
            }
         }

      } else {
         // report startElement first...
         if (!contentHnd->startElement(QString(), QString(), tags.top(), attList)) {
            reportParseError(contentHnd->errorString());
            return false;
         }
         // ... followed by endElement
         if (!contentHnd->endElement(QString(), QString(), tags.pop())) {
            reportParseError(contentHnd->errorString());
            return false;
         }
      }

   } else {
      tags.pop_back();
      namespaceSupport.popContext();
   }

   return true;
}

bool QXmlSimpleReaderPrivate::processElementETagBegin2()
{
   const QString &name = QXmlSimpleReaderPrivate::name();

   // pop the stack and compare it with the name
   if (tags.pop() != name) {
      reportParseError(QString::fromLatin1(XMLERR_TAGMISMATCH));
      return false;
   }

   // call the handler
   if (contentHnd) {
      QString uri, lname;

      if (useNamespaces) {
         namespaceSupport.processName(name, false, uri, lname);
      }
      if (!contentHnd->endElement(uri, lname, name)) {
         reportParseError(contentHnd->errorString());
         return false;
      }
   }

   if (useNamespaces) {
      NamespaceMap prefixesBefore, prefixesAfter;
      if (contentHnd) {
         prefixesBefore = namespaceSupport.d->ns;
      }

      namespaceSupport.popContext();
      // call the handler for prefix mapping
      if (contentHnd) {
         prefixesAfter = namespaceSupport.d->ns;

         if (prefixesBefore.size() != prefixesAfter.size()) {
            for (NamespaceMap::const_iterator it = prefixesBefore.constBegin(); it != prefixesBefore.constEnd(); ++it) {
               if (!it.key().isEmpty() && !prefixesAfter.contains(it.key())) {
                  if (!contentHnd->endPrefixMapping(it.key())) {
                     reportParseError(contentHnd->errorString());
                     return false;
                  }
               }
            }
         }
      }
   }
   return true;
}

bool QXmlSimpleReaderPrivate::processElementAttribute()
{
   QString uri, lname, prefix;
   const QString &name = QXmlSimpleReaderPrivate::name();
   const QString &string = QXmlSimpleReaderPrivate::string();

   // add the attribute to the list
   if (useNamespaces) {
      // is it a namespace declaration?
      namespaceSupport.splitName(name, prefix, lname);

      if (prefix == "xmlns") {
         // namespace declaration
         namespaceSupport.setPrefix(lname, string);

         if (useNamespacePrefixes) {
            // according to http://www.w3.org/2000/xmlns/, the "prefix"
            // xmlns maps to the namespace name
            // http://www.w3.org/2000/xmlns/
            attList.append(name, "http://www.w3.org/2000/xmlns/", lname, string);
         }

         // call the handler for prefix mapping
         if (contentHnd) {
            if (!contentHnd->startPrefixMapping(lname, string)) {
               reportParseError(contentHnd->errorString());
               return false;
            }
         }

      } else {
         // no namespace delcaration
         namespaceSupport.processName(name, true, uri, lname);
         attList.append(name, uri, lname, string);
      }
   } else {
      // no namespace support
      attList.append(name, uri, lname, string);
   }
   return true;
}

/*
  Parse a content [43].

  A content is only used between tags. If a end tag is found the < is already
  read and the head stand on the '/' of the end tag '</name>'.
*/
bool QXmlSimpleReaderPrivate::parseContent()
{
   const signed char Init             =  0;
   const signed char ChD              =  1; // CharData
   const signed char ChD1             =  2; // CharData help state
   const signed char ChD2             =  3; // CharData help state
   const signed char Ref              =  4; // Reference
   const signed char Lt               =  5; // '<' read
   const signed char PInstr           =  6; // PI
   const signed char PInstrR          =  7; // same as PInstr, but already reported
   const signed char Elem             =  8; // Element
   const signed char Em               =  9; // '!' read
   const signed char Com              = 10; // Comment
   const signed char ComR             = 11; // same as Com, but already reported
   const signed char CDS              = 12; // CDSect
   const signed char CDS1             = 13; // read a CDSect
   const signed char CDS2             = 14; // read a CDSect (help state)
   const signed char CDS3             = 15; // read a CDSect (help state)
   const signed char Done             = 16; // finished reading content

   const signed char InpLt            = 0; // <
   const signed char InpGt            = 1; // >
   const signed char InpSlash         = 2; // /
   const signed char InpQMark         = 3; // ?
   const signed char InpEMark         = 4; // !
   const signed char InpAmp           = 5; // &
   const signed char InpDash          = 6; // -
   const signed char InpOpenB         = 7; // [
   const signed char InpCloseB        = 8; //]
   const signed char InpUnknown       = 9;

   static const signed char mapCLT2FSMChar[] = {
      InpUnknown, // white space
      InpUnknown, // %
      InpAmp,     // &
      InpGt,      // >
      InpLt,      // <
      InpSlash,   // /
      InpQMark,   // ?
      InpEMark,   // !
      InpDash,    // -
      InpCloseB,  //]
      InpOpenB,   // [
      InpUnknown, // =
      InpUnknown, // "
      InpUnknown, // '
      InpUnknown  // unknown
   };

   static const signed char table[16][10] = {
      /*  InpLt  InpGt  InpSlash  InpQMark  InpEMark  InpAmp  InpDash  InpOpenB  InpCloseB  InpUnknown */
      { Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD1,      ChD  }, // Init
      { Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD1,      ChD  }, // ChD
      { Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD2,      ChD  }, // ChD1
      { Lt,    -1,    ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD2,      ChD  }, // ChD2
      { Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD,       ChD  }, // Ref (same as Init)
      { -1,    -1,    Done,     PInstr,   Em,       -1,     -1,      -1,       -1,        Elem }, // Lt
      { Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD,       ChD  }, // PInstr (same as Init)
      { Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD,       ChD  }, // PInstrR
      { Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD,       ChD  }, // Elem (same as Init)
      { -1,    -1,    -1,       -1,       -1,       -1,     Com,     CDS,      -1,        -1   }, // Em
      { Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD,       ChD  }, // Com (same as Init)
      { Lt,    ChD,   ChD,      ChD,      ChD,      Ref,    ChD,     ChD,      ChD,       ChD  }, // ComR
      { CDS1,  CDS1,  CDS1,     CDS1,     CDS1,     CDS1,   CDS1,    CDS1,     CDS2,      CDS1 }, // CDS
      { CDS1,  CDS1,  CDS1,     CDS1,     CDS1,     CDS1,   CDS1,    CDS1,     CDS2,      CDS1 }, // CDS1
      { CDS1,  CDS1,  CDS1,     CDS1,     CDS1,     CDS1,   CDS1,    CDS1,     CDS3,      CDS1 }, // CDS2
      { CDS1,  Init,  CDS1,     CDS1,     CDS1,     CDS1,   CDS1,    CDS1,     CDS3,      CDS1 }  // CDS3
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      contentCharDataRead = false;
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseContent (cont) in state %d", state);
#endif

      if (! parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();

#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }

         if (!(this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseContent, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case Ref:
            if (!contentCharDataRead) {
               contentCharDataRead = parseReference_charDataRead;
            }
            break;
         case PInstr:
            if (contentHnd) {
               if (!contentHnd->processingInstruction(name(), string())) {
                  reportParseError(contentHnd->errorString());
                  return false;
               }
            }
            state = PInstrR;
            break;

         case Com:
            if (lexicalHnd) {
               if (!lexicalHnd->comment(string())) {
                  reportParseError(lexicalHnd->errorString());
                  return false;
               }
            }
            state = ComR;
            break;

         case CDS:
            stringClear();
            break;

         case CDS2:
            if (!atEnd() && c != QChar(']')) {
               stringAddC(QChar(']'));
            }
            break;

         case CDS3:
            // test if this skipping was legal
            if (! atEnd()) {

               if (c == '>') {

                  // the end of the CDSect
                  if (lexicalHnd) {
                     if (! lexicalHnd->startCDATA()) {
                        reportParseError(lexicalHnd->errorString());
                        return false;
                     }
                  }
                  if (contentHnd) {
                     if (! contentHnd->characters(string())) {
                        reportParseError(contentHnd->errorString());
                        return false;
                     }
                  }
                  if (lexicalHnd) {
                     if (! lexicalHnd->endCDATA()) {
                        reportParseError(lexicalHnd->errorString());
                        return false;
                     }
                  }

               } else if (c == QChar(']')) {
                  // three or more ']'
                  stringAddC(QChar(']'));

               } else {
                  // after ']]' comes another character
                  stringAddC(']');
                  stringAddC(']');
               }
            }
            break;

         case Done:
            // call the handler for CharData
            if (contentHnd) {
               if (contentCharDataRead) {
                  if (reportWhitespaceCharData || !string().simplified().isEmpty()) {
                     if (!contentHnd->characters(string())) {
                        reportParseError(contentHnd->errorString());
                        return false;
                     }
                  }
               }
            }
            // Done
            return true;
         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_ERRORPARSINGCONTENT));
            return false;
      }

      // get input (use lookup-table instead of nested ifs for performance reasons)
      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseContent, state);
         return false;
      }

      if (c > 0xFF) {
         input = InpUnknown;
      } else {
         input = mapCLT2FSMChar[charLookupTable[c.unicode()]];
      }

      state = table[state][input];

      switch (state) {
         case Init:
            // skip the ending '>' of a CDATASection
            next();
            break;

         case ChD:
            // on first call: clear string
            if (!contentCharDataRead) {
               contentCharDataRead = true;
               stringClear();
            }
            stringAddC();
            if (reportEntities) {
               if (!reportEndEntities()) {
                  return false;
               }
            }
            next();
            break;
         case ChD1:
            // on first call: clear string
            if (!contentCharDataRead) {
               contentCharDataRead = true;
               stringClear();
            }
            stringAddC();
            if (reportEntities) {
               if (!reportEndEntities()) {
                  return false;
               }
            }
            next();
            break;
         case ChD2:
            stringAddC();
            if (reportEntities) {
               if (!reportEndEntities()) {
                  return false;
               }
            }
            next();
            break;
         case Ref:
            if (!contentCharDataRead) {
               // reference may be CharData; so clear string to be safe
               stringClear();
               parseReference_context = InContent;
               if (!parseReference()) {
                  parseFailed(&QXmlSimpleReaderPrivate::parseContent, state);
                  return false;
               }
            } else {
               if (reportEntities) {
                  // report character data in chunks
                  if (contentHnd) {
                     if (reportWhitespaceCharData || ! string().simplified().isEmpty()) {
                        if (!contentHnd->characters(string())) {
                           reportParseError(contentHnd->errorString());
                           return false;
                        }
                     }
                  }
                  stringClear();
               }
               parseReference_context = InContent;
               if (!parseReference()) {
                  parseFailed(&QXmlSimpleReaderPrivate::parseContent, state);
                  return false;
               }
            }
            break;
         case Lt:
            // call the handler for CharData

            if (contentHnd) {
               if (contentCharDataRead) {

                  if (reportWhitespaceCharData || ! string().simplified().isEmpty()) {
                     if (! contentHnd->characters(string())) {
                        reportParseError(contentHnd->errorString());
                        return false;
                     }
                  }
               }
            }

            contentCharDataRead = false;
            next();
            break;

         case PInstr:
         case PInstrR:
            parsePI_xmldecl = false;
            if (!parsePI()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseContent, state);
               return false;
            }
            break;

         case Elem:
            if (!parseElement()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseContent, state);
               return false;
            }
            break;

         case Em:
            next();
            break;

         case Com:
         case ComR:
            if (!parseComment()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseContent, state);
               return false;
            }
            break;

         case CDS:
            parseString_s = "[CDATA[";

            if (!parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseContent, state);
               return false;
            }
            break;
         case CDS1:
            stringAddC();
            next();
            break;
         case CDS2:
            // skip ']'
            next();
            break;
         case CDS3:
            // skip ']'...
            next();
            break;
      }
   }

   return false;
}

bool QXmlSimpleReaderPrivate::reportEndEntities()
{
   int count = (int)xmlRefStack.count();
   while (count != 0 && xmlRefStack.top().isEmpty()) {
      if (contentHnd) {
         if (reportWhitespaceCharData || !string().simplified().isEmpty()) {
            if (!contentHnd->characters(string())) {
               reportParseError(contentHnd->errorString());
               return false;
            }
         }
      }
      stringClear();
      if (lexicalHnd) {
         if (!lexicalHnd->endEntity(xmlRefStack.top().name)) {
            reportParseError(lexicalHnd->errorString());
            return false;
         }
      }
      xmlRefStack.pop_back();
      count--;
   }
   return true;
}

/*
  Parse Misc [27].
*/
bool QXmlSimpleReaderPrivate::parseMisc()
{
   const signed char Init             = 0;
   const signed char Lt               = 1; // '<' was read
   const signed char Comment          = 2; // read comment
   const signed char eatWS            = 3; // eat whitespaces
   const signed char PInstr           = 4; // read PI
   const signed char Comment2         = 5; // read comment

   const signed char InpWs            = 0; // S
   const signed char InpLt            = 1; // <
   const signed char InpQm            = 2; // ?
   const signed char InpEm            = 3; // !
   const signed char InpUnknown       = 4;

   static const signed char table[3][5] = {
      /*  InpWs   InpLt  InpQm  InpEm     InpUnknown */
      { eatWS,  Lt,    -1,    -1,       -1        }, // Init
      { -1,     -1,    PInstr, Comment,  -1        }, // Lt
      { -1,     -1,    -1,    -1,       Comment2  }  // Comment
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseMisc (cont) in state %d", state);
#endif

      if (! parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();

#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }

         if (! (this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseMisc, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case eatWS:
            return true;

         case PInstr:
            if (contentHnd) {
               if (! contentHnd->processingInstruction(name(), string())) {
                  reportParseError(contentHnd->errorString());
                  return false;
               }
            }

            return true;

         case Comment2:
            if (lexicalHnd) {
               if (! lexicalHnd->comment(string())) {
                  reportParseError(lexicalHnd->errorString());
                  return false;
               }
            }
            return true;

         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_UNEXPECTEDCHARACTER));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseMisc, state);
         return false;
      }

      if (is_S(c)) {
         input = InpWs;
      } else if (c == QChar('<')) {
         input = InpLt;

      } else if (c == QChar('?')) {
         input = InpQm;

      } else if (c == QChar('!')) {
         input = InpEm;

      } else {
         input = InpUnknown;
      }
      state = table[state][input];

      switch (state) {
         case eatWS:
            if (!eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseMisc, state);
               return false;
            }
            break;

         case Lt:
            next();
            break;

         case PInstr:
            parsePI_xmldecl = false;
            if (! parsePI()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseMisc, state);
               return false;
            }
            break;

         case Comment:
            next();
            break;

         case Comment2:
            if (!parseComment()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseMisc, state);
               return false;
            }
            break;
      }
   }

   return false;
}

/*
  Parse a processing instruction [16].

  If xmldec is true, it tries to parse a PI or a XML declaration [23].

  Precondition: the beginning '<' of the PI is already read and the head stand
  on the '?' of '<?'.

  If this funktion was successful, the head-position is on the first
  character after the PI.
*/
bool QXmlSimpleReaderPrivate::parsePI()
{
   const signed char Init             =  0;
   const signed char QmI              =  1; // ? was read
   const signed char Name             =  2; // read Name
   const signed char XMLDecl          =  3; // read XMLDecl
   const signed char Ws1              =  4; // eat ws after "xml" of XMLDecl
   const signed char PInstr           =  5; // read PI
   const signed char Ws2              =  6; // eat ws after Name of PI
   const signed char Version          =  7; // read versionInfo
   const signed char Ws3              =  8; // eat ws after versionInfo
   const signed char EorSD            =  9; // read EDecl or SDDecl
   const signed char Ws4              = 10; // eat ws after EDecl or SDDecl
   const signed char SD               = 11; // read SDDecl
   const signed char Ws5              = 12; // eat ws after SDDecl
   const signed char ADone            = 13; // almost done
   const signed char Char             = 14; // Char was read
   const signed char Qm               = 15; // Qm was read
   const signed char Done             = 16; // finished reading content

   const signed char InpWs            = 0; // whitespace
   const signed char InpNameBe        = 1; // NameBeginning
   const signed char InpGt            = 2; // >
   const signed char InpQm            = 3; // ?
   const signed char InpUnknown       = 4;

   static const signed char table[16][5] = {
      /*  InpWs,  InpNameBe  InpGt  InpQm   InpUnknown  */
      { -1,     -1,        -1,    QmI,    -1     }, // Init
      { -1,     Name,      -1,    -1,     -1     }, // QmI
      { -1,     -1,        -1,    -1,     -1     }, // Name (this state is left not through input)
      { Ws1,    -1,        -1,    -1,     -1     }, // XMLDecl
      { -1,     Version,   -1,    -1,     -1     }, // Ws1
      { Ws2,    -1,        -1,    Qm,     -1     }, // PInstr
      { Char,   Char,      Char,  Qm,     Char   }, // Ws2
      { Ws3,    -1,        -1,    ADone,  -1     }, // Version
      { -1,     EorSD,     -1,    ADone,  -1     }, // Ws3
      { Ws4,    -1,        -1,    ADone,  -1     }, // EorSD
      { -1,     SD,        -1,    ADone,  -1     }, // Ws4
      { Ws5,    -1,        -1,    ADone,  -1     }, // SD
      { -1,     -1,        -1,    ADone,  -1     }, // Ws5
      { -1,     -1,        Done,  -1,     -1     }, // ADone
      { Char,   Char,      Char,  Qm,     Char   }, // Char
      { Char,   Char,      Done,  Qm,     Char   }, // Qm
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parsePI (cont) in state %d", state);
#endif

      if (! parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();
#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }
         if (! (this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parsePI, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case Name:
            // test what name was read and determine the next state
            // (not very beautiful, I admit)
            if (name().toLower() == "xml") {
               if (parsePI_xmldecl && name() == "xml") {
                  state = XMLDecl;

               } else {
                  reportParseError(QString::fromLatin1(XMLERR_INVALIDNAMEFORPI));
                  return false;
               }

            } else {
               state = PInstr;
               stringClear();
            }
            break;

         case Version:
            // get version (syntax like an attribute)
            if (name() != "version") {
               reportParseError(QString::fromLatin1(XMLERR_VERSIONEXPECTED));
               return false;
            }
            xmlVersion = string();
            break;

         case EorSD:
            // get the EDecl or SDDecl (syntax like an attribute)
            if (name() == "standalone") {
               if (string() == "yes") {
                  standalone = QXmlSimpleReaderPrivate::Yes;

               } else if (string() == "no") {
                  standalone = QXmlSimpleReaderPrivate::No;

               } else {
                  reportParseError(QString::fromLatin1(XMLERR_WRONGVALUEFORSDECL));
                  return false;
               }

            } else if (name() == "encoding") {
               encoding = string();

            } else {
               reportParseError(QString::fromLatin1(XMLERR_EDECLORSDDECLEXPECTED));
               return false;
            }
            break;

         case SD:
            if (name() != "standalone") {
               reportParseError(QString::fromLatin1(XMLERR_SDDECLEXPECTED));
               return false;
            }

            if (string() == "yes") {
               standalone = QXmlSimpleReaderPrivate::Yes;

            } else if (string() == "no") {
               standalone = QXmlSimpleReaderPrivate::No;

            } else {
               reportParseError(QString::fromLatin1(XMLERR_WRONGVALUEFORSDECL));
               return false;
            }
            break;

         case Qm:
            // test if the skipping was legal
            if (!atEnd() && c != '>') {
               stringAddC('?');
            }
            break;

         case Done:
            return true;

         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_UNEXPECTEDCHARACTER));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parsePI, state);
         return false;
      }
      if (is_S(c)) {
         input = InpWs;

      } else if (determineNameChar(c) == NameBeginning) {
         input = InpNameBe;

      } else if (c == '>') {
         input = InpGt;

      } else if (c == '?') {
         input = InpQm;

      } else {
         input = InpUnknown;
      }
      state = table[state][input];

      switch (state) {
         case QmI:
            next();
            break;

         case Name:
            parseName_useRef = false;
            if (!parseName()) {
               parseFailed(&QXmlSimpleReaderPrivate::parsePI, state);
               return false;
            }
            break;

         case Ws1:
         case Ws2:
         case Ws3:
         case Ws4:
         case Ws5:
            if (!eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parsePI, state);
               return false;
            }
            break;

         case Version:
            if (!parseAttribute()) {
               parseFailed(&QXmlSimpleReaderPrivate::parsePI, state);
               return false;
            }
            break;

         case EorSD:
            if (!parseAttribute()) {
               parseFailed(&QXmlSimpleReaderPrivate::parsePI, state);
               return false;
            }
            break;

         case SD:
            // get the SDDecl (syntax like an attribute)
            if (standalone != QXmlSimpleReaderPrivate::Unknown) {
               // already parsed the standalone declaration
               reportParseError(QString::fromLatin1(XMLERR_UNEXPECTEDCHARACTER));
               return false;
            }

            if (!parseAttribute()) {
               parseFailed(&QXmlSimpleReaderPrivate::parsePI, state);
               return false;
            }
            break;

         case ADone:
            next();
            break;

         case Char:
            stringAddC();
            next();
            break;

         case Qm:
            // skip the '?'
            next();
            break;

         case Done:
            next();
            break;
      }
   }
   return false;
}

/*
  Parse a document type definition (doctypedecl [28]).

  Precondition: the beginning '<!' of the doctype is already read the head
  stands on the 'D' of '<!DOCTYPE'.

  If this function was successful, the head-position is on the first
  character after the document type definition.
*/
bool QXmlSimpleReaderPrivate::parseDoctype()
{
   const signed char Init             =  0;
   const signed char Doctype          =  1; // read the doctype
   const signed char Ws1              =  2; // eat_ws
   const signed char Doctype2         =  3; // read the doctype, part 2
   const signed char Ws2              =  4; // eat_ws
   const signed char Sys              =  5; // read SYSTEM or PUBLIC
   const signed char Ws3              =  6; // eat_ws
   const signed char MP               =  7; // markupdecl or PEReference
   const signed char MPR              =  8; // same as MP, but already reported
   const signed char PER              =  9; // PERReference
   const signed char Mup              = 10; // markupdecl
   const signed char Ws4              = 11; // eat_ws
   const signed char MPE              = 12; // end of markupdecl or PEReference
   const signed char Done             = 13;

   const signed char InpWs            = 0;
   const signed char InpD             = 1; // 'D'
   const signed char InpS             = 2; // 'S' or 'P'
   const signed char InpOB            = 3; // [
   const signed char InpCB            = 4; //]
   const signed char InpPer           = 5; // %
   const signed char InpGt            = 6; // >
   const signed char InpUnknown       = 7;

   static const signed char table[13][8] = {
      /*  InpWs,  InpD       InpS       InpOB  InpCB  InpPer InpGt  InpUnknown */
      { -1,     Doctype,   -1,        -1,    -1,    -1,    -1,    -1        }, // Init
      { Ws1,    -1,        -1,        -1,    -1,    -1,    -1,    -1        }, // Doctype
      { -1,     Doctype2,  Doctype2,  -1,    -1,    -1,    -1,    Doctype2  }, // Ws1
      { Ws2,    -1,        Sys,       MP,    -1,    -1,    Done,  -1        }, // Doctype2
      { -1,     -1,        Sys,       MP,    -1,    -1,    Done,  -1        }, // Ws2
      { Ws3,    -1,        -1,        MP,    -1,    -1,    Done,  -1        }, // Sys
      { -1,     -1,        -1,        MP,    -1,    -1,    Done,  -1        }, // Ws3
      { -1,     -1,        -1,        -1,    MPE,   PER,   -1,    Mup       }, // MP
      { -1,     -1,        -1,        -1,    MPE,   PER,   -1,    Mup       }, // MPR
      { Ws4,    -1,        -1,        -1,    MPE,   PER,   -1,    Mup       }, // PER
      { Ws4,    -1,        -1,        -1,    MPE,   PER,   -1,    Mup       }, // Mup
      { -1,     -1,        -1,        -1,    MPE,   PER,   -1,    Mup       }, // Ws4
      { -1,     -1,        -1,        -1,    -1,    -1,    Done,  -1        }  // MPE
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      startDTDwasReported = false;
      systemId.clear();
      publicId.clear();
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseDoctype (cont) in state %d", state);
#endif

      if (!parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();
#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }
         if (! (this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseDoctype, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case Doctype2:
            doctype = name();
            break;

         case MP:
            if (! startDTDwasReported && lexicalHnd ) {
               startDTDwasReported = true;
               if (! lexicalHnd->startDTD(doctype, publicId, systemId)) {
                  reportParseError(lexicalHnd->errorString());
                  return false;
               }
            }
            state = MPR;
            break;

         case Done:
            return true;

         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_ERRORPARSINGDOCTYPE));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseDoctype, state);
         return false;
      }
      if (is_S(c)) {
         input = InpWs;

      } else if (c == 'D') {
         input = InpD;

      } else if (c == 'S') {
         input = InpS;

      } else if (c == 'P') {
         input = InpS;

      } else if (c == '[') {
         input = InpOB;

      } else if (c == ']') {
         input = InpCB;

      } else if (c == '%') {
         input = InpPer;

      } else if (c == '>') {
         input = InpGt;

      } else {
         input = InpUnknown;
      }
      state = table[state][input];

      switch (state) {
         case Doctype:
            parseString_s = "DOCTYPE";

            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseDoctype, state);
               return false;
            }
            break;

         case Ws1:
         case Ws2:
         case Ws3:
         case Ws4:
            if (! eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseDoctype, state);
               return false;
            }
            break;

         case Doctype2:
            parseName_useRef = false;
            if (! parseName()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseDoctype, state);
               return false;
            }
            break;

         case Sys:
            parseExternalID_allowPublicID = false;
            if (! parseExternalID()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseDoctype, state);
               return false;
            }
            thisPublicId = publicId;
            thisSystemId = systemId;
            break;

         case MP:
         case MPR:
            if (!next_eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseDoctype, state);
               return false;
            }
            break;
         case PER:
            parsePEReference_context = InDTD;
            if (!parsePEReference()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseDoctype, state);
               return false;
            }
            break;
         case Mup:
            if (dtdRecursionLimit > 0 && parameterEntities.size() > dtdRecursionLimit) {
               reportParseError(QString::fromLatin1("DTD parsing exceeded recursion limit of %1.").formatArg(dtdRecursionLimit));
               return false;
            }

            if (!parseMarkupdecl()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseDoctype, state);
               return false;
            }
            break;
         case MPE:
            if (!next_eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseDoctype, state);
               return false;
            }
            break;
         case Done:
            if (lexicalHnd) {
               if (!startDTDwasReported) {
                  startDTDwasReported = true;
                  if (!lexicalHnd->startDTD(doctype, publicId, systemId)) {
                     reportParseError(lexicalHnd->errorString());
                     return false;
                  }
               }
               if (!lexicalHnd->endDTD()) {
                  reportParseError(lexicalHnd->errorString());
                  return false;
               }
            }
            next();
            break;
      }
   }
   return false;
}

/*
  Parse a ExternalID [75].

  If allowPublicID is true parse ExternalID [75] or PublicID [83].
*/
bool QXmlSimpleReaderPrivate::parseExternalID()
{
   const signed char Init             =  0;
   const signed char Sys              =  1; // parse 'SYSTEM'
   const signed char SysWS            =  2; // parse the whitespace after 'SYSTEM'
   const signed char SysSQ            =  3; // parse SystemLiteral with '
   const signed char SysSQ2           =  4; // parse SystemLiteral with '
   const signed char SysDQ            =  5; // parse SystemLiteral with "
   const signed char SysDQ2           =  6; // parse SystemLiteral with "
   const signed char Pub              =  7; // parse 'PUBLIC'
   const signed char PubWS            =  8; // parse the whitespace after 'PUBLIC'
   const signed char PubSQ            =  9; // parse PubidLiteral with '
   const signed char PubSQ2           = 10; // parse PubidLiteral with '
   const signed char PubDQ            = 11; // parse PubidLiteral with "
   const signed char PubDQ2           = 12; // parse PubidLiteral with "
   const signed char PubE             = 13; // finished parsing the PubidLiteral
   const signed char PubWS2           = 14; // parse the whitespace after the PubidLiteral
   const signed char PDone            = 15; // done if allowPublicID is true
   const signed char Done             = 16;

   const signed char InpSQ            = 0; // '
   const signed char InpDQ            = 1; // "
   const signed char InpS             = 2; // S
   const signed char InpP             = 3; // P
   const signed char InpWs            = 4; // white space
   const signed char InpUnknown       = 5;

   static const signed char table[15][6] = {
      /*  InpSQ    InpDQ    InpS     InpP     InpWs     InpUnknown */
      { -1,      -1,      Sys,     Pub,     -1,       -1      }, // Init
      { -1,      -1,      -1,      -1,      SysWS,    -1      }, // Sys
      { SysSQ,   SysDQ,   -1,      -1,      -1,       -1      }, // SysWS
      { Done,    SysSQ2,  SysSQ2,  SysSQ2,  SysSQ2,   SysSQ2  }, // SysSQ
      { Done,    SysSQ2,  SysSQ2,  SysSQ2,  SysSQ2,   SysSQ2  }, // SysSQ2
      { SysDQ2,  Done,    SysDQ2,  SysDQ2,  SysDQ2,   SysDQ2  }, // SysDQ
      { SysDQ2,  Done,    SysDQ2,  SysDQ2,  SysDQ2,   SysDQ2  }, // SysDQ2
      { -1,      -1,      -1,      -1,      PubWS,    -1      }, // Pub
      { PubSQ,   PubDQ,   -1,      -1,      -1,       -1      }, // PubWS
      { PubE,    -1,      PubSQ2,  PubSQ2,  PubSQ2,   PubSQ2  }, // PubSQ
      { PubE,    -1,      PubSQ2,  PubSQ2,  PubSQ2,   PubSQ2  }, // PubSQ2
      { -1,      PubE,    PubDQ2,  PubDQ2,  PubDQ2,   PubDQ2  }, // PubDQ
      { -1,      PubE,    PubDQ2,  PubDQ2,  PubDQ2,   PubDQ2  }, // PubDQ2
      { PDone,   PDone,   PDone,   PDone,   PubWS2,   PDone   }, // PubE
      { SysSQ,   SysDQ,   PDone,   PDone,   PDone,    PDone   }  // PubWS2
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      systemId.clear();
      publicId.clear();
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseExternalID (cont) in state %d", state);
#endif

      if (!parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();

#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }

         if (! (this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseExternalID, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case PDone:
            if (parseExternalID_allowPublicID) {
               publicId = string();
               return true;
            } else {
               reportParseError(QString::fromLatin1(XMLERR_UNEXPECTEDCHARACTER));
               return false;
            }
         case Done:
            return true;
         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_UNEXPECTEDCHARACTER));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseExternalID, state);
         return false;
      }
      if (is_S(c)) {
         input = InpWs;

      } else if (c == '\'') {
         input = InpSQ;

      } else if (c == '"') {
         input = InpDQ;

      } else if (c == 'S') {
         input = InpS;

      } else if (c == 'P') {
         input = InpP;

      } else {
         input = InpUnknown;
      }
      state = table[state][input];

      switch (state) {
         case Sys:
            parseString_s = "SYSTEM";
            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseExternalID, state);
               return false;
            }
            break;

         case SysWS:
            if (! eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseExternalID, state);
               return false;
            }
            break;

         case SysSQ:
         case SysDQ:
            stringClear();
            next();
            break;

         case SysSQ2:
         case SysDQ2:
            stringAddC();
            next();
            break;

         case Pub:
            parseString_s = "PUBLIC";
            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseExternalID, state);
               return false;
            }
            break;

         case PubWS:
            if (! eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseExternalID, state);
               return false;
            }
            break;

         case PubSQ:
         case PubDQ:
            stringClear();
            next();
            break;

         case PubSQ2:
         case PubDQ2:
            stringAddC();
            next();
            break;

         case PubE:
            next();
            break;

         case PubWS2:
            publicId = string();
            if (! eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseExternalID, state);
               return false;
            }
            break;

         case Done:
            systemId = string();
            next();
            break;
      }
   }
   return false;
}

/*
  Parse a markupdecl [29].
*/
bool QXmlSimpleReaderPrivate::parseMarkupdecl()
{
   const signed char Init             = 0;
   const signed char Lt               = 1; // < was read
   const signed char Em               = 2; // ! was read
   const signed char CE               = 3; // E was read
   const signed char Qm               = 4; // ? was read
   const signed char Dash             = 5; // - was read
   const signed char CA               = 6; // A was read
   const signed char CEL              = 7; // EL was read
   const signed char CEN              = 8; // EN was read
   const signed char CN               = 9; // N was read
   const signed char Done             = 10;

   const signed char InpLt            = 0; // <
   const signed char InpQm            = 1; // ?
   const signed char InpEm            = 2; // !
   const signed char InpDash          = 3; // -
   const signed char InpA             = 4; // A
   const signed char InpE             = 5; // E
   const signed char InpL             = 6; // L
   const signed char InpN             = 7; // N
   const signed char InpUnknown       = 8;

   static const signed char table[4][9] = {
      /*  InpLt  InpQm  InpEm  InpDash  InpA   InpE   InpL   InpN   InpUnknown */
      { Lt,    -1,    -1,    -1,      -1,    -1,    -1,    -1,    -1     }, // Init
      { -1,    Qm,    Em,    -1,      -1,    -1,    -1,    -1,    -1     }, // Lt
      { -1,    -1,    -1,    Dash,    CA,    CE,    -1,    CN,    -1     }, // Em
      { -1,    -1,    -1,    -1,      -1,    -1,    CEL,   CEN,   -1     }  // CE
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseMarkupdecl (cont) in state %d", state);
#endif

      if (!parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();

#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }
         if (!(this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseMarkupdecl, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case Qm:
            if (contentHnd) {
               if (!contentHnd->processingInstruction(name(), string())) {
                  reportParseError(contentHnd->errorString());
                  return false;
               }
            }
            return true;
         case Dash:
            if (lexicalHnd) {
               if (!lexicalHnd->comment(string())) {
                  reportParseError(lexicalHnd->errorString());
                  return false;
               }
            }
            return true;

         case CA:
            return true;

         case CEL:
            return true;

         case CEN:
            return true;

         case CN:
            return true;

         case Done:
            return true;

         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_LETTEREXPECTED));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseMarkupdecl, state);
         return false;
      }

      if  (c == '<') {
         input = InpLt;

      } else if (c == '?') {
         input = InpQm;

      } else if (c == '!') {
         input = InpEm;

      } else if (c == '-') {
         input = InpDash;

      } else if (c == 'A') {
         input = InpA;

      } else if (c == 'E') {
         input = InpE;

      } else if (c == 'L') {
         input = InpL;

      } else if (c == 'N') {
         input = InpN;

      } else {
         input = InpUnknown;
      }
      state = table[state][input];

      switch (state) {
         case Lt:
            next();
            break;

         case Em:
            next();
            break;

         case CE:
            next();
            break;

         case Qm:
            parsePI_xmldecl = false;
            if (! parsePI()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseMarkupdecl, state);
               return false;
            }
            break;

         case Dash:
            if (! parseComment()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseMarkupdecl, state);
               return false;
            }
            break;

         case CA:
            if (! parseAttlistDecl()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseMarkupdecl, state);
               return false;
            }
            break;

         case CEL:
            if (! parseElementDecl()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseMarkupdecl, state);
               return false;
            }
            break;

         case CEN:
            if (!parseEntityDecl()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseMarkupdecl, state);
               return false;
            }
            break;

         case CN:
            if (! parseNotationDecl()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseMarkupdecl, state);
               return false;
            }
            break;
      }
   }

   return false;
}

/*
  Parse a PEReference [69]
*/
bool QXmlSimpleReaderPrivate::parsePEReference()
{
   const signed char Init             = 0;
   const signed char Next             = 1;
   const signed char Name             = 2;
   const signed char NameR            = 3; // same as Name, but already reported
   const signed char Done             = 4;

   const signed char InpSemi          = 0; // ;
   const signed char InpPer           = 1; // %
   const signed char InpUnknown       = 2;

   static const signed char table[4][3] = {
      /*  InpSemi  InpPer  InpUnknown */
      { -1,      Next,   -1    }, // Init
      { -1,      -1,     Name  }, // Next
      { Done,    -1,     -1    }, // Name
      { Done,    -1,     -1    }  // NameR
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parsePEReference (cont) in state %d", state);
#endif

      if (!parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();

#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }

         if (!(this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parsePEReference, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case Name: {
            bool skipIt = true;
            QString xmlRefString;

            QMap<QString, QString>::iterator it;
            it = parameterEntities.find(ref());

            if (it != parameterEntities.end()) {
               skipIt = false;
               xmlRefString = *it;

            } else if (entityRes) {
               QMap<QString, QXmlSimpleReaderPrivate::ExternParameterEntity>::iterator it2;

               it2 = externParameterEntities.find(ref());
               QXmlInputSource *ret = nullptr;

               if (it2 != externParameterEntities.end()) {
                  if (!entityRes->resolveEntity((*it2).publicId, (*it2).systemId, ret)) {
                     delete ret;
                     reportParseError(entityRes->errorString());
                     return false;
                  }

                  if (ret) {
                     xmlRefString = ret->data();
                     delete ret;
                     if (! stripTextDecl(xmlRefString)) {
                        reportParseError(QString::fromLatin1(XMLERR_ERRORINTEXTDECL));
                        return false;
                     }
                     skipIt = false;
                  }
               }
            }

            if (skipIt) {
               if (contentHnd) {
                  if (! contentHnd->skippedEntity('%' + ref())) {
                     reportParseError(contentHnd->errorString());
                     return false;
                  }
               }

            } else {
               if (parsePEReference_context == InEntityValue) {
                  // Included in literal
                  if (! insertXmlRef(xmlRefString, ref(), true)) {
                     return false;
                  }
               } else if (parsePEReference_context == InDTD) {
                  // Included as PE
                  if (! insertXmlRef(' ' + xmlRefString + ' ', ref(), false)) {
                     return false;
                  }
               }
            }
         }
         state = NameR;
         break;

         case Done:
            return true;

         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_LETTEREXPECTED));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parsePEReference, state);
         return false;
      }
      if        (c == ';') {
         input = InpSemi;

      } else if (c == '%') {
         input = InpPer;

      } else {
         input = InpUnknown;
      }
      state = table[state][input];

      switch (state) {
         case Next:
            next();
            break;

         case Name:
         case NameR:
            parseName_useRef = true;
            if (!parseName()) {
               parseFailed(&QXmlSimpleReaderPrivate::parsePEReference, state);
               return false;
            }
            break;

         case Done:
            next();
            break;
      }
   }

   return false;
}

/*
  Parse a AttlistDecl [52].

  Precondition: the beginning '<!' is already read and the head
  stands on the 'A' of '<!ATTLIST'
*/
bool QXmlSimpleReaderPrivate::parseAttlistDecl()
{
   const signed char Init             =  0;
   const signed char Attlist          =  1; // parse the string "ATTLIST"
   const signed char Ws               =  2; // whitespace read
   const signed char Name             =  3; // parse name
   const signed char Ws1              =  4; // whitespace read
   const signed char Attdef           =  5; // parse the AttDef
   const signed char Ws2              =  6; // whitespace read
   const signed char Atttype          =  7; // parse the AttType
   const signed char Ws3              =  8; // whitespace read
   const signed char DDecH            =  9; // DefaultDecl with #
   const signed char DefReq           = 10; // parse the string "REQUIRED"
   const signed char DefImp           = 11; // parse the string "IMPLIED"
   const signed char DefFix           = 12; // parse the string "FIXED"
   const signed char Attval           = 13; // parse the AttValue
   const signed char Ws4              = 14; // whitespace read
   const signed char Done             = 15;

   const signed char InpWs            = 0; // white space
   const signed char InpGt            = 1; // >
   const signed char InpHash          = 2; // #
   const signed char InpA             = 3; // A
   const signed char InpI             = 4; // I
   const signed char InpF             = 5; // F
   const signed char InpR             = 6; // R
   const signed char InpUnknown       = 7;

   static const signed char table[15][8] = {
      /*  InpWs    InpGt    InpHash  InpA      InpI     InpF     InpR     InpUnknown */
      { -1,      -1,      -1,      Attlist,  -1,      -1,      -1,      -1      }, // Init
      { Ws,      -1,      -1,      -1,       -1,      -1,      -1,      -1      }, // Attlist
      { -1,      -1,      -1,      Name,     Name,    Name,    Name,    Name    }, // Ws
      { Ws1,     Done,    Attdef,  Attdef,   Attdef,  Attdef,  Attdef,  Attdef  }, // Name
      { -1,      Done,    Attdef,  Attdef,   Attdef,  Attdef,  Attdef,  Attdef  }, // Ws1
      { Ws2,     -1,      -1,      -1,       -1,      -1,      -1,      -1      }, // Attdef
      { -1,      Atttype, Atttype, Atttype,  Atttype, Atttype, Atttype, Atttype }, // Ws2
      { Ws3,     -1,      -1,      -1,       -1,      -1,      -1,      -1      }, // Attype
      { -1,      Attval,  DDecH,   Attval,   Attval,  Attval,  Attval,  Attval  }, // Ws3
      { -1,      -1,      -1,      -1,       DefImp,  DefFix,  DefReq,  -1      }, // DDecH
      { Ws4,     Ws4,     -1,      -1,       -1,      -1,      -1,      -1      }, // DefReq
      { Ws4,     Ws4,     -1,      -1,       -1,      -1,      -1,      -1      }, // DefImp
      { Ws3,     -1,      -1,      -1,       -1,      -1,      -1,      -1      }, // DefFix
      { Ws4,     Ws4,     -1,      -1,       -1,      -1,      -1,      -1      }, // Attval
      { -1,      Done,    Attdef,  Attdef,   Attdef,  Attdef,  Attdef,  Attdef  }  // Ws4
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseAttlistDecl (cont) in state %d", state);
#endif

      if (!parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();
#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }
         if (!(this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseAttlistDecl, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case Name:
            attDeclEName = name();
            break;
         case Attdef:
            attDeclAName = name();
            break;
         case Done:
            return true;
         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_LETTEREXPECTED));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseAttlistDecl, state);
         return false;
      }

      if (is_S(c)) {
         input = InpWs;

      } else if (c == '>') {
         input = InpGt;

      } else if (c == '#') {
         input = InpHash;

      } else if (c == 'A') {
         input = InpA;

      } else if (c == 'I') {
         input = InpI;

      } else if (c == 'F') {
         input = InpF;

      } else if (c == 'R') {
         input = InpR;

      } else {
         input = InpUnknown;
      }
      state = table[state][input];

      switch (state) {
         case Attlist:
            parseString_s = "ATTLIST";
            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttlistDecl, state);
               return false;
            }
            break;

         case Ws:
         case Ws1:
         case Ws2:
         case Ws3:
            if (! eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttlistDecl, state);
               return false;
            }
            break;

         case Name:
            parseName_useRef = false;
            if (! parseName()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttlistDecl, state);
               return false;
            }
            break;

         case Attdef:
            parseName_useRef = false;
            if (!parseName()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttlistDecl, state);
               return false;
            }
            break;

         case Atttype:
            if (! parseAttType()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttlistDecl, state);
               return false;
            }
            break;

         case DDecH:
            next();
            break;

         case DefReq:
            parseString_s = "REQUIRED";
            if (!parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttlistDecl, state);
               return false;
            }
            break;

         case DefImp:
            parseString_s = "IMPLIED";
            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttlistDecl, state);
               return false;
            }
            break;

         case DefFix:
            parseString_s = "FIXED";
            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttlistDecl, state);
               return false;
            }
            break;

         case Attval:
            if (! parseAttValue()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttlistDecl, state);
               return false;
            }
            break;

         case Ws4:
            if (declHnd) {
               // ### not all values are computed yet...
               if (! declHnd->attributeDecl(attDeclEName, attDeclAName, QString(), QString(), QString())) {
                  reportParseError(declHnd->errorString());
                  return false;
               }
            }

            if (!eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttlistDecl, state);
               return false;
            }
            break;

         case Done:
            next();
            break;
      }
   }

   return false;
}

/*
  Parse a AttType [54]
*/
bool QXmlSimpleReaderPrivate::parseAttType()
{
   const signed char Init             =  0;
   const signed char ST               =  1; // StringType
   const signed char TTI              =  2; // TokenizedType starting with 'I'
   const signed char TTI2             =  3; // TokenizedType helpstate
   const signed char TTI3             =  4; // TokenizedType helpstate
   const signed char TTE              =  5; // TokenizedType starting with 'E'
   const signed char TTEY             =  6; // TokenizedType starting with 'ENTITY'
   const signed char TTEI             =  7; // TokenizedType starting with 'ENTITI'
   const signed char N                =  8; // N read (TokenizedType or Notation)
   const signed char TTNM             =  9; // TokenizedType starting with 'NM'
   const signed char TTNM2            = 10; // TokenizedType helpstate
   const signed char NO               = 11; // Notation
   const signed char NO2              = 12; // Notation helpstate
   const signed char NO3              = 13; // Notation helpstate
   const signed char NOName           = 14; // Notation, read name
   const signed char NO4              = 15; // Notation helpstate
   const signed char EN               = 16; // Enumeration
   const signed char ENNmt            = 17; // Enumeration, read Nmtoken
   const signed char EN2              = 18; // Enumeration helpstate
   const signed char ADone            = 19; // almost done (make next and accept)
   const signed char Done             = 20;

   const signed char InpWs            =  0; // whitespace
   const signed char InpOp            =  1; // (
   const signed char InpCp            =  2; //)
   const signed char InpPipe          =  3; // |
   const signed char InpC             =  4; // C
   const signed char InpE             =  5; // E
   const signed char InpI             =  6; // I
   const signed char InpM             =  7; // M
   const signed char InpN             =  8; // N
   const signed char InpO             =  9; // O
   const signed char InpR             = 10; // R
   const signed char InpS             = 11; // S
   const signed char InpY             = 12; // Y
   const signed char InpUnknown       = 13;

   static const signed char table[19][14] = {
      /*  InpWs    InpOp    InpCp    InpPipe  InpC     InpE     InpI     InpM     InpN     InpO     InpR     InpS     InpY     InpUnknown */
      { -1,      EN,      -1,      -1,      ST,      TTE,     TTI,     -1,      N,       -1,      -1,      -1,      -1,      -1     }, // Init
      { Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done   }, // ST
      { Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    TTI2,    Done,    Done,    Done   }, // TTI
      { Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    TTI3,    Done,    Done   }, // TTI2
      { Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done   }, // TTI3
      { -1,      -1,      -1,      -1,      -1,      -1,      TTEI,    -1,      -1,      -1,      -1,      -1,      TTEY,    -1     }, // TTE
      { Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done   }, // TTEY
      { Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done   }, // TTEI
      { -1,      -1,      -1,      -1,      -1,      -1,      -1,      TTNM,    -1,      NO,      -1,      -1,      -1,      -1     }, // N
      { Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    TTNM2,   Done,    Done   }, // TTNM
      { Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done,    Done   }, // TTNM2
      { NO2,     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }, // NO
      { -1,      NO3,     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }, // NO2
      { NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName,  NOName }, // NO3
      { NO4,     -1,      ADone,   NO3,     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }, // NOName
      { -1,      -1,      ADone,   NO3,     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }, // NO4
      { -1,      -1,      ENNmt,   -1,      ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt,   ENNmt  }, // EN
      { EN2,     -1,      ADone,   EN,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }, // ENNmt
      { -1,      -1,      ADone,   EN,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1     }  // EN2
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseAttType (cont) in state %d", state);
#endif

      if (!parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();

#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }
         if (!(this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseAttType, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case ADone:
            return true;
         case Done:
            return true;
         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_LETTEREXPECTED));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseAttType, state);
         return false;
      }
      if (is_S(c)) {
         input = InpWs;

      } else if (c == '(') {
         input = InpOp;
      } else if (c == ')') {
         input = InpCp;

      } else if (c == '|') {
         input = InpPipe;

      } else if (c == 'C') {
         input = InpC;

      } else if (c == 'E') {
         input = InpE;

      } else if (c == 'I') {
         input = InpI;

      } else if (c == 'M') {
         input = InpM;

      } else if (c == 'N') {
         input = InpN;

      } else if (c == 'O') {
         input = InpO;

      } else if (c == 'R') {
         input = InpR;

      } else if (c == 'S') {
         input = InpS;

      } else if (c == 'Y') {
         input = InpY;

      } else {
         input = InpUnknown;
      }

      state = table[state][input];

      switch (state) {
         case ST:
            parseString_s = "CDATA";
            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttType, state);
               return false;
            }
            break;

         case TTI:
            parseString_s = "ID";
            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttType, state);
               return false;
            }
            break;

         case TTI2:
            parseString_s = "REF";
            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttType, state);
               return false;
            }
            break;

         case TTI3:
            next(); // S
            break;

         case TTE:
            parseString_s = "ENTIT";
            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttType, state);
               return false;
            }
            break;

         case TTEY:
            next(); // Y
            break;
         case TTEI:
            parseString_s = "IES";
            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttType, state);
               return false;
            }
            break;

         case N:
            next(); // N
            break;

         case TTNM:
            parseString_s = "MTOKEN";
            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttType, state);
               return false;
            }
            break;

         case TTNM2:
            next(); // S
            break;

         case NO:
            parseString_s = "OTATION";
            if (!parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttType, state);
               return false;
            }
            break;

         case NO2:
            if (!eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttType, state);
               return false;
            }
            break;

         case NO3:
            if (! next_eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttType, state);
               return false;
            }
            break;

         case NOName:
            parseName_useRef = false;
            if (! parseName()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttType, state);
               return false;
            }
            break;

         case NO4:
            if (! eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttType, state);
               return false;
            }
            break;

         case EN:
            if (! next_eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttType, state);
               return false;
            }
            break;

         case ENNmt:
            if (! parseNmtoken()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttType, state);
               return false;
            }
            break;

         case EN2:
            if (! eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttType, state);
               return false;
            }
            break;

         case ADone:
            next();
            break;
      }
   }
   return false;
}

/*
  Parse a AttValue [10]

  Precondition: the head stands on the beginning " or '

  If this function was successful, the head stands on the first
  character after the closing " or ' and the value of the attribute
  is in string().
*/
bool QXmlSimpleReaderPrivate::parseAttValue()
{
   const signed char Init             = 0;
   const signed char Dq               = 1; // double quotes were read
   const signed char DqRef            = 2; // read references in double quotes
   const signed char DqC              = 3; // signed character read in double quotes
   const signed char Sq               = 4; // single quotes were read
   const signed char SqRef            = 5; // read references in single quotes
   const signed char SqC              = 6; // signed character read in single quotes
   const signed char Done             = 7;

   const signed char InpDq            = 0; // "
   const signed char InpSq            = 1; // '
   const signed char InpAmp           = 2; // &
   const signed char InpLt            = 3; // <
   const signed char InpUnknown       = 4;

   static const signed char table[7][5] = {
      /*  InpDq  InpSq  InpAmp  InpLt InpUnknown */
      { Dq,    Sq,    -1,     -1,   -1    }, // Init
      { Done,  DqC,   DqRef,  -1,   DqC   }, // Dq
      { Done,  DqC,   DqRef,  -1,   DqC   }, // DqRef
      { Done,  DqC,   DqRef,  -1,   DqC   }, // DqC
      { SqC,   Done,  SqRef,  -1,   SqC   }, // Sq
      { SqC,   Done,  SqRef,  -1,   SqC   }, // SqRef
      { SqC,   Done,  SqRef,  -1,   SqC   }  // SqRef
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseAttValue (cont) in state %d", state);
#endif

      if (! parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();

#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }

         if (!(this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseAttValue, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case Done:
            return true;

         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_UNEXPECTEDCHARACTER));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseAttValue, state);
         return false;
      }

      if (c == '"') {
         input = InpDq;

      } else if (c == '\'') {
         input = InpSq;

      } else if (c == '&') {
         input = InpAmp;

      } else if (c == '<') {
         input = InpLt;

      } else {
         input = InpUnknown;
      }
      state = table[state][input];

      switch (state) {
         case Dq:
         case Sq:
            stringClear();
            next();
            break;
         case DqRef:
         case SqRef:
            parseReference_context = InAttributeValue;
            if (!parseReference()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttValue, state);
               return false;
            }
            break;
         case DqC:
         case SqC:
            stringAddC();
            next();
            break;
         case Done:
            next();
            break;
      }
   }
   return false;
}

/*
  Parse a elementdecl [45].

  Precondition: the beginning '<!E' is already read and the head
  stands on the 'L' of '<!ELEMENT'
*/
bool QXmlSimpleReaderPrivate::parseElementDecl()
{
   const signed char Init             =  0;
   const signed char Elem             =  1; // parse the beginning string
   const signed char Ws1              =  2; // whitespace required
   const signed char Nam              =  3; // parse Name
   const signed char Ws2              =  4; // whitespace required
   const signed char Empty            =  5; // read EMPTY
   const signed char Any              =  6; // read ANY
   const signed char Cont             =  7; // read contentspec (except ANY or EMPTY)
   const signed char Mix              =  8; // read Mixed
   const signed char Mix2             =  9; //
   const signed char Mix3             = 10; //
   const signed char MixN1            = 11; //
   const signed char MixN2            = 12; //
   const signed char MixN3            = 13; //
   const signed char MixN4            = 14; //
   const signed char Cp               = 15; // parse cp
   const signed char Cp2              = 16; //
   const signed char WsD              = 17; // eat whitespace before Done
   const signed char Done             = 18;

   const signed char InpWs            =  0;
   const signed char InpGt            =  1; // >
   const signed char InpPipe          =  2; // |
   const signed char InpOp            =  3; // (
   const signed char InpCp            =  4; //)
   const signed char InpHash          =  5; // #
   const signed char InpQm            =  6; // ?
   const signed char InpAst           =  7; // *
   const signed char InpPlus          =  8; // +
   const signed char InpA             =  9; // A
   const signed char InpE             = 10; // E
   const signed char InpL             = 11; // L
   const signed char InpUnknown       = 12;

   static const signed char table[18][13] = {
      /*  InpWs   InpGt  InpPipe  InpOp  InpCp   InpHash  InpQm  InpAst  InpPlus  InpA    InpE    InpL    InpUnknown */
      { -1,     -1,    -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     Elem,   -1     }, // Init
      { Ws1,    -1,    -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Elem
      { -1,     -1,    -1,      -1,    -1,     -1,      -1,    -1,     -1,      Nam,    Nam,    Nam,    Nam    }, // Ws1
      { Ws2,    -1,    -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Nam
      { -1,     -1,    -1,      Cont,  -1,     -1,      -1,    -1,     -1,      Any,    Empty,  -1,     -1     }, // Ws2
      { WsD,    Done,  -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Empty
      { WsD,    Done,  -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Any
      { -1,     -1,    -1,      Cp,    Cp,     Mix,     -1,    -1,     -1,      Cp,     Cp,     Cp,     Cp     }, // Cont
      { Mix2,   -1,    MixN1,   -1,    Mix3,   -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Mix
      { -1,     -1,    MixN1,   -1,    Mix3,   -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Mix2
      { WsD,    Done,  -1,      -1,    -1,     -1,      -1,    WsD,    -1,      -1,     -1,     -1,     -1     }, // Mix3
      { -1,     -1,    -1,      -1,    -1,     -1,      -1,    -1,     -1,      MixN2,  MixN2,  MixN2,  MixN2  }, // MixN1
      { MixN3,  -1,    MixN1,   -1,    MixN4,  -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // MixN2
      { -1,     -1,    MixN1,   -1,    MixN4,  -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // MixN3
      { -1,     -1,    -1,      -1,    -1,     -1,      -1,    WsD,    -1,      -1,     -1,     -1,     -1     }, // MixN4
      { WsD,    Done,  -1,      -1,    -1,     -1,      Cp2,   Cp2,    Cp2,     -1,     -1,     -1,     -1     }, // Cp
      { WsD,    Done,  -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }, // Cp2
      { -1,     Done,  -1,      -1,    -1,     -1,      -1,    -1,     -1,      -1,     -1,     -1,     -1     }  // WsD
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseElementDecl (cont) in state %d", state);
#endif

      if (!parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();

#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }

         if (!(this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseElementDecl, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case Done:
            return true;
         case -1:
            reportParseError(QString::fromLatin1(XMLERR_UNEXPECTEDCHARACTER));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseElementDecl, state);
         return false;
      }
      if (is_S(c)) {
         input = InpWs;

      } else if (c == '>') {
         input = InpGt;

      } else if (c == '|') {
         input = InpPipe;

      } else if (c == '(') {
         input = InpOp;

      } else if (c == ')') {
         input = InpCp;

      } else if (c == '#') {
         input = InpHash;

      } else if (c == '?') {
         input = InpQm;

      } else if (c == '*') {
         input = InpAst;

      } else if (c == '+') {
         input = InpPlus;

      } else if (c == 'A') {
         input = InpA;

      } else if (c == 'E') {
         input = InpE;

      } else if (c == 'L') {
         input = InpL;

      } else {
         input = InpUnknown;
      }

      state = table[state][input];

      switch (state) {
         case Elem:
            parseString_s = "LEMENT";
            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElementDecl, state);
               return false;
            }
            break;

         case Ws1:
            if (! eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElementDecl, state);
               return false;
            }
            break;

         case Nam:
            parseName_useRef = false;
            if (! parseName()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElementDecl, state);
               return false;
            }
            break;

         case Ws2:
            if (! eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElementDecl, state);
               return false;
            }
            break;

         case Empty:
            parseString_s = "EMPTY";
            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElementDecl, state);
               return false;
            }
            break;

         case Any:
            parseString_s = "ANY";
            if (!parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElementDecl, state);
               return false;
            }
            break;

         case Cont:
            if (!next_eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElementDecl, state);
               return false;
            }
            break;

         case Mix:
            parseString_s = "#PCDATA";
            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElementDecl, state);
               return false;
            }
            break;

         case Mix2:
            if (! eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElementDecl, state);
               return false;
            }
            break;

         case Mix3:
            next();
            break;

         case MixN1:
            if (! next_eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElementDecl, state);
               return false;
            }
            break;

         case MixN2:
            parseName_useRef = false;
            if (! parseName()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElementDecl, state);
               return false;
            }
            break;
         case MixN3:
            if (!eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElementDecl, state);
               return false;
            }
            break;
         case MixN4:
            next();
            break;
         case Cp:
            if (!parseChoiceSeq()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElementDecl, state);
               return false;
            }
            break;
         case Cp2:
            next();
            break;
         case WsD:
            if (!next_eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseElementDecl, state);
               return false;
            }
            break;
         case Done:
            next();
            break;
      }
   }
   return false;
}

/*
  Parse a NotationDecl [82].

  Precondition: the beginning '<!' is already read and the head
  stands on the 'N' of '<!NOTATION'
*/
bool QXmlSimpleReaderPrivate::parseNotationDecl()
{
   const signed char Init             = 0;
   const signed char Not              = 1; // read NOTATION
   const signed char Ws1              = 2; // eat whitespaces
   const signed char Nam              = 3; // read Name
   const signed char Ws2              = 4; // eat whitespaces
   const signed char ExtID            = 5; // parse ExternalID
   const signed char ExtIDR           = 6; // same as ExtID, but already reported
   const signed char Ws3              = 7; // eat whitespaces
   const signed char Done             = 8;

   const signed char InpWs            = 0;
   const signed char InpGt            = 1; // >
   const signed char InpN             = 2; // N
   const signed char InpUnknown       = 3;

   static const signed char table[8][4] = {
      /*  InpWs   InpGt  InpN    InpUnknown */
      { -1,     -1,    Not,    -1     }, // Init
      { Ws1,    -1,    -1,     -1     }, // Not
      { -1,     -1,    Nam,    Nam    }, // Ws1
      { Ws2,    Done,  -1,     -1     }, // Nam
      { -1,     Done,  ExtID,  ExtID  }, // Ws2
      { Ws3,    Done,  -1,     -1     }, // ExtID
      { Ws3,    Done,  -1,     -1     }, // ExtIDR
      { -1,     Done,  -1,     -1     }  // Ws3
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseNotationDecl (cont) in state %d", state);
#endif

      if (! parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();

#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }
         if (!(this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseNotationDecl, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case ExtID:
            // call the handler
            if (dtdHnd) {
               if (!dtdHnd->notationDecl(name(), publicId, systemId)) {
                  reportParseError(dtdHnd->errorString());
                  return false;
               }
            }
            state = ExtIDR;
            break;

         case Done:
            return true;

         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_UNEXPECTEDCHARACTER));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseNotationDecl, state);
         return false;
      }
      if (is_S(c)) {
         input = InpWs;

      } else if (c == '>') {
         input = InpGt;

      } else if (c == 'N') {
         input = InpN;

      } else {
         input = InpUnknown;
      }
      state = table[state][input];

      switch (state) {
         case Not:
            parseString_s = "NOTATION";
            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseNotationDecl, state);
               return false;
            }
            break;

         case Ws1:
            if (! eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseNotationDecl, state);
               return false;
            }
            break;

         case Nam:
            parseName_useRef = false;
            if (! parseName()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseNotationDecl, state);
               return false;
            }
            break;

         case Ws2:
            if (! eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseNotationDecl, state);
               return false;
            }
            break;

         case ExtID:
         case ExtIDR:
            parseExternalID_allowPublicID = true;
            if (!parseExternalID()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseNotationDecl, state);
               return false;
            }
            break;
         case Ws3:
            if (!eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseNotationDecl, state);
               return false;
            }
            break;
         case Done:
            next();
            break;
      }
   }
   return false;
}

/*
  Parse choice [49] or seq [50].

  Precondition: the beginning '('S? is already read and the head
  stands on the first non-whitespace character after it.
*/
bool QXmlSimpleReaderPrivate::parseChoiceSeq()
{
   const signed char Init             = 0;
   const signed char Ws1              = 1; // eat whitespace
   const signed char CoS              = 2; // choice or set
   const signed char Ws2              = 3; // eat whitespace
   const signed char More             = 4; // more cp to read
   const signed char Name             = 5; // read name
   const signed char Done             = 6; //

   const signed char InpWs            = 0; // S
   const signed char InpOp            = 1; // (
   const signed char InpCp            = 2; //)
   const signed char InpQm            = 3; // ?
   const signed char InpAst           = 4; // *
   const signed char InpPlus          = 5; // +
   const signed char InpPipe          = 6; // |
   const signed char InpComm          = 7; // ,
   const signed char InpUnknown       = 8;

   static const signed char table[6][9] = {
      /*  InpWs   InpOp  InpCp  InpQm  InpAst  InpPlus  InpPipe  InpComm  InpUnknown */
      { -1,     Ws1,   -1,    -1,    -1,     -1,      -1,      -1,      Name  }, // Init
      { -1,     CoS,   -1,    -1,    -1,     -1,      -1,      -1,      CoS   }, // Ws1
      { Ws2,    -1,    Done,  Ws2,   Ws2,    Ws2,     More,    More,    -1    }, // CS
      { -1,     -1,    Done,  -1,    -1,     -1,      More,    More,    -1    }, // Ws2
      { -1,     Ws1,   -1,    -1,    -1,     -1,      -1,      -1,      Name  }, // More (same as Init)
      { Ws2,    -1,    Done,  Ws2,   Ws2,    Ws2,     More,    More,    -1    }  // Name (same as CS)
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      state = Init;
   } else {
      state = parseStack->pop().state;
#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseChoiceSeq (cont) in state %d", state);
#endif
      if (!parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;
         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();
#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }
         if (!(this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseChoiceSeq, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case Done:
            return true;
         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_UNEXPECTEDCHARACTER));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseChoiceSeq, state);
         return false;
      }
      if (is_S(c)) {
         input = InpWs;

      } else if (c == '(') {
         input = InpOp;

      } else if (c == ')') {
         input = InpCp;

      } else if (c == '?') {
         input = InpQm;

      } else if (c == '*') {
         input = InpAst;

      } else if (c == '+') {
         input = InpPlus;

      } else if (c == '|') {
         input = InpPipe;

      } else if (c == ',') {
         input = InpComm;

      } else {
         input = InpUnknown;
      }
      state = table[state][input];

      switch (state) {
         case Ws1:
            if (! next_eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseChoiceSeq, state);
               return false;
            }
            break;

         case CoS:
            if (! parseChoiceSeq()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseChoiceSeq, state);
               return false;
            }
            break;

         case Ws2:
            if (! next_eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseChoiceSeq, state);
               return false;
            }
            break;

         case More:
            if (! next_eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseChoiceSeq, state);
               return false;
            }
            break;

         case Name:
            parseName_useRef = false;
            if (!parseName()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseChoiceSeq, state);
               return false;
            }
            break;

         case Done:
            next();
            break;
      }
   }

   return false;
}

bool QXmlSimpleReaderPrivate::isExpandedEntityValueTooLarge(QString *errorMessage)
{
   QString entityNameBuffer;

   // For every entity check how many times all entity names were referenced in its value
   for (QMap<QString, QString>::const_iterator toSearchIt = entities.constBegin();
               toSearchIt != entities.constEnd(); ++toSearchIt) {
      const QString &toSearch = toSearchIt.key();

      // Do not check the same entities twice.
      if (! literalEntitySizes.contains(toSearch)) {
         // The amount of characters that weren't entity names, but literals, like 'X'.
         QString leftOvers = entities.value(toSearch);

         // How many times was entityName referenced by toSearch?
         for (QMap<QString, QString>::const_iterator referencedIt = entities.constBegin();
                     referencedIt != entities.constEnd(); ++referencedIt) {
            const QString &entityName = referencedIt.key();

            for (int i = 0; i < leftOvers.size() && i != -1; ) {
               entityNameBuffer = '&' + entityName + ';';

               i = leftOvers.indexOf(entityNameBuffer, i);
               if (i != -1) {
                  leftOvers.remove(i, entityName.size() + 2);
                  // The entityName we're currently trying to find was matched in this string; increase our count.
                  ++referencesToOtherEntities[toSearch][entityName];
               }
            }
         }

         literalEntitySizes[toSearch] = leftOvers.size();
      }
   }

   for (auto entityIt = referencesToOtherEntities.constBegin(); entityIt != referencesToOtherEntities.constEnd(); ++entityIt) {
      const QString &entity = entityIt.key();

      QHash<QString, int>::iterator expandedIt = expandedSizes.find(entity);

      if (expandedIt == expandedSizes.end()) {
         expandedIt = expandedSizes.insert(entity, literalEntitySizes.value(entity));

         for (auto referenceIt = entityIt->constBegin(); referenceIt != entityIt->constEnd(); ++referenceIt) {
            const QString &referenceTo = referenceIt.key();
            const int references = referencesToOtherEntities.value(entity).value(referenceTo);

            // The total size of an entity's value is the expanded size of all of its referenced entities, plus its literal size.
            *expandedIt += expandedSizes.value(referenceTo) * references + literalEntitySizes.value(referenceTo) * references;
         }

         if (*expandedIt > entityCharacterLimit) {

            if (errorMessage) {
               *errorMessage = QString::fromLatin1("The XML entity \"%1\" expands to a string that is too large to process (%2 characters > %3).")
                           .formatArgs(entity, QString::number(*expandedIt), QString::number(entityCharacterLimit));
            }
            return true;
         }
      }
   }
   return false;
}

/*
  Parse a EntityDecl [70].

  Precondition: the beginning '<!E' is already read and the head
  stand on the 'N' of '<!ENTITY'
*/
bool QXmlSimpleReaderPrivate::parseEntityDecl()
{
   const signed char Init             =  0;
   const signed char Ent              =  1; // parse "ENTITY"
   const signed char Ws1              =  2; // white space read
   const signed char Name             =  3; // parse name
   const signed char Ws2              =  4; // white space read
   const signed char EValue           =  5; // parse entity value
   const signed char EValueR          =  6; // same as EValue, but already reported
   const signed char ExtID            =  7; // parse ExternalID
   const signed char Ws3              =  8; // white space read
   const signed char Ndata            =  9; // parse "NDATA"
   const signed char Ws4              = 10; // white space read
   const signed char NNam             = 11; // parse name
   const signed char NNamR            = 12; // same as NNam, but already reported
   const signed char PEDec            = 13; // parse PEDecl
   const signed char Ws6              = 14; // white space read
   const signed char PENam            = 15; // parse name
   const signed char Ws7              = 16; // white space read
   const signed char PEVal            = 17; // parse entity value
   const signed char PEValR           = 18; // same as PEVal, but already reported
   const signed char PEEID            = 19; // parse ExternalID
   const signed char PEEIDR           = 20; // same as PEEID, but already reported
   const signed char WsE              = 21; // white space read
   const signed char Done             = 22;
   const signed char EDDone           = 23; // done, but also report an external, unparsed entity decl

   const signed char InpWs            = 0; // white space
   const signed char InpPer           = 1; // %
   const signed char InpQuot          = 2; // " or '
   const signed char InpGt            = 3; // >
   const signed char InpN             = 4; // N
   const signed char InpUnknown       = 5;

   static const signed char table[22][6] = {
      /*  InpWs  InpPer  InpQuot  InpGt  InpN    InpUnknown */
      { -1,    -1,     -1,      -1,    Ent,    -1      }, // Init
      { Ws1,   -1,     -1,      -1,    -1,     -1      }, // Ent
      { -1,    PEDec,  -1,      -1,    Name,   Name    }, // Ws1
      { Ws2,   -1,     -1,      -1,    -1,     -1      }, // Name
      { -1,    -1,     EValue,  -1,    -1,     ExtID   }, // Ws2
      { WsE,   -1,     -1,      Done,  -1,     -1      }, // EValue
      { WsE,   -1,     -1,      Done,  -1,     -1      }, // EValueR
      { Ws3,   -1,     -1,      EDDone, -1,     -1      }, // ExtID
      { -1,    -1,     -1,      EDDone, Ndata,  -1      }, // Ws3
      { Ws4,   -1,     -1,      -1,    -1,     -1      }, // Ndata
      { -1,    -1,     -1,      -1,    NNam,   NNam    }, // Ws4
      { WsE,   -1,     -1,      Done,  -1,     -1      }, // NNam
      { WsE,   -1,     -1,      Done,  -1,     -1      }, // NNamR
      { Ws6,   -1,     -1,      -1,    -1,     -1      }, // PEDec
      { -1,    -1,     -1,      -1,    PENam,  PENam   }, // Ws6
      { Ws7,   -1,     -1,      -1,    -1,     -1      }, // PENam
      { -1,    -1,     PEVal,   -1,    -1,     PEEID   }, // Ws7
      { WsE,   -1,     -1,      Done,  -1,     -1      }, // PEVal
      { WsE,   -1,     -1,      Done,  -1,     -1      }, // PEValR
      { WsE,   -1,     -1,      Done,  -1,     -1      }, // PEEID
      { WsE,   -1,     -1,      Done,  -1,     -1      }, // PEEIDR
      { -1,    -1,     -1,      Done,  -1,     -1      }  // WsE
   };

   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      state = Init;

   } else {

      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseEntityDecl (cont) in state %d", state);
#endif

      if (! parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();

#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif

         }

         if (! (this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case EValue:
            if ( !entityExist(name())) {
               QString errorMessage;
               if (isExpandedEntityValueTooLarge(&errorMessage)) {
                  reportParseError(errorMessage);
                  return false;
               }

               entities.insert(name(), string());
               if (declHnd) {
                  if (!declHnd->internalEntityDecl(name(), string())) {
                     reportParseError(declHnd->errorString());
                     return false;
                  }
               }
            }
            state = EValueR;
            break;

         case NNam:
            if ( !entityExist(name())) {
               externEntities.insert(name(), QXmlSimpleReaderPrivate::ExternEntity(publicId, systemId, ref()));
               if (dtdHnd) {
                  if (!dtdHnd->unparsedEntityDecl(name(), publicId, systemId, ref())) {
                     reportParseError(declHnd->errorString());
                     return false;
                  }
               }
            }
            state = NNamR;
            break;

         case PEVal:
            if ( !entityExist(name())) {
               parameterEntities.insert(name(), string());
               if (declHnd) {
                  if (!declHnd->internalEntityDecl('%' + name(), string())) {
                     reportParseError(declHnd->errorString());
                     return false;
                  }
               }
            }
            state = PEValR;
            break;

         case PEEID:
            if ( !entityExist(name())) {
               externParameterEntities.insert(name(), QXmlSimpleReaderPrivate::ExternParameterEntity(publicId, systemId));
               if (declHnd) {
                  if (!declHnd->externalEntityDecl('%' + name(), publicId, systemId)) {
                     reportParseError(declHnd->errorString());
                     return false;
                  }
               }
            }
            state = PEEIDR;
            break;

         case EDDone:
            if ( !entityExist(name())) {
               externEntities.insert(name(), QXmlSimpleReaderPrivate::ExternEntity(publicId, systemId, QString()));
               if (declHnd) {
                  if (!declHnd->externalEntityDecl(name(), publicId, systemId)) {
                     reportParseError(declHnd->errorString());
                     return false;
                  }
               }
            }
            return true;
         case Done:
            return true;
         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_LETTEREXPECTED));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
         return false;
      }

      if (is_S(c)) {
         input = InpWs;

      } else if (c == '%') {
         input = InpPer;

      } else if (c == '"' || c == '\'') {
         input = InpQuot;

      } else if (c == '>') {
         input = InpGt;

      } else if (c == 'N') {
         input = InpN;

      } else {
         input = InpUnknown;

      }

      state = table[state][input];

      switch (state) {
         case Ent:
            parseString_s = "NTITY";
            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
               return false;
            }
            break;

         case Ws1:
            if (! eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
               return false;
            }
            break;

         case Name:
            parseName_useRef = false;
            if (! parseName()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
               return false;
            }
            break;

         case Ws2:
            if (! eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
               return false;
            }
            break;

         case EValue:
         case EValueR:
            if (! parseEntityValue()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
               return false;
            }
            break;
         case ExtID:
            parseExternalID_allowPublicID = false;
            if (!parseExternalID()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
               return false;
            }
            break;

         case Ws3:
            if (! eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
               return false;
            }
            break;

         case Ndata:
            parseString_s = "NDATA";
            if (! parseString()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
               return false;
            }
            break;
         case Ws4:
            if (!eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
               return false;
            }
            break;
         case NNam:
         case NNamR:
            parseName_useRef = true;
            if (!parseName()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
               return false;
            }
            break;
         case PEDec:
            next();
            break;
         case Ws6:
            if (!eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
               return false;
            }
            break;
         case PENam:
            parseName_useRef = false;
            if (!parseName()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
               return false;
            }
            break;
         case Ws7:
            if (!eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
               return false;
            }
            break;
         case PEVal:
         case PEValR:
            if (!parseEntityValue()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
               return false;
            }
            break;
         case PEEID:
         case PEEIDR:
            parseExternalID_allowPublicID = false;
            if (!parseExternalID()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
               return false;
            }
            break;
         case WsE:
            if (!eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityDecl, state);
               return false;
            }
            break;
         case EDDone:
            next();
            break;
         case Done:
            next();
            break;
      }
   }
   return false;
}

/*
  Parse a EntityValue [9]
*/
bool QXmlSimpleReaderPrivate::parseEntityValue()
{
   const signed char Init             = 0;
   const signed char Dq               = 1; // EntityValue is double quoted
   const signed char DqC              = 2; // signed character
   const signed char DqPER            = 3; // PERefence
   const signed char DqRef            = 4; // Reference
   const signed char Sq               = 5; // EntityValue is double quoted
   const signed char SqC              = 6; // signed character
   const signed char SqPER            = 7; // PERefence
   const signed char SqRef            = 8; // Reference
   const signed char Done             = 9;

   const signed char InpDq            = 0; // "
   const signed char InpSq            = 1; // '
   const signed char InpAmp           = 2; // &
   const signed char InpPer           = 3; // %
   const signed char InpUnknown       = 4;

   static const signed char table[9][5] = {
      /*  InpDq  InpSq  InpAmp  InpPer  InpUnknown */
      { Dq,    Sq,    -1,     -1,     -1    }, // Init
      { Done,  DqC,   DqRef,  DqPER,  DqC   }, // Dq
      { Done,  DqC,   DqRef,  DqPER,  DqC   }, // DqC
      { Done,  DqC,   DqRef,  DqPER,  DqC   }, // DqPER
      { Done,  DqC,   DqRef,  DqPER,  DqC   }, // DqRef
      { SqC,   Done,  SqRef,  SqPER,  SqC   }, // Sq
      { SqC,   Done,  SqRef,  SqPER,  SqC   }, // SqC
      { SqC,   Done,  SqRef,  SqPER,  SqC   }, // SqPER
      { SqC,   Done,  SqRef,  SqPER,  SqC   }  // SqRef
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      state = Init;
   } else {
      state = parseStack->pop().state;
#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseEntityValue (cont) in state %d", state);
#endif
      if (!parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;
         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();
#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }
         if (!(this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseEntityValue, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case Done:
            return true;
         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_LETTEREXPECTED));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseEntityValue, state);
         return false;
      }

      if (c == '"') {
         input = InpDq;

      } else if (c == '\'') {
         input = InpSq;

      } else if (c == '&') {
         input = InpAmp;

      } else if (c == '%') {
         input = InpPer;

      } else {
         input = InpUnknown;
      }
      state = table[state][input];

      switch (state) {
         case Dq:
         case Sq:
            stringClear();
            next();
            break;

         case DqC:
         case SqC:
            stringAddC();
            next();
            break;

         case DqPER:
         case SqPER:
            parsePEReference_context = InEntityValue;
            if (!parsePEReference()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityValue, state);
               return false;
            }
            break;

         case DqRef:
         case SqRef:
            parseReference_context = InEntityValue;
            if (!parseReference()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseEntityValue, state);
               return false;
            }
            break;
         case Done:
            next();
            break;
      }
   }
   return false;
}

/*
  Parse a comment [15].

  Precondition: the beginning '<!' of the comment is already read and the head
  stands on the first '-' of '<!--'.

  If this funktion was successful, the head-position is on the first
  character after the comment.
*/
bool QXmlSimpleReaderPrivate::parseComment()
{
   const signed char Init             = 0;
   const signed char Dash1            = 1; // the first dash was read
   const signed char Dash2            = 2; // the second dash was read
   const signed char Com              = 3; // read comment
   const signed char Com2             = 4; // read comment (help state)
   const signed char ComE             = 5; // finished reading comment
   const signed char Done             = 6;

   const signed char InpDash          = 0; // -
   const signed char InpGt            = 1; // >
   const signed char InpUnknown       = 2;

   static const signed char table[6][3] = {
      /*  InpDash  InpGt  InpUnknown */
      { Dash1,   -1,    -1  }, // Init
      { Dash2,   -1,    -1  }, // Dash1
      { Com2,    Com,   Com }, // Dash2
      { Com2,    Com,   Com }, // Com
      { ComE,    Com,   Com }, // Com2
      { -1,      Done,  -1  }  // ComE
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseComment (cont) in state %d", state);
#endif

      if (!parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();

#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }
         if (! (this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseComment, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case Dash2:
            stringClear();
            break;

         case Com2:
            // if next character is not a dash than don't skip it
            if (! atEnd() && c != '-') {
               stringAddC('-');
            }
            break;

         case Done:
            return true;

         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_ERRORPARSINGCOMMENT));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseComment, state);
         return false;
      }

      if (c == '-') {
         input = InpDash;

      } else if (c == '>') {
         input = InpGt;

      } else {
         input = InpUnknown;
      }

      state = table[state][input];

      switch (state) {
         case Dash1:
            next();
            break;

         case Dash2:
            next();
            break;

         case Com:
            stringAddC();
            next();
            break;

         case Com2:
            next();
            break;

         case ComE:
            next();
            break;

         case Done:
            next();
            break;
      }
   }

   return false;
}

/*
    Parse an Attribute [41].

    Precondition: the head stands on the first character of the name
    of the attribute (i.e. all whitespaces are already parsed).

    The head stand on the next character after the end quotes. The
    variable name contains the name of the attribute and the variable
    string contains the value of the attribute.
*/
bool QXmlSimpleReaderPrivate::parseAttribute()
{
   const int Init             = 0;
   const int PName            = 1; // parse name
   const int Ws               = 2; // eat ws
   const int Eq               = 3; // the '=' was read
   const int Quotes           = 4; // " or ' were read

   const int InpNameBe        = 0;
   const int InpEq            = 1; // =
   const int InpDq            = 2; // "
   const int InpSq            = 3; // '
   const int InpUnknown       = 4;

   static const int table[4][5] = {
      /*  InpNameBe  InpEq  InpDq    InpSq    InpUnknown */
      { PName,     -1,    -1,      -1,      -1    }, // Init
      { -1,        Eq,    -1,      -1,      Ws    }, // PName
      { -1,        Eq,    -1,      -1,      -1    }, // Ws
      { -1,        -1,    Quotes,  Quotes,  -1    }  // Eq
   };
   int state;
   int input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseAttribute (cont) in state %d", state);
#endif

      if (!parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();
#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }
         if (!(this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseAttribute, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case Quotes:
            // Done
            return true;

         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_UNEXPECTEDCHARACTER));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseAttribute, state);
         return false;
      }

      if (determineNameChar(c) == NameBeginning) {
         input = InpNameBe;

      } else if (c == '=') {
         input = InpEq;

      } else if (c == '"') {
         input = InpDq;

      } else if (c == '\'') {
         input = InpSq;

      } else {
         input = InpUnknown;
      }

      state = table[state][input];

      switch (state) {
         case PName:
            parseName_useRef = false;
            if (!parseName()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttribute, state);
               return false;
            }
            break;
         case Ws:
            if (!eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttribute, state);
               return false;
            }
            break;
         case Eq:
            if (!next_eat_ws()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttribute, state);
               return false;
            }
            break;
         case Quotes:
            if (!parseAttValue()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseAttribute, state);
               return false;
            }
            break;
      }
   }
   return false;
}

/*
  Parse a Name [5] and store the name in name or ref (if useRef is true).
*/
bool QXmlSimpleReaderPrivate::parseName()
{
   const int Init             = 0;
   const int Name1            = 1; // parse first character of the name
   const int Name             = 2; // parse name
   const int Done             = 3;

   static const int table[3][3] = {
      /*  InpNameBe  InpNameCh  InpUnknown */
      { Name1,     -1,        -1    }, // Init
      { Name,      Name,      Done  }, // Name1
      { Name,      Name,      Done  }  // Name
   };
   int state;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseName (cont) in state %d", state);
#endif

      if (!parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();

#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }
         if (!(this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseName, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case Done:
            return true;
         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_LETTEREXPECTED));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseName, state);
         return false;
      }

      // we can safely do the (int) cast thanks to the Q_ASSERTs earlier in this function
      state = table[state][(int)fastDetermineNameChar(c)];

      switch (state) {
         case Name1:
            if (parseName_useRef) {
               refClear();
               refAddC();
            } else {
               nameClear();
               nameAddC();
            }
            next();
            break;
         case Name:
            if (parseName_useRef) {
               refAddC();
            } else {
               nameAddC();
            }
            next();
            break;
      }
   }
   return false;
}

/*
  Parse a Nmtoken [7] and store the name in name.
*/
bool QXmlSimpleReaderPrivate::parseNmtoken()
{
   const signed char Init             = 0;
   const signed char NameF            = 1;
   const signed char Name             = 2;
   const signed char Done             = 3;

   const signed char InpNameCh        = 0; // NameChar without InpNameBe
   const signed char InpUnknown       = 1;

   static const signed char table[3][2] = {
      /*  InpNameCh  InpUnknown */
      { NameF,     -1    }, // Init
      { Name,      Done  }, // NameF
      { Name,      Done  }  // Name
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseNmtoken (cont) in state %d", state);
#endif

      if (!parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();
#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }
         if (!(this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseNmtoken, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case Done:
            return true;
         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_LETTEREXPECTED));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseNmtoken, state);
         return false;
      }
      if (determineNameChar(c) == NotName) {
         input = InpUnknown;
      } else {
         input = InpNameCh;
      }
      state = table[state][input];

      switch (state) {
         case NameF:
            nameClear();
            nameAddC();
            next();
            break;
         case Name:
            nameAddC();
            next();
            break;
      }
   }
   return false;
}

/*
  Parse a Reference [67].

  parseReference_charDataRead is set to true if the reference must not be
  parsed. The character(s) which the reference mapped to are appended to
  string. The head stands on the first character after the reference.

  parseReference_charDataRead is set to false if the reference must be parsed.
  The charachter(s) which the reference mapped to are inserted at the reference
  position. The head stands on the first character of the replacement).
*/
bool QXmlSimpleReaderPrivate::parseReference()
{
   // temporary variables (only used in very local context, so they don't
   // interfere with incremental parsing)
   uint tmp;
   bool ok;

   const signed char Init             =  0;
   const signed char SRef             =  1; // start of a reference
   const signed char ChRef            =  2; // parse CharRef
   const signed char ChDec            =  3; // parse CharRef decimal
   const signed char ChHexS           =  4; // start CharRef hexadecimal
   const signed char ChHex            =  5; // parse CharRef hexadecimal
   const signed char Name             =  6; // parse name
   const signed char DoneD            =  7; // done CharRef decimal
   const signed char DoneH            =  8; // done CharRef hexadecimal
   const signed char DoneN            =  9; // done EntityRef

   const signed char InpAmp           = 0; // &
   const signed char InpSemi          = 1; // ;
   const signed char InpHash          = 2; // #
   const signed char InpX             = 3; // x
   const signed char InpNum           = 4; // 0-9
   const signed char InpHex           = 5; // a-f A-F
   const signed char InpUnknown       = 6;

   static const signed char table[8][7] = {
      /*  InpAmp  InpSemi  InpHash  InpX     InpNum  InpHex  InpUnknown */
      { SRef,   -1,      -1,      -1,      -1,     -1,     -1    }, // Init
      { -1,     -1,      ChRef,   Name,    Name,   Name,   Name  }, // SRef
      { -1,     -1,      -1,      ChHexS,  ChDec,  -1,     -1    }, // ChRef
      { -1,     DoneD,   -1,      -1,      ChDec,  -1,     -1    }, // ChDec
      { -1,     -1,      -1,      -1,      ChHex,  ChHex,  -1    }, // ChHexS
      { -1,     DoneH,   -1,      -1,      ChHex,  ChHex,  -1    }, // ChHex
      { -1,     DoneN,   -1,      -1,      -1,     -1,     -1    }  // Name
   };
   signed char state;
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      parseReference_charDataRead = false;
      state = Init;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseReference (cont) in state %d", state);
#endif

      if (!parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();

#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }

         if (!(this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseReference, state);
            return false;
         }
      }
   }

   for (;;) {
      switch (state) {
         case DoneD:
            return true;
         case DoneH:
            return true;
         case DoneN:
            return true;
         case -1:
            // Error
            reportParseError(QString::fromLatin1(XMLERR_ERRORPARSINGREFERENCE));
            return false;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseReference, state);
         return false;
      }

      if (c > 0xFF) {
         input = InpUnknown;

      } else if (c == '&') {
         input = InpAmp;

      } else if (c == ';') {
         input = InpSemi;

      } else if (c == '#') {
         input = InpHash;

      } else if (c == 'x') {
         input = InpX;

      } else if ('0' <= c && c <= '9') {
         input = InpNum;

      } else if ('a' <= c && c <= 'f') {
         input = InpHex;

      } else if ('A' <= c && c <= 'F') {
         input = InpHex;

      } else {
         input = InpUnknown;
      }

      state = table[state][input];

      switch (state) {
         case SRef:
            refClear();
            next();
            break;
         case ChRef:
            next();
            break;
         case ChDec:
            refAddC();
            next();
            break;
         case ChHexS:
            next();
            break;
         case ChHex:
            refAddC();
            next();
            break;
         case Name:
            // read the name into the ref
            parseName_useRef = true;
            if (! parseName()) {
               parseFailed(&QXmlSimpleReaderPrivate::parseReference, state);
               return false;
            }
            break;

         case DoneD:
            tmp = ref().toInteger<uint>(&ok, 10);

            if (ok) {
               stringAddC(QChar(char32_t(tmp)));

            } else {
               reportParseError(QString::fromLatin1(XMLERR_ERRORPARSINGREFERENCE));
               return false;
            }

            parseReference_charDataRead = true;
            next();
            break;

         case DoneH:
            tmp = ref().toInteger<uint>(&ok, 16);

            if (ok) {
               stringAddC(QChar(char32_t(tmp)));
            } else {
               reportParseError(QString::fromLatin1(XMLERR_ERRORPARSINGREFERENCE));
               return false;
            }

            parseReference_charDataRead = true;
            next();
            break;

         case DoneN:
            if (! processReference()) {
               return false;
            }
            next();
            break;
      }
   }
   return false;
}

bool QXmlSimpleReaderPrivate::processReference()
{
   QString reference = ref();
   if (reference == "amp") {
      if (parseReference_context == InEntityValue) {
         // Bypassed
         stringAddC('&');
         stringAddC('a');
         stringAddC('m');
         stringAddC('p');
         stringAddC(';');

      } else {
         // Included or Included in literal
         stringAddC('&');
      }
      parseReference_charDataRead = true;

   } else if (reference == "lt") {
      if (parseReference_context == InEntityValue) {
         // Bypassed
         stringAddC('&');
         stringAddC('l');
         stringAddC('t');
         stringAddC(';');

      } else {
         // Included or Included in literal
         stringAddC('<');
      }
      parseReference_charDataRead = true;

   } else if (reference == "gt") {
      if (parseReference_context == InEntityValue) {
         // Bypassed
         stringAddC('&');
         stringAddC('g');
         stringAddC('t');
         stringAddC(';');

      } else {
         // Included or Included in literal
         stringAddC('>');
      }
      parseReference_charDataRead = true;

   } else if (reference == "apos") {
      if (parseReference_context == InEntityValue) {
         // Bypassed
         stringAddC('&');
         stringAddC('a');
         stringAddC('p');
         stringAddC('o');
         stringAddC('s');
         stringAddC(';');

      } else {
         // Included or Included in literal
         stringAddC('\'');
      }
      parseReference_charDataRead = true;

   } else if (reference == "quot") {
      if (parseReference_context == InEntityValue) {
         // Bypassed
         stringAddC('&');
         stringAddC('q');
         stringAddC('u');
         stringAddC('o');
         stringAddC('t');
         stringAddC(';');

      } else {
         // Included or Included in literal
         stringAddC('"');
      }
      parseReference_charDataRead = true;

   } else {
      QMap<QString, QString>::iterator it;
      it = entities.find(reference);

      if (it != entities.end()) {
         // "Internal General"

         switch (parseReference_context) {
            case InContent:
               // Included
               if (! insertXmlRef(*it, reference, false)) {
                  return false;
               }
               parseReference_charDataRead = false;
               break;

            case InAttributeValue:
               // Included in literal
               if (! insertXmlRef(*it, reference, true)) {
                  return false;
               }
               parseReference_charDataRead = false;
               break;

            case InEntityValue: {
               // Bypassed
               stringAddC('&');
               for (int i = 0; i < (int)reference.length(); i++) {
                  stringAddC(reference[i]);
               }

               stringAddC(';');
               parseReference_charDataRead = true;
            }
            break;

            case InDTD:
               // Forbidden
               parseReference_charDataRead = false;
               reportParseError(QString::fromLatin1(XMLERR_INTERNALGENERALENTITYINDTD));
               return false;
         }

      } else {
         QMap<QString, QXmlSimpleReaderPrivate::ExternEntity>::iterator itExtern;
         itExtern = externEntities.find(reference);

         if (itExtern == externEntities.end()) {
            // entity not declared
            // ### check this case for conformance
            if (parseReference_context == InEntityValue) {
               // Bypassed
               stringAddC(QChar('&'));
               for (int i = 0; i < (int)reference.length(); i++) {
                  stringAddC(reference[i]);
               }

               stringAddC(QChar(';'));
               parseReference_charDataRead = true;

            } else {
               // if we have some char data read, report it now
               if (parseReference_context == InContent) {
                  if (contentCharDataRead) {
                     if (reportWhitespaceCharData || !string().simplified().isEmpty()) {
                        if (contentHnd != nullptr && !contentHnd->characters(string())) {
                           reportParseError(contentHnd->errorString());
                           return false;
                        }
                     }
                     stringClear();
                     contentCharDataRead = false;
                  }
               }

               if (contentHnd) {
                  qt_xml_skipped_entity_in_content = parseReference_context == InContent;
                  if (!contentHnd->skippedEntity(reference)) {
                     qt_xml_skipped_entity_in_content = false;
                     reportParseError(contentHnd->errorString());
                     return false; // error
                  }
                  qt_xml_skipped_entity_in_content = false;
               }
            }
         } else if ((*itExtern).notation.isEmpty()) {
            // "External Parsed General"
            switch (parseReference_context) {
               case InContent: {
                  // Included if validating
                  bool skipIt = true;

                  if (entityRes) {
                     QXmlInputSource *ret = nullptr;
                     if (!entityRes->resolveEntity((*itExtern).publicId, (*itExtern).systemId, ret)) {
                        delete ret;
                        reportParseError(entityRes->errorString());
                        return false;
                     }

                     if (ret) {
                        QString xmlRefString;
                        QString buffer = ret->data();

                        while (buffer.length() > 0) {
                           xmlRefString += buffer;
                           ret->fetchData();
                           buffer = ret->data();
                        }

                        delete ret;
                        if (!stripTextDecl(xmlRefString)) {
                           reportParseError(QString::fromLatin1(XMLERR_ERRORINTEXTDECL));
                           return false;
                        }

                        if (! insertXmlRef(xmlRefString, reference, false)) {
                           return false;
                        }

                        skipIt = false;
                     }
                  }
                  if (skipIt && contentHnd) {
                     qt_xml_skipped_entity_in_content = true;
                     if (! contentHnd->skippedEntity(reference)) {
                        qt_xml_skipped_entity_in_content = false;
                        reportParseError(contentHnd->errorString());
                        return false; // error
                     }
                     qt_xml_skipped_entity_in_content = false;
                  }
                  parseReference_charDataRead = false;
               }
               break;

               case InAttributeValue:
                  // Forbidden
                  parseReference_charDataRead = false;
                  reportParseError(QString::fromLatin1(XMLERR_EXTERNALGENERALENTITYINAV));
                  return false;

               case InEntityValue: {
                  // Bypassed
                  stringAddC('&');

                  for (int i = 0; i < (int)reference.length(); i++) {
                     stringAddC(reference[i]);
                  }

                  stringAddC(';');
                  parseReference_charDataRead = true;
               }
               break;

               case InDTD:
                  // Forbidden
                  parseReference_charDataRead = false;
                  reportParseError(QString::fromLatin1(XMLERR_EXTERNALGENERALENTITYINDTD));
                  return false;
            }
         } else {
            // "Unparsed"
            // ### notify for "Occurs as Attribute Value" missing (but this is no refence, anyway)
            // Forbidden
            parseReference_charDataRead = false;
            reportParseError(QString::fromLatin1(XMLERR_UNPARSEDENTITYREFERENCE));
            return false; // error
         }
      }
   }
   return true; // no error
}

bool QXmlSimpleReaderPrivate::parseString()
{
   const signed char InpCharExpected  = 0; // the character that was expected
   const signed char InpUnknown       = 1;

   signed char state; // state in this function is the position in the string s
   signed char input;

   if (parseStack == nullptr || parseStack->isEmpty()) {
      Done = parseString_s.length();
      state = 0;

   } else {
      state = parseStack->pop().state;

#if defined(CS_SHOW_DEBUG_XML)
      qDebug("QXmlSimpleReader: parseString (cont) in state %d", state);
#endif

      if (!parseStack->isEmpty()) {
         ParseFunction function = parseStack->top().function;

         if (function == &QXmlSimpleReaderPrivate::eat_ws) {
            parseStack->pop();

#if defined(CS_SHOW_DEBUG_XML)
            qDebug("QXmlSimpleReader: eat_ws (cont)");
#endif
         }

         if (! (this->*function)()) {
            parseFailed(&QXmlSimpleReaderPrivate::parseString, state);
            return false;
         }
      }
   }

   for (;;) {
      if (state == Done) {
         return true;
      }

      if (atEnd()) {
         unexpectedEof(&QXmlSimpleReaderPrivate::parseString, state);
         return false;
      }
      if (c == parseString_s[(int)state]) {
         input = InpCharExpected;

      } else {
         input = InpUnknown;
      }

      if (input == InpCharExpected) {
         ++state;

      } else {
         // Error
         reportParseError(QString::fromLatin1(XMLERR_UNEXPECTEDCHARACTER));
         return false;
      }

      next();
   }

   return false;
}

bool QXmlSimpleReaderPrivate::insertXmlRef(const QString &data, const QString &name, bool inLiteral)
{
   if (inLiteral) {
      QString tmp = data;
      xmlRefStack.push(XmlRef(name, tmp.replace(QChar('\"'), QString("&quot;")).replace(QChar('\''), QString("&apos;"))));
   } else {
      xmlRefStack.push(XmlRef(name, data));
   }

   int n = qMax(parameterEntities.count(), entities.count());
   if (xmlRefStack.count() > n + 1) {
      // recursive entities
      reportParseError(QString::fromLatin1(XMLERR_RECURSIVEENTITIES));
      return false;
   }

   if (reportEntities && lexicalHnd) {
      if (!lexicalHnd->startEntity(name)) {
         reportParseError(lexicalHnd->errorString());
         return false;
      }
   }
   return true;
}

void QXmlSimpleReaderPrivate::next()
{
   int count = xmlRefStack.size();
   while (count != 0) {
      if (xmlRefStack.top().isEmpty()) {
         xmlRefStack.pop_back();
         --count;

      } else {
         c = xmlRefStack.top().next();
         return;
      }
   }

   // the following could be written nicer, but since it is a time-critical
   // function, rather optimize for speed
   ushort uc = c.unicode();
   c = inputSource->next();

   // If we are not incremental parsing, we just skip over EndOfData chars to give the
   // parser an uninterrupted stream of document chars.
   if (c == QXmlInputSource::EndOfData && parseStack == nullptr) {
      c = inputSource->next();
   }

   if (uc == '\n') {
      ++lineNr;
      columnNr = -1;

   } else if (uc == '\r') {
      if (c != '\n') {
         ++lineNr;
         columnNr = -1;
      }
   }

   ++columnNr;
}

/*
  This private function moves the cursor to the next non-whitespace character.
  This function does not move the cursor if the actual cursor position is a
  non-whitespace charcter.

  Returns false when you use incremental parsing and this function reaches EOF
  with reading only whitespace characters. In this case it also poplulates the
  parseStack with useful information. In all other cases, this function returns
  true.
*/
bool QXmlSimpleReaderPrivate::eat_ws()
{
   while (! atEnd()) {
      if (! is_S(c)) {
         return true;
      }
      next();
   }

   if (parseStack != nullptr) {
      unexpectedEof(&QXmlSimpleReaderPrivate::eat_ws, 0);
      return false;
   }
   return true;
}

bool QXmlSimpleReaderPrivate::next_eat_ws()
{
   next();
   return eat_ws();
}

void QXmlSimpleReaderPrivate::init(const QXmlInputSource *i)
{
   lineNr = 0;
   columnNr = -1;
   inputSource = const_cast<QXmlInputSource *>(i);
   initData();

   externParameterEntities.clear();
   parameterEntities.clear();
   externEntities.clear();
   entities.clear();

   tags.clear();

   doctype.clear();
   xmlVersion.clear();
   encoding.clear();
   standalone = QXmlSimpleReaderPrivate::Unknown;
   error.clear();
}

/*
  This private function initializes the XML data related variables. Especially,
  it reads the data from the input source.
*/
void QXmlSimpleReaderPrivate::initData()
{
   c = QXmlInputSource::EndOfData;
   xmlRefStack.clear();
   next();
}

/*
  Returns true if a entity with the name \a e exists,
  otherwise returns false.
*/
bool QXmlSimpleReaderPrivate::entityExist(const QString &e) const
{
   if ( parameterEntities.find(e) == parameterEntities.end() &&
         externParameterEntities.find(e) == externParameterEntities.end() &&
         externEntities.find(e) == externEntities.end() &&
         entities.find(e) == entities.end()) {
      return false;
   } else {
      return true;
   }
}

void QXmlSimpleReaderPrivate::reportParseError(const QString &error)
{
   this->error = error;
   if (errorHnd) {
      if (this->error.isEmpty()) {
         const QXmlParseException ex(QString::fromLatin1(XMLERR_OK), columnNr + 1, lineNr + 1,
                                     thisPublicId, thisSystemId);
         errorHnd->fatalError(ex);
      } else {
         const QXmlParseException ex(this->error, columnNr + 1, lineNr + 1,
                                     thisPublicId, thisSystemId);
         errorHnd->fatalError(ex);
      }
   }
}

/*
  This private function is called when a parsing function encounters an
  unexpected EOF. It decides what to do (depending on incremental parsing or
  not). \a where is a pointer to the function where the error occurred and \a
  state is the parsing state in this function.
*/
void QXmlSimpleReaderPrivate::unexpectedEof(ParseFunction where, int state)
{
   if (parseStack == nullptr) {
      reportParseError(QString::fromLatin1(XMLERR_UNEXPECTEDEOF));

   } else {
      if (c == QXmlInputSource::EndOfDocument) {
         reportParseError(QString::fromLatin1(XMLERR_UNEXPECTEDEOF));
      } else {
         pushParseState(where, state);
      }
   }
}

/*
  This private function is called when a parse...() function returned false. It
  determines if there was an error or if incremental parsing simply went out of
  data and does the right thing for the case. \a where is a pointer to the
  function where the error occurred and \a state is the parsing state in this
  function.
*/
void QXmlSimpleReaderPrivate::parseFailed(ParseFunction where, int state)
{
   if (parseStack != nullptr && error.isEmpty()) {
      pushParseState(where, state);
   }
}

/*
  This private function pushes the function pointer \a function and state \a
  state to the parse stack. This is used when you are doing an incremental
  parsing and reach the end of file too early.

  Only call this function when d->parseStack!=0.
*/
void QXmlSimpleReaderPrivate::pushParseState(ParseFunction function, int state)
{
   QXmlSimpleReaderPrivate::ParseState ps;
   ps.function = function;
   ps.state = state;
   parseStack->push(ps);
}

const QString &QXmlSimpleReaderPrivate::string()
{
   return stringValue;
}

const QString &QXmlSimpleReaderPrivate::name()
{
   return nameValue;
}

const QString &QXmlSimpleReaderPrivate::ref()
{
   return refValue;
}

void QXmlSimpleReaderPrivate::stringAddC(QChar ch)
{
   stringValue += ch;
}

void QXmlSimpleReaderPrivate::nameAddC(QChar ch)
{
   nameValue += ch;
}

void QXmlSimpleReaderPrivate::refAddC(QChar ch)
{
   refValue += ch;
}
