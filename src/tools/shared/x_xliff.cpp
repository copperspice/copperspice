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

#include "translator.h"

#include <QDebug>
#include <QMap>
#include <QStack>
#include <QString>
#include <QTextCodec>
#include <QTextStream>

#include <QXmlAttributes>
#include <QXmlDefaultHandler>
#include <QXmlParseException>


// The string value is historical and reflects the main purpose: Keeping
// obsolete entries separate from the magic file message (which both have
// no location information, but typically reside at opposite ends of the file).
#define MAGIC_OBSOLETE_REFERENCE "Obsolete_PO_entries"

QT_BEGIN_NAMESPACE

/**
 * Implementation of XLIFF file format for Linguist
 */

//static const char *restypeDomain = "x-gettext-domain";

static const char *restypeContext = "x-trolltech-linguist-context";
static const char *restypePlurals = "x-gettext-plurals";
static const char *restypeDummy = "x-dummy";
static const char *dataTypeUIFile = "x-trolltech-designer-ui";
static const char *contextMsgctxt = "x-gettext-msgctxt"; // XXX Troll invention, so far.
static const char *contextOldMsgctxt = "x-gettext-previous-msgctxt"; // XXX Troll invention, so far.
static const char *attribPlural = "trolltech:plural";
static const char *XLIFF11namespaceURI = "urn:oasis:names:tc:xliff:document:1.1";
static const char *XLIFF12namespaceURI = "urn:oasis:names:tc:xliff:document:1.2";
static const char *TrollTsNamespaceURI = "urn:trolltech:names:ts:document:1.0";

#define COMBINE4CHARS(c1, c2, c3, c4) \
    (int(c1) << 24 | int(c2) << 16 | int(c3) << 8 | int(c4) )

static QString dataType(const TranslatorMessage &m)
{
   QByteArray fileName  = m.fileName().toLatin1();
   unsigned int extHash = 0;

   int pos = fileName.count() - 1;

   for (int pass = 0; pass < 4 && pos >= 0; ++pass, --pos) {
      if (fileName.at(pos) == '.') {
         break;
      }
      extHash |= ((int)fileName.at(pos) << (8 * pass));
   }

   switch (extHash) {
      case COMBINE4CHARS(0, 'c', 'p', 'p'):
      case COMBINE4CHARS(0, 'c', 'x', 'x'):
      case COMBINE4CHARS(0, 'c', '+', '+'):
      case COMBINE4CHARS(0, 'h', 'p', 'p'):
      case COMBINE4CHARS(0, 'h', 'x', 'x'):
      case COMBINE4CHARS(0, 'h', '+', '+'):
         return QString("cpp");

      case COMBINE4CHARS(0, 0, 0, 'c'):
      case COMBINE4CHARS(0, 0, 0, 'h'):
      case COMBINE4CHARS(0, 0, 'c', 'c'):
      case COMBINE4CHARS(0, 0, 'c', 'h'):
      case COMBINE4CHARS(0, 0, 'h', 'h'):
         return QString("c");

      case COMBINE4CHARS(0, 0, 'u', 'i'):
         return QString::fromLatin1(dataTypeUIFile);   //### form?

      default:
         return QString("plaintext");                  // we give up
   }
}

static void writeIndent(QTextStream &ts, int indent)
{
   ts << QString().fill(QLatin1Char(' '), indent * 2);
}

struct CharMnemonic {
   char ch;
   char escape;
   const char *mnemonic;
};

static const CharMnemonic charCodeMnemonics[] = {
   {0x07, 'a', "bel"},
   {0x08, 'b', "bs"},
   {0x09, 't', "tab"},
   {0x0a, 'n', "lf"},
   {0x0b, 'v', "vt"},
   {0x0c, 'f', "ff"},
   {0x0d, 'r', "cr"}
};

static char charFromEscape(char escape)
{
   for (uint i = 0; i < sizeof(charCodeMnemonics) / sizeof(CharMnemonic); ++i) {
      CharMnemonic cm =  charCodeMnemonics[i];
      if (cm.escape == escape) {
         return cm.ch;
      }
   }

   Q_ASSERT(0);
   return escape;
}

static QString numericEntity(int ch, bool makePhs)
{
   // ### This needs to be reviewed, to reflect the updated XLIFF-PO spec.
   if (! makePhs || ch < 7 || ch > 0x0d) {
      return QString("&#x%1;").formatArg(QString::number(ch, 16));
   }

   CharMnemonic cm = charCodeMnemonics[int(ch) - 7];
   QString name    = QString::fromLatin1(cm.mnemonic);
   char escapechar = cm.escape;

   static int id = 0;

   QString retval = QString("<ph id=\"ph%1\" ctype=\"x-ch-%2\">\\%3</ph>").formatArg(++id).formatArg(name).formatArg(escapechar);

   return retval;
}

static QString protect(const QString &str, bool makePhs = true)
{
   QString result;

   for (QChar c : str) {

      switch (c.unicode()) {
         case '\"':
            result += QLatin1String("&quot;");
            break;

         case '&':
            result += QLatin1String("&amp;");
            break;

         case '>':
            result += QLatin1String("&gt;");
            break;

         case '<':
            result += QLatin1String("&lt;");
            break;

         case '\'':
            result += QLatin1String("&apos;");
            break;

         default:
            if (c < 0x20 && c != '\r' && c != '\n' && c != '\t') {
               result += numericEntity(c.unicode(), makePhs);

            } else {
               // this also covers surrogates
               result += c;
            }
      }
   }

   return result;
}

static void writeExtras(QTextStream &ts, int indent, const TranslatorMessage::ExtraData &extras, const QRegularExpression &drops)
{
   for (auto it = extras.begin(); it != extras.end(); ++it) {
      QRegularExpressionMatch match = drops.match(it.key());

      if (! match.hasMatch()) {
         writeIndent(ts, indent);

         ts << "<trolltech:" << it.key() << '>'
            << protect(it.value())
            << "</trolltech:" << it.key() << ">\n";
      }
   }
}

static void writeLineNumber(QTextStream &ts, const TranslatorMessage &msg, int indent)
{
   if (msg.lineNumber() == -1) {
      return;
   }

   writeIndent(ts, indent);
   ts << "<context-group purpose=\"location\"><context context-type=\"linenumber\">"
      << msg.lineNumber() << "</context></context-group>\n";

   for (const TranslatorMessage::Reference &ref : msg.extraReferences()) {
      writeIndent(ts, indent);
      ts << "<context-group purpose=\"location\">";

      if (ref.fileName() != msg.fileName()) {
         ts << "<context context-type=\"sourcefile\">" << ref.fileName() << "</context>";
      }
      ts << "<context context-type=\"linenumber\">" << ref.lineNumber()
         << "</context></context-group>\n";
   }
}

static void writeComment(QTextStream &ts, const TranslatorMessage &msg, const QRegularExpression &drops, int indent)
{
   if (! msg.comment().isEmpty()) {
      writeIndent(ts, indent);
      ts << "<context-group><context context-type=\"" << contextMsgctxt << "\">"
         << protect(msg.comment(), false)
         << "</context></context-group>\n";
   }

   if (! msg.oldComment().isEmpty()) {
      writeIndent(ts, indent);
      ts << "<context-group><context context-type=\"" << contextOldMsgctxt << "\">"
         << protect(msg.oldComment(), false)
         << "</context></context-group>\n";
   }

   writeExtras(ts, indent, msg.extras(), drops);

   if (! msg.extraComment().isEmpty()) {
      writeIndent(ts, indent);
      ts << "<note annotates=\"source\" from=\"developer\">"
         << protect(msg.extraComment()) << "</note>\n";
   }

   if (! msg.translatorComment().isEmpty()) {
      writeIndent(ts, indent);
      ts << "<note from=\"translator\">"
         << protect(msg.translatorComment()) << "</note>\n";
   }
}

static void writeTransUnits(QTextStream &ts, const TranslatorMessage &msg, const QRegularExpression &drops, int indent)
{
   static int msgid;
   QString msgidstr = !msg.id().isEmpty() ? msg.id() : QString("_msg%1").formatArg(++msgid);

   QStringList translns = msg.translations();
   QHash<QString, QString>::const_iterator it;

   QString pluralStr;
   QStringList sources(msg.sourceText());

   if ((it = msg.extras().find("po-msgid_plural")) != msg.extras().end()) {
      sources.append(*it);
   }

   QStringList oldsources;
   if (!msg.oldSourceText().isEmpty()) {
      oldsources.append(msg.oldSourceText());
   }

   if ((it = msg.extras().find("po-old_msgid_plural")) != msg.extras().end()) {
      if (oldsources.isEmpty()) {

         if (sources.count() == 2) {
            oldsources.append("");
         } else {
            pluralStr = ' ' + QString::fromLatin1(attribPlural) + "=\"yes\"";
         }
      }

      oldsources.append(*it);
   }

   QStringList::const_iterator
   srcit    = sources.begin(), srcend = sources.end(),
   oldsrcit = oldsources.begin(), oldsrcend = oldsources.end(),
   transit  = translns.begin(), transend = translns.end();

   int plural = 0;
   QString source;

   while (srcit != srcend || oldsrcit != oldsrcend || transit != transend) {
      QByteArray attribs;
      QByteArray state;

      if (msg.type() == TranslatorMessage::Obsolete) {
         if (!msg.isPlural()) {
            attribs = " translate=\"no\"";
         }
      } else if (msg.type() == TranslatorMessage::Type::Finished) {
         attribs = " approved=\"yes\"";
      } else if (transit != transend && !transit->isEmpty()) {
         state = " state=\"needs-review-translation\"";
      }
      writeIndent(ts, indent);
      ts << "<trans-unit id=\"" << msgidstr;
      if (msg.isPlural()) {
         ts << "[" << plural++ << "]";
      }
      ts << "\"" << attribs << ">\n";
      ++indent;

      writeIndent(ts, indent);
      if (srcit != srcend) {
         source = *srcit;
         ++srcit;
      } // else just repeat last element
      ts << "<source xml:space=\"preserve\">" << protect(source) << "</source>\n";

      bool puttrans = false;
      QString translation;
      if (transit != transend) {
         translation = *transit;
         translation.replace(QChar(Translator::BinaryVariantSeparator),
                             QChar(Translator::TextVariantSeparator));
         ++transit;
         puttrans = true;
      }
      do {
         if (oldsrcit != oldsrcend && !oldsrcit->isEmpty()) {
            writeIndent(ts, indent);
            ts << "<alt-trans>\n";
            ++indent;
            writeIndent(ts, indent);
            ts << "<source xml:space=\"preserve\"" << pluralStr << '>' << protect(*oldsrcit) << "</source>\n";
            if (!puttrans) {
               writeIndent(ts, indent);
               ts << "<target restype=\"" << restypeDummy << "\"/>\n";
            }
         }

         if (puttrans) {
            writeIndent(ts, indent);
            ts << "<target xml:space=\"preserve\"" << state << ">" << protect(translation) << "</target>\n";
         }

         if (oldsrcit != oldsrcend) {
            if (!oldsrcit->isEmpty()) {
               --indent;
               writeIndent(ts, indent);
               ts << "</alt-trans>\n";
            }
            ++oldsrcit;
         }

         puttrans = false;
      } while (srcit == srcend && oldsrcit != oldsrcend);

      if (!msg.isPlural()) {
         writeLineNumber(ts, msg, indent);
         writeComment(ts, msg, drops, indent);
      }

      --indent;
      writeIndent(ts, indent);
      ts << "</trans-unit>\n";
   }
}

static void writeMessage(QTextStream &ts, const TranslatorMessage &msg, const QRegularExpression &drops, int indent)
{
   if (msg.isPlural()) {
      writeIndent(ts, indent);
      ts << "<group restype=\"" << restypePlurals << "\"";
      if (!msg.id().isEmpty()) {
         ts << " id=\"" << msg.id() << "\"";
      }
      if (msg.type() == TranslatorMessage::Obsolete) {
         ts << " translate=\"no\"";
      }
      ts << ">\n";
      ++indent;
      writeLineNumber(ts, msg, indent);
      writeComment(ts, msg, drops, indent);

      writeTransUnits(ts, msg, drops, indent);
      --indent;
      writeIndent(ts, indent);
      ts << "</group>\n";
   } else {
      writeTransUnits(ts, msg, drops, indent);
   }
}


class XLIFFHandler : public QXmlDefaultHandler
{
 public:
   XLIFFHandler(Translator &translator, ConversionData &cd);

   bool startElement(const QString &namespaceURI, const QString &localName,
                     const QString &qName, const QXmlAttributes &atts );
   bool endElement(const QString &namespaceURI, const QString &localName,
                   const QString &qName );
   bool characters(const QString &ch);
   bool fatalError(const QXmlParseException &exception);

   bool endDocument();

 private:
   enum XliffContext {
      XC_xliff,
      XC_group,
      XC_trans_unit,
      XC_context_group,
      XC_context_group_any,
      XC_context,
      XC_context_filename,
      XC_context_linenumber,
      XC_context_context,
      XC_context_comment,
      XC_context_old_comment,
      XC_ph,
      XC_extra_comment,
      XC_translator_comment,
      XC_restype_context,
      XC_restype_translation,
      XC_restype_plurals,
      XC_alt_trans
   };
   void pushContext(XliffContext ctx);
   bool popContext(XliffContext ctx);
   XliffContext currentContext() const;
   bool hasContext(XliffContext ctx) const;
   bool finalizeMessage(bool isPlural);

 private:
   Translator &m_translator;
   ConversionData &m_cd;
   TranslatorMessage::Type m_type;
   QString m_language;
   QString m_sourceLanguage;
   QString m_context;
   QString m_id;
   QStringList m_sources;
   QStringList m_oldSources;
   QString m_comment;
   QString m_oldComment;
   QString m_extraComment;
   QString m_translatorComment;
   bool m_isPlural;
   bool m_hadAlt;
   QStringList m_translations;
   QString m_fileName;
   int     m_lineNumber;
   QString m_extraFileName;
   TranslatorMessage::References m_refs;
   TranslatorMessage::ExtraData m_extra;

   QString accum;
   QString m_ctype;

   const QString m_URITT;     // convenience and efficiency
   const QString m_URI;       // ...
   const QString m_URI12;     // ...
   QStack<int> m_contextStack;
};

XLIFFHandler::XLIFFHandler(Translator &translator, ConversionData &cd)
   : m_translator(translator), m_cd(cd), m_type(TranslatorMessage::Type::Finished), m_lineNumber(-1),
     m_URITT(QString::fromLatin1(TrollTsNamespaceURI)), m_URI(QString::fromLatin1(XLIFF11namespaceURI)),
     m_URI12(QString::fromLatin1(XLIFF12namespaceURI))
{}


void XLIFFHandler::pushContext(XliffContext ctx)
{
   m_contextStack.push_back(ctx);
}

// Only pops it off if the top of the stack contains ctx
bool XLIFFHandler::popContext(XliffContext ctx)
{
   if (!m_contextStack.isEmpty() && m_contextStack.top() == ctx) {
      m_contextStack.pop();
      return true;
   }
   return false;
}

XLIFFHandler::XliffContext XLIFFHandler::currentContext() const
{
   if (!m_contextStack.isEmpty()) {
      return (XliffContext)m_contextStack.top();
   }
   return XC_xliff;
}

// traverses to the top to check all of the parent contexes.
bool XLIFFHandler::hasContext(XliffContext ctx) const
{
   for (int i = m_contextStack.count() - 1; i >= 0; --i) {
      if (m_contextStack.at(i) == ctx) {
         return true;
      }
   }
   return false;
}

bool XLIFFHandler::startElement(const QString &namespaceURI,
                                const QString &localName, const QString &qName, const QXmlAttributes &atts )
{
   (void) qName;

   if (namespaceURI == m_URITT) {
      goto bail;
   }
   if (namespaceURI != m_URI && namespaceURI != m_URI12) {
      return false;
   }
   if (localName == QLatin1String("xliff")) {
      // make sure that the stack is not empty during parsing
      pushContext(XC_xliff);
   } else if (localName == QLatin1String("file")) {
      m_fileName = atts.value(QLatin1String("original"));
      m_language = atts.value(QLatin1String("target-language"));
      m_language.replace(QLatin1Char('-'), QLatin1Char('_'));
      m_sourceLanguage = atts.value(QLatin1String("source-language"));
      m_sourceLanguage.replace(QLatin1Char('-'), QLatin1Char('_'));

      if (m_sourceLanguage == QLatin1String("en")) {
         m_sourceLanguage.clear();
      }

   } else if (localName == "group") {

      if (atts.value("restype") == QString::fromLatin1(restypeContext)) {
         m_context = atts.value(QLatin1String("resname"));
         pushContext(XC_restype_context);

      } else {
         if (atts.value(QLatin1String("restype")) == QString::fromLatin1(restypePlurals)) {
            pushContext(XC_restype_plurals);

            m_id = atts.value("id");
            if (atts.value("translate") == "no") {
               m_type = TranslatorMessage::Obsolete;
            }

         } else {
            pushContext(XC_group);
         }
      }

   } else if (localName == "trans-unit") {

      if (! hasContext(XC_restype_plurals) || m_sources.isEmpty() )
         if (atts.value("translate") == "no") {
            m_type = TranslatorMessage::Obsolete;
         }

      if (!hasContext(XC_restype_plurals)) {
         m_id = atts.value(QLatin1String("id"));
         if (m_id.startsWith(QLatin1String("_msg"))) {
            m_id.clear();
         }
      }

      if (m_type != TranslatorMessage::Obsolete &&
            atts.value(QLatin1String("approved")) != "yes") {
         m_type = TranslatorMessage::Type::Unfinished;
      }
      pushContext(XC_trans_unit);
      m_hadAlt = false;

   } else if (localName == QLatin1String("alt-trans")) {
      pushContext(XC_alt_trans);

   } else if (localName == QLatin1String("source")) {
      m_isPlural = atts.value(QString::fromLatin1(attribPlural)) == "yes";

   } else if (localName == "target") {
      if (atts.value(QLatin1String("restype")) != QString::fromLatin1(restypeDummy)) {
         pushContext(XC_restype_translation);
      }

   } else if (localName == "context-group") {
      QString purpose = atts.value("purpose");

      if (purpose == "location") {
         pushContext(XC_context_group);
      } else {
         pushContext(XC_context_group_any);
      }

   } else if (currentContext() == XC_context_group && localName == "context") {
      QString ctxtype = atts.value("context-type");

      if (ctxtype == "linenumber") {
         pushContext(XC_context_linenumber);

      } else if (ctxtype == "sourcefile") {
         pushContext(XC_context_filename);
      }

   } else if (currentContext() == XC_context_group_any && localName == "context") {
      QString ctxtype = atts.value("context-type");

      if (ctxtype == QString::fromLatin1(contextMsgctxt)) {
         pushContext(XC_context_comment);

      } else if (ctxtype == QString::fromLatin1(contextOldMsgctxt)) {
         pushContext(XC_context_old_comment);
      }

   } else if (localName == "note") {
      if (atts.value("annotates") == "source" && atts.value("from") == "developer") {
         pushContext(XC_extra_comment);

      } else {
         pushContext(XC_translator_comment);
      }

   } else if (localName == "ph") {
      QString ctype = atts.value("ctype");

      if (ctype.startsWith("x-ch-")) {
         m_ctype = ctype.mid(5);
      }

      pushContext(XC_ph);
   }

bail:
   if (currentContext() != XC_ph) {
      accum.clear();
   }
   return true;
}

bool XLIFFHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
   (void) qName;

   if (namespaceURI == m_URITT) {
      if (hasContext(XC_trans_unit) || hasContext(XC_restype_plurals)) {
         m_extra[localName] = accum;
      } else {
         m_translator.setExtra(localName, accum);
      }
      return true;
   }

   if (namespaceURI != m_URI && namespaceURI != m_URI12) {
      return false;
   }

   if (localName == QLatin1String("xliff")) {
      popContext(XC_xliff);

   } else if (localName == QLatin1String("source")) {
      if (hasContext(XC_alt_trans)) {
         if (m_isPlural && m_oldSources.isEmpty()) {
            m_oldSources.append(QString());
         }
         m_oldSources.append(accum);
         m_hadAlt = true;
      } else {
         m_sources.append(accum);
      }

   } else if (localName == QLatin1String("target")) {
      if (popContext(XC_restype_translation)) {
         accum.replace(QChar(Translator::TextVariantSeparator),
                       QChar(Translator::BinaryVariantSeparator));
         m_translations.append(accum);
      }

   } else if (localName == QLatin1String("context-group")) {
      if (popContext(XC_context_group)) {
         m_refs.append(TranslatorMessage::Reference(m_extraFileName.isEmpty() ? m_fileName : m_extraFileName, m_lineNumber));
         m_extraFileName.clear();
         m_lineNumber = -1;
      } else {
         popContext(XC_context_group_any);
      }

   } else if (localName == QLatin1String("context")) {
      if (popContext(XC_context_linenumber)) {
         bool ok;
         m_lineNumber = accum.trimmed().toInteger<int>(&ok);
         if (!ok) {
            m_lineNumber = -1;
         }

      } else if (popContext(XC_context_filename)) {
         m_extraFileName = accum;

      } else if (popContext(XC_context_comment)) {
         m_comment = accum;

      } else if (popContext(XC_context_old_comment)) {
         m_oldComment = accum;

      }

   } else if (localName == QLatin1String("note")) {
      if (popContext(XC_extra_comment)) {
         m_extraComment = accum;
      } else if (popContext(XC_translator_comment)) {
         m_translatorComment = accum;
      }

   } else if (localName == QLatin1String("ph")) {
      m_ctype.clear();
      popContext(XC_ph);

   } else if (localName == QLatin1String("trans-unit")) {
      popContext(XC_trans_unit);
      if (!m_hadAlt) {
         m_oldSources.append(QString());
      }
      if (!hasContext(XC_restype_plurals)) {
         if (!finalizeMessage(false)) {
            return false;
         }
      }

   } else if (localName == QLatin1String("alt-trans")) {
      popContext(XC_alt_trans);

   } else if (localName == QLatin1String("group")) {
      if (popContext(XC_restype_plurals)) {
         if (!finalizeMessage(true)) {
            return false;
         }

      } else if (popContext(XC_restype_context)) {
         m_context.clear();
      } else {
         popContext(XC_group);
      }
   }

   return true;
}

bool XLIFFHandler::characters(const QString &ch)
{
   if (currentContext() == XC_ph) {
      // handle the content of <ph> elements
      for (int i = 0; i < ch.count(); ++i) {
         QChar chr = ch.at(i);

         if (accum.endsWith('\\')) {
            accum.chop(1);
            accum.append( QChar(charFromEscape(chr.toLatin1())) );

         } else {
            accum.append(chr);
         }
      }

   } else {
      QString t = ch;
      t.replace("\r", "");
      accum.append(t);
   }

   return true;
}

bool XLIFFHandler::endDocument()
{
   m_translator.setLanguageCode(m_language);
   m_translator.setSourceLanguageCode(m_sourceLanguage);
   return true;
}

bool XLIFFHandler::finalizeMessage(bool isPlural)
{
   if (m_sources.isEmpty()) {
      m_cd.appendError("XLIFF syntax error: Message without source string.");
      return false;
   }

   if (m_type == TranslatorMessage::Obsolete && m_refs.size() == 1
         && m_refs.at(0).fileName() == QString::fromLatin1(MAGIC_OBSOLETE_REFERENCE)) {
      m_refs.clear();
   }

   TranslatorMessage msg(m_context, m_sources[0], m_comment, QString(), QString(), -1, m_translations, m_type, isPlural);
   msg.setId(m_id);
   msg.setReferences(m_refs);
   msg.setOldComment(m_oldComment);
   msg.setExtraComment(m_extraComment);
   msg.setTranslatorComment(m_translatorComment);

   if (m_sources.count() > 1 && m_sources[1] != m_sources[0]) {
      m_extra.insert("po-msgid_plural", m_sources[1]);
   }

   if (!m_oldSources.isEmpty()) {
      if (!m_oldSources[0].isEmpty()) {
         msg.setOldSourceText(m_oldSources[0]);
      }
      if (m_oldSources.count() > 1 && m_oldSources[1] != m_oldSources[0]) {
         m_extra.insert("po-old_msgid_plural", m_oldSources[1]);
      }
   }

   msg.setExtras(m_extra);
   m_translator.append(msg);

   m_id.clear();
   m_sources.clear();
   m_oldSources.clear();
   m_translations.clear();
   m_comment.clear();
   m_oldComment.clear();
   m_extraComment.clear();
   m_translatorComment.clear();
   m_extra.clear();
   m_refs.clear();
   m_type = TranslatorMessage::Type::Finished;

   return true;
}

bool XLIFFHandler::fatalError(const QXmlParseException &exception)
{
   QString msg = QString("XML error: Parse error at line %1, column %2 (%3).\n")
                 .formatArg(exception.lineNumber()).formatArg(exception.columnNumber()).formatArg(exception.message());

   m_cd.appendError(msg);

   return false;
}

bool loadXLIFF(Translator &translator, QIODevice &dev, ConversionData &cd)
{
   QXmlInputSource in(&dev);
   QXmlSimpleReader reader;

   XLIFFHandler hand(translator, cd);
   reader.setContentHandler(&hand);
   reader.setErrorHandler(&hand);

   return reader.parse(in);
}

bool saveXLIFF(const Translator &translator, QIODevice &dev, ConversionData &cd)
{
   bool ok = true;
   int indent = 0;

   QTextStream ts(&dev);
   ts.setCodec(QTextCodec::codecForName("UTF-8"));

   QStringList dtgs = cd.dropTags();
   dtgs << "po-(old_)?msgid_plural";

   QRegularExpression drops(dtgs.join("|"), QPatternOption::ExactMatchOption);

   QHash<QString, QHash<QString, QList<TranslatorMessage> >> messageOrder;
   QHash<QString, QList<QString>> contextOrder;
   QList<QString> fileOrder;

   for (const TranslatorMessage &msg : translator.messages()) {
      QString fn = msg.fileName();

      if (fn.isEmpty() && msg.type() == TranslatorMessage::Obsolete) {
         fn = QLatin1String(MAGIC_OBSOLETE_REFERENCE);
      }

      QHash<QString, QList<TranslatorMessage> > &file = messageOrder[fn];
      if (file.isEmpty()) {
         fileOrder.append(fn);
      }

      QList<TranslatorMessage> &context = file[msg.context()];
      if (context.isEmpty()) {
         contextOrder[fn].append(msg.context());
      }
      context.append(msg);
   }

   ts.setFieldAlignment(QTextStream::AlignRight);
   ts << "<?xml version=\"1.0\"";
   ts << " encoding=\"utf-8\"?>\n";
   ts << "<xliff version=\"1.2\" xmlns=\"" << XLIFF12namespaceURI
      << "\" xmlns:trolltech=\"" << TrollTsNamespaceURI << "\">\n";

   ++indent;
   writeExtras(ts, indent, translator.extras(), drops);

   QString sourceLanguageCode = translator.sourceLanguageCode();

   if (sourceLanguageCode.isEmpty() || sourceLanguageCode == QLatin1String("C")) {
      sourceLanguageCode = QLatin1String("en");
   } else {
      sourceLanguageCode.replace(QLatin1Char('_'), QLatin1Char('-'));
   }

   QString languageCode = translator.languageCode();
   languageCode.replace(QLatin1Char('_'), QLatin1Char('-'));

   for (const QString &fn : fileOrder) {
      writeIndent(ts, indent);
      ts << "<file original=\"" << fn << "\""
         << " datatype=\"" << dataType(messageOrder[fn].begin()->first()) << "\""
         << " source-language=\"" << sourceLanguageCode.toLatin1() << "\""
         << " target-language=\"" << languageCode.toLatin1() << "\""
         << "><body>\n";
      ++indent;

      for (const QString &ctx : contextOrder[fn]) {
         if (! ctx.isEmpty()) {
            writeIndent(ts, indent);
            ts << "<group restype=\"" << restypeContext << "\""
               << " resname=\"" << protect(ctx) << "\">\n";
            ++indent;
         }

         for (const TranslatorMessage &msg : messageOrder[fn][ctx]) {
            writeMessage(ts, msg, drops, indent);
         }

         if (!ctx.isEmpty()) {
            --indent;
            writeIndent(ts, indent);
            ts << "</group>\n";
         }
      }

      --indent;
      writeIndent(ts, indent);
      ts << "</body></file>\n";
   }
   --indent;
   writeIndent(ts, indent);
   ts << "</xliff>\n";

   return ok;
}

int initXLIFF()
{
   Translator::FileFormat format;
   format.extension = QLatin1String("xlf");
   format.description = QObject::tr("XLIFF localization files");
   format.fileType = Translator::FileFormat::TranslationSource;
   format.priority = 1;
   format.loader = &loadXLIFF;
   format.saver = &saveXLIFF;
   Translator::registerFileFormat(format);
   return 1;
}

Q_CONSTRUCTOR_FUNCTION(initXLIFF)

QT_END_NAMESPACE
