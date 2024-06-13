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

#include <similartext.h>

#include <qdebug.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

#include <qlocale_p.h>
#include <qtranslator_p.h>

#ifdef Q_OS_WIN
#include <io.h>       // for _setmode
#include <fcntl.h>    // for _O_BINARY
#endif

#include <iostream>
#include <stdio.h>

Translator::Translator()
   : m_locationsType(AbsoluteLocations), m_indexOk(true)
{
}

void Translator::registerFileFormat(const FileFormat &format)
{
   QList<Translator::FileFormat> &formats = registeredFileFormats();

   for (int i = 0; i < formats.size(); ++i) {
      if (format.fileType == formats[i].fileType && format.priority < formats[i].priority) {
         formats.insert(i, format);
         return;
      }
   }

   formats.append(format);
}

QList<Translator::FileFormat> &Translator::registeredFileFormats()
{
   static QList<Translator::FileFormat> theFormats;
   return theFormats;
}

void Translator::addIndex(int idx, const TranslatorMessage &msg) const
{
   if (msg.sourceText().isEmpty() && msg.id().isEmpty()) {
      m_ctxCmtIdx[msg.context()] = idx;

   } else {
      m_msgIdx[TMMKey(msg)] = idx;
      if (!msg.id().isEmpty()) {
         m_idMsgIdx[msg.id()] = idx;
      }
   }
}

void Translator::delIndex(int idx) const
{
   const TranslatorMessage &msg = m_messages.at(idx);

   if (msg.sourceText().isEmpty() && msg.id().isEmpty()) {
      m_ctxCmtIdx.remove(msg.context());
   } else {
      m_msgIdx.remove(TMMKey(msg));

      if (! msg.id().isEmpty()) {
         m_idMsgIdx.remove(msg.id());
      }
   }
}

void Translator::ensureIndexed() const
{
   if (!m_indexOk) {
      m_indexOk = true;
      m_ctxCmtIdx.clear();
      m_idMsgIdx.clear();
      m_msgIdx.clear();

      for (int i = 0; i < m_messages.count(); i++) {
         addIndex(i, m_messages.at(i));
      }
   }
}

void Translator::replaceSorted(const TranslatorMessage &msg)
{
   int index = find(msg);

   if (index == -1) {
      appendSorted(msg);

   } else {
      delIndex(index);
      m_messages[index] = msg;
      addIndex(index, msg);
   }
}

static QString elidedId(const QString &id, int len)
{
   return id.length() <= len ? id : id.left(len - 5) + "[...]";
}

static QString makeMsgId(const TranslatorMessage &msg)
{
   QString id = msg.context() + "//" + elidedId(msg.sourceText(), 100);

   if (! msg.comment().isEmpty()) {
      id += "//" + elidedId(msg.comment(), 30);
   }

   return id;
}

void Translator::extend(const TranslatorMessage &msg, ConversionData &cd)
{
   int index = find(msg);

   if (index == -1) {
      append(msg);

   } else {
      TranslatorMessage &emsg = m_messages[index];
      if (emsg.sourceText().isEmpty()) {
         delIndex(index);
         emsg.setSourceText(msg.sourceText());
         addIndex(index, msg);

      } else if (! msg.sourceText().isEmpty() && emsg.sourceText() != msg.sourceText()) {
         cd.appendError(QString("Contradicting source text for message with id '%1'.").formatArg(emsg.id()));
         return;
      }

      if (emsg.extras().isEmpty()) {
         emsg.setExtras(msg.extras());

      } else if (!msg.extras().isEmpty() && emsg.extras() != msg.extras()) {

         cd.appendError(QString("Contradicting meta data for %1.")
                        .formatArg(! emsg.id().isEmpty()
                                   ? QString("message with id '%1'").formatArg(emsg.id())
                                   : QString("message '%1'").formatArg(makeMsgId(msg))));
         return;
      }
      emsg.addReferenceUniq(msg.fileName(), msg.lineNumber());

      if (! msg.extraComment().isEmpty()) {
         QString cmt = emsg.extraComment();

         if (! cmt.isEmpty()) {
            QStringList cmts = cmt.split("\n----------\n");

            if (! cmts.contains(msg.extraComment())) {
               cmts.append(msg.extraComment());
               cmt = cmts.join("\n----------\n");
            }

         } else {
            cmt = msg.extraComment();
         }

         emsg.setExtraComment(cmt);
      }

   }
}

void Translator::insert(int idx, const TranslatorMessage &msg)
{
   if (m_indexOk) {
      if (idx == m_messages.count()) {
         addIndex(idx, msg);
      } else {
         m_indexOk = false;
      }
   }

   m_messages.insert(idx, msg);
}

void Translator::append(const TranslatorMessage &msg)
{
   insert(m_messages.count(), msg);
}

void Translator::appendSorted(const TranslatorMessage &msg)
{
   int msgLine = msg.lineNumber();
   if (msgLine < 0) {
      append(msg);
      return;
   }

   int bestIdx = 0; // Best insertion point found so far
   int bestScore = 0; // Its category: 0 = no hit, 1 = pre or post, 2 = middle
   int bestSize = 0; // The length of the region. Longer is better within one category.

   // The insertion point to use should this region turn out to be the best one so far
   int thisIdx = 0;
   int thisScore = 0;
   int thisSize = 0;

   // Working vars
   int prevLine = 0;
   int curIdx = 0;

   for (const TranslatorMessage &mit : m_messages) {
      bool sameFile = mit.fileName() == msg.fileName() && mit.context() == msg.context();
      int curLine;

      if (sameFile && (curLine = mit.lineNumber()) >= prevLine) {
         if (msgLine >= prevLine && msgLine < curLine) {
            thisIdx = curIdx;
            thisScore = thisSize ? 2 : 1;
         }

         ++thisSize;
         prevLine = curLine;

      } else {
         if (thisSize) {
            if (!thisScore) {
               thisIdx = curIdx;
               thisScore = 1;
            }
            if (thisScore > bestScore || (thisScore == bestScore && thisSize > bestSize)) {
               bestIdx = thisIdx;
               bestScore = thisScore;
               bestSize = thisSize;
            }
            thisScore = 0;
            thisSize = sameFile ? 1 : 0;
            prevLine = 0;
         }
      }
      ++curIdx;
   }

   if (thisSize && !thisScore) {
      thisIdx = curIdx;
      thisScore = 1;
   }

   if (thisScore > bestScore || (thisScore == bestScore && thisSize > bestSize)) {
      insert(thisIdx, msg);
   } else if (bestScore) {
      insert(bestIdx, msg);
   } else {
      append(msg);
   }
}

static QString guessFormat(const QString &filename, const QString &format)
{
   if (format != "auto") {
      return format;
   }

   for (const Translator::FileFormat &fmt : Translator::registeredFileFormats()) {
      if (filename.endsWith('.' + fmt.extension, Qt::CaseInsensitive)) {
         return fmt.extension;
      }
   }

   return QString("ts");
}

bool Translator::load(const QString &filename, ConversionData &cd, const QString &format)
{
   cd.m_sourceDir = QFileInfo(filename).absoluteDir();
   cd.m_sourceFileName = filename;

   QFile file;

   if (filename.isEmpty() || filename == "-") {

#ifdef Q_OS_WIN
      ::_setmode(0, _O_BINARY);
#endif

      if (! file.open(stdin, QIODevice::ReadOnly)) {
         cd.appendError(QString("Unable to open stdin (%1)").formatArg(file.errorString()));
         return false;
      }

   } else {
      file.setFileName(filename);

      if (! file.open(QIODevice::ReadOnly)) {
         cd.appendError(QString("Unable to open %1: %2").formatArgs(filename, file.errorString()));
         return false;
      }
   }

   QString fmt = guessFormat(filename, format);

   for (const FileFormat &format : registeredFileFormats()) {
      if (fmt == format.extension) {
         if (format.loader) {
            return (*format.loader)(*this, file, cd);
         }

         cd.appendError(QString("No loader for format %1 found").formatArg(fmt));

         return false;
      }
   }

   cd.appendError(QString("Unknown format %1 for file %2").formatArgs(format, filename));

   return false;
}

bool Translator::save(const QString &filename, ConversionData &cd, const QString &format) const
{
   QFile file;

   if (filename.isEmpty() || filename == "-") {

#ifdef Q_OS_WIN
      ::_setmode(1, _O_BINARY);
#endif

      if (! file.open(stdout, QIODevice::WriteOnly)) {
         cd.appendError(QString("Unable to open stdout (%1)").formatArg(file.errorString()));
         return false;
      }

   } else {
      file.setFileName(filename);
      if (! file.open(QIODevice::WriteOnly)) {
         cd.appendError(QString("Unable to create %1: %2").formatArgs(filename, file.errorString()));
         return false;
      }
   }

   QString fmt = guessFormat(filename, format);
   cd.m_targetDir = QFileInfo(filename).absoluteDir();

   for (const FileFormat &format : registeredFileFormats()) {
      if (fmt == format.extension) {
         if (format.saver) {
            return (*format.saver)(*this, file, cd);
         }

         cd.appendError(QString("Unable to save %1 files").formatArg(fmt));

         return false;
      }
   }

   cd.appendError(QString("Unknown format %1 for file %2").formatArg(format).formatArg(filename));

   return false;
}

QString Translator::makeLanguageCode(QLocale::Language language, QLocale::Country country)
{
   QString result = QLocalePrivate::languageCode(language);

   if (language != QLocale::C && country != QLocale::AnyCountry) {
      result.append('_');
      result.append(QLocalePrivate::countryCode(country));
   }

   return result;
}

void Translator::languageAndCountry(const QString &languageCode, QLocale::Language *lang, QLocale::Country *country)
{
   QLocale::Script script;
   QLocalePrivate::getLangAndCountry(languageCode, *lang, script, *country);
}

int Translator::find(const TranslatorMessage &msg) const
{
   ensureIndexed();
   if (msg.id().isEmpty()) {
      return m_msgIdx.value(TMMKey(msg), -1);
   }

   int i = m_idMsgIdx.value(msg.id(), -1);

   if (i >= 0) {
      return i;
   }

   i = m_msgIdx.value(TMMKey(msg), -1);

   // If both have an id, then find only by id.
   return i >= 0 && m_messages.at(i).id().isEmpty() ? i : -1;
}

int Translator::find(const QString &context, const QString &comment, const QList<TranslatorMessage::Reference> &data) const
{
   if (! data.isEmpty()) {
      for (auto iter = m_messages.constBegin(); iter != m_messages.constEnd(); ++iter) {

         if (iter->context() == context && iter->comment() == comment) {
            for (const auto &item_refA : iter->allReferences()) {

               for (const auto &item_refB : data) {
                  if (item_refA == item_refB) {
                     return iter - m_messages.constBegin();
                  }
               }

            }
         }
      }
   }

   return -1;
}

int Translator::find(const QString &context) const
{
   ensureIndexed();
   return m_ctxCmtIdx.value(context, -1);
}

void Translator::stripObsoleteMessages()
{
   for (auto it = m_messages.begin(); it != m_messages.end(); ) {
      if (it->type() == TranslatorMessage::Type::Obsolete || it->type() == TranslatorMessage::Type::Vanished) {
         it = m_messages.erase(it);

      } else {
         ++it;
      }
   }

   m_indexOk = false;
}

void Translator::stripFinishedMessages()
{
   for (auto it = m_messages.begin(); it != m_messages.end(); ) {
      if (it->type() == TranslatorMessage::Type::Finished) {
         it = m_messages.erase(it);
      } else {
         ++it;
      }
   }

   m_indexOk = false;
}

void Translator::stripEmptyContexts()
{
   m_indexOk = false;
}

void Translator::stripNonPluralForms()
{
   for (auto it = m_messages.begin(); it != m_messages.end(); ) {
      if (! it->isPlural()) {
         it = m_messages.erase(it);
      } else {
         ++it;
      }
   }

   m_indexOk = false;
}

void Translator::stripIdenticalSourceTranslations()
{
   for (auto it = m_messages.begin(); it != m_messages.end(); ) {
      // we need to have just one translation, and it be equal to the source
      if (it->translations().count() == 1 && it->translation() == it->sourceText()) {
         it = m_messages.erase(it);
      } else {
         ++it;
      }
   }

   m_indexOk = false;
}

void Translator::dropTranslations()
{
   for (auto it = m_messages.begin(); it != m_messages.end(); ++it) {
      if (it->type() == TranslatorMessage::Type::Finished) {
         it->setType(TranslatorMessage::Type::Unfinished);
      }
      it->setTranslation(QString());
   }
}

void Translator::dropUiLines()
{
   QString uiXt  = QString(".ui");
   QString juiXt = QString(".jui");

   for (auto it = m_messages.begin(); it != m_messages.end(); ++it) {

      QHash<QString, int> have;
      QList<TranslatorMessage::Reference> refs;

      for (const TranslatorMessage::Reference &itref : it->allReferences()) {
         const QString &fn = itref.fileName();

         if (fn.endsWith(uiXt) || fn.endsWith(juiXt)) {
            if (++have[fn] == 1) {
               refs.append(TranslatorMessage::Reference(fn, -1));
            }

         } else {
            refs.append(itref);
         }
      }

      it->setReferences(refs);
   }
}

struct TranslatorMessageIdPtr {
   explicit TranslatorMessageIdPtr(const TranslatorMessage &tm) {
      ptr = &tm;
   }

   inline const TranslatorMessage *operator->() const {
      return ptr;
   }

   const TranslatorMessage *ptr;
};

inline int qHash(TranslatorMessageIdPtr tmp)
{
   return qHash(tmp->id());
}

inline bool operator==(TranslatorMessageIdPtr tmp1, TranslatorMessageIdPtr tmp2)
{
   return tmp1->id() == tmp2->id();
}

struct TranslatorMessageContentPtr {
   explicit TranslatorMessageContentPtr(const TranslatorMessage &tm) {
      ptr = &tm;
   }

   inline const TranslatorMessage *operator->() const {
      return ptr;
   }

   const TranslatorMessage *ptr;
};

inline int qHash(TranslatorMessageContentPtr tmp)
{
   int hash = qHash(tmp->context()) ^ qHash(tmp->sourceText());

   // Special treatment for context comments (empty source)
   if (! tmp->sourceText().isEmpty()) {
      hash ^= qHash(tmp->comment());
   }

   return hash;
}

inline bool operator==(TranslatorMessageContentPtr tmp1, TranslatorMessageContentPtr tmp2)
{
   if (tmp1->context() != tmp2->context() || tmp1->sourceText() != tmp2->sourceText()) {
      return false;
   }

   // Special treatment for context comments (empty source)
   if (tmp1->sourceText().isEmpty()) {
      return true;
   }

   return tmp1->comment() == tmp2->comment();
}

Translator::Duplicates Translator::resolveDuplicates()
{
   Duplicates dups;
   QHash<TranslatorMessageIdPtr, int> idRefs;
   QHash<TranslatorMessageContentPtr, int> contentRefs;

   for (int i = 0; i < m_messages.count();) {
      const TranslatorMessage &msg = m_messages.at(i);
      TranslatorMessage *omsg;
      int oi;
      QSet<int> *pDup;

      if (! msg.id().isEmpty()) {
         QHash<TranslatorMessageIdPtr, int>::const_iterator it = idRefs.constFind(TranslatorMessageIdPtr(msg));

         if (it != idRefs.constEnd()) {
            oi   = *it;
            omsg = &m_messages[oi];
            pDup = &dups.byId;
            goto gotDupe;
         }
      }

      {
         QHash<TranslatorMessageContentPtr, int>::const_iterator it = contentRefs.constFind(TranslatorMessageContentPtr(msg));

         if (it != contentRefs.constEnd()) {
            oi = *it;
            omsg = &m_messages[oi];
            if (msg.id().isEmpty() || omsg->id().isEmpty()) {
               if (!msg.id().isEmpty() && omsg->id().isEmpty()) {
                  omsg->setId(msg.id());
                  idRefs[TranslatorMessageIdPtr(*omsg)] = oi;
               }
               pDup = &dups.byContents;
               goto gotDupe;
            }
            // This is really a content dupe, but with two distinct IDs.
         }
      }

      if (! msg.id().isEmpty()) {
         idRefs[TranslatorMessageIdPtr(msg)] = i;
      }

      contentRefs[TranslatorMessageContentPtr(msg)] = i;
      ++i;
      continue;

   gotDupe:
      pDup->insert(oi);

      if (! omsg->isTranslated() && msg.isTranslated()) {
         omsg->setTranslations(msg.translations());
      }

      m_indexOk = false;
      m_messages.removeAt(i);
   }

   return dups;
}

void Translator::reportDuplicates(const Duplicates &dupes, const QString &fileName, bool verbose)
{
   if (!dupes.byId.isEmpty() || !dupes.byContents.isEmpty()) {
      std::cerr << "Warning: dropping duplicate messages in '" << csPrintable(fileName);

      if (! verbose) {
         std::cerr << "'\n(try -verbose for more info).\n";

      } else {
         std::cerr << "':\n";

         for (int i : dupes.byId) {
            std::cerr << "\n* ID: " << csPrintable(message(i).id()) << std::endl;
         }

         for (int j : dupes.byContents) {
            const TranslatorMessage &msg = message(j);
            std::cerr << "\n* Context: " << csPrintable(msg.context())
                      << "\n* Source: "  << csPrintable(msg.sourceText()) << std::endl;

            if (! msg.comment().isEmpty()) {
               std::cerr << "* Comment: " << csPrintable(msg.comment()) << std::endl;
            }
         }

         std::cerr << std::endl;
      }
   }
}

// Used by lupdate to be able to search using absolute paths during merging
void Translator::makeFileNamesAbsolute(const QDir &originalPath)
{
   for (auto it = m_messages.begin(); it != m_messages.end(); ++it) {
      TranslatorMessage &msg = *it;

      QList<TranslatorMessage::Reference> refs = msg.allReferences();
      msg.setReferences(QList<TranslatorMessage::Reference>());

      for (const auto &ref : refs) {
         QString fileName = ref.fileName();
         QFileInfo fi (fileName);

         if (fi.isRelative()) {
            fileName = originalPath.absoluteFilePath(fileName);
         }

         msg.addReference(fileName, ref.lineNumber());
      }
   }
}

QList<TranslatorMessage> Translator::messages() const
{
   return m_messages;
}

QStringList Translator::normalizedTranslations(const TranslatorMessage &msg, int numPlurals)
{
   QStringList translations = msg.translations();
   int numTranslations = msg.isPlural() ? numPlurals : 1;

   // make sure that the stringlist always has the size of the
   // language's current numerus, or 1 if its not plural

   if (translations.count() > numTranslations) {
      for (int i = translations.count(); i > numTranslations; --i) {
         translations.removeLast();
      }

   } else if (translations.count() < numTranslations) {
      for (int i = translations.count(); i < numTranslations; ++i) {
         translations.append(QString());
      }
   }
   return translations;
}

void Translator::normalizeTranslations(ConversionData &cd)
{
   bool truncated = false;

   QLocale::Language l;
   QLocale::Country c;
   languageAndCountry(languageCode(), &l, &c);

   int numPlurals = 1;

   if (l != QLocale::C) {
      QStringList forms;

      if (getCountInfo(l, c, nullptr, &forms, nullptr)) {
         numPlurals = forms.count();   // includes singular
      }
   }

   for (int i = 0; i < m_messages.count(); ++i) {
      const TranslatorMessage &msg = m_messages.at(i);
      QStringList tlns = msg.translations();
      int ccnt = msg.isPlural() ? numPlurals : 1;

      if (tlns.count() != ccnt) {
         while (tlns.count() < ccnt) {
            tlns.append(QString());
         }

         while (tlns.count() > ccnt) {
            tlns.removeLast();
            truncated = true;
         }
         m_messages[i].setTranslations(tlns);
      }
   }

   if (truncated)
      cd.appendError("Removed plural forms as the target language has less forms.\n"
                     "If this sounds wrong, possibly the target language is not set or recognized.");
}

QString Translator::guessLanguageCodeFromFileName(const QString &filename)
{
   QString str = filename;

   for (const FileFormat &format : registeredFileFormats()) {
      if (str.endsWith(format.extension)) {
         str = str.left(str.size() - format.extension.size() - 1);
         break;
      }
   }

   static QRegularExpression regExp("[\\._]");

   while (true) {
      QLocale locale(str);

      if (locale.language() != QLocale::C) {
         return locale.name();
      }

      auto pos = str.indexOfFast(regExp);

      if (pos == str.cend()) {
         break;
      }

      str = QString(pos + 1, str.cend());
   }

   return QString();
}

bool Translator::hasExtra(const QString &key) const
{
   return m_extra.contains(key);
}

QString Translator::extra(const QString &key) const
{
   return m_extra[key];
}

void Translator::setExtra(const QString &key, const QString &value)
{
   m_extra[key] = value;
}

void Translator::dump() const
{
   for (int i = 0; i != messageCount(); ++i) {
      message(i).dump();
   }
}

