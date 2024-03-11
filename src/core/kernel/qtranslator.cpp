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

#include <qtranslator.h>

#include <qalgorithms.h>
#include <qcoreapplication.h>
#include <qdatastream.h>
#include <qdir.h>
#include <qendian.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qhash.h>
#include <qlocale.h>
#include <qmap.h>
#include <qplatformdefs.h>
#include <qresource.h>
#include <qstring.h>
#include <qstringlist.h>

#include <qcoreapplication_p.h>
#include <qtranslator_p.h>

#if defined(Q_OS_UNIX)

#define QT_USE_MMAP
#include <qcore_unix_p.h>
#endif

// most of the headers below are already included in qplatformdefs.h
// this lacks Large File support but that's probably irrelevant
#if defined(QT_USE_MMAP)

// for mmap
#include <sys/mman.h>
#include <errno.h>
#endif

#include <stdlib.h>

/*
$ mcookie
3cb86418caef9c95cd211cbf60a1bddd
$
*/

// magic number for the file
static constexpr const int MagicLength = 16;

static const uchar magic[MagicLength] = {
   0x3c, 0xb8, 0x64, 0x18, 0xca, 0xef, 0x9c, 0x95,
   0xcd, 0x21, 0x1c, 0xbf, 0x60, 0xa1, 0xbd, 0xdd
};

static bool match(const uchar *found, quint32 foundLen, const char *target, uint targetLen)
{
   // catch the case if found has a zero-terminating symbol and len includes it.
   // (normalize it to be without the zero-terminating symbol)
   if (foundLen > 0 && found[foundLen - 1] == '\0') {
      --foundLen;
   }

   return ((targetLen == foundLen) && memcmp(found, target, foundLen) == 0);
}

static void elfHash_start(const char *name, uint &h)
{
   const uchar *k;
   uint g;

   k = (const uchar *) name;

   while (*k) {
      h = (h << 4) + *k++;

      if ((g = (h & 0xf0000000)) != 0) {
         h ^= g >> 24;
      }

      h &= ~g;
   }

}

static void elfHash_finish(uint &h)
{
   if (h == 0) {
      h = 1;
   }
}

static uint elfHash(const char *name)
{
   uint hash = 0;

   elfHash_start(name, hash);
   elfHash_finish(hash);

   return hash;
}

static bool isValidCountRules(const QVector<std::variant<CountGuide, int>> &data)
{
   uint rulesSize = data.size();

   if (rulesSize == 0) {
      return true;
   }

   quint32 offset = 0;

   try {

      while (true) {
         CountGuide opcode = std::get<CountGuide>(data[offset]);
         CountGuide option = static_cast<CountGuide>(opcode & CountGuide::OperatorMask);

         if (opcode & CountGuide::OperatorInvalid) {
            return false;                  // Bad operation
         }

         ++offset;

         if (offset == rulesSize) {
            return false;                  // Missing operand
         }

         // right operand
         ++offset;

         switch (option) {
            case CountGuide::Equal:
            case CountGuide::LessThan:
            case CountGuide::LessThanEqual:
               break;

            case CountGuide::Between:
               if (offset != rulesSize) {
                  // third operand
                  ++offset;
                  break;
               }

               return false;               // Missing operand

            default:
               return false;               // Bad option (0)
         }

         if (offset == rulesSize) {
            return true;
         }

         CountGuide tmp = std::get<CountGuide>(data[offset]);

         if ((tmp == CountGuide::And) || (tmp == CountGuide::Or) || (tmp == CountGuide::LastEntry)) {
            // keep going

         } else {
            // something is not correct
            break;
         }

         ++offset;

         if (offset == rulesSize) {
            break;
         }
      }

   } catch (std::bad_variant_access &e) {
      // ignore exception

   }

   // bad option
   return false;
}

static uint countHelper(int n, const QVector<std::variant<CountGuide, int>> &data)
{
   uint result = 0;
   uint cnt    = 0;

   uint rulesSize = data.size();

   if (rulesSize == 0) {
      return result;
   }

   while (true) {
      bool orExprTruthValue = false;

      while (true) {
         bool andExprTruthValue = true;

         while (true) {
            bool truthValue = true;

            CountGuide opcode = std::get<CountGuide>(data[cnt]);
            int leftOperand   = n;

            ++cnt;

            if (opcode & CountGuide::Remainder_10) {
               leftOperand %= 10;

            } else if (opcode & CountGuide::Remainder_100) {
               leftOperand %= 100;

            } else if (opcode & CountGuide::Divide_1000) {
               while (leftOperand >= 1000) {
                  leftOperand /= 1000;
               }
            }

            CountGuide op = static_cast<CountGuide>(opcode & CountGuide::OperatorMask);

            int rightOperand = std::get<int>(data[cnt]);
            ++cnt;

            switch (op) {
               case CountGuide::Equal:
                  truthValue = (leftOperand == rightOperand);
                  break;

               case CountGuide::LessThan:
                  truthValue = (leftOperand < rightOperand);
                  break;

               case CountGuide::LessThanEqual:
                  truthValue = (leftOperand <= rightOperand);
                  break;

               case CountGuide::Between: {
                  int bottom = rightOperand;

                  int top    = std::get<int>(data[cnt]);
                  truthValue = (leftOperand >= bottom && leftOperand <= top);

                  ++cnt;

                  break;
               }

               default:
                  // not a valid case
                  break;
            }

            if (opcode & CountGuide::Not) {
               truthValue = ! truthValue;
            }

            andExprTruthValue = andExprTruthValue && truthValue;

            if (cnt == rulesSize || std::get<CountGuide>(data[cnt]) != CountGuide::And) {
               break;
            }

            ++cnt;
         }

         orExprTruthValue = orExprTruthValue || andExprTruthValue;

         if (cnt == rulesSize || std::get<CountGuide>(data[cnt]) != CountGuide::Or) {
            break;
         }

         ++cnt;
      }

      if (orExprTruthValue) {
         return result;
      }

      ++result;

      if (cnt == rulesSize) {
         return result;
      }

      ++cnt;
   }

   return 0;
}

class QTranslatorPrivate
{
   Q_DECLARE_PUBLIC(QTranslator)

 public:
   QTranslatorPrivate()
      : used_mmap(0), unmapPointer(nullptr), unmapLength(0), resource(nullptr),
        messageArray(nullptr), offsetArray(nullptr), contextArray(nullptr),
        messageLength(0), offsetLength(0), contextLength(0)
   {
   }

   virtual ~QTranslatorPrivate()
   {
   }

   bool used_mmap;
   char *unmapPointer;
   quint32 unmapLength;

   // The resource object in case we loaded the translations from a resource
   QResource *resource;

   // used if the translator has dependencies
   QList<QTranslator *> subTranslators;

   // for squeezed but non-file data, this is what needs to be deleted
   const uchar *messageArray;
   const uchar *offsetArray;
   const uchar *contextArray;

   QVector<std::variant<CountGuide, int>> m_countRules;

   uint messageLength;
   uint offsetLength;
   uint contextLength;

   bool do_load(const QString &filename, const QString &directory);
   bool do_load(const uchar *data, int len, const QString &directory);
   QString do_translate(const char *context, const char *text, const char *comment, std::optional<int> numArg) const;
   void clear();

 protected:
   QTranslator *q_ptr;
};

QTranslator::QTranslator(QObject *parent)
   : QObject(parent), d_ptr(new QTranslatorPrivate)
{
   d_ptr->q_ptr = this;
}

QTranslator::~QTranslator()
{
   if (QCoreApplication::instance()) {
      QCoreApplication::removeTranslator(this);
   }

   Q_D(QTranslator);
   d->clear();
}

bool QTranslator::load(const QString &filename, const QString &directory,
      const QString &search_delimiters, const QString &suffix)
{
   Q_D(QTranslator);
   d->clear();

   QString prefix;

   if (QFileInfo(filename).isRelative()) {
      prefix = directory;

      if (prefix.length() && ! prefix.endsWith('/')) {
         prefix += '/';
      }
   }

   QString fname = filename;
   QString realname;
   QString delims;
   delims = search_delimiters.isEmpty() ? QString("_.") : search_delimiters;

   for (;;) {
      QFileInfo fi;

      realname = prefix + fname + (suffix.isEmpty() ? QString(".qm") : suffix);
      fi.setFile(realname);

      if (fi.isReadable() && fi.isFile()) {
         break;
      }

      realname = prefix + fname;
      fi.setFile(realname);

      if (fi.isReadable() && fi.isFile()) {
         break;
      }

      int rightmost = 0;

      for (int i = 0; i < (int)delims.length(); i++) {
         int k = fname.lastIndexOf(delims[i]);

         if (k > rightmost) {
            rightmost = k;
         }
      }

      // no truncations? fail
      if (rightmost == 0) {
         return false;
      }

      fname.truncate(rightmost);
   }

   // realname is now the fully qualified name of a readable file.
   return d->do_load(realname, directory);
}

bool QTranslatorPrivate::do_load(const QString &realname, const QString &directory)
{
   bool ok = false;

   const bool isResourceFile = realname.startsWith(':');

   if (isResourceFile) {
      // If the translation is in a non-compressed resource file, the data is already in
      // memory, so no need to use QFile to copy it again.

      Q_ASSERT(! resource);
      resource = new QResource(realname);

      if (resource->isValid() && ! resource->isCompressed() && resource->size() > MagicLength
            && ! memcmp(resource->data(), magic, MagicLength)) {

         unmapLength  = resource->size();
         unmapPointer = reinterpret_cast<char *>(const_cast<uchar *>(resource->data()));
         used_mmap = false;
         ok = true;

      } else {
         delete resource;
         resource = nullptr;
      }
   }

   if (! ok) {
      QFile file(realname);

      if (! file.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
         return false;
      }

      qint64 fileSize = file.size();

      if (fileSize <= MagicLength || quint32(-1) <= fileSize) {
         return false;
      }

      {
         char magicBuffer[MagicLength];

         if (MagicLength != file.read(magicBuffer, MagicLength) || memcmp(magicBuffer, magic, MagicLength)) {
            return false;
         }
      }

      unmapLength = quint32(fileSize);

#ifdef QT_USE_MMAP

#ifndef MAP_FILE
#define MAP_FILE 0
#endif

#ifndef MAP_FAILED
#define MAP_FAILED -1
#endif

      int fd = file.handle();

      if (fd >= 0) {

         char *ptr;
         ptr = reinterpret_cast<char *>(mmap(nullptr, unmapLength, PROT_READ,
                           MAP_FILE | MAP_PRIVATE, fd, 0));

         if (ptr && ptr != reinterpret_cast<char *>(MAP_FAILED)) {
            file.close();

            used_mmap    = true;
            unmapPointer = ptr;

            ok = true;
         }
      }

#endif // QT_USE_MMAP

      if (! ok) {
         unmapPointer = new char[unmapLength];

         if (unmapPointer) {
            file.seek(0);
            qint64 readResult = file.read(unmapPointer, unmapLength);

            if (readResult == qint64(unmapLength)) {
               ok = true;
            }
         }
      }
   }

   if (ok && do_load(reinterpret_cast<const uchar *>(unmapPointer), unmapLength, directory)) {
      return true;
   }

#if defined(QT_USE_MMAP)

   if (used_mmap) {
      used_mmap = false;
      munmap(unmapPointer, unmapLength);
   } else
#endif

      if (! resource) {
         delete [] unmapPointer;
      }

   delete resource;
   resource = nullptr;

   unmapPointer = nullptr;
   unmapLength  = 0;
   return false;
}

static QString find_translation(const QLocale &locale, const QString &filename, const QString &prefix,
      const QString &directory, const QString &suffix)
{
   QString path;

   if (QFileInfo(filename).isRelative()) {
      path = directory;

      if (! path.isEmpty() && ! path.endsWith('/')) {
         path += '/';
      }
   }

   QFileInfo fi;
   QString realname;
   QStringList fuzzyLocales;

   QStringList languages = locale.uiLanguages();

#if defined(Q_OS_UNIX)

   for (int i = languages.size() - 1; i >= 0; --i) {
      QString lang = languages.at(i);
      QString lowerLang = lang.toLower();

      if (lang != lowerLang) {
         languages.insert(i + 1, lowerLang);
      }
   }

#endif

   // try explicit locales names first
   for (QString localeName : languages) {
      localeName.replace(QChar('-'), QChar('_'));

      realname = path + filename + prefix + localeName + (suffix.isEmpty() ? QString(".qm") : suffix);
      fi.setFile(realname);

      if (fi.isReadable() && fi.isFile()) {
         return realname;
      }

      realname = path + filename + prefix + localeName;
      fi.setFile(realname);

      if (fi.isReadable() && fi.isFile()) {
         return realname;
      }

      fuzzyLocales.append(localeName);
   }

   // start guessing
   for (QString localeName : fuzzyLocales) {
      while (true) {
         int rightmost = localeName.lastIndexOf(QLatin1Char('_'));

         // no truncations? fail
         if (rightmost <= 0) {
            break;
         }

         localeName.truncate(rightmost);

         realname = path + filename + prefix + localeName + (suffix.isEmpty() ? QString(".qm") : suffix);
         fi.setFile(realname);

         if (fi.isReadable() && fi.isFile()) {
            return realname;
         }

         realname = path + filename + prefix + localeName;
         fi.setFile(realname);

         if (fi.isReadable() && fi.isFile()) {
            return realname;
         }
      }
   }

   if (! suffix.isEmpty()) {
      realname = path + filename + suffix;

      fi.setFile(realname);

      if (fi.isReadable() && fi.isFile()) {
         return realname;
      }
   }

   realname = path + filename + prefix;
   fi.setFile(realname);

   if (fi.isReadable() && fi.isFile()) {
      return realname;
   }

   realname = path + filename;
   fi.setFile(realname);

   if (fi.isReadable() && fi.isFile()) {
      return realname;
   }

   return QString();
}

bool QTranslator::load(const QLocale &locale, const QString &filename, const QString &prefix,
      const QString &directory, const QString &suffix)
{
   Q_D(QTranslator);

   d->clear();
   QString fname = find_translation(locale, filename, prefix, directory, suffix);

   return ! fname.isEmpty() && d->do_load(fname, directory);
}

bool QTranslator::load(const uchar *data, int len, const QString &directory)
{
   Q_D(QTranslator);

   d->clear();

   if (! data || len < MagicLength || memcmp(data, magic, MagicLength)) {
      return false;
   }

   return d->do_load(data, len, directory);
}

static quint8 read8(const uchar *data)
{
   return *data;
}

static quint16 read16(const uchar *data)
{
   // reading a big endian 16 bit integer
   return (data[0] << 8) | data[1];
}

static quint32 read32(const uchar *data)
{
   // reading a big endian 32 bit integer
   return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | (data[3]);
}

bool QTranslatorPrivate::do_load(const uchar *data, int len, const QString &directory)
{
   bool ok = true;

   QStringList dependencies;

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

      if (quint32(end - data) < blockLen) {
         ok = false;
         break;
      }

      if (tag == TranslatorCategory::Contexts) {
         contextArray  = data;
         contextLength = blockLen;

      } else if (tag == TranslatorCategory::Hashes) {
         offsetArray  = data;
         offsetLength = blockLen;

      } else if (tag == TranslatorCategory::Messages) {
         messageArray  = data;
         messageLength = blockLen;

      } else if (tag == TranslatorCategory::CountRules) {

         for (quint32 cnt = 0; cnt < blockLen; cnt += 2)  {
            quint8 which = data[cnt];

            // retrieve and enum or int ( some rule )
            switch (which) {
               case 0:
                  m_countRules.append(static_cast<CountGuide>(data[cnt + 1]));
                  break;

               case 1:
                  m_countRules.append(static_cast<int>(data[cnt + 1]));
                  break;
            }
         }

      } else if (tag == TranslatorCategory::Dependencies) {
         QDataStream stream(QByteArray::fromRawData((const char *)data, blockLen));
         QString dep;

         while (! stream.atEnd()) {
            stream >> dep;
            dependencies.append(dep);
         }
      }

      data += blockLen;
   }

   if (dependencies.isEmpty() && (! offsetArray || ! messageArray)) {
      ok = false;
   }

   if (ok && ! isValidCountRules(m_countRules)) {
      ok = false;
   }

   if (ok) {
      const int dependenciesCount = dependencies.count();

      for (int i = 0 ; i < dependenciesCount; ++i) {
         QTranslator *translator = new QTranslator;
         subTranslators.append(translator);

         ok = translator->load(dependencies.at(i), directory);

         if (! ok) {
            break;
         }
      }

      // In case some dependencies fail to load, unload all the other ones too.
      if (! ok) {
         qDeleteAll(subTranslators);
         subTranslators.clear();
      }
   }

   if (! ok) {
      messageArray       = nullptr;
      contextArray       = nullptr;
      offsetArray        = nullptr;

      m_countRules.clear();

      messageLength      = 0;
      contextLength      = 0;
      offsetLength       = 0;
   }

   return ok;
}

static QString getMessage(const uchar *data, const uchar *end, const char *context, const char *text,
      const char *comment, uint numerus)
{
   QString retval;

   const uchar *tn = nullptr;
   uint tn_length  = 0;

   const uint sourceTextLen = uint(strlen(text));
   const uint contextLen    = uint(strlen(context));
   const uint commentLen    = uint(strlen(comment));

   bool done = false;

   while (! done) {
      TranslatorTag tag;

      if (data < end) {
         tag = static_cast<TranslatorTag>(read8(data));
         ++data;

      } else {
         return retval;
      }

      switch (tag) {
         case TranslatorTag::End:
            done = true;
            break;

         case TranslatorTag::Translation: {
            quint32 len = read32(data);

            if (len == 0xffffffff) {
               // indicates QByteArray was null
               len = 0;
            }

            data += 4;

            if (numerus == 0) {
               tn_length = len;
               tn = data;
            }

            --numerus;

            data += len;
            break;
         }

         case TranslatorTag::Obsolete1:
            data += 4;
            break;

         case TranslatorTag::SourceText: {
            quint32 len = read32(data);
            data += 4;

            if (len == 0xffffffff) {
               // indicates QByteArray was null
               len = 0;
            }

            if (! match(data, len, text, sourceTextLen)) {
               return retval;
            }

            data += len;
            break;
         }

         case TranslatorTag::Context: {
            quint32 len = read32(data);
            data += 4;

            if (len == 0xffffffff) {
               // indicates QByteArray was null
               len = 0;
            }

            if (! match(data, len, context, contextLen)) {
               return retval;
            }

            data += len;
            break;
         }

         case TranslatorTag::Comment: {
            quint32 len = read32(data);
            data += 4;

            if (len == 0xffffffff) {
               // indicates QByteArray was null
               len = 0;
            }

            if (*data != 0 && ! match(data, len, comment, commentLen)) {
               return retval;
            }

            data += len;
            break;
         }

         default:
            // "unknown tag"
            return retval;
      }
   }

   if (! tn) {
      return retval;
   }

   QString str = QString::fromUtf8((const char *)tn, tn_length);

   return str;
}

QString QTranslatorPrivate::do_translate(const char *context, const char *text, const char *comment,
      std::optional<int> numArg) const
{
   QString retval;

   if (context == nullptr) {
      context = "";
   }

   if (text == nullptr) {
      text = "";
   }

   if (comment == nullptr) {
      comment = "";
   }

   uint numerus    = 0;
   size_t numItems = 0;

   if (! offsetLength) {
      goto searchDependencies;
   }

   // Check if the context belongs to this QTranslator. If many
   // translators are installed, this step is necessary.

   if (contextLength != 0) {
      quint16 hTableSize = read16(contextArray);
      uint g = elfHash(context) % hTableSize;

      const uchar *c = contextArray + 2 + (g << 1);
      quint16 off = read16(c);
      c += 2;

      if (off == 0) {
         return retval;
      }

      c = contextArray + (2 + (hTableSize << 1) + (off << 1));

      const uint contextLen = uint(strlen(context));

      for (;;) {
         quint8 len = read8(c++);

         if (len == 0) {
            return retval;
         }

         if (match(c, len, context, contextLen)) {
            break;
         }

         c += len;
      }
   }

   numItems = offsetLength / (2 * sizeof(quint32));

   if (numItems == 0) {
      goto searchDependencies;
   }

   if (numArg.has_value()) {
      numerus = countHelper(numArg.value(), m_countRules);
   }

   while (true) {
      quint32 h = 0;

      elfHash_start(text, h);
      elfHash_start(comment, h);
      elfHash_finish(h);

      const uchar *start = offsetArray;
      const uchar *end   = start + ((numItems - 1) << 3);

      while (start <= end) {
         const uchar *middle = start + (((end - start) >> 4) << 3);
         uint hash = read32(middle);

         if (h == hash) {
            start = middle;
            break;

         } else if (hash < h) {
            start = middle + 8;

         } else {
            end = middle - 8;
         }
      }

      if (start <= end) {
         // go back on equal key
         while (start != offsetArray && read32(start) == read32(start - 8)) {
            start -= 8;
         }

         while (start < offsetArray + offsetLength) {
            quint32 rh = read32(start);
            start += 4;

            if (rh != h) {
               break;
            }

            quint32 ro = read32(start);
            start += 4;

            QString tn = getMessage(messageArray + ro, messageArray + messageLength, context, text, comment, numerus);

            if (! tn.isEmpty()) {
               return tn;
            }
         }
      }

      if (comment[0] == '\0') {
         break;
      }

      comment = "";
   }

searchDependencies:

   for (QTranslator *translator : subTranslators) {
      QString tn = translator->translate(context, text, comment, numArg);

      if (! tn.isEmpty()) {
         retval = tn;
         break;
      }
   }

   return retval;
}

void QTranslatorPrivate::clear()
{
   Q_Q(QTranslator);

   if (unmapPointer != nullptr && unmapLength != 0) {

#if defined(QT_USE_MMAP)

      if (used_mmap) {
         used_mmap = false;
         munmap(unmapPointer, unmapLength);
      } else
#endif
         if (resource == nullptr) {
            delete [] unmapPointer;
         }
   }

   delete resource;

   resource           = nullptr;
   unmapPointer       = nullptr;
   messageArray       = nullptr;
   contextArray       = nullptr;
   offsetArray        = nullptr;

   m_countRules.clear();

   unmapLength        = 0;
   messageLength      = 0;
   contextLength      = 0;
   offsetLength       = 0;

   qDeleteAll(subTranslators);

   subTranslators.clear();

   if (QCoreApplicationPrivate::isTranslatorInstalled(q)) {
      QCoreApplication::postEvent(QCoreApplication::instance(), new QEvent(QEvent::LanguageChange));
   }
}

QString QTranslator::replacePercentN(QString text, int numArg)
{
   // %Ln or %n

   int percentPos = 0;
   int len = 0;

   while ((percentPos = text.indexOf('%', percentPos + len)) != -1) {
      len = 1;
      QString fmt;

      if (text.at(percentPos + len) == 'L') {
         fmt = QString("%L1");
         ++len;

      } else {
         fmt = QString("%1");

      }

      if (text.at(percentPos + len) == 'n') {
         fmt = fmt.formatArg(numArg);
         ++len;

         text.replace(percentPos, len, fmt);
         len = fmt.length();
      }
   }

   return text;
}

QString QTranslator::translate(const char *context, const char *text, const char *comment,
      std::optional<int> numArg) const
{
   Q_D(const QTranslator);

   QString retval = d->do_translate(context, text, comment, numArg);

   if (! retval.isEmpty() && numArg.has_value()) {
      retval = replacePercentN(retval, numArg.value());
   }

   return retval;
}

QString QTranslator::translate(const QString &context, const QString &text, const QString &comment,
      std::optional<int> numArg) const
{
   return translate(context.constData(), text.constData(), comment.constData(), numArg);
}

bool QTranslator::isEmpty() const
{
   Q_D(const QTranslator);

   return ! d->unmapPointer && ! d->unmapLength && ! d->messageArray &&
         ! d->offsetArray && ! d->contextArray && d->subTranslators.isEmpty();
}
