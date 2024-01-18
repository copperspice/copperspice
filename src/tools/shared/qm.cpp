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

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmap.h>
#include <qstring.h>
#include <qtextcodec.h>

// magic number for the file
static const int MagicLength = 16;

static const uchar magic[MagicLength] = {
   0x3c, 0xb8, 0x64, 0x18, 0xca, 0xef, 0x9c, 0x95,
   0xcd, 0x21, 0x1c, 0xbf, 0x60, 0xa1, 0xbd, 0xdd
};

enum class TranslatorPrefix {
   NoPrefix,
   Hash,
   HashContext,
   HashContextSourceText,
   HashContextSourceTextComment
};

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

   if (! h) {
      h = 1;
   }

   return h;
}

class ByteTranslatorMessage
{
 public:
   ByteTranslatorMessage(const QByteArray &context, const QByteArray &sourceText, const QByteArray &comment,
            const QStringList &translations)
      : m_context(context), m_sourcetext(sourceText), m_comment(comment), m_translations(translations) {
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

   Releaser() {
   }

   Releaser(const Releaser &) = delete;
   Releaser &operator=(const Releaser &) = delete;

   bool save(QIODevice *iod);

   void insert(const TranslatorMessage &msg, const QStringList &tlns, bool forceComment);
   void insertIdBased(const TranslatorMessage &message, const QStringList &tlns);

   void squeeze(TranslatorMessage::SaveMode mode);

   void setCountRules(const QVector<std::variant<CountGuide, int>> &rules);
   void setDependencies(const QStringList &dependencies);

 private:
   // This should reproduce the byte array fetched from the source file, which
   // on turn should be the same as passed to the actual tr(...) calls
   QByteArray originalBytes(const QString &str) const;

   static TranslatorPrefix commonPrefix(const ByteTranslatorMessage &m1, const ByteTranslatorMessage &m2);

   static uint msgHash(const ByteTranslatorMessage &msg);

   void writeMessage(const ByteTranslatorMessage &msg, QDataStream &stream,
                     TranslatorMessage::SaveMode strip, TranslatorPrefix prefix) const;

   // for squeezed but non-file data, this is what needs to be deleted
   QByteArray m_messageArray;
   QByteArray m_offsetArray;
   QByteArray m_contextArray;
   QMap<ByteTranslatorMessage, void *> m_messages;
   QStringList m_dependencies;
   QByteArray m_dependencyArray;

   QVector<std::variant<CountGuide, int>> m_countRules;
};

QByteArray Releaser::originalBytes(const QString &str) const
{
   if (str.isEmpty()) {
      // do not use QByteArray() without the quotes, result of the serialization will be different
      return QByteArray("");
   }

   return str.toUtf8();
}

uint Releaser::msgHash(const ByteTranslatorMessage &msg)
{
   return elfHash(msg.sourceText() + msg.comment());
}

TranslatorPrefix Releaser::commonPrefix(const ByteTranslatorMessage &m1, const ByteTranslatorMessage &m2)
{
   if (msgHash(m1) != msgHash(m2)) {
      return TranslatorPrefix::NoPrefix;
   }

   if (m1.context() != m2.context()) {
      return TranslatorPrefix::Hash;
   }

   if (m1.sourceText() != m2.sourceText()) {
      return TranslatorPrefix::HashContext;
   }

   if (m1.comment() != m2.comment()) {
      return TranslatorPrefix::HashContextSourceText;
   }

   return TranslatorPrefix::HashContextSourceTextComment;
}

void Releaser::writeMessage(const ByteTranslatorMessage &msg, QDataStream &stream,
            TranslatorMessage::SaveMode mode, TranslatorPrefix prefix) const
{
   for (int i = 0; i < msg.translations().count(); ++i) {
      stream << quint8(TranslatorTag::Translation) << msg.translations().at(i);
   }

   if (mode == TranslatorMessage::SaveMode::Everything) {
      prefix = TranslatorPrefix::HashContextSourceTextComment;
   }

   // lrelease produces an invalid qm file for QByteArrays which are null
   switch (prefix) {
      default:
      case TranslatorPrefix::HashContextSourceTextComment:
         stream << quint8(TranslatorTag::Comment) << msg.comment();
         [[fallthrough]];

      case TranslatorPrefix::HashContextSourceText:
         stream << quint8(TranslatorTag::SourceText) << msg.sourceText();
         [[fallthrough]];

      case TranslatorPrefix::HashContext:
         stream << quint8(TranslatorTag::Context) << msg.context();
         break;
   }

   stream << quint8(TranslatorTag::End);
}

bool Releaser::save(QIODevice *iod)
{
   QDataStream s(iod);
   s.writeRawData((const char *)magic, MagicLength);

   if (! m_dependencyArray.isEmpty()) {
      quint32 arraySize = quint32(m_dependencyArray.size());

      s << static_cast<quint8>(TranslatorCategory::Dependencies) << arraySize;
      s.writeRawData(m_dependencyArray.constData(), arraySize);
   }

   if (! m_offsetArray.isEmpty()) {
      quint32 arraySize = quint32(m_offsetArray.size());

      s << static_cast<quint8>(TranslatorCategory::Hashes) << arraySize;
      s.writeRawData(m_offsetArray.constData(), arraySize);
   }

   if (! m_messageArray.isEmpty()) {
      quint32 arraySize = quint32(m_messageArray.size());

      s << static_cast<quint8>(TranslatorCategory::Messages) << arraySize;
      s.writeRawData(m_messageArray.constData(), arraySize);
   }

   if (! m_contextArray.isEmpty()) {
      quint32 arraySize = quint32(m_contextArray.size());

      s << static_cast<quint8>(TranslatorCategory::Contexts) << arraySize;
      s.writeRawData(m_contextArray.constData(), arraySize);
   }

   if (! m_countRules.isEmpty()) {
      quint32 arraySize = m_countRules.size();

      s << static_cast<quint8>(TranslatorCategory::CountRules) << arraySize * 2;

      for (auto item : m_countRules) {
         quint8 which = item.index();

         // indicator what the data will be
         s << static_cast<quint8>(which);

         switch (which) {
            case 0:
               s << static_cast<quint8>(std::get<CountGuide>(item));
               break;

            case 1:
               s << static_cast<quint8>(std::get<int>(item));
               break;
         }
      }
   }

   return true;
}

void Releaser::squeeze(TranslatorMessage::SaveMode mode)
{
   m_dependencyArray.clear();

   QDataStream depstream(&m_dependencyArray, QIODevice::WriteOnly);

   for (const QString &dep : m_dependencies) {
      depstream << dep;
   }

   if (m_messages.isEmpty() && mode == TranslatorMessage::SaveMode::Everything) {
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
   QMap<ByteTranslatorMessage, void *>::const_iterator iter;
   QMap<ByteTranslatorMessage, void *>::const_iterator next;

   int cpPrev = 0;
   int cpNext = 0;

   for (iter = messages.cbegin(); iter != messages.cend(); ++iter) {
      cpPrev = cpNext;
      next   = iter;
      ++next;

      if (next == messages.constEnd()) {
         cpNext = 0;

      } else {
         cpNext = static_cast<int>(commonPrefix(iter.key(), next.key()));
      }

      offsets.insert(Offset(msgHash(iter.key()), ms.device()->pos()), nullptr);
      writeMessage(iter.key(), ms, mode, TranslatorPrefix(qMax(cpPrev, cpNext + 1)));
   }

   QMap<Offset, void *>::iterator offset;
   offset = offsets.begin();

   QDataStream ds(&m_offsetArray, QIODevice::WriteOnly);

   while (offset != offsets.end()) {
      Offset k = offset.key();
      ++offset;
      ds << quint32(k.h) << quint32(k.o);
   }

   if (mode == TranslatorMessage::SaveMode::Stripped) {
      QMap<QByteArray, int> contextSet;

      for (iter = messages.cbegin(); iter != messages.cend(); ++iter) {
         ++contextSet[iter.key().context()];
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

      for (c = contextSet.cbegin(); c != contextSet.cend(); ++c) {
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

void Releaser::insert(const TranslatorMessage &message, const QStringList &tlns, bool forceComment)
{
   ByteTranslatorMessage bmsg(originalBytes(message.context()), originalBytes(message.sourceText()),
            originalBytes(message.comment()), tlns);

   if (! forceComment) {
      ByteTranslatorMessage bmsg2( bmsg.context(), bmsg.sourceText(), QByteArray(), bmsg.translations());

      if (! m_messages.contains(bmsg2)) {
         m_messages.insert(bmsg2, nullptr);
         return;
      }
   }

   m_messages.insert(bmsg, nullptr);
}

void Releaser::insertIdBased(const TranslatorMessage &message, const QStringList &tlns)
{
   ByteTranslatorMessage bmsg(QByteArray(), originalBytes(message.id()), QByteArray(), tlns);
   m_messages.insert(bmsg, nullptr);
}

void Releaser::setCountRules(const QVector<std::variant<CountGuide, int>> &data)
{
   m_countRules = data;
}

void Releaser::setDependencies(const QStringList &dependencies)
{
   m_dependencies = dependencies;
}

static quint8 read8(const uchar *data)
{
   return *data;
}

static quint32 read32(const uchar *data)
{
   // reading a big endian 32 bit integer
   return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3]);
}

static void fromBytes(const char *str, int len, QString *out, bool *utf8Fail)
{
   static QTextCodec *utf8Codec = QTextCodec::codecForName("UTF-8");

   QTextCodec::ConverterState cvtState;

   *out      = utf8Codec->toUnicode(str, len, &cvtState);
   *utf8Fail = cvtState.invalidChars;
}

bool loadQM(Translator &translator, QIODevice &dev, ConversionData &cd)
{
   QByteArray ba = dev.readAll();
   const uchar *data = (uchar *)ba.data();
   int len = ba.size();

   if (len < MagicLength || memcmp(data, magic, MagicLength) != 0) {
      cd.appendError("QM-Format error: magic marker missing");
      return false;
   }

   // for squeezed but non-file data, this is what needs to be deleted
   const uchar *messageArray = nullptr;
   const uchar *offsetArray  = nullptr;

   uint offsetLength = 0;

   bool ok = true;
   const uchar *end = data + len;

   data += MagicLength;

   while (data < end - 4) {
      TranslatorCategory tag = static_cast<TranslatorCategory>(read8(data));
      ++data;

      quint32 blockLen = read32(data);
      data += 4;

      if (tag == TranslatorCategory::Invalid || blockLen == 0) {
         break;
      }

      if (data + blockLen > end) {
         ok = false;
         break;
      }

      if (tag == TranslatorCategory::Hashes) {
         offsetArray  = data;
         offsetLength = blockLen;

      } else if (tag == TranslatorCategory::Messages) {
         messageArray = data;

      } else if (tag == TranslatorCategory::Dependencies) {
         QStringList dependencies;
         QDataStream stream(QByteArray::fromRawData((const char *)data, blockLen));
         QString dep;

         while (! stream.atEnd()) {
            stream >> dep;
            dependencies.append(dep);
         }

         translator.setDependencies(dependencies);
      }

      data += blockLen;
   }

   size_t numItems = offsetLength / (2 * sizeof(quint32));
   QString strProN = "%n";

   QLocale::Language l;
   QLocale::Country c;

   Translator::languageAndCountry(translator.languageCode(), &l, &c);
   QStringList numerusForms;
   bool guessPlurals = true;

   if (getCountInfo(l, c, nullptr, &numerusForms, nullptr)) {
      guessPlurals = (numerusForms.count() == 1);
   }

   QString context;
   QString sourcetext;
   QString comment;

   bool utf8Fail = false;

   QStringList translations;

   for (const uchar *start = offsetArray; start != offsetArray + (numItems << 3); start += 8) {
      quint32 ro     = read32(start + 4);
      const uchar *m = messageArray + ro;

      bool done = false;

      while (! done) {
         TranslatorTag tag = static_cast<TranslatorTag>(read8(m));
         ++m;

         switch (tag) {
            case TranslatorTag::End:
               done = true;
               break;

            case TranslatorTag::Obsolete1:
               m += 4;
               break;

            case TranslatorTag::Translation: {
               quint32 len = read32(m);
               m += 4;

               if (len == 0xffffffff) {
                  // indicates QByteArray was null
                  len = 0;
               }

               QString str = QString::fromUtf8((const char *)m, len);
               translations << str;

               m += len;

               break;
            }

            case TranslatorTag::SourceText: {
               quint32 len = read32(m);
               m += 4;

               if (len == 0xffffffff) {
                  // indicates QByteArray was null
                  len = 0;
               }

               fromBytes((const char *)m, len, &sourcetext, &utf8Fail);

               m += len;
               break;
            }

            case TranslatorTag::Context: {
               quint32 len = read32(m);
               m += 4;

               if (len == 0xffffffff) {
                  // indicates QByteArray was null
                  len = 0;
               }

               fromBytes((const char *)m, len, &context, &utf8Fail);

               m += len;
               break;
            }

            case TranslatorTag::Comment: {
               quint32 len = read32(m);
               m += 4;

               if (len == 0xffffffff) {
                  // indicates QByteArray was null
                  len = 0;
               }

               fromBytes((const char *)m, len, &comment, &utf8Fail);

               m += len;
               break;
            }

            default:
               // "unknown tag"
               break;
         }
      }

      TranslatorMessage msg;
      msg.setType(TranslatorMessage::Type::Finished);

      if (translations.count() > 1) {
         // If guessPlurals is not false here, plural form discard messages will be spewn out later
         msg.setPlural(true);

      } else if (guessPlurals) {
         // might cause false positives, so it is a fallback only
         if (sourcetext.contains(strProN)) {
            msg.setPlural(true);
         }
      }

      msg.setTranslations(translations);
      translations.clear();

      msg.setContext(context);
      msg.setSourceText(sourcetext);
      msg.setComment(comment);
      translator.append(msg);
   }

   if (utf8Fail) {
      cd.appendError("Unable to read file with UTF-8 codec");
      return false;
   }

   return ok;
}

static bool containsStripped(const Translator &translator, const TranslatorMessage &msg)
{
   for (const TranslatorMessage &tmsg : translator.messages()) {

      if (tmsg.sourceText() == msg.sourceText() && tmsg.context() == msg.context() && tmsg.comment().isEmpty()) {
         return true;
      }
   }

   return false;
}

bool saveQM(const Translator &translator, QIODevice &dev, ConversionData &cd)
{
   Releaser releaser;
   QLocale::Language l;
   QLocale::Country c;

   Translator::languageAndCountry(translator.languageCode(), &l, &c);
   QVector<std::variant<CountGuide, int>> data;

   if (getCountInfo(l, c, &data, nullptr, nullptr)) {
      releaser.setCountRules(data);
   }

   int finished     = 0;
   int unfinished   = 0;
   int untranslated = 0;
   int missingIds   = 0;
   int droppedData  = 0;

   for (int i = 0; i != translator.messageCount(); ++i) {
      const TranslatorMessage &msg = translator.message(i);
      TranslatorMessage::Type typ  = msg.type();

      if (typ != TranslatorMessage::Type::Obsolete && typ != TranslatorMessage::Type::Vanished) {
         if (cd.m_idBased && msg.id().isEmpty()) {
            ++missingIds;
            continue;
         }

         if (typ == TranslatorMessage::Type::Unfinished) {
            if (msg.translation().isEmpty() && ! cd.m_idBased && cd.m_unTrPrefix.isEmpty()) {
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

         if (msg.type() == TranslatorMessage::Type::Unfinished && (cd.m_idBased || ! cd.m_unTrPrefix.isEmpty())) {
            for (int j = 0; j < tlns.size(); ++j) {
               if (tlns.at(j).isEmpty()) {
                  tlns[j] = cd.m_unTrPrefix + msg.sourceText();
               }
            }
         }

         if (cd.m_idBased) {
            if (! msg.context().isEmpty() || ! msg.comment().isEmpty()) {
               ++droppedData;
            }

            releaser.insertIdBased(msg, tlns);

         } else {
            // Drop the comment in (context, sourceText, comment),
            // unless the context is empty,
            // unless (context, sourceText, "") already exists or
            // unless we already dropped the comment of (context,
            // sourceText, comment0)

            bool forceComment = msg.comment().isEmpty() || msg.context().isEmpty() || containsStripped(translator, msg);
            releaser.insert(msg, tlns, forceComment);
         }
      }
   }

   if (missingIds) {
      cd.appendError(QCoreApplication::translate("lrelease", "Dropped %n message(s) which had no ID.",
                     nullptr, missingIds));
   }

   if (droppedData) {
      cd.appendError(QCoreApplication::translate("lrelease", "Excess context/disambiguation dropped from %n message(s).",
                     nullptr, droppedData));
   }

   releaser.setDependencies(translator.dependencies());
   releaser.squeeze(cd.m_saveMode);

   bool saved = releaser.save(&dev);

   if (saved && cd.isVerbose()) {
      int generatedCount = finished + unfinished;

      cd.appendError(QCoreApplication::translate("lrelease", "    Generated %n translation(s) (%1 finished and %2 unfinished)",
                     nullptr, generatedCount).formatArg(finished).formatArg(unfinished));

      if (untranslated) {
         cd.appendError(QCoreApplication::translate("lrelease", "    Ignored %n untranslated source text(s)",
                        nullptr, untranslated));
      }
   }

   return saved;
}

int initQM()
{
   Translator::FileFormat format;

   format.extension   = "qm";
   format.description = "Compiled Translations";
   format.fileType    = Translator::FileFormat::TranslationBinary;
   format.priority    = 0;
   format.loader      = &loadQM;
   format.saver       = &saveQM;
   Translator::registerFileFormat(format);

   return 1;
}

Q_CONSTRUCTOR_FUNCTION(initQM)

