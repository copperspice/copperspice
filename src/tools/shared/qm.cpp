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

#include "translator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QTextCodec>

QT_BEGIN_NAMESPACE

// magic number for the file
static const int MagicLength = 16;
static const uchar magic[MagicLength] = {
   0x3c, 0xb8, 0x64, 0x18, 0xca, 0xef, 0x9c, 0x95,
   0xcd, 0x21, 0x1c, 0xbf, 0x60, 0xa1, 0xbd, 0xdd
};


namespace {

enum Tag {
   Tag_End          = 1,
   Tag_SourceText16 = 2,
   Tag_Translation  = 3,
   Tag_Context16    = 4,
   Tag_Obsolete1    = 5,
   Tag_SourceText   = 6,
   Tag_Context      = 7,
   Tag_Comment      = 8,
   Tag_Obsolete2    = 9
};

enum Prefix {
   NoPrefix,
   Hash,
   HashContext,
   HashContextSourceText,
   HashContextSourceTextComment
};

} // namespace anon

static uint elfHash(const QByteArray &ba)
{
   const uchar *k = (const uchar *)ba.data();
   uint h = 0;
   uint g;

   if (k) {
      while (*k) {
         h = (h << 4) + *k++;
         if ((g = (h & 0xf0000000)) != 0) {
            h ^= g >> 24;
         }
         h &= ~g;
      }
   }
   if (!h) {
      h = 1;
   }
   return h;
}

class ByteTranslatorMessage
{
 public:
   ByteTranslatorMessage(
      const QByteArray &context,
      const QByteArray &sourceText,
      const QByteArray &comment,
      const QStringList &translations) :
      m_context(context),
      m_sourcetext(sourceText),
      m_comment(comment),
      m_translations(translations) {
   }
   const QByteArray &context() const {
      return m_context;
   }
   const QByteArray &sourceText() const {
      return m_sourcetext;
   }
   const QByteArray &comment() const {
      return m_comment;
   }
   const QStringList &translations() const {
      return m_translations;
   }
   bool operator<(const ByteTranslatorMessage &m) const;

 private:
   QByteArray m_context;
   QByteArray m_sourcetext;
   QByteArray m_comment;
   QStringList m_translations;
};

Q_DECLARE_TYPEINFO(ByteTranslatorMessage, Q_MOVABLE_TYPE);

bool ByteTranslatorMessage::operator<(const ByteTranslatorMessage &m) const
{
   if (m_context != m.m_context) {
      return m_context < m.m_context;
   }
   if (m_sourcetext != m.m_sourcetext) {
      return m_sourcetext < m.m_sourcetext;
   }
   return m_comment < m.m_comment;
}

class Releaser
{
 public:
   struct Offset {
      Offset()
         : h(0), o(0) {
      }
      Offset(uint hash, uint offset)
         : h(hash), o(offset) {
      }

      bool operator<(const Offset &other) const {
         return (h != other.h) ? h < other.h : o < other.o;
      }
      bool operator==(const Offset &other) const {
         return h == other.h && o == other.o;
      }
      uint h;
      uint o;
   };

   enum { Contexts = 0x2f, Hashes = 0x42, Messages = 0x69, NumerusRules = 0x88 };

   Releaser() : m_codec(0) {}

   void setCodecName(const QByteArray &codecName) {
      m_codec = QTextCodec::codecForName(codecName);
   }

   bool save(QIODevice *iod);

   void insert(const TranslatorMessage &msg, const QStringList &tlns, bool forceComment);
   void insertIdBased(const TranslatorMessage &message, const QStringList &tlns);

   void squeeze(TranslatorSaveMode mode);

   void setNumerusRules(const QByteArray &rules);

 private:
   Q_DISABLE_COPY(Releaser)

   // This should reproduce the byte array fetched from the source file, which
   // on turn should be the same as passed to the actual tr(...) calls
   QByteArray originalBytes(const QString &str, bool isUtf8) const;

   void insertInternal(const TranslatorMessage &message, const QStringList &tlns,
                       bool forceComment, bool isUtf8);

   static Prefix commonPrefix(const ByteTranslatorMessage &m1, const ByteTranslatorMessage &m2);

   static uint msgHash(const ByteTranslatorMessage &msg);

   void writeMessage(const ByteTranslatorMessage &msg, QDataStream &stream,
                     TranslatorSaveMode strip, Prefix prefix) const;

   // for squeezed but non-file data, this is what needs to be deleted
   QByteArray m_messageArray;
   QByteArray m_offsetArray;
   QByteArray m_contextArray;
   QMap<ByteTranslatorMessage, void *> m_messages;
   QByteArray m_numerusRules;

   // Used to reproduce the original bytes
   QTextCodec *m_codec;
};

QByteArray Releaser::originalBytes(const QString &str, bool isUtf8) const
{
   if (str.isEmpty()) {
      // Do not use QByteArray() here as the result of the serialization
      // will be different.
      return QByteArray("");
   }
   if (isUtf8) {
      return str.toUtf8();
   }
   return m_codec ? m_codec->fromUnicode(str) : str.toLatin1();
}

uint Releaser::msgHash(const ByteTranslatorMessage &msg)
{
   return elfHash(msg.sourceText() + msg.comment());
}

Prefix Releaser::commonPrefix(const ByteTranslatorMessage &m1, const ByteTranslatorMessage &m2)
{
   if (msgHash(m1) != msgHash(m2)) {
      return NoPrefix;
   }
   if (m1.context() != m2.context()) {
      return Hash;
   }
   if (m1.sourceText() != m2.sourceText()) {
      return HashContext;
   }
   if (m1.comment() != m2.comment()) {
      return HashContextSourceText;
   }
   return HashContextSourceTextComment;
}

void Releaser::writeMessage(const ByteTranslatorMessage &msg, QDataStream &stream,
                            TranslatorSaveMode mode, Prefix prefix) const
{
   for (int i = 0; i < msg.translations().count(); ++i) {
      stream << quint8(Tag_Translation) << msg.translations().at(i);
   }

   if (mode == SaveEverything) {
      prefix = HashContextSourceTextComment;
   }

   // lrelease produces "wrong" QM files for QByteArrays that are .isNull().
   switch (prefix) {
      default:
      case HashContextSourceTextComment:
         stream << quint8(Tag_Comment) << msg.comment();
      // fall through
      case HashContextSourceText:
         stream << quint8(Tag_SourceText) << msg.sourceText();
      // fall through
      case HashContext:
         stream << quint8(Tag_Context) << msg.context();
         break;
   }

   stream << quint8(Tag_End);
}


bool Releaser::save(QIODevice *iod)
{
   QDataStream s(iod);
   s.writeRawData((const char *)magic, MagicLength);

   if (!m_offsetArray.isEmpty()) {
      quint32 oas = quint32(m_offsetArray.size());
      s << quint8(Hashes) << oas;
      s.writeRawData(m_offsetArray.constData(), oas);
   }
   if (!m_messageArray.isEmpty()) {
      quint32 mas = quint32(m_messageArray.size());
      s << quint8(Messages) << mas;
      s.writeRawData(m_messageArray.constData(), mas);
   }
   if (!m_contextArray.isEmpty()) {
      quint32 cas = quint32(m_contextArray.size());
      s << quint8(Contexts) << cas;
      s.writeRawData(m_contextArray.constData(), cas);
   }
   if (!m_numerusRules.isEmpty()) {
      quint32 nrs = m_numerusRules.size();
      s << quint8(NumerusRules) << nrs;
      s.writeRawData(m_numerusRules.constData(), nrs);
   }
   return true;
}

void Releaser::squeeze(TranslatorSaveMode mode)
{
   if (m_messages.isEmpty() && mode == SaveEverything) {
      return;
   }

   QMap<ByteTranslatorMessage, void *> messages = m_messages;

   // re-build contents
   m_messageArray.clear();
   m_offsetArray.clear();
   m_contextArray.clear();
   m_messages.clear();

   QMap<Offset, void *> offsets;

   QDataStream ms(&m_messageArray, QIODevice::WriteOnly);
   QMap<ByteTranslatorMessage, void *>::const_iterator it, next;
   int cpPrev = 0, cpNext = 0;
   for (it = messages.constBegin(); it != messages.constEnd(); ++it) {
      cpPrev = cpNext;
      next = it;
      ++next;
      if (next == messages.constEnd()) {
         cpNext = 0;
      } else {
         cpNext = commonPrefix(it.key(), next.key());
      }
      offsets.insert(Offset(msgHash(it.key()), ms.device()->pos()), (void *)0);
      writeMessage(it.key(), ms, mode, Prefix(qMax(cpPrev, cpNext + 1)));
   }

   QMap<Offset, void *>::Iterator offset;
   offset = offsets.begin();
   QDataStream ds(&m_offsetArray, QIODevice::WriteOnly);
   while (offset != offsets.end()) {
      Offset k = offset.key();
      ++offset;
      ds << quint32(k.h) << quint32(k.o);
   }

   if (mode == SaveStripped) {
      QMap<QByteArray, int> contextSet;
      for (it = messages.constBegin(); it != messages.constEnd(); ++it) {
         ++contextSet[it.key().context()];
      }

      quint16 hTableSize;
      if (contextSet.size() < 200) {
         hTableSize = (contextSet.size() < 60) ? 151 : 503;
      } else if (contextSet.size() < 2500) {
         hTableSize = (contextSet.size() < 750) ? 1511 : 5003;
      } else {
         hTableSize = (contextSet.size() < 10000) ? 15013 : 3 * contextSet.size() / 2;
      }

      QMultiMap<int, QByteArray> hashMap;
      QMap<QByteArray, int>::const_iterator c;
      for (c = contextSet.constBegin(); c != contextSet.constEnd(); ++c) {
         hashMap.insert(elfHash(c.key()) % hTableSize, c.key());
      }

      /*
        The contexts found in this translator are stored in a hash
        table to provide fast lookup. The context array has the
        following format:

            quint16 hTableSize;
            quint16 hTable[hTableSize];
            quint8  contextPool[...];

        The context pool stores the contexts as Pascal strings:

            quint8  len;
            quint8  data[len];

        Let's consider the look-up of context "FunnyDialog".  A
        hash value between 0 and hTableSize - 1 is computed, say h.
        If hTable[h] is 0, "FunnyDialog" is not covered by this
        translator. Else, we check in the contextPool at offset
        2 * hTable[h] to see if "FunnyDialog" is one of the
        contexts stored there, until we find it or we meet the
        empty string.
      */
      m_contextArray.resize(2 + (hTableSize << 1));
      QDataStream t(&m_contextArray, QIODevice::WriteOnly);

      quint16 *hTable = new quint16[hTableSize];
      memset(hTable, 0, hTableSize * sizeof(quint16));

      t << hTableSize;
      t.device()->seek(2 + (hTableSize << 1));
      t << quint16(0); // the entry at offset 0 cannot be used
      uint upto = 2;

      auto entry = hashMap.constBegin();

      while (entry != hashMap.constEnd()) {
         int i = entry.key();
         hTable[i] = quint16(upto >> 1);

         do {
            const char *con = entry.value().constData();
            uint len = uint(entry.value().length());
            len = qMin(len, 255u);
            t << quint8(len);
            t.writeRawData(con, len);
            upto += 1 + len;
            ++entry;
         } while (entry != hashMap.constEnd() && entry.key() == i);
         if (upto & 0x1) {
            // offsets have to be even
            t << quint8(0); // empty string
            ++upto;
         }
      }
      t.device()->seek(2);
      for (int j = 0; j < hTableSize; j++) {
         t << hTable[j];
      }
      delete [] hTable;

      if (upto > 131072) {
         qWarning("Releaser::squeeze: Too many contexts");
         m_contextArray.clear();
      }
   }
}

void Releaser::insertInternal(const TranslatorMessage &message, const QStringList &tlns,
                              bool forceComment, bool isUtf8)
{
   ByteTranslatorMessage bmsg(originalBytes(message.context(), isUtf8),
                              originalBytes(message.sourceText(), isUtf8),
                              originalBytes(message.comment(), isUtf8),
                              tlns);
   if (!forceComment) {
      ByteTranslatorMessage bmsg2(
         bmsg.context(), bmsg.sourceText(), QByteArray(""), bmsg.translations());
      if (!m_messages.contains(bmsg2)) {
         m_messages.insert(bmsg2, 0);
         return;
      }
   }
   m_messages.insert(bmsg, 0);
}

void Releaser::insert(const TranslatorMessage &message, const QStringList &tlns, bool forceComment)
{
   insertInternal(message, tlns, forceComment, message.isUtf8());
   if (message.isUtf8() && message.isNonUtf8()) {
      insertInternal(message, tlns, forceComment, false);
   }
}

void Releaser::insertIdBased(const TranslatorMessage &message, const QStringList &tlns)
{
   ByteTranslatorMessage bmsg("", originalBytes(message.id(), false), "", tlns);
   m_messages.insert(bmsg, 0);
}

void Releaser::setNumerusRules(const QByteArray &rules)
{
   m_numerusRules = rules;
}

static quint8 read8(const uchar *data)
{
   return *data;
}

static quint32 read32(const uchar *data)
{
   return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3]);
}

static void fromBytes(const char *str, int len, QTextCodec *codec, QTextCodec *utf8Codec,
                      QString *out, QString *utf8Out,
                      bool *isSystem, bool *isUtf8, bool *needs8Bit)
{
   for (int i = 0; i < len; ++i)
      if (str[i] & 0x80) {
         if (utf8Codec) {
            QTextCodec::ConverterState cvtState;
            *utf8Out = utf8Codec->toUnicode(str, len, &cvtState);
            *isUtf8 = !cvtState.invalidChars;
         }
         QTextCodec::ConverterState cvtState;
         *out = codec->toUnicode(str, len, &cvtState);
         *isSystem = !cvtState.invalidChars;
         *needs8Bit = true;
         return;
      }
   *out = QString::fromLatin1(str, len);
   *isSystem = true;
   if (utf8Codec) {
      *utf8Out = *out;
      *isUtf8 = true;
   }
   *needs8Bit = false;
}

bool loadQM(Translator &translator, QIODevice &dev, ConversionData &cd)
{
   QByteArray ba = dev.readAll();
   const uchar *data = (uchar *)ba.data();
   int len = ba.size();
   if (len < MagicLength || memcmp(data, magic, MagicLength) != 0) {
      cd.appendError(QLatin1String("QM-Format error: magic marker missing"));
      return false;
   }

   enum { Contexts = 0x2f, Hashes = 0x42, Messages = 0x69, NumerusRules = 0x88 };

   // for squeezed but non-file data, this is what needs to be deleted
   const uchar *messageArray = 0;
   const uchar *offsetArray = 0;
   uint offsetLength = 0;

   bool ok = true;
   const uchar *end = data + len;

   data += MagicLength;

   while (data < end - 4) {
      quint8 tag = read8(data++);
      quint32 blockLen = read32(data);
      //qDebug() << "TAG:" << tag <<  "BLOCKLEN:" << blockLen;
      data += 4;
      if (!tag || !blockLen) {
         break;
      }
      if (data + blockLen > end) {
         ok = false;
         break;
      }

      if (tag == Hashes) {
         offsetArray = data;
         offsetLength = blockLen;
         //qDebug() << "HASHES: " << blockLen << QByteArray((const char *)data, blockLen).toHex();
      } else if (tag == Messages) {
         messageArray = data;
         //qDebug() << "MESSAGES: " << blockLen << QByteArray((const char *)data, blockLen).toHex();
      }

      data += blockLen;
   }


   size_t numItems = offsetLength / (2 * sizeof(quint32));
   //qDebug() << "NUMITEMS: " << numItems;

   QTextCodec *codec = QTextCodec::codecForName(
                          cd.m_codecForSource.isEmpty() ? QByteArray("Latin1") : cd.m_codecForSource);
   QTextCodec *utf8Codec = 0;
   if (codec->name() != "UTF-8") {
      utf8Codec = QTextCodec::codecForName("UTF-8");
   }

   QString strProN = QLatin1String("%n");
   QLocale::Language l;
   QLocale::Country c;
   Translator::languageAndCountry(translator.languageCode(), &l, &c);
   QStringList numerusForms;
   bool guessPlurals = true;
   if (getNumerusInfo(l, c, 0, &numerusForms, 0)) {
      guessPlurals = (numerusForms.count() == 1);
   }

   QString context, contextUtf8;
   bool contextIsSystem, contextIsUtf8, contextNeeds8Bit;
   QString sourcetext, sourcetextUtf8;
   bool sourcetextIsSystem, sourcetextIsUtf8, sourcetextNeeds8Bit;
   QString comment, commentUtf8;
   bool commentIsSystem, commentIsUtf8, commentNeeds8Bit;
   QStringList translations;

   for (const uchar *start = offsetArray; start != offsetArray + (numItems << 3); start += 8) {
      //quint32 hash = read32(start);
      quint32 ro = read32(start + 4);
      //qDebug() << "\nHASH:" << hash;
      const uchar *m = messageArray + ro;

      for (;;) {
         uchar tag = read8(m++);
         //qDebug() << "Tag:" << tag << " ADDR: " << m;
         switch (tag) {
            case Tag_End:
               goto end;
            case Tag_Translation: {
               int len = read32(m);
               if (len % 1) {
                  cd.appendError(QLatin1String("QM-Format error"));
                  return false;
               }
               m += 4;
               QString str = QString((const QChar *)m, len / 2);
               if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
                  for (int i = 0; i < str.length(); ++i)
                     str[i] = QChar((str.at(i).unicode() >> 8) +
                                    ((str.at(i).unicode() << 8) & 0xff00));
               }
               translations << str;
               m += len;
               break;
            }
            case Tag_Obsolete1:
               m += 4;
               //qDebug() << "OBSOLETE";
               break;
            case Tag_SourceText: {
               quint32 len = read32(m);
               m += 4;
               //qDebug() << "SOURCE LEN: " << len;
               //qDebug() << "SOURCE: " << QByteArray((const char*)m, len);
               fromBytes((const char *)m, len, codec, utf8Codec,
                         &sourcetext, &sourcetextUtf8,
                         &sourcetextIsSystem, &sourcetextIsUtf8, &sourcetextNeeds8Bit);
               m += len;
               break;
            }
            case Tag_Context: {
               quint32 len = read32(m);
               m += 4;
               //qDebug() << "CONTEXT LEN: " << len;
               //qDebug() << "CONTEXT: " << QByteArray((const char*)m, len);
               fromBytes((const char *)m, len, codec, utf8Codec,
                         &context, &contextUtf8,
                         &contextIsSystem, &contextIsUtf8, &contextNeeds8Bit);
               m += len;
               break;
            }
            case Tag_Comment: {
               quint32 len = read32(m);
               m += 4;
               //qDebug() << "COMMENT LEN: " << len;
               //qDebug() << "COMMENT: " << QByteArray((const char*)m, len);
               fromBytes((const char *)m, len, codec, utf8Codec,
                         &comment, &commentUtf8,
                         &commentIsSystem, &commentIsUtf8, &commentNeeds8Bit);
               m += len;
               break;
            }
            default:
               //qDebug() << "UNKNOWN TAG" << tag;
               break;
         }
      }
   end:
      ;
      TranslatorMessage msg;
      msg.setType(TranslatorMessage::Finished);
      if (translations.count() > 1) {
         // If guessPlurals is not false here, plural form discard messages
         // will be spewn out later.
         msg.setPlural(true);
      } else if (guessPlurals) {
         // This might cause false positives, so it is a fallback only.
         if (sourcetext.contains(strProN)) {
            msg.setPlural(true);
         }
      }
      msg.setTranslations(translations);
      translations.clear();
      if (contextNeeds8Bit || sourcetextNeeds8Bit || commentNeeds8Bit) {
         if (utf8Codec && contextIsUtf8 && sourcetextIsUtf8 && commentIsUtf8) {
            // The message is utf-8, but file is not.
            msg.setUtf8(true);
            msg.setContext(contextUtf8);
            msg.setSourceText(sourcetextUtf8);
            msg.setComment(commentUtf8);
            translator.append(msg);
            continue;
         }
         if (!(contextIsSystem && sourcetextIsSystem && commentIsSystem)) {
            cd.appendError(QLatin1String(
                              "Cannot read file with specified input codec"));
            return false;
         }
         // The message is 8-bit in the file's encoding (utf-8 or not).
      }
      msg.setContext(context);
      msg.setSourceText(sourcetext);
      msg.setComment(comment);
      translator.append(msg);
   }
   return ok;
}



static bool containsStripped(const Translator &translator, const TranslatorMessage &msg)
{
   foreach (const TranslatorMessage & tmsg, translator.messages())
   if (tmsg.sourceText() == msg.sourceText()
         && tmsg.context() == msg.context()
         && tmsg.comment().isEmpty()) {
      return true;
   }
   return false;
}

bool saveQM(const Translator &translator, QIODevice &dev, ConversionData &cd)
{
   Releaser releaser;
   QLocale::Language l;
   QLocale::Country c;
   Translator::languageAndCountry(translator.languageCode(), &l, &c);
   QByteArray rules;
   if (getNumerusInfo(l, c, &rules, 0, 0)) {
      releaser.setNumerusRules(rules);
   }
   releaser.setCodecName(translator.codecName());

   int finished = 0;
   int unfinished = 0;
   int untranslated = 0;
   int missingIds = 0;
   int droppedData = 0;

   for (int i = 0; i != translator.messageCount(); ++i) {
      const TranslatorMessage &msg = translator.message(i);
      TranslatorMessage::Type typ = msg.type();
      if (typ != TranslatorMessage::Obsolete) {
         if (cd.m_idBased && msg.id().isEmpty()) {
            ++missingIds;
            continue;
         }
         if (typ == TranslatorMessage::Unfinished) {
            if (msg.translation().isEmpty() && !cd.m_idBased && cd.m_unTrPrefix.isEmpty()) {
               ++untranslated;
               continue;
            } else {
               if (cd.ignoreUnfinished()) {
                  continue;
               }
               ++unfinished;
            }
         } else {
            ++finished;
         }
         QStringList tlns = msg.translations();
         if (msg.type() == TranslatorMessage::Unfinished
               && (cd.m_idBased || !cd.m_unTrPrefix.isEmpty()))
            for (int j = 0; j < tlns.size(); ++j)
               if (tlns.at(j).isEmpty()) {
                  tlns[j] = cd.m_unTrPrefix + msg.sourceText();
               }
         if (cd.m_idBased) {
            if (!msg.context().isEmpty() || !msg.comment().isEmpty()) {
               ++droppedData;
            }
            releaser.insertIdBased(msg, tlns);
         } else {
            // Drop the comment in (context, sourceText, comment),
            // unless the context is empty,
            // unless (context, sourceText, "") already exists or
            // unless we already dropped the comment of (context,
            // sourceText, comment0).
            bool forceComment =
               msg.comment().isEmpty()
               || msg.context().isEmpty()
               || containsStripped(translator, msg);
            releaser.insert(msg, tlns, forceComment);
         }
      }
   }

   if (missingIds)
      cd.appendError(QCoreApplication::translate("LRelease",
                     "Dropped %n message(s) which had no ID.", 0,
                     QCoreApplication::CodecForTr, missingIds));
   if (droppedData)
      cd.appendError(QCoreApplication::translate("LRelease",
                     "Excess context/disambiguation dropped from %n message(s).", 0,
                     QCoreApplication::CodecForTr, droppedData));

   releaser.squeeze(cd.m_saveMode);
   bool saved = releaser.save(&dev);
   if (saved && cd.isVerbose()) {
      int generatedCount = finished + unfinished;
      cd.appendError(QCoreApplication::translate("LRelease",
                     "    Generated %n translation(s) (%1 finished and %2 unfinished)", 0,
                     QCoreApplication::CodecForTr, generatedCount).arg(finished).arg(unfinished));
      if (untranslated)
         cd.appendError(QCoreApplication::translate("LRelease",
                        "    Ignored %n untranslated source text(s)", 0,
                        QCoreApplication::CodecForTr, untranslated));
   }
   return saved;
}

int initQM()
{
   Translator::FileFormat format;

   format.extension = QLatin1String("qm");
   format.description = QObject::tr("Compiled Qt translations");
   format.fileType = Translator::FileFormat::TranslationBinary;
   format.priority = 0;
   format.loader = &loadQM;
   format.saver = &saveQM;
   Translator::registerFileFormat(format);

   return 1;
}

Q_CONSTRUCTOR_FUNCTION(initQM)

QT_END_NAMESPACE
