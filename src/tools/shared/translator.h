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

#ifndef METATRANSLATOR_H
#define METATRANSLATOR_H

#include "translatormessage.h"

#include <QDir>
#include <QList>
#include <QLocale>
#include <QMultiHash>
#include <QString>
#include <QSet>

QT_BEGIN_NAMESPACE

class QIODevice;

// A struct of "interesting" data passed to and from the load and save routines
class ConversionData
{
 public:
   ConversionData() :
      m_verbose(false),
      m_ignoreUnfinished(false),
      m_sortContexts(false),
      m_noUiLines(false),
      m_idBased(false),
      m_saveMode(SaveEverything) {
   }

   // tag manipulation
   const QStringList &dropTags() const {
      return m_dropTags;
   }
   QStringList &dropTags() {
      return m_dropTags;
   }
   const QDir &targetDir() const {
      return m_targetDir;
   }
   bool isVerbose() const {
      return m_verbose;
   }
   bool ignoreUnfinished() const {
      return m_ignoreUnfinished;
   }
   bool sortContexts() const {
      return m_sortContexts;
   }

   void appendError(const QString &error) {
      m_errors.append(error);
   }
   QString error() const {
      return m_errors.isEmpty() ? QString() : m_errors.join(QLatin1String("\n")) + QLatin1Char('\n');
   }
   QStringList errors() const {
      return  m_errors;
   }
   void clearErrors() {
      m_errors.clear();
   }

 public:
   QString m_defaultContext;
   QByteArray m_codecForSource; // CPP, PO & QM specific
   QByteArray m_outputCodec; // CPP & PO specific
   QString m_unTrPrefix; // QM specific
   QString m_sourceFileName;
   QString m_targetFileName;
   QDir m_sourceDir;
   QDir m_targetDir; // FIXME: TS specific
   QSet<QString> m_projectRoots;
   QMultiHash<QString, QString> m_allCSources;
   QStringList m_includePath;
   QStringList m_dropTags;  // tags to be dropped
   QStringList m_errors;
   bool m_verbose;
   bool m_ignoreUnfinished;
   bool m_sortContexts;
   bool m_noUiLines;
   bool m_idBased;
   TranslatorSaveMode m_saveMode;
};

class TMMKey
{
 public:
   TMMKey(const TranslatorMessage &msg) {
      context = msg.context();
      source = msg.sourceText();
      comment = msg.comment();
   }
   bool operator==(const TMMKey &o) const {
      return context == o.context && source == o.source && comment == o.comment;
   }
   QString context, source, comment;
};
Q_DECLARE_TYPEINFO(TMMKey, Q_MOVABLE_TYPE);
inline uint qHash(const TMMKey &key)
{
   return qHash(key.context) ^ qHash(key.source) ^ qHash(key.comment);
}

class Translator
{
 public:
   Translator();

   bool load(const QString &filename, ConversionData &err, const QString &format /* = "auto" */);
   bool save(const QString &filename, ConversionData &err, const QString &format /* = "auto" */) const;

   int find(const TranslatorMessage &msg) const;
   int find(const QString &context,
            const QString &comment, const TranslatorMessage::References &refs) const;

   int find(const QString &context) const;

   void replaceSorted(const TranslatorMessage &msg);
   void extend(const TranslatorMessage &msg); // Only for single-location messages
   void append(const TranslatorMessage &msg);
   void appendSorted(const TranslatorMessage &msg);

   void stripObsoleteMessages();
   void stripFinishedMessages();
   void stripEmptyContexts();
   void stripNonPluralForms();
   void stripIdenticalSourceTranslations();
   void dropTranslations();
   void dropUiLines();
   void makeFileNamesAbsolute(const QDir &originalPath);

   struct Duplicates {
      QSet<int> byId, byContents;
   };
   Duplicates resolveDuplicates();
   void reportDuplicates(const Duplicates &dupes, const QString &fileName, bool verbose);

   void setCodecName(const QByteArray &name);
   void setCodec(QTextCodec *codec) {
      m_codec = codec;
   }
   QByteArray codecName() const;
   QTextCodec *codec() const {
      return m_codec;
   }

   QString languageCode() const {
      return m_language;
   }
   QString sourceLanguageCode() const {
      return m_sourceLanguage;
   }

   enum LocationsType { DefaultLocations, NoLocations, RelativeLocations, AbsoluteLocations };
   void setLocationsType(LocationsType lt) {
      m_locationsType = lt;
   }
   LocationsType locationsType() const {
      return m_locationsType;
   }

   static QString makeLanguageCode(QLocale::Language language, QLocale::Country country);
   static void languageAndCountry(const QString &languageCode,
                                  QLocale::Language *lang, QLocale::Country *country);
   void setLanguageCode(const QString &languageCode) {
      m_language = languageCode;
   }
   void setSourceLanguageCode(const QString &languageCode) {
      m_sourceLanguage = languageCode;
   }
   static QString guessLanguageCodeFromFileName(const QString &fileName);
   QList<TranslatorMessage> messages() const;
   static QStringList normalizedTranslations(const TranslatorMessage &m, int numPlurals);
   void normalizeTranslations(ConversionData &cd);
   QStringList normalizedTranslations(const TranslatorMessage &m, ConversionData &cd, bool *ok) const;

   int messageCount() const {
      return m_messages.size();
   }
   TranslatorMessage &message(int i) {
      return m_messages[i];
   }
   const TranslatorMessage &message(int i) const {
      return m_messages.at(i);
   }
   const TranslatorMessage &constMessage(int i) const {
      return m_messages.at(i);
   }
   void dump() const;

   // additional file format specific data
   // note: use '<fileformat>:' as prefix for file format specific members,
   // e.g. "po-flags", "po-msgid_plural"
   typedef TranslatorMessage::ExtraData ExtraData;
   QString extra(const QString &ba) const;
   void setExtra(const QString &ba, const QString &var);
   bool hasExtra(const QString &ba) const;
   const ExtraData &extras() const {
      return m_extra;
   }
   void setExtras(const ExtraData &extras) {
      m_extra = extras;
   }

   // registration of file formats
   typedef bool (*SaveFunction)(const Translator &, QIODevice &out, ConversionData &data);
   typedef bool (*LoadFunction)(Translator &, QIODevice &in, ConversionData &data);
   struct FileFormat {
      FileFormat() : loader(0), saver(0), priority(-1) {}
      QString extension; // such as "ts", "xlf", ...
      QString description; // human-readable description
      LoadFunction loader;
      SaveFunction saver;
      enum FileType { TranslationSource, TranslationBinary } fileType;
      int priority; // 0 = highest, -1 = invisible
   };
   static void registerFileFormat(const FileFormat &format);
   static QList<FileFormat> &registeredFileFormats();

   enum {
      TextVariantSeparator = 0x2762, // some weird character nobody ever heard of :-D
      BinaryVariantSeparator = 0x9c // unicode "STRING TERMINATOR"
   };

 private:
   void insert(int idx, const TranslatorMessage &msg);
   void addIndex(int idx, const TranslatorMessage &msg) const;
   void delIndex(int idx) const;
   void ensureIndexed() const;

   typedef QList<TranslatorMessage> TMM;       // int stores the sequence position.

   TMM m_messages;
   QTextCodec *m_codec;
   LocationsType m_locationsType;

   // A string beginning with a 2 or 3 letter language code (ISO 639-1
   // or ISO-639-2), followed by the optional country variant to distinguish
   //  between country-specific variations of the language. The language code
   // and country code are always separated by '_'
   // Note that the language part can also be a 3-letter ISO 639-2 code.
   // Legal examples:
   // 'pt'         portuguese, assumes portuguese from portugal
   // 'pt_BR'      Brazilian portuguese (ISO 639-1 language code)
   // 'por_BR'     Brazilian portuguese (ISO 639-2 language code)
   QString m_language;
   QString m_sourceLanguage;
   ExtraData m_extra;

   mutable bool m_indexOk;
   mutable QHash<QString, int> m_ctxCmtIdx;
   mutable QHash<QString, int> m_idMsgIdx;
   mutable QHash<TMMKey, int> m_msgIdx;
};

bool getNumerusInfo(QLocale::Language language, QLocale::Country country,
                    QByteArray *rules, QStringList *forms, const char **gettextRules);

QString getNumerusInfoString();

bool saveQM(const Translator &translator, QIODevice &dev, ConversionData &cd);

/*
  This is a quick hack. The proper way to handle this would be
  to extend Translator's interface.
*/
#define ContextComment "QT_LINGUIST_INTERNAL_CONTEXT_COMMENT"

QT_END_NAMESPACE

#endif
