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

#include <lupdate.h>
#include <translator.h>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlibraryinfo.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtranslator.h>

#include <iostream>

static QString m_defaultExtensions;

static void printOut(const QString &out)
{
   QTextStream stream(stdout);
   stream << out;
}

static void printErr(const QString &out)
{
   QTextStream stream(stderr);
   stream << out;
}

static void recursiveFileInfoList(const QDir &dir, const QSet<QString> &nameFilters, QDir::Filters filter,
            QFileInfoList *fileinfolist)
{
   for (const QFileInfo &fi : dir.entryInfoList(filter)) {
      if (fi.isDir()) {
         recursiveFileInfoList(QDir(fi.absoluteFilePath()), nameFilters, filter, fileinfolist);

      } else if (nameFilters.contains(fi.suffix())) {
         fileinfolist->append(fi);
      }
   }
}

static void printUsage()
{
   printOut(QString(
               "Usage:\n"
               "    lupdate [options] [project-file]...\n"
               "    lupdate [options] [source-file|path|@lst-file]... -ts ts-files|@lst-file\n\n"
               "lupdate is part of the Linguist tool chain. This program extracts translatable\n"
               "text from source, headers, and UI files.\n"
               "Extracted strings are stored in a human readable TS file. \n"
               "New and modified text can be merged into an existing TS files.\n\n"
               "Options:\n"
               "    -help\n"
               "           Display this information and exit.\n"
               "    -no-obsolete\n"
               "           Drop all obsolete and vanished strings.\n"
               "    -extensions <ext>[,<ext>]...\n"
               "           Process files with the given extensions only.\n"
               "           The extension list must be separated with commas, not with whitespace.\n"
               "           Default: '%1'.\n"
               "    -pluralonly\n"
               "           Only include plural form messages.\n"
               "    -silent\n"
               "           Do not explain what is being done.\n"
               "    -no-sort\n"
               "           Do not sort contexts in TS files.\n"
               "    -no-recursive\n"
               "           Do not recursively scan the following directories.\n"
               "    -recursive\n"
               "           Recursively scan the following directories (default).\n"
               "    -I <includepath> or -I<includepath>\n"
               "           Additional location to look for include files.\n"
               "           May be specified multiple times.\n"
               "    -locations {absolute|relative|none}\n"
               "           Specify/override how source code references are saved in TS files.\n"
               "           Guessed from existing TS files if not specified.\n"
               "           Default is absolute for new files.\n"
               "    -no-ui-lines\n"
               "           Do not record line numbers in references to UI files.\n"
               "    -disable-heuristic {sametext|similartext|number}\n"
               "           Disable the named merge heuristic. Can be specified multiple times.\n"
               "    -pro <filename>\n"
               "           Name of a .pro file. Useful for files with .pro file syntax but\n"
               "           different file suffix. Projects are recursed into and merged.\n"
               "    -source-language <language>[_<region>]\n"
               "           Specify the language of the source strings for new files.\n"
               "           Defaults to POSIX if not specified.\n"
               "    -target-language <language>[_<region>]\n"
               "           Specify the language of the translations for new files.\n"
               "           Guessed from the file name if not specified.\n"
               "    -ts <ts-file>...\n"
               "           Specify the output file(s). This will override the TRANSLATIONS\n"
               "    -version\n"
               "           Display the version of lupdate and exit.\n"
               "    @lst-file\n"
               "           Read additional file names (one per line) or includepaths (one per\n"
               "           line, and prefixed with -I) from lst-file.\n"
            ).formatArg(m_defaultExtensions));
}

static void updateTsFiles(const Translator &fetchedTor, const QStringList &tsFileNames,
                          const QStringList &alienFiles, const QString &sourceLanguage, const QString &targetLanguage,
                          UpdateOptions options, bool *fail)
{
   for (int i = 0; i < fetchedTor.messageCount(); i++) {
      const TranslatorMessage &msg = fetchedTor.constMessage(i);

      if (! msg.id().isEmpty() && msg.sourceText().isEmpty()) {
         printErr(QString("lupdate warning: Message with id '%1' has no source.\n").formatArg(msg.id()));
      }
   }

   QList<Translator> aliens;
   for (const QString &fileName : alienFiles) {
      ConversionData cd;
      Translator tor;

      if (! tor.load(fileName, cd, "auto")) {
         printErr(cd.error());
         *fail = true;
         continue;
      }

      tor.resolveDuplicates();
      aliens << tor;
   }
   QDir dir;
   QString err;

   for (const QString &fileName : tsFileNames) {
      QString fn = dir.relativeFilePath(fileName);

      ConversionData cd;
      Translator tor;
      cd.m_sortContexts = ! (options & NoSorting);

      if (QFile(fileName).exists()) {
         if (! tor.load(fileName, cd, "auto")) {
            printErr(cd.error());
            *fail = true;
            continue;
         }

         tor.resolveDuplicates();
         cd.clearErrors();

         if (!targetLanguage.isEmpty() && targetLanguage != tor.languageCode())
            printErr(QString("lupdate warning: Specified target language '%1' disagrees with"
                             " existing file's language '%2'. Ignoring.\n").formatArgs(targetLanguage, tor.languageCode()));

         if (! sourceLanguage.isEmpty() && sourceLanguage != tor.sourceLanguageCode())
            printErr(QString("lupdate warning: Specified source language '%1' disagrees with"
                             " existing file's language '%2'. Ignoring.\n").formatArgs(sourceLanguage, tor.sourceLanguageCode()));

      } else {
         if (! targetLanguage.isEmpty()) {
            tor.setLanguageCode(targetLanguage);

         } else {
            tor.setLanguageCode(Translator::guessLanguageCodeFromFileName(fileName));
         }

         if (!sourceLanguage.isEmpty()) {
            tor.setSourceLanguageCode(sourceLanguage);
         }
      }

      tor.makeFileNamesAbsolute(QFileInfo(fileName).absoluteDir());

      if (options & NoLocations) {
         tor.setLocationsType(Translator::NoLocations);
      } else if (options & RelativeLocations) {
         tor.setLocationsType(Translator::RelativeLocations);
      } else if (options & AbsoluteLocations) {
         tor.setLocationsType(Translator::AbsoluteLocations);
      }

      if (options & Verbose) {
         printOut(QString("Updating '%1'...\n").formatArg(fn));
      }

      UpdateOptions theseOptions = options;
      if (tor.locationsType() == Translator::NoLocations) {
         // Could be set from file
         theseOptions |= NoLocations;
      }

      Translator out = merge(tor, fetchedTor, aliens, theseOptions, err);

      if ((options & Verbose) && !err.isEmpty()) {
         printOut(err);
         err.clear();
      }

      if (options & PluralOnly) {
         if (options & Verbose) {
            printOut(QString("Stripping non plural forms in '%1'...\n").formatArg(fn));
         }
         out.stripNonPluralForms();
      }

      if (options & NoObsolete) {
         out.stripObsoleteMessages();
      }

      out.stripEmptyContexts();

      out.normalizeTranslations(cd);
      if (!cd.errors().isEmpty()) {
         printErr(cd.error());
         cd.clearErrors();
      }

      if (! out.save(fileName, cd, "auto")) {
         printErr(cd.error());
         *fail = true;
      }
   }
}

static bool processTs(Translator &fetchedTor, const QString &file, ConversionData &cd)
{
   for (const Translator::FileFormat &fmt : Translator::registeredFileFormats()) {
      if (file.endsWith('.' + fmt.extension, Qt::CaseInsensitive)) {
         Translator tor;

         if (tor.load(file, cd, fmt.extension)) {
            for (TranslatorMessage msg : tor.messages()) {
               msg.setType(TranslatorMessage::Type::Unfinished);
               msg.setTranslations(QStringList());
               msg.setTranslatorComment(QString());
               fetchedTor.extend(msg, cd);
            }
         }

         return true;
      }
   }

   return false;
}

static void processSources(Translator &fetchedTor, const QStringList &sourceFiles, ConversionData &cd)
{
   QStringList sourceFilesCpp;

   for (const auto &item : sourceFiles) {

      if (item.endsWith(".java", Qt::CaseInsensitive)) {
         loadJava(fetchedTor, item, cd);

      } else if (item.endsWith(".ui", Qt::CaseInsensitive) || item.endsWith(".jui", Qt::CaseInsensitive)) {
         loadUI(fetchedTor, item, cd);

      } else if (item.endsWith(".js", Qt::CaseInsensitive) || item.endsWith(".qs", Qt::CaseInsensitive)) {
         // loadQScript(fetchedTor, item, cd);
         printErr(QString("Unsupported file type %1\n").formatArg(item));

      } else if (! processTs(fetchedTor, item, cd)) {
         sourceFilesCpp << item;
      }
   }

   loadCPP(fetchedTor, sourceFilesCpp, cd);

   if (! cd.error().isEmpty()) {
      printErr(cd.error());
   }
}

int main(int argc, char **argv)
{
   QCoreApplication app(argc, argv);

#ifndef Q_OS_WIN
   QTranslator translator;
   QTranslator qtTranslator;

   QString sysLocale   = QLocale::system().name();
   QString resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);

   if (translator.load("linguist_" + sysLocale, resourceDir) && qtTranslator.load("qt_" + sysLocale, resourceDir)) {
      app.installTranslator(&translator);
      app.installTranslator(&qtTranslator);
   }
#endif

   m_defaultExtensions = "java,jui,ui,c,c++,cc,cpp,cxx,ch,h,h++,hh,hpp,hxx,js,qs,qml,qrc";

   QStringList args = app.arguments();
   QStringList tsFileNames;

   QMultiHash<QString, QString> allCSources;
   QSet<QString> projectRoots;
   QStringList sourceFiles;
   QStringList resourceFiles;
   QStringList includePath;
   QStringList alienFiles;
   QString targetLanguage;
   QString sourceLanguage;

   // verbose is on by default
   UpdateOptions options = Verbose | HeuristicSameText | HeuristicSimilarText | HeuristicNumber;
   int numFiles = 0;

   bool metTsFlag     = false;
   bool metXTsFlag    = false;
   bool recursiveScan = true;

   QString extensions = m_defaultExtensions;
   QSet<QString> extensionsNameFilters;

   for (int i = 1; i < argc; ++i) {
      QString arg = args.at(i);

      if (arg == "-help" || arg == "--help" || arg == "-h") {
         printUsage();
         return 0;

      } else if (arg == "-list-languages") {
         printOut(getCountInfoString());
         return 0;

      } else if (arg == "-pluralonly") {
         options |= PluralOnly;
         continue;

      } else if (arg == "-noobsolete" || arg == "-no-obsolete") {
         options |= NoObsolete;
         continue;

      } else if (arg == "-silent") {
         options &= ~Verbose;
         continue;

      } else if (arg == "-target-language") {
         ++i;

         if (i == argc) {
            printErr("The option -target-language requires a parameter.\n");
            return 1;
         }

         targetLanguage = args[i];
         continue;

      } else if (arg == "-source-language") {
         ++i;

         if (i == argc) {
            printErr("The option -source-language requires a parameter.\n");
            return 1;
         }

         sourceLanguage = args[i];
         continue;

      } else if (arg == "-disable-heuristic") {
         ++i;

         if (i == argc) {
            printErr("The option -disable-heuristic requires a parameter.\n");
            return 1;
         }

         arg = args[i];
         if (arg == "sametext") {
            options &= ~HeuristicSameText;

         } else if (arg == "similartext") {
            options &= ~HeuristicSimilarText;

         } else if (arg == "number") {
            options &= ~HeuristicNumber;

         } else {
            printErr("Invalid heuristic name passed to -disable-heuristic.\n");
            return 1;
         }

         continue;

      } else if (arg == "-locations") {
         ++i;

         if (i == argc) {
            printErr("The option -locations requires a parameter.\n");
            return 1;
         }

         if (args[i] == "none") {
            options |= NoLocations;

         } else if (args[i] == "relative") {
            options |= RelativeLocations;

         } else if (args[i] == "absolute") {
            options |= AbsoluteLocations;

         } else {
            printErr("Invalid parameter passed to -locations.\n");
            return 1;
         }

         continue;

      } else if (arg == "-no-ui-lines") {
         options |= NoUiLines;
         continue;

      } else if (arg == "-verbose") {
         options |= Verbose;
         continue;

      } else if (arg == "-no-recursive") {
         recursiveScan = false;
         continue;

      } else if (arg == "-recursive") {
         recursiveScan = true;
         continue;

      } else if (arg == "-no-sort" || arg == "-nosort") {
         options |= NoSorting;
         continue;

      } else if (arg == "-version") {
         printOut(QString("lupdate version %1\n").formatArg(CS_VERSION_STR));
         return 0;

      } else if (arg == "-ts") {
         metTsFlag  = true;
         metXTsFlag = false;
         continue;
      } else if (arg == "-xts") {
         metTsFlag  = false;
         metXTsFlag = true;
         continue;
      } else if (arg == "-extensions") {
         ++i;

         if (i == argc) {
            printErr("The -extensions option should be followed by an extension list.\n");
            return 1;
         }

         extensions = args[i];
         continue;

      } else if (arg.startsWith("-I")) {
         if (arg.length() == 2) {
            ++i;

            if (i == argc) {
               printErr("The -I option should be followed by a path.\n");
               return 1;
            }
            includePath += args[i];

         } else {
            includePath += args[i].mid(2);
         }
         continue;

      } else if (arg.startsWith("-") && arg != "-") {
         printErr(QString("Unrecognized option '%1'.\n").formatArg(arg));
         return 1;
      }

      QStringList files;

      if (arg.startsWith("@")) {
         QFile lstFile(arg.mid(1));

         if (! lstFile.open(QIODevice::ReadOnly)) {
            printErr(QString("lupdate error: List file '%1' is not readable.\n").formatArg(lstFile.fileName()));
            return 1;
         }

         while (! lstFile.atEnd()) {
            QString lineContent = QString::fromUtf8(lstFile.readLine().trimmed());

            if (lineContent.startsWith("-I")) {
               if (lineContent.length() == 2) {
                  printErr("The -I option should be followed by a path.\n");
                  return 1;
               }

               includePath += lineContent.mid(2);

            } else {
               files << lineContent;
            }
         }

      } else {
         files << arg;
      }

      if (metTsFlag) {
         for (const QString &file : files) {
            bool found = false;

            for (const Translator::FileFormat &fmt : Translator::registeredFileFormats()) {

               if (file.endsWith(QChar('.') + fmt.extension, Qt::CaseInsensitive)) {
                  QFileInfo fi(file);

                  if (! fi.exists() || fi.isWritable()) {
                     tsFileNames.append(QFileInfo(file).absoluteFilePath());

                  } else {
                     printErr(QString("lupdate warning: File '%1' is not writable.\n").formatArg(file));
                  }

                  found = true;
                  break;
               }
            }

            if (! found) {
               printErr(QString("lupdate error: File '%1' has no recognized extension.\n").formatArg(file));
               return 1;
            }
         }

         ++numFiles;
      } else if (metXTsFlag) {
         alienFiles += files;

      } else {
         for (const QString &file : files) {
            QFileInfo fi(file);

            if (! fi.exists()) {
               printErr(QString("lupdate error: File '%1' does not exist.\n").formatArg(file));
               return 1;
            }

            if (fi.isDir()) {
               if (options & Verbose) {
                  printOut(QString("Scanning directory '%1'...\n").formatArg(file));
               }

               QDir dir = QDir(fi.filePath());
               projectRoots.insert(dir.absolutePath() + '/');

               if (extensionsNameFilters.isEmpty()) {
                  for (QString ext : extensions.split(',')) {
                     ext = ext.trimmed();
                     if (ext.startsWith('.')) {
                        ext.remove(0, 1);
                     }
                     extensionsNameFilters.insert(ext);
                  }
               }

               QDir::Filters filters = QDir::Files | QDir::NoSymLinks;
               if (recursiveScan) {
                  filters |= QDir::AllDirs | QDir::NoDotAndDotDot;
               }

               QFileInfoList fileinfolist;
               recursiveFileInfoList(dir, extensionsNameFilters, filters, &fileinfolist);
               int scanRootLen = dir.absolutePath().length();

               for (const QFileInfo &fi : fileinfolist) {
                  QString fn = QDir::cleanPath(fi.absoluteFilePath());
                  if (fn.endsWith(".qrc", Qt::CaseInsensitive)) {
                     resourceFiles << fn;
                  } else {
                     sourceFiles << fn;

                     if (! fn.endsWith(".java") && ! fn.endsWith(".jui") && ! fn.endsWith(".ui") &&
                           ! fn.endsWith(".js") && ! fn.endsWith(".qs") && ! fn.endsWith(".qml")) {

                        int offset = 0;
                        int depth = 0;

                        do {
                           offset = fn.lastIndexOf('/', offset - 1);
                           QString ffn = fn.mid(offset + 1);
                           allCSources.insert(ffn, fn);
                        } while (++depth < 3 && offset > scanRootLen);
                     }
                  }
               }

            } else {
               QString fn = QDir::cleanPath(fi.absoluteFilePath());
               if (fn.endsWith(".qrc", Qt::CaseInsensitive)) {
                  resourceFiles << fn;

               } else {
                  sourceFiles << fn;

               }

               projectRoots.insert(fi.absolutePath() + QChar('/'));
            }
         }

         ++numFiles;
      }

   } // for args

   if (numFiles == 0) {
      printUsage();
      return 1;
   }

   if (! targetLanguage.isEmpty() && tsFileNames.count() != 1)
      printErr("lupdate warning: -target-language usually only"
               " makes sense with exactly one TS file.\n");

   bool fail = false;

   if (tsFileNames.isEmpty()) {
      printErr("lupdate warning: No TS files specified. Only diagnostics will be produced.\n");
   }

   Translator fetchedTor;
   ConversionData cd;

   cd.m_noUiLines    = options & NoUiLines;
   cd.m_projectRoots = projectRoots;
   cd.m_includePath  = includePath;
   cd.m_allCSources  = allCSources;

   processSources(fetchedTor, sourceFiles, cd);
   updateTsFiles(fetchedTor, tsFileNames, alienFiles, sourceLanguage, targetLanguage, options, &fail);

   return fail ? 1 : 0;
}
