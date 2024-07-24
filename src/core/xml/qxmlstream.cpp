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

#include <qxmlstream.h>
#include <qxmlstream_p.h>

#include <qbuffer.h>
#include <qcoreapplication.h>
#include <qdebug.h>
#include <qfile.h>
#include <qstack.h>
#include <qtextcodec.h>

#include <qxmlutils_p.h>

#include <stdlib.h>
#include <stdio.h>

QXmlStreamEntityResolver::~QXmlStreamEntityResolver()
{
}

QString QXmlStreamEntityResolver::resolveEntity(const QString &, const QString &)
{
   return QString();
}

QString QXmlStreamEntityResolver::resolveUndeclaredEntity(const QString &)
{
   return QString();
}

QString QXmlStreamReaderPrivate::resolveUndeclaredEntity(const QString &name)
{
   if (entityResolver) {
      return entityResolver->resolveUndeclaredEntity(name);
   }

   return QString();
}

void QXmlStreamReader::setEntityResolver(QXmlStreamEntityResolver *resolver)
{
   Q_D(QXmlStreamReader);
   d->entityResolver = resolver;
}

QXmlStreamEntityResolver *QXmlStreamReader::entityResolver() const
{
   Q_D(const QXmlStreamReader);
   return d->entityResolver;
}

QXmlStreamReader::QXmlStreamReader()
   : d_ptr(new QXmlStreamReaderPrivate(this))
{
}

QXmlStreamReader::QXmlStreamReader(QIODevice *device)
   : d_ptr(new QXmlStreamReaderPrivate(this))
{
   setDevice(device);
}

QXmlStreamReader::QXmlStreamReader(const QByteArray &data)
   : d_ptr(new QXmlStreamReaderPrivate(this))
{
   Q_D(QXmlStreamReader);
   d->dataBuffer = data;
}

QXmlStreamReader::QXmlStreamReader(const QString &data)
   : d_ptr(new QXmlStreamReaderPrivate(this))
{
   Q_D(QXmlStreamReader);

   d->dataBuffer = d->codec->fromUnicode(data);
   d->decoder    = d->codec->makeDecoder();

   d->lockEncoding = true;
}

QXmlStreamReader::QXmlStreamReader(const char *data)
   : d_ptr(new QXmlStreamReaderPrivate(this))
{
   Q_D(QXmlStreamReader);
   d->dataBuffer = QByteArray(data);
}

QXmlStreamReader::~QXmlStreamReader()
{
   Q_D(QXmlStreamReader);

   if (d->deleteDevice) {
      delete d->device;
   }
}

void QXmlStreamReader::setDevice(QIODevice *device)
{
   Q_D(QXmlStreamReader);

   if (d->deleteDevice) {
      delete d->device;
      d->deleteDevice = false;
   }

   d->device = device;
   d->init();
}

QIODevice *QXmlStreamReader::device() const
{
   Q_D(const QXmlStreamReader);
   return d->device;
}

void QXmlStreamReader::addData(const QByteArray &data)
{
   Q_D(QXmlStreamReader);

   if (d->device) {
      qWarning("QXmlStreamReader::addData() Invalid when using a device");
      return;
   }

   d->dataBuffer += data;
}

void QXmlStreamReader::addData(const QString &data)
{
   Q_D(QXmlStreamReader);
   d->lockEncoding = true;
   addData(d->codec->fromUnicode(data));
}

void QXmlStreamReader::addData(const char *data)
{
   addData(QByteArray(data));
}

void QXmlStreamReader::clear()
{
   Q_D(QXmlStreamReader);
   d->init();

   if (d->device) {
      if (d->deleteDevice) {
         delete d->device;
      }

      d->device = nullptr;
   }
}

bool QXmlStreamReader::atEnd() const
{
   Q_D(const QXmlStreamReader);

   if (d->atEnd && ((d->type == QXmlStreamReader::Invalid && d->error == PrematureEndOfDocumentError)
               || (d->type == QXmlStreamReader::EndDocument))) {

      if (d->device) {
         return d->device->atEnd();
      } else {
         return !d->dataBuffer.size();
      }
   }

   return (d->atEnd || d->type == QXmlStreamReader::Invalid);
}

QXmlStreamReader::TokenType QXmlStreamReader::readNext()
{
   Q_D(QXmlStreamReader);

   if (d->type != Invalid) {
      if (!d->hasCheckedStartDocument) {
         if (!d->checkStartDocument()) {
            return d->type;   // synthetic StartDocument or error
         }
      }

      d->parse();

      if (d->atEnd && d->type != EndDocument && d->type != Invalid) {
         d->raiseError(PrematureEndOfDocumentError);

      } else if (! d->atEnd && d->type == EndDocument) {
         d->raiseWellFormedError(QXmlStream::tr("Extra content at end of document."));
      }

   } else if (d->error == PrematureEndOfDocumentError) {
      // resume error
      d->type = NoToken;
      d->atEnd = false;
      d->token = -1;
      return readNext();
   }

   return d->type;
}

QXmlStreamReader::TokenType QXmlStreamReader::tokenType() const
{
   Q_D(const QXmlStreamReader);
   return d->type;
}

bool QXmlStreamReader::readNextStartElement()
{
   while (readNext() != Invalid) {
      if (isEndElement()) {
         return false;
      } else if (isStartElement()) {
         return true;
      }
   }

   return false;
}

void QXmlStreamReader::skipCurrentElement()
{
   int depth = 1;

   while (depth && readNext() != Invalid) {
      if (isEndElement()) {
         --depth;
      } else if (isStartElement()) {
         ++depth;
      }
   }
}

/*
 * Use the following Perl script to generate the error string index list:
===== PERL SCRIPT ====
print "static const char QXmlStreamReader_tokenTypeString_string[] =\n";
$counter = 0;
$i = 0;
while (<STDIN>) {
    chomp;
    print "    \"$_\\0\"\n";
    $sizes[$i++] = $counter;
    $counter += length 1 + $_;
}
print "    \"\\0\";\n\nstatic const short QXmlStreamReader_tokenTypeString_indices[] = {\n    ";
for ($j = 0; $j < $i; ++$j) {
    printf "$sizes[$j], ";
}
print "0\n};\n";
===== PERL SCRIPT ====

 * The input data is as follows (copied from qxmlstream.h):
NoToken
Invalid
StartDocument
EndDocument
StartElement
EndElement
Characters
Comment
DTD
EntityReference
ProcessingInstruction
*/
static const char QXmlStreamReader_tokenTypeString_string[] =
      "NoToken\0"
      "Invalid\0"
      "StartDocument\0"
      "EndDocument\0"
      "StartElement\0"
      "EndElement\0"
      "Characters\0"
      "Comment\0"
      "DTD\0"
      "EntityReference\0"
      "ProcessingInstruction\0";

static const short QXmlStreamReader_tokenTypeString_indices[] = {
   0, 8, 16, 30, 42, 55, 66, 77, 85, 89, 105, 0
};

void QXmlStreamReader::setNamespaceProcessing(bool enable)
{
   Q_D(QXmlStreamReader);
   d->namespaceProcessing = enable;
}

bool QXmlStreamReader::namespaceProcessing() const
{
   Q_D(const QXmlStreamReader);
   return d->namespaceProcessing;
}

QString QXmlStreamReader::tokenString() const
{
   Q_D(const QXmlStreamReader);
   return QString::fromUtf8(QXmlStreamReader_tokenTypeString_string + QXmlStreamReader_tokenTypeString_indices[d->type]);
}

QXmlStreamPrivateTagStack::QXmlStreamPrivateTagStack()
{
   tagStack.reserve(16);

   namespaceDeclarations.push(NamespaceDeclaration());
   NamespaceDeclaration &namespaceDeclaration = namespaceDeclarations.top();

   namespaceDeclaration.prefix       = "xml";
   namespaceDeclaration.namespaceUri = "http://www.w3.org/XML/1998/namespace";
}

QXmlStreamReaderPrivate::QXmlStreamReaderPrivate(QXmlStreamReader *q)
   : q_ptr(q)
{
   device       = nullptr;
   deleteDevice = false;
   decoder      = nullptr;

   stack_size  = 64;
   sym_stack   = nullptr;
   state_stack = nullptr;

   reallocateStack();
   entityResolver = nullptr;
   init();

   entityHash.insert("lt",   Entity::createLiteral("<"));
   entityHash.insert("gt",   Entity::createLiteral(">"));
   entityHash.insert("amp",  Entity::createLiteral("&"));
   entityHash.insert("apos", Entity::createLiteral("'"));
   entityHash.insert("quot", Entity::createLiteral("\""));
}

void QXmlStreamReaderPrivate::init()
{
   tos = 0;
   scanDtd = false;
   token = -1;
   token_char = 0;
   isEmptyElement = false;
   isWhitespace = true;
   isCDATA = false;
   standalone = false;
   tos = 0;
   resumeReduction = 0;
   state_stack[tos++] = 0;
   state_stack[tos] = 0;

   putStack.clear();
   putStack.reserve(32);
   textBuffer.clear();
   tagStack.clear();

   tagsDone = false;
   attributes.clear();
   attributes.reserve(16);
   lineNumber = lastLineStart = characterOffset = 0;
   nbytesread = 0;

   codec = QTextCodec::codecForMib(106);       // utf8

   delete decoder;
   decoder = nullptr;

   attributeStack.clear();
   attributeStack.reserve(16);
   entityParser = nullptr;
   hasCheckedStartDocument = false;
   normalizeLiterals = false;
   hasSeenTag = false;
   atEnd = false;
   inParseEntity = false;
   referenceToUnparsedEntityDetected = false;
   referenceToParameterEntityDetected = false;
   hasExternalDtdSubset = false;
   lockEncoding = false;
   namespaceProcessing = true;
   rawReadBuffer.clear();
   dataBuffer.clear();

   readBuffer.clear();
   readBuffer_Iter = readBuffer.begin();

   type  = QXmlStreamReader::NoToken;
   error = QXmlStreamReader::NoError;
}

void QXmlStreamReaderPrivate::parseEntity(const QString &value)
{
   Q_Q(QXmlStreamReader);

   if (value.isEmpty()) {
      return;
   }

   if (!entityParser) {
      entityParser = new QXmlStreamReaderPrivate(q);
   } else {
      entityParser->init();
   }

   entityParser->inParseEntity = true;
   entityParser->readBuffer    = value;
   entityParser->injectToken(PARSE_ENTITY);

   while (!entityParser->atEnd && entityParser->type != QXmlStreamReader::Invalid) {
      entityParser->parse();
   }

   if (entityParser->type == QXmlStreamReader::Invalid || entityParser->tagStack.size()) {
      raiseWellFormedError(QXmlStream::tr("Invalid entity value."));
   }

}

inline void QXmlStreamReaderPrivate::reallocateStack()
{
   stack_size <<= 1;

   sym_stack = reinterpret_cast<Value *> (realloc(sym_stack, stack_size * sizeof(Value)));
   Q_CHECK_PTR(sym_stack);

   state_stack = reinterpret_cast<int *> (realloc(state_stack, stack_size * sizeof(int)));
   Q_CHECK_PTR(sym_stack);
}

QXmlStreamReaderPrivate::~QXmlStreamReaderPrivate()
{
   delete decoder;

   free(sym_stack);
   free(state_stack);
   delete entityParser;
}

inline uint QXmlStreamReaderPrivate::filterCarriageReturn()
{
   uint peekc = peekChar();

   if (peekc == '\n') {
      if (putStack.size()) {
         putStack.pop();
      } else {
         ++readBuffer_Iter;
      }

      return peekc;
   }

   if (peekc == 0) {
      putChar('\r');
      return 0;
   }

   return '\n';
}

inline uint QXmlStreamReaderPrivate::getChar()
{
   uint c;

   if (putStack.size() != 0) {
      c = atEnd ? 0 : putStack.pop();

   } else if (readBuffer_Iter != readBuffer.cend() ) {
      c = readBuffer_Iter->unicode();
      ++readBuffer_Iter;

   } else {
      c = getChar_helper();

   }

   return c;
}

inline uint QXmlStreamReaderPrivate::peekChar()
{
   uint c;

   if (putStack.size() != 0) {
      c = putStack.top();

   } else if (readBuffer_Iter != readBuffer.cend() ) {
      c = readBuffer_Iter->unicode();

   } else {
      c = getChar_helper();

      if (c != 0) {
         --readBuffer_Iter;
      }
   }

   return c;
}

bool QXmlStreamReaderPrivate::scanUntil(const char *str, short tokenToInject)
{
   int pos = textBuffer.size();
   int oldLineNumber = lineNumber;

   while (uint c = getChar()) {
      // First, we do the validation & normalization.

      switch (c) {
         case '\r':
            if ((c = filterCarriageReturn()) == 0) {
               break;
            }

            [[fallthrough]];

         case '\n':
            ++lineNumber;
            lastLineStart = characterOffset + (readBuffer_Iter - readBuffer.cbegin());
            [[fallthrough]];

         case '\t':
            textBuffer += char32_t(c);
            continue;

         default:
            if (c < 0x20 || (c > 0xFFFD && c < 0x10000) || c > 0x10FFFF ) {
               raiseWellFormedError(QXmlStream::tr("Invalid XML character."));
               lineNumber = oldLineNumber;
               return false;
            }

            textBuffer += char32_t(c);
      }

      // Second, attempt to lookup str
      if (c == uint(*str)) {

         if (!*(str + 1)) {
            if (tokenToInject >= 0) {
               injectToken(tokenToInject);
            }

            return true;

         } else {
            if (scanString(str + 1, tokenToInject, false)) {
               return true;
            }
         }
      }
   }

   putString(textBuffer, pos);
   textBuffer.resize(pos);
   lineNumber = oldLineNumber;

   return false;
}

bool QXmlStreamReaderPrivate::scanString(const char *str, short tokenToInject, bool requireSpace)
{
   int n = 0;

   while (str[n]) {
      ushort c = getChar();

      if (c != ushort(str[n])) {
         if (c) {
            putChar(c);
         }

         while (n--) {
            putChar(ushort(str[n]));
         }

         return false;
      }

      ++n;
   }

   for (int i = 0; i < n; ++i) {
      textBuffer += QChar(ushort(str[i]));
   }

   if (requireSpace) {
      int s = fastScanSpace();

      if (!s || atEnd) {
         int pos = textBuffer.size() - n - s;
         putString(textBuffer, pos);
         textBuffer.resize(pos);
         return false;
      }
   }

   if (tokenToInject >= 0) {
      injectToken(tokenToInject);
   }

   return true;
}

bool QXmlStreamReaderPrivate::scanAfterLangleBang()
{
   switch (peekChar()) {
      case '[':
         return scanString(spell[CDATA_START], CDATA_START, false);

      case 'D':
         return scanString(spell[DOCTYPE], DOCTYPE);

      case 'A':
         return scanString(spell[ATTLIST], ATTLIST);

      case 'N':
         return scanString(spell[NOTATION], NOTATION);

      case 'E':
         if (scanString(spell[ELEMENT], ELEMENT)) {
            return true;
         }

         return scanString(spell[ENTITY], ENTITY);

      default:
         ;
   }

   return false;
}

bool QXmlStreamReaderPrivate::scanPublicOrSystem()
{
   switch (peekChar()) {
      case 'S':
         return scanString(spell[SYSTEM], SYSTEM);

      case 'P':
         return scanString(spell[PUBLIC], PUBLIC);

      default:
         ;
   }

   return false;
}

bool QXmlStreamReaderPrivate::scanNData()
{
   if (fastScanSpace()) {
      if (scanString(spell[NDATA], NDATA)) {
         return true;
      }

      putChar(' ');
   }

   return false;
}

bool QXmlStreamReaderPrivate::scanAfterDefaultDecl()
{
   switch (peekChar()) {
      case 'R':
         return scanString(spell[REQUIRED], REQUIRED, false);

      case 'I':
         return scanString(spell[IMPLIED], IMPLIED, false);

      case 'F':
         return scanString(spell[FIXED], FIXED, false);

      default:
         ;
   }

   return false;
}

bool QXmlStreamReaderPrivate::scanAttType()
{
   switch (peekChar()) {
      case 'C':
         return scanString(spell[CDATA], CDATA);

      case 'I':
         if (scanString(spell[ID], ID)) {
            return true;
         }

         if (scanString(spell[IDREF], IDREF)) {
            return true;
         }

         return scanString(spell[IDREFS], IDREFS);

      case 'E':
         if (scanString(spell[ENTITY], ENTITY)) {
            return true;
         }

         return scanString(spell[ENTITIES], ENTITIES);

      case 'N':
         if (scanString(spell[NOTATION], NOTATION)) {
            return true;
         }

         if (scanString(spell[NMTOKEN], NMTOKEN)) {
            return true;
         }

         return scanString(spell[NMTOKENS], NMTOKENS);

      default:
         ;
   }

   return false;
}

inline int QXmlStreamReaderPrivate::fastScanLiteralContent()
{
   int n = 0;
   uint c;

   while ((c = getChar())) {
      switch (ushort(c)) {
         case 0xfffe:
         case 0xffff:
         case 0:
            /* The putChar() call is necessary so the parser re-gets
             * the character from the input source, when raising an error. */
            putChar(c);
            return n;

         case '\r':
            if (filterCarriageReturn() == 0) {
               return n;
            }

            [[fallthrough]];

         case '\n':
            ++lineNumber;
            lastLineStart = characterOffset + (readBuffer_Iter - readBuffer.cbegin());

            [[fallthrough]];

         case ' ':
         case '\t':
            if (normalizeLiterals) {
               textBuffer += ' ';
            } else {
               textBuffer += char32_t(c);
            }

            ++n;
            break;

         case '&':
         case '<':
         case '\"':
         case '\'':
            if (!(c & 0xff0000)) {
               putChar(c);
               return n;
            }

            [[fallthrough]];

         default:
            textBuffer += char32_t(c);
            ++n;
      }
   }

   return n;
}

inline int QXmlStreamReaderPrivate::fastScanSpace()
{
   int n = 0;
   ushort c;

   while ((c = getChar())) {
      switch (c) {
         case '\r':
            if ((c = filterCarriageReturn()) == 0) {
               return n;
            }

            [[fallthrough]];

         case '\n':
            ++lineNumber;
            lastLineStart = characterOffset + (readBuffer_Iter - readBuffer.cbegin());
            [[fallthrough]];

         case ' ':
         case '\t':
            textBuffer += char32_t(c);
            ++n;
            break;

         default:
            putChar(c);
            return n;
      }
   }

   return n;
}

inline int QXmlStreamReaderPrivate::fastScanContentCharList()
{
   int n = 0;
   uint c;

   while ((c = getChar())) {
      switch (ushort(c)) {
         case 0xfffe:
         case 0xffff:
         case 0:
            putChar(c);
            return n;

         case ']': {
            isWhitespace = false;
            int pos = textBuffer.size();
            textBuffer += QChar(ushort(c));
            ++n;

            while ((c = getChar()) == ']') {
               textBuffer += QChar(ushort(c));
               ++n;
            }

            if (c == 0) {
               putString(textBuffer, pos);
               textBuffer.resize(pos);

            } else if (c == '>' && textBuffer.at(textBuffer.size() - 2) == ']') {
               raiseWellFormedError(QXmlStream::tr("Sequence ']]>' not allowed in content."));

            } else {
               putChar(c);
               break;
            }

            return n;
         }
         break;

         case '\r':
            if ((c = filterCarriageReturn()) == 0) {
               return n;
            }

            [[fallthrough]];

         case '\n':
            ++lineNumber;
            lastLineStart = characterOffset + (readBuffer_Iter - readBuffer.cbegin());
            [[fallthrough]];

         case ' ':
         case '\t':
            textBuffer += QChar(ushort(c));
            ++n;
            break;

         case '&':
         case '<':
            if (!(c & 0xff0000)) {
               putChar(c);
               return n;
            }

            [[fallthrough]];

         default:
            if (c < 0x20) {
               putChar(c);
               return n;
            }

            isWhitespace = false;
            textBuffer += QChar(ushort(c));
            ++n;
      }
   }

   return n;
}

inline int QXmlStreamReaderPrivate::fastScanName(int *prefix)
{
   int n = 0;
   ushort c;

   while ((c = getChar())) {
      switch (c) {
         case '\n':
         case ' ':
         case '\t':
         case '\r':
         case '&':
         case '#':
         case '\'':
         case '\"':
         case '<':
         case '>':
         case '[':
         case ']':
         case '=':
         case '%':
         case '/':
         case ';':
         case '?':
         case '!':
         case '^':
         case '|':
         case ',':
         case '(':
         case ')':
         case '+':
         case '*':
            putChar(c);

            if (prefix && *prefix == n + 1) {
               *prefix = 0;
               putChar(':');
               --n;
            }

            return n;

         case ':':
            if (prefix) {
               if (*prefix == 0) {
                  *prefix = n + 2;
               } else { // only one colon allowed according to the namespace spec.
                  putChar(c);
                  return n;
               }
            } else {
               putChar(c);
               return n;
            }

            [[fallthrough]];

         default:
            textBuffer += QChar(c);
            ++n;
      }
   }

   if (prefix) {
      *prefix = 0;
   }

   int pos = textBuffer.size() - n;
   putString(textBuffer, pos);
   textBuffer.resize(pos);

   return 0;
}

enum NameChar {
   NameBeginning,
   NameNotBeginning,
   NotName
};

static const char Begi = static_cast<char>(NameBeginning);
static const char NtBg = static_cast<char>(NameNotBeginning);
static const char NotN = static_cast<char>(NotName);

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

   if (!(uc & ~0x7f)) {
      // uc < 128
      return static_cast<NameChar>(nameCharTable[uc]);
   }

   QChar::Category cat = ch.category();

   // ### some these categories might be slightly wrong
   if ((cat >= QChar::Letter_Uppercase && cat <= QChar::Letter_Other) || cat == QChar::Number_Letter) {
      return NameBeginning;
   }

   if ((cat >= QChar::Number_DecimalDigit && cat <= QChar::Number_Other)
         || (cat >= QChar::Mark_NonSpacing && cat <= QChar::Mark_Enclosing)) {
      return NameNotBeginning;
   }

   return NotName;
}

inline int QXmlStreamReaderPrivate::fastScanNMTOKEN()
{
   int n = 0;
   uint c;

   while ((c = getChar())) {
      if (fastDetermineNameChar(char32_t(c)) == NotName) {
         putChar(c);
         return n;

      } else {
         ++n;
         textBuffer += char32_t(c);
      }
   }

   int pos = textBuffer.size() - n;
   putString(textBuffer, pos);
   textBuffer.resize(pos);

   return n;
}

void QXmlStreamReaderPrivate::putString(const QString &s, int from)
{
   putStack.reserve(s.size());

   for (int i = s.size() - 1; i >= from; --i) {
      putStack.push(s.at(i).unicode());
   }
}

void QXmlStreamReaderPrivate::putStringLiteral(const QString &s)
{
   putStack.reserve(s.size());

   for (int i = s.size() - 1; i >= 0; --i) {
      putStack.push(((LETTER << 16) | s.at(i).unicode()));
   }
}

void QXmlStreamReaderPrivate::putReplacement(const QString &s)
{
   putStack.reserve(s.size());

   for (int i = s.size() - 1; i >= 0; --i) {
      ushort c = s.at(i).unicode();

      if (c == '\n' || c == '\r') {
         putStack.push(((LETTER << 16) | c));
      } else {
         putStack.push(c);
      }
   }
}
void QXmlStreamReaderPrivate::putReplacementInAttributeValue(const QString &s)
{
   putStack.reserve(s.size());

   for (int i = s.size() - 1; i >= 0; --i) {
      ushort c = s.at(i).unicode();

      if (c == '&' || c == ';') {
         putStack.push(c);

      } else if (c == '\n' || c == '\r') {
         putStack.push(' ');

      } else {
         putStack.push(((LETTER << 16) | c));
      }
   }
}

ushort QXmlStreamReaderPrivate::getChar_helper()
{
   const int BUFFER_SIZE = 8192;

   characterOffset = characterOffset + (readBuffer_Iter - readBuffer.cbegin());
   readBuffer.resize(0);

   if (decoder) {
      nbytesread = 0;
   }

   if (device) {
      rawReadBuffer.resize(BUFFER_SIZE);
      int nbytesreadOrMinus1 = device->read(rawReadBuffer.data() + nbytesread, BUFFER_SIZE - nbytesread);
      nbytesread += qMax(nbytesreadOrMinus1, 0);

   } else {
      if (nbytesread) {
         rawReadBuffer += dataBuffer;
      } else {
         rawReadBuffer = dataBuffer;
      }

      nbytesread = rawReadBuffer.size();
      dataBuffer.clear();
   }

   if (! nbytesread) {
      atEnd = true;
      return 0;
   }

   if (! decoder) {

      if (nbytesread < 4) {
         // the 4 is to cover 0xef 0xbb 0xbf plus one extra for the utf8 codec
         atEnd = true;
         return 0;
      }

      int mib = 106; // UTF-8

      // look for byte order mark
      uchar ch1 = rawReadBuffer.at(0);
      uchar ch2 = rawReadBuffer.at(1);
      uchar ch3 = rawReadBuffer.at(2);
      uchar ch4 = rawReadBuffer.at(3);

      if ((ch1 == 0 && ch2 == 0 && ch3 == 0xfe && ch4 == 0xff) ||
            (ch1 == 0xff && ch2 == 0xfe && ch3 == 0 && ch4 == 0)) {
         mib = 1017;   // UTF-32 with byte order mark

      } else if (ch1 == 0x3c && ch2 == 0x00 && ch3 == 0x00 && ch4 == 0x00) {
         mib = 1019;   // UTF-32LE

      } else if (ch1 == 0x00 && ch2 == 0x00 && ch3 == 0x00 && ch4 == 0x3c) {
         mib = 1018;   // UTF-32BE

      } else if ((ch1 == 0xfe && ch2 == 0xff) || (ch1 == 0xff && ch2 == 0xfe)) {
         mib = 1015;   // UTF-16 with byte order mark

      } else if (ch1 == 0x3c && ch2 == 0x00) {
         mib = 1014;   // UTF-16LE

      } else if (ch1 == 0x00 && ch2 == 0x3c) {
         mib = 1013;   // UTF-16BE
      }

      codec = QTextCodec::codecForMib(mib);

      Q_ASSERT(codec);
      decoder = codec->makeDecoder();
   }

   decoder->toUnicode(&readBuffer, rawReadBuffer.constData(), nbytesread);
   readBuffer_Iter = readBuffer.cbegin();

   if (lockEncoding && decoder->hasFailure()) {
      raiseWellFormedError(QXmlStream::tr("Encountered incorrectly encoded content."));

      readBuffer.clear();
      readBuffer_Iter = readBuffer.cbegin();

      return 0;
   }

   if (readBuffer_Iter != readBuffer.cend()) {
      ushort c = readBuffer_Iter->unicode();
      ++readBuffer_Iter;

      return c;
   }

   atEnd = true;
   return 0;
}

QStringView QXmlStreamReaderPrivate::namespaceForPrefix(QStringView prefix)
{
   for (int j = namespaceDeclarations.size() - 1; j >= 0; --j) {
      const NamespaceDeclaration &namespaceDeclaration = namespaceDeclarations.at(j);

      if (namespaceDeclaration.prefix == prefix) {
         return namespaceDeclaration.namespaceUri;
      }
   }

   if (namespaceProcessing && !prefix.isEmpty()) {
      raiseWellFormedError(QXmlStream::tr("Namespace prefix '%1' not declared").formatArg(prefix.toString()));
   }

   return QStringView();
}

void QXmlStreamReaderPrivate::resolveTag()
{
   int n = attributeStack.size();

   if (namespaceProcessing) {
      for (int a = 0; a < dtdAttributes.size(); ++a) {
         DtdAttribute &dtdAttribute = dtdAttributes[a];

         if (!dtdAttribute.isNamespaceAttribute || dtdAttribute.defaultValue.isEmpty()
               || dtdAttribute.tagName != qualifiedName || dtdAttribute.attributeQualifiedName.isEmpty()) {
            continue;
         }

         int i = 0;

         while (i < n && symName(attributeStack[i].key) != dtdAttribute.attributeQualifiedName) {
            ++i;
         }

         if (i != n) {
            continue;
         }

         if (dtdAttribute.attributePrefix.isEmpty() && dtdAttribute.attributeName == "xmlns") {
            namespaceDeclarations.push(NamespaceDeclaration());
            NamespaceDeclaration &namespaceDeclaration = namespaceDeclarations.top();

            namespaceDeclaration.prefix.clear();

            QStringView ns(dtdAttribute.defaultValue);

            if (ns == "http://www.w3.org/2000/xmlns/" || ns == "http://www.w3.org/XML/1998/namespace") {
               raiseWellFormedError(QXmlStream::tr("Illegal namespace declaration."));

            } else {
               namespaceDeclaration.namespaceUri = ns;
            }

         } else if (dtdAttribute.attributePrefix == "xmlns") {
            namespaceDeclarations.push(NamespaceDeclaration());
            NamespaceDeclaration &namespaceDeclaration = namespaceDeclarations.top();

            QStringView namespacePrefix = dtdAttribute.attributeName;
            QStringView namespaceUri    = dtdAttribute.defaultValue;

            if (( (namespacePrefix == "xml") ^ (namespaceUri == "http://www.w3.org/XML/1998/namespace"))
                  || namespaceUri    == "http://www.w3.org/2000/xmlns/" || namespaceUri.isEmpty()
                  || namespacePrefix == "xmlns") {

               raiseWellFormedError(QXmlStream::tr("Illegal namespace declaration."));
            }

            namespaceDeclaration.prefix       = namespacePrefix;
            namespaceDeclaration.namespaceUri = namespaceUri;
         }
      }
   }

   tagStack.top().namespaceDeclaration.namespaceUri = namespaceUri = namespaceForPrefix(prefix);

   attributes.resize(n);

   for (int i = 0; i < n; ++i) {
      QXmlStreamAttribute &attribute = attributes[i];
      Attribute &attrib = attributeStack[i];

      QStringView prefix(symPrefix(attrib.key));
      QStringView name(symString(attrib.key));
      QStringView qualifiedName(symName(attrib.key));
      QStringView value(symString(attrib.value));

      attribute.m_name  = name;
      attribute.m_qualifiedName = qualifiedName;
      attribute.m_value = value;

      if (! prefix.isEmpty()) {
         attribute.m_namespaceUri = namespaceForPrefix(prefix);
      }

      for (int j = 0; j < i; ++j) {
         if (attributes[j].name() == attribute.name()
               && attributes[j].namespaceUri() == attribute.namespaceUri()
               && (namespaceProcessing || attributes[j].qualifiedName() == attribute.qualifiedName())) {
            raiseWellFormedError(QXmlStream::tr("Attribute redefined."));
         }
      }
   }

   for (int a = 0; a < dtdAttributes.size(); ++a) {
      DtdAttribute &dtdAttribute = dtdAttributes[a];

      if (dtdAttribute.isNamespaceAttribute
            || dtdAttribute.defaultValue.isEmpty()
            || dtdAttribute.tagName != qualifiedName
            || dtdAttribute.attributeQualifiedName.isEmpty()) {
         continue;
      }

      int i = 0;

      while (i < n && symName(attributeStack[i].key) != dtdAttribute.attributeQualifiedName) {
         ++i;
      }

      if (i != n) {
         continue;
      }

      QXmlStreamAttribute attribute;
      attribute.m_name =  dtdAttribute.attributeName;
      attribute.m_qualifiedName = dtdAttribute.attributeQualifiedName;
      attribute.m_value = dtdAttribute.defaultValue;

      if (! dtdAttribute.attributePrefix.isEmpty()) {
         attribute.m_namespaceUri = namespaceForPrefix(dtdAttribute.attributePrefix);
      }

      attribute.m_isDefault = true;
      attributes.append(attribute);
   }

   attributeStack.clear();
}

void QXmlStreamReaderPrivate::resolvePublicNamespaces()
{
   const Tag &tag = tagStack.top();
   int n = namespaceDeclarations.size() - tag.namespaceDeclarationsSize;
   publicNamespaceDeclarations.resize(n);

   for (int i = 0; i < n; ++i) {
      const NamespaceDeclaration &namespaceDeclaration = namespaceDeclarations.at(tag.namespaceDeclarationsSize + i);
      QXmlStreamNamespaceDeclaration &publicNamespaceDeclaration = publicNamespaceDeclarations[i];

      publicNamespaceDeclaration.m_prefix       = namespaceDeclaration.prefix;
      publicNamespaceDeclaration.m_namespaceUri = namespaceDeclaration.namespaceUri;
   }
}

void QXmlStreamReaderPrivate::resolveDtd()
{
   publicNotationDeclarations.resize(notationDeclarations.size());

   for (int i = 0; i < notationDeclarations.size(); ++i) {
      const QXmlStreamReaderPrivate::NotationDeclaration &notationDeclaration = notationDeclarations.at(i);
      QXmlStreamNotationDeclaration &publicNotationDeclaration = publicNotationDeclarations[i];

      publicNotationDeclaration.m_name     = notationDeclaration.name;
      publicNotationDeclaration.m_systemId = notationDeclaration.systemId;
      publicNotationDeclaration.m_publicId = notationDeclaration.publicId;

   }

   notationDeclarations.clear();
   publicEntityDeclarations.resize(entityDeclarations.size());

   for (int i = 0; i < entityDeclarations.size(); ++i) {
      const QXmlStreamReaderPrivate::EntityDeclaration &entityDeclaration = entityDeclarations.at(i);
      QXmlStreamEntityDeclaration &publicEntityDeclaration = publicEntityDeclarations[i];

      publicEntityDeclaration.m_name         = entityDeclaration.name;
      publicEntityDeclaration.m_notationName = entityDeclaration.notationName;
      publicEntityDeclaration.m_systemId     = entityDeclaration.systemId;
      publicEntityDeclaration.m_publicId     = entityDeclaration.publicId;
      publicEntityDeclaration.m_value        = entityDeclaration.value;
   }

   entityDeclarations.clear();
   parameterEntityHash.clear();
}

QChar QXmlStreamReaderPrivate::resolveCharRef(int symbolIndex)
{
   bool ok = true;
   uint s;

   if (sym(symbolIndex).c == 'x') {
      s = symString(symbolIndex, 1).toString().toInteger<uint>(&ok, 16);

   } else {
      s = symString(symbolIndex).toString().toInteger<uint>(&ok, 10);
   }

   if (ok) {
      ok = (s == 0x9 || s == 0xa || s == 0xd || (s >= 0x20 && s <= 0xd7ff)
                  || (s >= 0xe000 && s <= 0xfffd) || (s >= 0x10000 && s <= 0x10ffff));
   }

   return ok ? char32_t(s) : char32_t(0);
}

void QXmlStreamReaderPrivate::checkPublicLiteral(QStringView publicId)
{
   //#x20 | #xD | #xA | [a-zA-Z0-9] | [-'()+,./:=?;!*#@$_%]

   for (auto item : publicId) {
      char32_t uc = item.unicode();

      if (uc < 256) {

         switch (uc) {
            case ' ':
            case '\n':
            case '\r':
            case '-':
            case '(':
            case ')':
            case '+':
            case ',':
            case '.':
            case '/':
            case ':':
            case '=':
            case '?':
            case ';':
            case '!':
            case '*':
            case '#':
            case '@':
            case '$':
            case '_':
            case '%':
            case '\'':
            case '\"':
               continue;

            default:
               if ((uc >= 'a' && uc <= 'z') || (uc >= 'A' && uc <= 'Z') || (uc >= '0' && uc <= '9')) {
                  continue;
               }
         }
      }

      raiseWellFormedError(QXmlStream::tr("Unexpected character '%1' in public id literal.").formatArg(item));
      break;
   }
}

bool QXmlStreamReaderPrivate::checkStartDocument()
{
   hasCheckedStartDocument = true;

   if (scanString(spell[XML], XML)) {
      return true;
   }

   type = QXmlStreamReader::StartDocument;

   if (atEnd) {
      hasCheckedStartDocument = false;
      raiseError(QXmlStreamReader::PrematureEndOfDocumentError);
   }

   return false;
}

void QXmlStreamReaderPrivate::startDocument()
{
   QString err;

   if (documentVersion != "1.0") {
      if (documentVersion.contains(' ')) {
         err = QXmlStream::tr("Invalid XML version string.");

      } else {
         err = QXmlStream::tr("Unsupported XML version.");
      }
   }

   int n = attributeStack.size();

   /* We use this bool to ensure that the pesudo attributes are in the
    * proper order:
    *
    * [23]     XMLDecl     ::=     '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>' */
   bool hasStandalone = false;

   for (int i = 0; err.isEmpty() && i < n; ++i) {
      Attribute &attrib = attributeStack[i];
      QStringView prefix(symPrefix(attrib.key));
      QStringView key(symString(attrib.key));
      QStringView value(symString(attrib.value));

      if (prefix.isEmpty() && key == "encoding") {
         const QString name(value.toString());
         documentEncoding = value;

         if (hasStandalone) {
            err = QXmlStream::tr("The standalone pseudo attribute must appear after the encoding.");
         }

         if (! QXmlUtils::isEncName(name)) {
            err = QXmlStream::tr("%1 is an invalid encoding name.").formatArg(name);

         } else {
            QTextCodec *const newCodec = QTextCodec::codecForName(name.toLatin1());

            if (! newCodec) {
               err = QXmlStream::tr("Encoding %1 is unsupported").formatArg(name);

            } else if (newCodec != codec && ! lockEncoding) {
               codec = newCodec;
               delete decoder;

               decoder = codec->makeDecoder();
               decoder->toUnicode(&readBuffer, rawReadBuffer.data(), nbytesread);
               readBuffer_Iter = readBuffer.cbegin();

            }
         }

      } else if (prefix.isEmpty() && key == "standalone") {
         hasStandalone = true;

         if (value == "yes") {
            standalone = true;

         } else if (value == "no") {
            standalone = false;

         } else {
            err = QXmlStream::tr("Standalone accepts only yes or no.");
         }

      } else {
         err = QXmlStream::tr("Invalid attribute in XML declaration.");
      }
   }

   if (! err.isEmpty()) {
      raiseWellFormedError(err);
   }

   attributeStack.clear();
}

void QXmlStreamReaderPrivate::raiseError(QXmlStreamReader::Error error, const QString &message)
{
   this->error = error;
   errorString = message;

   if (errorString.isEmpty()) {
      if (error == QXmlStreamReader::PrematureEndOfDocumentError) {
         errorString = QXmlStream::tr("Premature end of document.");

      } else if (error == QXmlStreamReader::CustomError) {
         errorString = QXmlStream::tr("Invalid document.");

      }
   }

   type = QXmlStreamReader::Invalid;
}

void QXmlStreamReaderPrivate::raiseWellFormedError(const QString &message)
{
   raiseError(QXmlStreamReader::NotWellFormedError, message);
}

void QXmlStreamReaderPrivate::parseError()
{
   if (token == EOF_SYMBOL) {
      raiseError(QXmlStreamReader::PrematureEndOfDocumentError);
      return;
   }

   const int nmax = 4;
   QString error_message;

   int ers = state_stack[tos];
   int nexpected = 0;
   int expected[nmax];

   if (token != ERROR)
      for (int tk = 0; tk < TERMINAL_COUNT; ++tk) {
         int k = t_action(ers, tk);

         if (k <= 0) {
            continue;
         }

         if (spell[tk]) {
            if (nexpected < nmax) {
               expected[nexpected++] = tk;
            }
         }
      }

   error_message.clear ();

   if (nexpected && nexpected < nmax) {
      bool first = true;

      for (int s = 0; s < nexpected; ++s) {

         if (first) {
            error_message += QXmlStream::tr ("Expected ");

         } else if (s == nexpected - 1) {
            error_message += nexpected > 2 ? ", or " : " or ";

         } else {
            error_message += ", ";
         }

         first = false;
         error_message += "\'";
         error_message += spell[expected[s]];
         error_message += "\'";
      }

      error_message += QXmlStream::tr(", but got \'");
      error_message += spell[token];
      error_message += "\'";

   } else {
      error_message += QXmlStream::tr("Unexpected \'");
      error_message += spell[token];
      error_message += "\'";
   }

   error_message += '.';

   raiseWellFormedError(error_message);
}

void QXmlStreamReaderPrivate::resume(int rule)
{
   resumeReduction = rule;

   if (error == QXmlStreamReader::NoError) {
      raiseError(QXmlStreamReader::PrematureEndOfDocumentError);
   }
}

qint64 QXmlStreamReader::lineNumber() const
{
   Q_D(const QXmlStreamReader);
   return d->lineNumber + 1; // in public we start with 1
}

qint64 QXmlStreamReader::columnNumber() const
{
   Q_D(const QXmlStreamReader);
   return d->characterOffset - d->lastLineStart + (d->readBuffer_Iter - d->readBuffer.cbegin());
}

qint64 QXmlStreamReader::characterOffset() const
{
   Q_D(const QXmlStreamReader);
   return d->characterOffset + (d->readBuffer_Iter - d->readBuffer.cbegin());
}

QStringView QXmlStreamReader::text() const
{
   Q_D(const QXmlStreamReader);
   return d->text;
}

QXmlStreamNotationDeclarations QXmlStreamReader::notationDeclarations() const
{
   Q_D(const QXmlStreamReader);

   if (d->notationDeclarations.size()) {
      const_cast<QXmlStreamReaderPrivate *>(d)->resolveDtd();
   }

   return d->publicNotationDeclarations;
}

QXmlStreamEntityDeclarations QXmlStreamReader::entityDeclarations() const
{
   Q_D(const QXmlStreamReader);

   if (d->entityDeclarations.size()) {
      const_cast<QXmlStreamReaderPrivate *>(d)->resolveDtd();
   }

   return d->publicEntityDeclarations;
}

QStringView QXmlStreamReader::dtdName() const
{
   Q_D(const QXmlStreamReader);

   if (d->type == QXmlStreamReader::DTD) {
      return d->dtdName;
   }

   return QStringView();
}

QStringView QXmlStreamReader::dtdPublicId() const
{
   Q_D(const QXmlStreamReader);

   if (d->type == QXmlStreamReader::DTD) {
      return d->dtdPublicId;
   }

   return QStringView();
}

QStringView QXmlStreamReader::dtdSystemId() const
{
   Q_D(const QXmlStreamReader);

   if (d->type == QXmlStreamReader::DTD) {
      return d->dtdSystemId;
   }

   return QStringView();
}

QXmlStreamNamespaceDeclarations QXmlStreamReader::namespaceDeclarations() const
{
   Q_D(const QXmlStreamReader);

   if (d->publicNamespaceDeclarations.isEmpty() && d->type == StartElement) {
      const_cast<QXmlStreamReaderPrivate *>(d)->resolvePublicNamespaces();
   }

   return d->publicNamespaceDeclarations;
}

void QXmlStreamReader::addExtraNamespaceDeclaration(const QXmlStreamNamespaceDeclaration &extraNamespaceDeclaration)
{
   Q_D(QXmlStreamReader);

   d->namespaceDeclarations.push(QXmlStreamReaderPrivate::NamespaceDeclaration());
   QXmlStreamReaderPrivate::NamespaceDeclaration &namespaceDeclaration = d->namespaceDeclarations.top();

   namespaceDeclaration.prefix       = extraNamespaceDeclaration.prefix();
   namespaceDeclaration.namespaceUri = extraNamespaceDeclaration.namespaceUri();
}

void QXmlStreamReader::addExtraNamespaceDeclarations(const QXmlStreamNamespaceDeclarations &extraNamespaceDeclarations)
{
   for (int i = 0; i < extraNamespaceDeclarations.size(); ++i) {
      addExtraNamespaceDeclaration(extraNamespaceDeclarations.at(i));
   }
}

QString QXmlStreamReader::readElementText(ReadElementTextBehaviour behaviour)
{
   Q_D(QXmlStreamReader);

   if (isStartElement()) {
      QString result;

      while (true) {
         switch (readNext()) {
            case Characters:
            case EntityReference:
               result.append(d->text);
               break;

            case EndElement:
               return result;

            case ProcessingInstruction:
            case Comment:
               break;

            case StartElement:
               if (behaviour == SkipChildElements) {
                  skipCurrentElement();
                  break;

               } else if (behaviour == IncludeChildElements) {
                  result += readElementText(behaviour);
                  break;
               }

               [[fallthrough]];

            default:
               if (d->error || behaviour == ErrorOnUnexpectedElement) {
                  if (! d->error) {
                     d->raiseError(UnexpectedElementError, QXmlStream::tr("Expected character data."));
                  }

                  return result;
               }
         }
      }
   }

   return QString();
}

QString QXmlStreamReader::readElementText()
{
   return readElementText(ErrorOnUnexpectedElement);
}

void QXmlStreamReader::raiseError(const QString &message)
{
   Q_D(QXmlStreamReader);
   d->raiseError(CustomError, message);
}

QString QXmlStreamReader::errorString() const
{
   Q_D(const QXmlStreamReader);

   if (d->type == QXmlStreamReader::Invalid) {
      return d->errorString;
   }

   return QString();
}

QXmlStreamReader::Error QXmlStreamReader::error() const
{
   Q_D(const QXmlStreamReader);

   if (d->type == QXmlStreamReader::Invalid) {
      return d->error;
   }

   return NoError;
}

QStringView QXmlStreamReader::processingInstructionTarget() const
{
   Q_D(const QXmlStreamReader);
   return d->processingInstructionTarget;
}

QStringView QXmlStreamReader::processingInstructionData() const
{
   Q_D(const QXmlStreamReader);
   return d->processingInstructionData;
}

QStringView QXmlStreamReader::name() const
{
   Q_D(const QXmlStreamReader);
   return d->name;
}

QStringView QXmlStreamReader::namespaceUri() const
{
   Q_D(const QXmlStreamReader);
   return d->namespaceUri;
}

QStringView QXmlStreamReader::qualifiedName() const
{
   Q_D(const QXmlStreamReader);
   return d->qualifiedName;
}

QStringView QXmlStreamReader::prefix() const
{
   Q_D(const QXmlStreamReader);
   return d->prefix;
}

QXmlStreamAttributes QXmlStreamReader::attributes() const
{
   Q_D(const QXmlStreamReader);
   return d->attributes;
}

QXmlStreamAttribute::QXmlStreamAttribute()
{
   m_isDefault = false;
}

QXmlStreamAttribute::~QXmlStreamAttribute()
{
}

QXmlStreamAttribute::QXmlStreamAttribute(const QString &namespaceUri, const QString &name, const QString &value)
{
   m_namespaceUri  = namespaceUri;
   m_name          = name;
   m_qualifiedName = m_name;
   m_value         = value;
}

QXmlStreamAttribute::QXmlStreamAttribute(const QString &qualifiedName, const QString &value)
{
   auto iter_colon = qualifiedName.indexOfFast(':') + 1;
   m_name          = QStringView(iter_colon, qualifiedName.cend());

   m_qualifiedName = qualifiedName;
   m_value         = value;
}

QXmlStreamAttribute::QXmlStreamAttribute(const QXmlStreamAttribute &other)
{
   *this = other;
}

QXmlStreamAttribute &QXmlStreamAttribute::operator=(const QXmlStreamAttribute &other)
{
   m_name          = other.m_name;
   m_namespaceUri  = other.m_namespaceUri;
   m_qualifiedName = other.m_qualifiedName;
   m_value         = other.m_value;
   m_isDefault     = other.m_isDefault;

   return *this;
}

QXmlStreamNotationDeclaration::QXmlStreamNotationDeclaration()
{
}

QXmlStreamNotationDeclaration::QXmlStreamNotationDeclaration(const QXmlStreamNotationDeclaration &other)
{
   *this = other;
}

QXmlStreamNotationDeclaration &QXmlStreamNotationDeclaration::operator=(const QXmlStreamNotationDeclaration &other)
{
   m_name = other.m_name;
   m_systemId = other.m_systemId;
   m_publicId = other.m_publicId;
   return *this;
}

QXmlStreamNotationDeclaration::~QXmlStreamNotationDeclaration()
{
}

QXmlStreamNamespaceDeclaration::QXmlStreamNamespaceDeclaration()
{
}

QXmlStreamNamespaceDeclaration::QXmlStreamNamespaceDeclaration(const QString &prefix, const QString &namespaceUri)
{
   m_prefix = prefix;
   m_namespaceUri = namespaceUri;
}

QXmlStreamNamespaceDeclaration::QXmlStreamNamespaceDeclaration(const QXmlStreamNamespaceDeclaration &other)
{
   *this = other;
}

QXmlStreamNamespaceDeclaration &QXmlStreamNamespaceDeclaration::operator=(const QXmlStreamNamespaceDeclaration &other)
{
   m_prefix = other.m_prefix;
   m_namespaceUri = other.m_namespaceUri;
   return *this;
}

QXmlStreamNamespaceDeclaration::~QXmlStreamNamespaceDeclaration()
{
}

QXmlStreamEntityDeclaration::QXmlStreamEntityDeclaration()
{
}

QXmlStreamEntityDeclaration::QXmlStreamEntityDeclaration(const QXmlStreamEntityDeclaration &other)
{
   *this = other;
}

QXmlStreamEntityDeclaration &QXmlStreamEntityDeclaration::operator=(const QXmlStreamEntityDeclaration &other)
{
   m_name = other.m_name;
   m_notationName = other.m_notationName;
   m_systemId = other.m_systemId;
   m_publicId = other.m_publicId;
   m_value = other.m_value;
   return *this;
}

QXmlStreamEntityDeclaration::~QXmlStreamEntityDeclaration()
{
}

QStringView QXmlStreamAttributes::value(const QString &namespaceUri, const QString &name) const
{
   for (int i = 0; i < size(); ++i) {
      const QXmlStreamAttribute &attribute = at(i);

      if (attribute.name() == name && attribute.namespaceUri() == namespaceUri) {
         return attribute.value();
      }
   }

   return QStringView();
}

QStringView QXmlStreamAttributes::value(const QString &qualifiedName) const
{
   for (int i = 0; i < size(); ++i) {
      const QXmlStreamAttribute &attribute = at(i);

      if (attribute.qualifiedName() == qualifiedName) {
         return attribute.value();
      }
   }

   return QStringView();
}

void QXmlStreamAttributes::append(const QString &namespaceUri, const QString &name, const QString &value)
{
   append(QXmlStreamAttribute(namespaceUri, name, value));
}

void QXmlStreamAttributes::append(const QString &qualifiedName, const QString &value)
{
   append(QXmlStreamAttribute(qualifiedName, value));
}

bool QXmlStreamReader::isWhitespace() const
{
   Q_D(const QXmlStreamReader);
   return d->type == QXmlStreamReader::Characters && d->isWhitespace;
}

bool QXmlStreamReader::isCDATA() const
{
   Q_D(const QXmlStreamReader);
   return d->type == QXmlStreamReader::Characters && d->isCDATA;
}

bool QXmlStreamReader::isStandaloneDocument() const
{
   Q_D(const QXmlStreamReader);
   return d->standalone;
}

QStringView QXmlStreamReader::documentVersion() const
{
   Q_D(const QXmlStreamReader);

   if (d->type == QXmlStreamReader::StartDocument) {
      return d->documentVersion;
   }

   return QStringView();
}

QStringView QXmlStreamReader::documentEncoding() const
{
   Q_D(const QXmlStreamReader);

   if (d->type == QXmlStreamReader::StartDocument) {
      return d->documentEncoding;
   }

   return QStringView();
}

class QXmlStreamWriterPrivate : public QXmlStreamPrivateTagStack
{
   QXmlStreamWriter *q_ptr;
   Q_DECLARE_PUBLIC(QXmlStreamWriter)

 public:
   QXmlStreamWriterPrivate(QXmlStreamWriter *q);

   ~QXmlStreamWriterPrivate() {
      if (deleteDevice) {
         delete device;
      }

      delete encoder;
   }

   void write(QStringView str);
   void write(const QString &str);
   void writeEscaped(const QString &, bool escapeWhitespace = false);
   void write(const char *s, int len);

   template <int N>
   void write(const char (&s)[N]) {
      write(s, N - 1);
   }

   bool finishStartElement(bool contents = true);
   void writeStartElement(const QString &namespaceUri, const QString &name);

   QIODevice *device;
   QString *stringDevice;

   uint deleteDevice : 1;
   uint inStartElement : 1;
   uint inEmptyElement : 1;
   uint lastWasStartElement : 1;
   uint wroteSomething : 1;
   uint hasError : 1;
   uint autoFormatting : 1;
   uint isCodecASCIICompatible : 1;
   QByteArray autoFormattingIndent;
   NamespaceDeclaration emptyNamespace;
   int lastNamespaceDeclaration;

   QTextCodec *codec;
   QTextEncoder *encoder;

   void checkIfASCIICompatibleCodec();

   NamespaceDeclaration &findNamespace(const QString &namespaceUri, bool writeDeclaration = false,
         bool noDefault = false);

   void writeNamespaceDeclaration(const NamespaceDeclaration &namespaceDeclaration);

   int namespacePrefixCount;

   void indent(int level);
};

QXmlStreamWriterPrivate::QXmlStreamWriterPrivate(QXmlStreamWriter *q)
   : autoFormattingIndent(4, ' ')
{
   q_ptr        = q;
   device       = nullptr;
   stringDevice = nullptr;
   deleteDevice = false;

   codec = QTextCodec::codecForMib(106);                   // utf8
   encoder = codec->makeEncoder(QTextCodec::IgnoreHeader); // no byte order mark for utf8

   checkIfASCIICompatibleCodec();
   inStartElement = inEmptyElement = false;
   wroteSomething = false;
   hasError = false;
   lastWasStartElement = false;
   lastNamespaceDeclaration = 1;
   autoFormatting = false;
   namespacePrefixCount = 0;
}

void QXmlStreamWriterPrivate::checkIfASCIICompatibleCodec()
{
   Q_ASSERT(encoder);

   // assumes ASCII-compatibility for all 8-bit encodings
   const QByteArray bytes = encoder->fromUnicode(" ");
   isCodecASCIICompatible = (bytes.count() == 1);
}

void QXmlStreamWriterPrivate::write(QStringView str)
{
   if (device) {

      if (hasError) {
         return;
      }

      QByteArray bytes = encoder->fromUnicode(str);

      if (device->write(bytes) != bytes.size()) {
         hasError = true;
      }

   } else if (stringDevice != nullptr) {
      stringDevice->append(str);

   } else {
      qWarning("QXmlStreamWriter::write() No device available");
   }
}

void QXmlStreamWriterPrivate::write(const QString &s)
{
   if (device) {
      if (hasError) {
         return;
      }

      QByteArray bytes = encoder->fromUnicode(s);

      if (device->write(bytes) != bytes.size()) {
         hasError = true;
      }

   } else if (stringDevice) {
      stringDevice->append(s);
   } else {
      qWarning("QXmlStreamWriter::write() No device available");
   }
}

void QXmlStreamWriterPrivate::writeEscaped(const QString &s, bool escapeWhitespace)
{
   QString escaped;

   for ( int i = 0; i < s.size(); ++i ) {
      QChar c = s.at(i);

      if (c.unicode() == '<' ) {
         escaped.append(QLatin1String("&lt;"));

      } else if (c.unicode() == '>' ) {
         escaped.append(QLatin1String("&gt;"));

      } else if (c.unicode() == '&' ) {
         escaped.append(QLatin1String("&amp;"));

      } else if (c.unicode() == '\"' ) {
         escaped.append(QLatin1String("&quot;"));

      } else if (escapeWhitespace && c.isSpace()) {
         if (c.unicode() == '\n') {
            escaped.append(QLatin1String("&#10;"));

         } else if (c.unicode() == '\r') {
            escaped.append(QLatin1String("&#13;"));

         } else if (c.unicode() == '\t') {
            escaped.append(QLatin1String("&#9;"));

         } else {
            escaped += c;
         }

      } else {
         escaped += QChar(c);
      }
   }

   write(escaped);
}

// Converts from ASCII to output encoding
void QXmlStreamWriterPrivate::write(const char *s, int len)
{
   if (device) {
      if (hasError) {
         return;
      }

      if (isCodecASCIICompatible) {
         if (device->write(s, len) != len) {
            hasError = true;
         }

         return;
      }
   }

   write(QString::fromLatin1(s, len));
}

void QXmlStreamWriterPrivate::writeNamespaceDeclaration(const NamespaceDeclaration &namespaceDeclaration)
{
   if (namespaceDeclaration.prefix.isEmpty()) {
      write(" xmlns=\"");
      write(namespaceDeclaration.namespaceUri);
      write("\"");
   } else {
      write(" xmlns:");
      write(namespaceDeclaration.prefix);
      write("=\"");
      write(namespaceDeclaration.namespaceUri);
      write("\"");
   }
}

bool QXmlStreamWriterPrivate::finishStartElement(bool contents)
{
   bool hadSomethingWritten = wroteSomething;
   wroteSomething = contents;

   if (!inStartElement) {
      return hadSomethingWritten;
   }

   if (inEmptyElement) {
      write("/>");
      QXmlStreamWriterPrivate::Tag tag = tagStack_pop();
      lastNamespaceDeclaration = tag.namespaceDeclarationsSize;
      lastWasStartElement = false;

   } else {
      write(">");
   }

   inStartElement = inEmptyElement = false;
   lastNamespaceDeclaration = namespaceDeclarations.size();
   return hadSomethingWritten;
}

QXmlStreamPrivateTagStack::NamespaceDeclaration &QXmlStreamWriterPrivate::findNamespace(const QString &namespaceUri,
      bool writeDeclaration, bool noDefault)
{
   for (int j = namespaceDeclarations.size() - 1; j >= 0; --j) {
      NamespaceDeclaration &namespaceDeclaration = namespaceDeclarations[j];

      if (namespaceDeclaration.namespaceUri == namespaceUri) {
         if (! noDefault || !namespaceDeclaration.prefix.isEmpty()) {
            return namespaceDeclaration;
         }
      }
   }

   if (namespaceUri.isEmpty()) {
      return emptyNamespace;
   }

   namespaceDeclarations.push(NamespaceDeclaration());
   NamespaceDeclaration &namespaceDeclaration = namespaceDeclarations.top();

   if (namespaceUri.isEmpty()) {
      namespaceDeclaration.prefix.clear();

   } else {
      QString s;
      int n = ++namespacePrefixCount;

      while (true) {
         s     = "n" + QString::number(n++);
         int j = namespaceDeclarations.size() - 2;

         while (j >= 0 && namespaceDeclarations.at(j).prefix != s) {
            --j;
         }

         if (j < 0) {
            break;
         }
      }

      namespaceDeclaration.prefix = s;
   }

   namespaceDeclaration.namespaceUri = namespaceUri;

   if (writeDeclaration) {
      writeNamespaceDeclaration(namespaceDeclaration);
   }

   return namespaceDeclaration;
}

void QXmlStreamWriterPrivate::indent(int level)
{
   write("\n");

   for (int i = level; i > 0; --i) {
      write(autoFormattingIndent.constData(), autoFormattingIndent.length());
   }
}

QXmlStreamWriter::QXmlStreamWriter()
   : d_ptr(new QXmlStreamWriterPrivate(this))
{
}

QXmlStreamWriter::QXmlStreamWriter(QIODevice *device)
   : d_ptr(new QXmlStreamWriterPrivate(this))
{
   Q_D(QXmlStreamWriter);
   d->device = device;
}

QXmlStreamWriter::QXmlStreamWriter(QByteArray *array)
   : d_ptr(new QXmlStreamWriterPrivate(this))
{
   Q_D(QXmlStreamWriter);
   d->device = new QBuffer(array);
   d->device->open(QIODevice::WriteOnly);
   d->deleteDevice = true;
}

QXmlStreamWriter::QXmlStreamWriter(QString *string)
   : d_ptr(new QXmlStreamWriterPrivate(this))
{
   Q_D(QXmlStreamWriter);
   d->stringDevice = string;
}

QXmlStreamWriter::~QXmlStreamWriter()
{
}

void QXmlStreamWriter::setDevice(QIODevice *device)
{
   Q_D(QXmlStreamWriter);

   if (device == d->device) {
      return;
   }

   d->stringDevice = nullptr;

   if (d->deleteDevice) {
      delete d->device;
      d->deleteDevice = false;
   }

   d->device = device;
}

QIODevice *QXmlStreamWriter::device() const
{
   Q_D(const QXmlStreamWriter);
   return d->device;
}

void QXmlStreamWriter::setCodec(QTextCodec *codec)
{
   Q_D(QXmlStreamWriter);

   if (codec) {
      d->codec = codec;
      delete d->encoder;
      d->encoder = codec->makeEncoder(QTextCodec::IgnoreHeader); // no byte order mark for utf8
      d->checkIfASCIICompatibleCodec();
   }
}

void QXmlStreamWriter::setCodec(const char *codecName)
{
   setCodec(QTextCodec::codecForName(codecName));
}

QTextCodec *QXmlStreamWriter::codec() const
{
   Q_D(const QXmlStreamWriter);
   return d->codec;
}

void QXmlStreamWriter::setAutoFormatting(bool enable)
{
   Q_D(QXmlStreamWriter);
   d->autoFormatting = enable;
}

bool QXmlStreamWriter::autoFormatting() const
{
   Q_D(const QXmlStreamWriter);
   return d->autoFormatting;
}

void QXmlStreamWriter::setAutoFormattingIndent(int spacesOrTabs)
{
   Q_D(QXmlStreamWriter);
   d->autoFormattingIndent = QByteArray(qAbs(spacesOrTabs), spacesOrTabs >= 0 ? ' ' : '\t');
}

int QXmlStreamWriter::autoFormattingIndent() const
{
   Q_D(const QXmlStreamWriter);
   return d->autoFormattingIndent.count(' ') - d->autoFormattingIndent.count('\t');
}

bool QXmlStreamWriter::hasError() const
{
   Q_D(const QXmlStreamWriter);
   return d->hasError;
}

void QXmlStreamWriter::writeAttribute(const QString &qualifiedName, const QString &value)
{
   Q_D(QXmlStreamWriter);

   Q_ASSERT(d->inStartElement);
   Q_ASSERT(qualifiedName.count(':') <= 1);

   d->write(" ");
   d->write(qualifiedName);
   d->write("=\"");
   d->writeEscaped(value, true);
   d->write("\"");
}

void QXmlStreamWriter::writeAttribute(const QString &namespaceUri, const QString &name, const QString &value)
{
   Q_D(QXmlStreamWriter);
   Q_ASSERT(d->inStartElement);
   Q_ASSERT(!name.contains(':'));

   QXmlStreamWriterPrivate::NamespaceDeclaration &namespaceDeclaration = d->findNamespace(namespaceUri, true, true);
   d->write(" ");

   if (!namespaceDeclaration.prefix.isEmpty()) {
      d->write(namespaceDeclaration.prefix);
      d->write(":");
   }

   d->write(name);
   d->write("=\"");
   d->writeEscaped(value, true);
   d->write("\"");
}

void QXmlStreamWriter::writeAttribute(const QXmlStreamAttribute &attribute)
{
   if (attribute.namespaceUri().isEmpty()) {
      writeAttribute(attribute.qualifiedName().toString(), attribute.value());
   } else {
      writeAttribute(attribute.namespaceUri().toString(), attribute.name(), attribute.value());
   }
}

void QXmlStreamWriter::writeAttributes(const QXmlStreamAttributes &attributes)
{
   Q_D(QXmlStreamWriter);

   Q_ASSERT(d->inStartElement);
   (void) d;

   for (int i = 0; i < attributes.size(); ++i) {
      writeAttribute(attributes.at(i));
   }
}

void QXmlStreamWriter::writeCDATA(const QString &text)
{
   Q_D(QXmlStreamWriter);
   d->finishStartElement();

   QString copy(text);
   copy.replace(QLatin1String("]]>"), QLatin1String("]]]]><![CDATA[>"));

   d->write("<![CDATA[");
   d->write(copy);
   d->write("]]>");
}

void QXmlStreamWriter::writeCharacters(const QString &text)
{
   Q_D(QXmlStreamWriter);
   d->finishStartElement();
   d->writeEscaped(text);
}

void QXmlStreamWriter::writeComment(const QString &text)
{
   Q_D(QXmlStreamWriter);
   Q_ASSERT(!text.contains(QLatin1String("--")) && !text.endsWith(QLatin1Char('-')));

   if (!d->finishStartElement(false) && d->autoFormatting) {
      d->indent(d->tagStack.size());
   }

   d->write("<!--");
   d->write(text);
   d->write("-->");
   d->inStartElement = d->lastWasStartElement = false;
}

void QXmlStreamWriter::writeDTD(const QString &dtd)
{
   Q_D(QXmlStreamWriter);
   d->finishStartElement();

   if (d->autoFormatting) {
      d->write("\n");
   }

   d->write(dtd);

   if (d->autoFormatting) {
      d->write("\n");
   }
}

void QXmlStreamWriter::writeEmptyElement(const QString &qualifiedName)
{
   Q_D(QXmlStreamWriter);
   Q_ASSERT(qualifiedName.count(QLatin1Char(':')) <= 1);
   d->writeStartElement(QString(), qualifiedName);
   d->inEmptyElement = true;
}

void QXmlStreamWriter::writeEmptyElement(const QString &namespaceUri, const QString &name)
{
   Q_D(QXmlStreamWriter);
   Q_ASSERT(!name.contains(QLatin1Char(':')));
   d->writeStartElement(namespaceUri, name);
   d->inEmptyElement = true;
}

void QXmlStreamWriter::writeTextElement(const QString &qualifiedName, const QString &text)
{
   writeStartElement(qualifiedName);
   writeCharacters(text);
   writeEndElement();
}

void QXmlStreamWriter::writeTextElement(const QString &namespaceUri, const QString &name, const QString &text)
{
   writeStartElement(namespaceUri, name);
   writeCharacters(text);
   writeEndElement();
}

void QXmlStreamWriter::writeEndDocument()
{
   Q_D(QXmlStreamWriter);

   while (d->tagStack.size()) {
      writeEndElement();
   }

   d->write("\n");
}

void QXmlStreamWriter::writeEndElement()
{
   Q_D(QXmlStreamWriter);

   if (d->tagStack.isEmpty()) {
      return;
   }

   // shortcut: if nothing was written, close as empty tag
   if (d->inStartElement && !d->inEmptyElement) {
      d->write("/>");
      d->lastWasStartElement = d->inStartElement = false;

      QXmlStreamWriterPrivate::Tag tag = d->tagStack_pop();
      d->lastNamespaceDeclaration = tag.namespaceDeclarationsSize;
      return;
   }

   if (!d->finishStartElement(false) && !d->lastWasStartElement && d->autoFormatting) {
      d->indent(d->tagStack.size() - 1);
   }

   if (d->tagStack.isEmpty()) {
      return;
   }

   d->lastWasStartElement = false;

   QXmlStreamWriterPrivate::Tag tag = d->tagStack_pop();
   d->lastNamespaceDeclaration = tag.namespaceDeclarationsSize;
   d->write("</");

   if (!tag.namespaceDeclaration.prefix.isEmpty()) {
      d->write(tag.namespaceDeclaration.prefix);
      d->write(":");
   }

   d->write(tag.name);
   d->write(">");
}

void QXmlStreamWriter::writeEntityReference(const QString &name)
{
   Q_D(QXmlStreamWriter);
   d->finishStartElement();
   d->write("&");
   d->write(name);
   d->write(";");
}

void QXmlStreamWriter::writeNamespace(const QString &namespaceUri, const QString &prefix)
{
   Q_D(QXmlStreamWriter);
   Q_ASSERT(!namespaceUri.isEmpty());
   Q_ASSERT(prefix != QLatin1String("xmlns"));

   if (prefix.isEmpty()) {
      d->findNamespace(namespaceUri, d->inStartElement);

   } else {
      Q_ASSERT(! ((prefix == "xml") ^ (namespaceUri == "http://www.w3.org/XML/1998/namespace")));
      Q_ASSERT(namespaceUri != "http://www.w3.org/2000/xmlns/");

      d->namespaceDeclarations.push(QXmlStreamWriterPrivate::NamespaceDeclaration());
      QXmlStreamWriterPrivate::NamespaceDeclaration &namespaceDeclaration = d->namespaceDeclarations.top();

      namespaceDeclaration.prefix       = prefix;
      namespaceDeclaration.namespaceUri = namespaceUri;

      if (d->inStartElement) {
         d->writeNamespaceDeclaration(namespaceDeclaration);
      }
   }
}

void QXmlStreamWriter::writeDefaultNamespace(const QString &namespaceUri)
{
   Q_D(QXmlStreamWriter);
   Q_ASSERT(namespaceUri != QLatin1String("http://www.w3.org/XML/1998/namespace"));
   Q_ASSERT(namespaceUri != QLatin1String("http://www.w3.org/2000/xmlns/"));

   d->namespaceDeclarations.push(QXmlStreamWriterPrivate::NamespaceDeclaration());
   QXmlStreamWriterPrivate::NamespaceDeclaration &namespaceDeclaration = d->namespaceDeclarations.top();

   namespaceDeclaration.prefix.clear();
   namespaceDeclaration.namespaceUri = namespaceUri;

   if (d->inStartElement) {
      d->writeNamespaceDeclaration(namespaceDeclaration);
   }
}

void QXmlStreamWriter::writeProcessingInstruction(const QString &target, const QString &data)
{
   Q_D(QXmlStreamWriter);
   Q_ASSERT(!data.contains(QLatin1String("?>")));

   if (!d->finishStartElement(false) && d->autoFormatting) {
      d->indent(d->tagStack.size());
   }

   d->write("<?");
   d->write(target);

   if (!data.isEmpty()) {
      d->write(" ");
      d->write(data);
   }

   d->write("?>");
}

void QXmlStreamWriter::writeStartDocument()
{
   writeStartDocument("1.0");
}

void QXmlStreamWriter::writeStartDocument(const QString &version)
{
   Q_D(QXmlStreamWriter);
   d->finishStartElement(false);
   d->write("<?xml version=\"");
   d->write(version);

   if (d->device) {
      // stringDevice does not get any encoding
      d->write("\" encoding=\"");

      d->write(d->codec->name().constData(), d->codec->name().length());
   }

   d->write("\"?>");
}

void QXmlStreamWriter::writeStartDocument(const QString &version, bool standalone)
{
   Q_D(QXmlStreamWriter);
   d->finishStartElement(false);
   d->write("<?xml version=\"");
   d->write(version);

   if (d->device) { // stringDevice does not get any encoding
      d->write("\" encoding=\"");

      d->write(d->codec->name().constData(), d->codec->name().length());
   }

   if (standalone) {
      d->write("\" standalone=\"yes\"?>");
   } else {
      d->write("\" standalone=\"no\"?>");
   }
}

void QXmlStreamWriter::writeStartElement(const QString &qualifiedName)
{
   Q_D(QXmlStreamWriter);
   Q_ASSERT(qualifiedName.count(QLatin1Char(':')) <= 1);
   d->writeStartElement(QString(), qualifiedName);
}

void QXmlStreamWriter::writeStartElement(const QString &namespaceUri, const QString &name)
{
   Q_D(QXmlStreamWriter);
   Q_ASSERT(! name.contains(QLatin1Char(':')));

   d->writeStartElement(namespaceUri, name);
}

void QXmlStreamWriterPrivate::writeStartElement(const QString &namespaceUri, const QString &name)
{
   if (! finishStartElement(false) && autoFormatting) {
      indent(tagStack.size());
   }

   Tag &tag = tagStack_push();
   tag.name = name;
   tag.namespaceDeclaration = findNamespace(namespaceUri);

   write("<");

   if (!tag.namespaceDeclaration.prefix.isEmpty()) {
      write(tag.namespaceDeclaration.prefix);
      write(":");
   }

   write(tag.name);
   inStartElement = lastWasStartElement = true;

   for (int i = lastNamespaceDeclaration; i < namespaceDeclarations.size(); ++i) {
      writeNamespaceDeclaration(namespaceDeclarations[i]);
   }

   tag.namespaceDeclarationsSize = lastNamespaceDeclaration;
}

void QXmlStreamWriter::writeCurrentToken(const QXmlStreamReader &reader)
{
   switch (reader.tokenType()) {
      case QXmlStreamReader::NoToken:
         break;

      case QXmlStreamReader::StartDocument:
         writeStartDocument();
         break;

      case QXmlStreamReader::EndDocument:
         writeEndDocument();
         break;

      case QXmlStreamReader::StartElement: {
         QXmlStreamNamespaceDeclarations namespaceDeclarations = reader.namespaceDeclarations();

         for (int i = 0; i < namespaceDeclarations.size(); ++i) {
            const QXmlStreamNamespaceDeclaration &namespaceDeclaration = namespaceDeclarations.at(i);
            writeNamespace(namespaceDeclaration.namespaceUri().toString(),
                  namespaceDeclaration.prefix().toString());
         }

         writeStartElement(reader.namespaceUri().toString(), reader.name().toString());
         writeAttributes(reader.attributes());
      }
      break;

      case QXmlStreamReader::EndElement:
         writeEndElement();
         break;

      case QXmlStreamReader::Characters:
         if (reader.isCDATA()) {
            writeCDATA(reader.text().toString());
         } else {
            writeCharacters(reader.text().toString());
         }

         break;

      case QXmlStreamReader::Comment:
         writeComment(reader.text().toString());
         break;

      case QXmlStreamReader::DTD:
         writeDTD(reader.text().toString());
         break;

      case QXmlStreamReader::EntityReference:
         writeEntityReference(reader.name().toString());
         break;

      case QXmlStreamReader::ProcessingInstruction:
         writeProcessingInstruction(reader.processingInstructionTarget().toString(),
               reader.processingInstructionData().toString());
         break;

      default:
         Q_ASSERT(reader.tokenType() != QXmlStreamReader::Invalid);
         qWarning("QXmlStreamWriter::writeCurrentToken() Invalid state");
         break;
   }
}
