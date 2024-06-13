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

#include <translator.h>

#include <qdebug.h>
#include <qhash.h>
#include <qiodevice.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qtextstream.h>

#include <ctype.h>

// Uncomment if you wish to hard wrap long lines in .po files. Note that this
// affects only msg strings, not comments.
//#define HARD_WRAP_LONG_WORDS

static const int MAX_LEN = 79;

static QString poEscapedString(const QString &prefix, const QString &keyword,
                               bool noWrap, const QString &ba)
{
   QStringList lines;
   int off = 0;
   QString res;

   while (off < ba.length()) {
      ushort c = ba[off++].unicode();
      switch (c) {
         case '\n':
            res += QLatin1String("\\n");
            lines.append(res);
            res.clear();
            break;

         case '\r':
            res += QLatin1String("\\r");
            break;

         case '\t':
            res += QLatin1String("\\t");
            break;

         case '\v':
            res += QLatin1String("\\v");
            break;

         case '\a':
            res += QLatin1String("\\a");
            break;

         case '\b':
            res += QLatin1String("\\b");
            break;

         case '\f':
            res += QLatin1String("\\f");
            break;

         case '"':
            res += QLatin1String("\\\"");
            break;

         case '\\':
            res += QLatin1String("\\\\");
            break;

         default:
            if (c < 32) {
               res += QLatin1String("\\x");
               res += QString::number(c, 16);
               if (off < ba.length() && isxdigit(ba[off].unicode())) {
                  res += QLatin1String("\"\"");
               }
            } else {
               res += QChar(c);
            }
            break;
      }
   }
   if (!res.isEmpty()) {
      lines.append(res);
   }
   if (!lines.isEmpty()) {
      if (!noWrap) {
         if (lines.count() != 1 ||
               lines.first().length() > MAX_LEN - keyword.length() - prefix.length() - 3) {

            QStringList olines = lines;
            lines = QStringList(QString());
            const int maxlen = MAX_LEN - prefix.length() - 2;

            for (const QString &line : olines) {
               int off = 0;

               while (off + maxlen < line.length()) {
                  int idx = line.lastIndexOf(QLatin1Char(' '), off + maxlen - 1) + 1;

                  if (idx == off) {
#ifdef HARD_WRAP_LONG_WORDS
                     // This doesn't seem too nice, but who knows ...
                     idx = off + maxlen;
#else
                     idx = line.indexOf(QLatin1Char(' '), off + maxlen) + 1;
                     if (!idx) {
                        break;
                     }
#endif
                  }
                  lines.append(line.mid(off, idx - off));
                  off = idx;
               }
               lines.append(line.mid(off));
            }
         }
      } else if (lines.count() > 1) {
         lines.prepend(QString());
      }
   }
   return prefix + keyword + QLatin1String(" \"") +
          lines.join(QLatin1String("\"\n") + prefix + QLatin1Char('"')) +
          QLatin1String("\"\n");
}

static QString poEscapedLines(const QString &prefix, bool addSpace, const QStringList &lines)
{
   QString out;
   for (const QString &line : lines) {
      out += prefix;
      if (addSpace && !line.isEmpty()) {
         out += QLatin1Char(' ' );
      }
      out += line;
      out += QLatin1Char('\n');
   }
   return out;
}

static QString poEscapedLines(const QString &prefix, bool addSpace, const QString &in0)
{
   QString in = in0;
   if (in.endsWith(QLatin1Char('\n'))) {
      in.chop(1);
   }
   return poEscapedLines(prefix, addSpace, in.split(QLatin1Char('\n')));
}

static QString poWrappedEscapedLines(const QString &prefix, bool addSpace, const QString &line)
{
   const int maxlen = MAX_LEN - prefix.length();
   QStringList lines;
   int off = 0;
   while (off + maxlen < line.length()) {
      int idx = line.lastIndexOf(QLatin1Char(' '), off + maxlen - 1);
      if (idx < off) {
#if 0 //def HARD_WRAP_LONG_WORDS
         // This cannot work without messing up semantics, so do not even try.
#else
         idx = line.indexOf(QLatin1Char(' '), off + maxlen);
         if (idx < 0) {
            break;
         }
#endif
      }
      lines.append(line.mid(off, idx - off));
      off = idx + 1;
   }
   lines.append(line.mid(off));
   return poEscapedLines(prefix, addSpace, lines);
}

struct PoItem {
 public:
   PoItem()
      : isPlural(false), isFuzzy(false) {
   }


 public:
   QByteArray id;
   QByteArray context;
   QByteArray tscomment;
   QByteArray oldTscomment;
   QByteArray lineNumber;
   QByteArray fileName;
   QByteArray references;
   QByteArray translatorComments;
   QByteArray automaticComments;
   QByteArray msgId;
   QByteArray oldMsgId;
   QList<QByteArray> msgStr;
   bool isPlural;
   bool isFuzzy;
   QHash<QString, QString> extra;
};


static bool isTranslationLine(const QByteArray &line)
{
   return line.startsWith("#~ msgstr") || line.startsWith("msgstr");
}

static QByteArray slurpEscapedString(const QList<QByteArray> &lines, int &l,
                                     int offset, const QByteArray &prefix, ConversionData &cd)
{
   QByteArray msg;
   int stoff;

   for (; l < lines.size(); ++l) {
      const QByteArray &line = lines.at(l);

      if (line.isEmpty() || !line.startsWith(prefix)) {
         break;
      }

      while (isspace(line[offset])) { // No length check, as string has no trailing spaces.
         offset++;
      }

      if (line[offset] != '"') {
         break;
      }
      offset++;

      while (true) {
         if (offset == line.length()) {
            goto premature_eol;
         }
         uchar c = line[offset++];
         if (c == '"') {
            if (offset == line.length()) {
               break;
            }
            while (isspace(line[offset])) {
               offset++;
            }
            if (line[offset++] != '"') {
               cd.appendError(QString("PO parsing error: extra characters on line %1.").formatArg(l + 1));
               break;
            }
            continue;
         }

         if (c == '\\') {
            if (offset == line.length()) {
               goto premature_eol;
            }
            c = line[offset++];
            switch (c) {
               case 'r':
                  msg += '\r'; // Maybe just throw it away?
                  break;
               case 'n':
                  msg += '\n';
                  break;
               case 't':
                  msg += '\t';
                  break;
               case 'v':
                  msg += '\v';
                  break;
               case 'a':
                  msg += '\a';
                  break;

               case 'b':
                  msg += '\b';
                  break;

               case 'f':
                  msg += '\f';
                  break;

               case '"':
                  msg += '"';
                  break;

               case '\\':
                  msg += '\\';
                  break;

               case '0':
               case '1':
               case '2':
               case '3':
               case '4':
               case '5':
               case '6':
               case '7':
                  stoff = offset - 1;
                  while ((c = line[offset]) >= '0' && c <= '7')
                     if (++offset == line.length()) {
                        goto premature_eol;
                     }
                  msg += line.mid(stoff, offset - stoff).toUInt(nullptr, 8);
                  break;

               case 'x':
                  stoff = offset;
                  while (isxdigit(line[offset]))
                     if (++offset == line.length()) {
                        goto premature_eol;
                     }
                  msg += line.mid(stoff, offset - stoff).toUInt(nullptr, 16);
                  break;

               default:
                  cd.appendError(QString("PO parsing error: invalid escape '\\%1' (line %2).").formatArg(c).formatArg(l + 1));
                  msg += '\\';
                  msg += c;
                  break;
            }

         } else {
            msg += c;
         }
      }
      offset = prefix.size();
   }
   --l;
   return msg;

premature_eol:
   cd.appendError(QString("PO parsing error: premature end of line %1.").formatArg(l + 1));
   return QByteArray();

}

static void slurpComment(QByteArray &msg, const QList<QByteArray> &lines, int &l)
{
   QByteArray prefix = lines.at(l);
   for (int i = 1; ; i++) {
      if (prefix.at(i) != ' ') {
         prefix.truncate(i);
         break;
      }
   }
   for (; l < lines.size(); ++l) {
      const QByteArray &line = lines.at(l);
      if (line.startsWith(prefix)) {
         msg += line.mid(prefix.size());
      } else if (line != "#") {
         break;
      }
      msg += '\n';
   }
   --l;
}

static void splitContext(QByteArray *comment, QByteArray *context)
{
   char *data = comment->data();
   int len = comment->size();
   int sep = -1, j = 0;

   for (int i = 0; i < len; i++, j++) {
      if (data[i] == '~' && i + 1 < len) {
         i++;
      } else if (data[i] == '|') {
         sep = j;
      }
      data[j] = data[i];
   }
   if (sep >= 0) {
      QByteArray tmp = comment->mid(sep + 1, j - sep - 1);
      comment->truncate(sep);
      *context = *comment;
      *comment = tmp;
   } else {
      comment->truncate(j);
   }
}

static QString makePoHeader(const QString &str)
{
   return QLatin1String("po-header-") + str.toLower().replace(QLatin1Char('-'), QLatin1Char('_'));
}

static QByteArray QByteArrayList_join(const QList<QByteArray> &that, char sep)
{
   int totalLength = 0;
   const int size = that.size();

   for (int i = 0; i < size; ++i) {
      totalLength += that.at(i).size();
   }

   if (size > 0) {
      totalLength += size - 1;
   }

   QByteArray res;
   if (totalLength == 0) {
      return res;
   }
   res.reserve(totalLength);
   for (int i = 0; i < that.size(); ++i) {
      if (i) {
         res += sep;
      }
      res += that.at(i);
   }
   return res;
}

bool loadPO(Translator &translator, QIODevice &dev, ConversionData &cd)
{
   QTextCodec *codec = QTextCodec::codecForName(
                          cd.m_codecForSource.isEmpty() ? QByteArray("UTF-8") : cd.m_codecForSource);
   bool error = false;

   // format of a .po file entry:
   // white-space
   // #  translator-comments
   // #. automatic-comments
   // #: reference...
   // #, flag...
   // #~ msgctxt, msgid*, msgstr - used for obsoleted messages
   // #| msgctxt, msgid* previous untranslated-string - for fuzzy message
   // msgctx string-context
   // msgid untranslated-string
   // -- For singular:
   // msgstr translated-string
   // -- For plural:
   // msgid_plural untranslated-string-plural
   // msgstr[0] translated-string
   // ...

   // we need line based lookahead below.
   QList<QByteArray> lines;

   while (!dev.atEnd()) {
      lines.append(dev.readLine().trimmed());
   }
   lines.append(QByteArray());

   int l = 0, lastCmtLine = -1;
   bool qtContexts = false;
   PoItem item;

   for (; l != lines.size(); ++l) {
      QByteArray line = lines.at(l);
      if (line.isEmpty()) {
         continue;
      }

      if (isTranslationLine(line)) {
         bool isObsolete = line.startsWith("#~ msgstr");
         const QByteArray prefix = isObsolete ? "#~ " : "";

         while (true) {
            int idx = line.indexOf(' ', prefix.length());
            QByteArray str = slurpEscapedString(lines, l, idx, prefix, cd);
            item.msgStr.append(str);
            if (l + 1 >= lines.size() || !isTranslationLine(lines.at(l + 1))) {
               break;
            }
            ++l;
            line = lines.at(l);
         }

         if (item.msgId.isEmpty()) {
            QHash<QString, QByteArray> extras;
            QList<QByteArray> hdrOrder;
            QByteArray pluralForms;

            for (const QByteArray &hdr : item.msgStr.first().split('\n')) {
               if (hdr.isEmpty()) {
                  continue;
               }

               int idx = hdr.indexOf(':');
               if (idx < 0) {
                  cd.appendError(QString("Unexpected PO header format '%1'").formatArg(QString::fromLatin1(hdr)));
                  error = true;
                  break;
               }

               QByteArray hdrName  = hdr.left(idx).trimmed();
               QByteArray hdrValue = hdr.mid(idx + 1).trimmed();
               hdrOrder << hdrName;

               if (hdrName == "X-Language") {
                  translator.setLanguageCode(QString::fromLatin1(hdrValue));

               } else if (hdrName == "X-Source-Language") {
                  translator.setSourceLanguageCode(QString::fromLatin1(hdrValue));

               } else if (hdrName == "X-Qt-Contexts") {
                  qtContexts = (hdrValue == "true");

               } else if (hdrName == "Plural-Forms") {
                  pluralForms  = hdrValue;

               } else if (hdrName == "MIME-Version") {
                  // just assume it is 1.0

               } else if (hdrName == "Content-Type") {
                  if (cd.m_codecForSource.isEmpty()) {

                     if (! hdrValue.startsWith("text/plain; charset=")) {
                        cd.appendError(QString("Unexpected Content-Type header '%1'").formatArg(QString::fromLatin1(hdrValue)));
                        error = true;

                        // This will avoid a flood of conversion errors.
                        codec = QTextCodec::codecForName("latin1");

                     } else {
                        QByteArray cod = hdrValue.mid(20);
                        QTextCodec *cdc = QTextCodec::codecForName(cod);

                        if (! cdc) {
                           cd.appendError(QString("Unsupported codec '%1'").formatArg(QString::fromLatin1(cod)));
                           error = true;

                           // This will avoid a flood of conversion errors.
                           codec = QTextCodec::codecForName("latin1");

                        } else {
                           codec = cdc;
                        }
                     }
                  }

               } else if (hdrName == "Content-Transfer-Encoding") {
                  if (hdrValue != "8bit") {
                     cd.appendError(QString("Unexpected Content-Transfer-Encoding '%1'").formatArg(QString::fromLatin1(hdrValue)));
                     return false;
                  }

               } else if (hdrName == "X-Virgin-Header") {
                  // legacy

               } else {
                  extras[makePoHeader(QString::fromLatin1(hdrName))] = hdrValue;
               }
            }

            if (! pluralForms.isEmpty()) {

               if (translator.languageCode().isEmpty()) {
                  extras[makePoHeader("Plural-Forms")] = pluralForms;
               } else {
                  // FIXME: make a consistency check
               }
            }

            // Eliminate the field if only headers we added are present in standard order.
            // Keep in sync with savePO

            static const char *const dfltHdrs[] = {
               "MIME-Version", "Content-Type", "Content-Transfer-Encoding",
               "Plural-Forms", "X-Language", "X-Source-Language", "X-Qt-Contexts"
            };

            uint cdh = 0;
            for (int cho = 0; cho < hdrOrder.length(); cho++) {

               for (;; cdh++) {
                  if (cdh == sizeof(dfltHdrs) / sizeof(dfltHdrs[0])) {
                     extras["po-headers"] = QByteArrayList_join(hdrOrder, ',');
                     goto doneho;
                  }

                  if (hdrOrder.at(cho) == dfltHdrs[cdh]) {
                     cdh++;
                     break;
                  }
               }
            }

         doneho:
            if (lastCmtLine != -1) {
               extras["po-header_comment"] = QByteArrayList_join(lines.mid(0, lastCmtLine + 1), '\n');
            }

            for (auto it = extras.constBegin(), end = extras.constEnd(); it != end; ++it) {
               translator.setExtra(it.key(), codec->toUnicode(it.value()));
            }

            item = PoItem();
            continue;
         }

         // build translator message
         TranslatorMessage msg;
         msg.setContext(codec->toUnicode(item.context));

         if (! item.references.isEmpty()) {
            QString xrefs;

            QStringList list = codec->toUnicode(item.references).split(QRegularExpression("\\s"), QStringParser::SkipEmptyParts);

            for (const QString &ref : list) {

               int pos  = ref.indexOf(':');
               int lpos = ref.lastIndexOf(':');

               if (pos != -1 && pos == lpos) {
                  bool ok;
                  int lno = ref.mid(pos + 1).toInteger<int>(&ok);

                  if (ok) {
                     msg.addReference(ref.left(pos), lno);
                     continue;
                  }
               }

               if (! xrefs.isEmpty()) {
                  xrefs += ' ';
               }

               xrefs += ref;
            }

            if (! xrefs.isEmpty()) {
               item.extra["po-references"] = xrefs;
            }
         }

         msg.setId(codec->toUnicode(item.id));
         msg.setSourceText(codec->toUnicode(item.msgId));
         msg.setOldSourceText(codec->toUnicode(item.oldMsgId));
         msg.setComment(codec->toUnicode(item.tscomment));
         msg.setOldComment(codec->toUnicode(item.oldTscomment));
         msg.setExtraComment(codec->toUnicode(item.automaticComments));
         msg.setTranslatorComment(codec->toUnicode(item.translatorComments));
         msg.setPlural(item.isPlural || item.msgStr.size() > 1);

         QStringList translations;

         for (const QByteArray &bstr : item.msgStr) {
            QString str = codec->toUnicode(bstr);
            str.replace(QChar(Translator::TextVariantSeparator), QChar(Translator::BinaryVariantSeparator));
            translations << str;
         }

         msg.setTranslations(translations);

         if (isObsolete) {
            msg.setType(TranslatorMessage::Obsolete);

         } else if (item.isFuzzy || (!msg.sourceText().isEmpty() && !msg.isTranslated())) {
            msg.setType(TranslatorMessage::Type::Unfinished);

         } else {
            msg.setType(TranslatorMessage::Type::Finished);

         }

         msg.setExtras(item.extra);
         translator.append(msg);
         item = PoItem();

      } else if (line.startsWith('#')) {
         switch (line.size() < 2 ? 0 : line.at(1)) {
            case ':':
               item.references += line.mid(3);
               item.references += '\n';
               break;

            case ',': {
               QStringList flags = QString::fromLatin1(line.mid(2)).split(QRegularExpression("[, ]"), QStringParser::SkipEmptyParts);

               if (flags.removeOne("fuzzy")) {
                  item.isFuzzy = true;
               }

               flags.removeOne("qt-format");
               TranslatorMessage::ExtraData::const_iterator it = item.extra.find("po-flags");

               if (it != item.extra.end()) {
                  flags.prepend(*it);
               }

               if (!flags.isEmpty()) {
                  item.extra[QLatin1String("po-flags")] = flags.join(", ");
               }

               break;
            }

            case 0:
               item.translatorComments += '\n';
               break;

            case ' ':
               slurpComment(item.translatorComments, lines, l);
               break;

            case '.':
               if (line.startsWith("#. ts-context ")) { // legacy
                  item.context = line.mid(14);
               } else if (line.startsWith("#. ts-id ")) {
                  item.id = line.mid(9);
               } else {
                  item.automaticComments += line.mid(3);
                  item.automaticComments += '\n';
               }
               break;
            case '|':
               if (line.startsWith("#| msgid ")) {
                  item.oldMsgId = slurpEscapedString(lines, l, 9, "#| ", cd);

               } else if (line.startsWith("#| msgid_plural ")) {
                  QByteArray extra = slurpEscapedString(lines, l, 16, "#| ", cd);

                  if (extra != item.oldMsgId) {
                     item.extra[QLatin1String("po-old_msgid_plural")] = codec->toUnicode(extra);
                  }

               } else if (line.startsWith("#| msgctxt ")) {
                  item.oldTscomment = slurpEscapedString(lines, l, 11, "#| ", cd);

                  if (qtContexts) {
                     splitContext(&item.oldTscomment, &item.context);
                  }

               } else {
                  cd.appendError(QString("PO-format parse error in line %1: '%2'").formatArg(l + 1).formatArg(codec->toUnicode(lines[l])));
                  error = true;
               }
               break;

            case '~':
               if (line.startsWith("#~ msgid ")) {
                  item.msgId = slurpEscapedString(lines, l, 9, "#~ ", cd);

               } else if (line.startsWith("#~ msgid_plural ")) {
                  QByteArray extra = slurpEscapedString(lines, l, 16, "#~ ", cd);

                  if (extra != item.msgId) {
                     item.extra[QLatin1String("po-msgid_plural")] = codec->toUnicode(extra);
                  }

                  item.isPlural = true;

               } else if (line.startsWith("#~ msgctxt ")) {
                  item.tscomment = slurpEscapedString(lines, l, 11, "#~ ", cd);

                  if (qtContexts) {
                     splitContext(&item.tscomment, &item.context);
                  }

               } else {
                  cd.appendError(QString("PO-format parse error in line %1: '%2'").formatArg(l + 1).formatArg(codec->toUnicode(lines[l])));
                  error = true;
               }
               break;

            default:
               cd.appendError(QString("PO-format parse error in line %1: '%2'").formatArg(l + 1).formatArg(codec->toUnicode(lines[l])));
               error = true;
               break;
         }
         lastCmtLine = l;

      } else if (line.startsWith("msgctxt ")) {
         item.tscomment = slurpEscapedString(lines, l, 8, QByteArray(), cd);

         if (qtContexts) {
            splitContext(&item.tscomment, &item.context);
         }

      } else if (line.startsWith("msgid ")) {
         item.msgId = slurpEscapedString(lines, l, 6, QByteArray(), cd);

      } else if (line.startsWith("msgid_plural ")) {
         QByteArray extra = slurpEscapedString(lines, l, 13, QByteArray(), cd);
         if (extra != item.msgId) {
            item.extra[QLatin1String("po-msgid_plural")] = codec->toUnicode(extra);
         }
         item.isPlural = true;

      } else {
         cd.appendError(QString("PO-format error in line %1: '%2'").formatArg(l + 1).formatArg(codec->toUnicode(lines[l])));
         error = true;
      }
   }

   return !error && cd.errors().isEmpty();
}

static void addPoHeader(Translator::ExtraData &headers, QStringList &hdrOrder, const char *name, const QString &value)
{
   QString qName = QString::fromLatin1(name);

   if (! hdrOrder.contains(qName)) {
      hdrOrder << qName;
   }

   headers[makePoHeader(qName)] = value;
}

static QString escapeComment(const QString &in, bool escape)
{
   QString out = in;
   if (escape) {
      out.replace(QLatin1Char('~'), QLatin1String("~~"));
      out.replace(QLatin1Char('|'), QLatin1String("~|"));
   }
   return out;
}

bool savePO(const Translator &translator, QIODevice &dev, ConversionData &cd)
{
   QString str_format = QLatin1String("-format");

   bool ok = true;
   QTextStream out(&dev);

   if (cd.m_outputCodec.isEmpty()) {
      out.setCodec("UTF-8");

   } else {
      out.setCodec(cd.m_outputCodec.constData());

   }

   bool qtContexts = false;
   for (const TranslatorMessage &msg : translator.messages())

      if (!msg.context().isEmpty()) {
         qtContexts = true;
         break;
      }

   QString cmt = translator.extra(QLatin1String("po-header_comment"));
   if (!cmt.isEmpty()) {
      out << cmt << '\n';
   }

   out << "msgid \"\"\n";
   Translator::ExtraData headers = translator.extras();
   QStringList hdrOrder = translator.extra(QLatin1String("po-headers")) .split(QLatin1Char(','), QStringParser::SkipEmptyParts);

   // Keep in sync with loadPO
   addPoHeader(headers, hdrOrder, "MIME-Version", QLatin1String("1.0"));
   addPoHeader(headers, hdrOrder, "Content-Type", QLatin1String("text/plain; charset=" + out.codec()->name()));
   addPoHeader(headers, hdrOrder, "Content-Transfer-Encoding", QLatin1String("8bit"));

   if (!translator.languageCode().isEmpty()) {
      QLocale::Language l;
      QLocale::Country c;

      Translator::languageAndCountry(translator.languageCode(), &l, &c);
      const char *gettextRules;

      if (getNumerusInfo(l, c, nullptr, nullptr, &gettextRules)) {
         addPoHeader(headers, hdrOrder, "Plural-Forms", QString::fromLatin1(gettextRules));
      }

      addPoHeader(headers, hdrOrder, "X-Language", translator.languageCode());
   }

   if (! translator.sourceLanguageCode().isEmpty()) {
      addPoHeader(headers, hdrOrder, "X-Source-Language", translator.sourceLanguageCode());
   }

   if (qtContexts) {
      addPoHeader(headers, hdrOrder, "X-Qt-Contexts", QLatin1String("true"));
   }

   QString hdrStr;
   for (const QString &hdr : hdrOrder) {
      hdrStr += hdr;
      hdrStr += QLatin1String(": ");
      hdrStr += headers.value(makePoHeader(hdr));
      hdrStr += QLatin1Char('\n');
   }
   out << poEscapedString(QString(), QString::fromLatin1("msgstr"), true, hdrStr);

   for (const TranslatorMessage &msg : translator.messages()) {
      out << endl;

      if (!msg.translatorComment().isEmpty()) {
         out << poEscapedLines(QLatin1String("#"), true, msg.translatorComment());
      }

      if (!msg.extraComment().isEmpty()) {
         out << poEscapedLines(QLatin1String("#."), true, msg.extraComment());
      }

      if (!msg.id().isEmpty()) {
         out << QLatin1String("#. ts-id ") << msg.id() << '\n';
      }

      QString xrefs = msg.extra(QLatin1String("po-references"));
      if (!msg.fileName().isEmpty() || !xrefs.isEmpty()) {
         QStringList refs;

         for (const TranslatorMessage::Reference &ref : msg.allReferences()) {
            refs.append(QString("%2:%1").formatArg(ref.lineNumber()).formatArg(ref.fileName()));
         }

         if (! xrefs.isEmpty()) {
            refs << xrefs;
         }
         out << poWrappedEscapedLines(QLatin1String("#:"), true, refs.join(" "));
      }

      bool noWrap = false;
      bool skipFormat = false;
      QStringList flags;
      if (msg.type() == TranslatorMessage::Type::Unfinished && msg.isTranslated()) {
         flags.append(QLatin1String("fuzzy"));
      }
      TranslatorMessage::ExtraData::const_iterator itr =
         msg.extras().find(QLatin1String("po-flags"));
      if (itr != msg.extras().end()) {
         QStringList atoms = itr->split(QLatin1String(", "));
         for (const QString &atom : atoms)
            if (atom.endsWith(str_format)) {
               skipFormat = true;
               break;
            }
         if (atoms.contains(QLatin1String("no-wrap"))) {
            noWrap = true;
         }
         flags.append(*itr);
      }
      if (!skipFormat) {
         QString source = msg.sourceText();
         // This is fuzzy logic, as we don't know whether the string is
         // actually used with QString::arg().
         for (int off = 0; (off = source.indexOf(QLatin1Char('%'), off)) >= 0; ) {
            if (++off >= source.length()) {
               break;
            }
            if (source.at(off) == QLatin1Char('n') || source.at(off).isDigit()) {
               flags.append(QLatin1String("qt-format"));
               break;
            }
         }
      }
      if (!flags.isEmpty()) {
         out << "#, " << flags.join(QLatin1String(", ")) << '\n';
      }

      QString prefix = QLatin1String("#| ");

      if (!msg.oldComment().isEmpty()) {
         out << poEscapedString(prefix, QLatin1String("msgctxt"), noWrap, escapeComment(msg.oldComment(), qtContexts));
      }
      if (!msg.oldSourceText().isEmpty()) {
         out << poEscapedString(prefix, QLatin1String("msgid"), noWrap, msg.oldSourceText());
      }

      QString plural = msg.extra(QLatin1String("po-old_msgid_plural"));

      if (!plural.isEmpty()) {
         out << poEscapedString(prefix, QLatin1String("msgid_plural"), noWrap, plural);
      }
      prefix = (msg.type() == TranslatorMessage::Obsolete) ? QString("#~ ") : QString("");

      if (!msg.context().isEmpty()) {
         out << poEscapedString(prefix, "msgctxt", noWrap,
                                escapeComment(msg.context(), true) + '|' + escapeComment(msg.comment(), true));

      } else if (! msg.comment().isEmpty()) {
         out << poEscapedString(prefix, "msgctxt", noWrap, escapeComment(msg.comment(), qtContexts));
      }

      out << poEscapedString(prefix, "msgid", noWrap, msg.sourceText());

      if (!msg.isPlural()) {
         QString transl = msg.translation();
         transl.replace(QChar(Translator::BinaryVariantSeparator), QChar(Translator::TextVariantSeparator));
         out << poEscapedString(prefix, QLatin1String("msgstr"), noWrap, transl);

      } else {
         QString plural = msg.extra(QLatin1String("po-msgid_plural"));
         if (plural.isEmpty()) {
            plural = msg.sourceText();
         }

         out << poEscapedString(prefix, QLatin1String("msgid_plural"), noWrap, plural);
         const QStringList &translations = msg.translations();

         for (int i = 0; i != translations.size(); ++i) {
            QString str = translations.at(i);
            str.replace(QChar(Translator::BinaryVariantSeparator), QChar(Translator::TextVariantSeparator));
            out << poEscapedString(prefix, QString::fromLatin1("msgstr[%1]").formatArg(i), noWrap, str);
         }
      }
   }
   return ok;
}

static bool savePOT(const Translator &translator, QIODevice &dev, ConversionData &cd)
{
   Translator ttor = translator;
   ttor.dropTranslations();
   return savePO(ttor, dev, cd);
}

int initPO()
{
   Translator::FileFormat format;
   format.extension = QLatin1String("po");
   format.description = QObject::tr("GNU Gettext localization files");
   format.loader = &loadPO;
   format.saver = &savePO;
   format.fileType = Translator::FileFormat::TranslationSource;
   format.priority = 1;
   Translator::registerFileFormat(format);
   format.extension = QLatin1String("pot");
   format.description = QObject::tr("GNU Gettext localization template files");
   format.loader = &loadPO;
   format.saver = &savePOT;
   format.fileType = Translator::FileFormat::TranslationSource;
   format.priority = -1;
   Translator::registerFileFormat(format);
   return 1;
}

Q_CONSTRUCTOR_FUNCTION(initPO)

