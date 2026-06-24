/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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
#include <qlibraryinfo.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtranslator.h>

#include <iostream>

static int usage(const QStringList &args)
{
   (void) args;

   QString loaders;
   QString line("    %1 - %2\n");

   for (const Translator::FileFormat &format : Translator::registeredFileFormats()) {
      loaders += line.formatArg(format.extension, -5).formatArg(format.description);
   }

   std::cout << csPrintable(QString(
         "\nUsage:\n"
         "    lconvert [options] <infile> [<infile>...]\n\n"
         "lconvert is part of Linguist tool chain. It can be used as a\n"
         "stand-alone tool to convert and filter translation data files.\n"
         "The following file formats are supported:\n\n%1\n"
         "If multiple input files are specified, they are merged with\n"
         "translations from later files taking precedence.\n\n"
         "Options:\n"
         "    -h\n"
         "    --help  Display this information and exit.\n\n"
         "    -i <infile>\n"
         "    --input-file <infile>\n"
         "           Specify input file. Use if <infile> might start with a dash.\n"
         "           This option can be used several times to merge inputs.\n"
         "           May be '-' (standard input) for use in a pipe.\n\n"
         "    -o <outfile>\n"
         "    --output-file <outfile>\n"
         "           Specify output file. Default is '-' (standard output).\n\n"
         "    -if <informat>\n"
         "    --input-format <format>\n"
         "           Specify input format for subsequent <infile>s.\n"
         "           The format is auto-detected from the file name and defaults to 'ts'.\n\n"
         "    -of <outformat>\n"
         "    --output-format <outformat>\n"
         "           Specify output format. See -if.\n\n"
         "    --drop-tags <regexp>\n"
         "           Drop named extra tags when writing TS or XLIFF files.\n"
         "           May be specified repeatedly.\n\n"
         "    --drop-translations\n"
         "           Drop existing translations and reset the status to 'unfinished'.\n"
         "           Note: this implies --no-obsolete.\n\n"
         "    --source-language <language>[_<region>]\n"
         "           Specify/override the language of the source strings. Defaults to\n"
         "           POSIX if not specified and the file does not name it yet.\n\n"
         "    --target-language <language>[_<region>]\n"
         "           Specify/override the language of the translation.\n"
         "           The target language is guessed from the file name if this option\n"
         "           is not specified and the file contents name no language yet.\n\n"
         "    --no-obsolete\n"
         "           Drop obsolete messages.\n\n"
         "    --no-finished\n"
         "           Drop finished messages.\n\n"
         "    --sort-contexts\n"
         "           Sort contexts in output TS file alphabetically.\n\n"
         "    --locations {absolute|relative|none}\n"
         "           Override how source code references are saved in TS files.\n"
         "           Default is absolute.\n\n"
         "    --no-ui-lines\n"
         "           Drop line numbers from references to UI files.\n\n"
         "    --verbose\n"
         "           be a bit more verbose\n\n"
         "Long options can be specified with only one leading dash, too.\n\n"
         "Return value:\n"
         "    0 on success\n"
         "    1 on command line parse failures\n"
         "    2 on read failures\n"
         "    3 on write failures\n").formatArg(loaders));

   return 1;
}

struct File {
   QString name;
   QString format;
};

int main(int argc, char *argv[])
{
   QCoreApplication app(argc, argv);

#ifndef Q_OS_WIN
   QTranslator trObj1;
   QTranslator trObj2;

   QString sysLocale   = QLocale::system().name();
   QString resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);

   if (trObj1.load("linguist_" + sysLocale, resourceDir) && trObj2.load("cs_" + sysLocale, resourceDir)) {
      app.installTranslator(&trObj1);
      app.installTranslator(&trObj2);
   }
#endif

   QStringList args = app.arguments();

   QList<File> inFiles;

   QString inFormat("auto");
   QString outFileName;
   QString outFormat("auto");
   QString targetLanguage;
   QString sourceLanguage;

   bool dropTranslations = false;
   bool noObsolete = false;
   bool noFinished = false;
   bool verbose    = false;
   bool noUiLines  = false;

   Translator::LocationType locations = Translator::LocationType::Default;

   ConversionData cd;
   Translator trObj;

   for (int i = 1; i < args.size(); ++i) {
      if (args[i].startsWith("--")) {
         args[i].remove(0, 1);
      }

      if (args[i] == "-o" || args[i] == "-output-file") {
         ++i;

         if (i >= args.size()) {
            return usage(args);
         }

         outFileName = args[i];

      } else if (args[i] == "-of" || args[i] == "-output-format") {
         ++i;

         if (i >= args.size()) {
            return usage(args);
         }

         outFormat = args[i];

      } else if (args[i] == "-i" || args[i] == "-input-file") {
         ++i;

         if (i >= args.size()) {
            return usage(args);
         }

         File file;
         file.name   = args[i];
         file.format = inFormat;

         inFiles.append(file);

      } else if (args[i] == "-if" || args[i] == "-input-format") {
         ++i;

         if (i >= args.size()) {
            return usage(args);
         }

         inFormat = args[i];

      } else if (args[i] == "-drop-tags") {
         ++i;

         if (i >= args.size()) {
            return usage(args);
         }

         cd.m_dropTags.append(args[i]);

      } else if (args[i] == "-drop-translations") {
         dropTranslations = true;

      } else if (args[i] == "-target-language") {
         ++i;

         if (i >= args.size()) {
            return usage(args);
         }

         targetLanguage = args[i];

      } else if (args[i] == "-source-language") {
         ++i;

         if (i >= args.size()) {
            return usage(args);
         }

         sourceLanguage = args[i];

      } else if (args[i].startsWith("-h")) {
         usage(args);
         return 0;

      } else if (args[i] == "-no-obsolete") {
         noObsolete = true;

      } else if (args[i] == "-no-finished") {
         noFinished = true;

      } else if (args[i] == "-sort-contexts") {
         cd.m_sortContexts = true;

      } else if (args[i] == "-locations") {
         ++i;

         if (i >= args.size()) {
            return usage(args);
         }

         if (args[i] == "none") {
            locations = Translator::LocationType::None;

         } else if (args[i] == "relative") {
            locations = Translator::LocationType::Relative;

         } else if (args[i] == "absolute") {
            locations = Translator::LocationType::Absolute;

         } else {
            return usage(args);
         }

      } else if (args[i] == "-no-ui-lines") {
         noUiLines = true;

      } else if (args[i] == "-verbose") {
         verbose = true;

      } else if (args[i].startsWith('-')) {
         return usage(args);

      } else {
         File file;
         file.name = args[i];
         file.format = inFormat;
         inFiles.append(file);
      }
   }

   if (inFiles.isEmpty()) {
      return usage(args);
   }

   trObj.setLanguageCode(Translator::guessLanguageCodeFromFileName(inFiles[0].name));

   if (! trObj.load(inFiles[0].name, cd, inFiles[0].format)) {
      std::cerr << csPrintable(cd.error());
      return 2;
   }

   trObj.reportDuplicates(trObj.resolveDuplicates(), inFiles[0].name, verbose);

   for (int i = 1; i < inFiles.size(); ++i) {
      Translator trObj3;

      if (! trObj3.load(inFiles[i].name, cd, inFiles[i].format)) {
         std::cerr << csPrintable(cd.error());

         return 2;
      }

      trObj3.reportDuplicates(trObj3.resolveDuplicates(), inFiles[i].name, verbose);

      for (int j = 0; j < trObj3.messageCount(); ++j) {
         trObj.replaceSorted(trObj3.message(j));
      }
   }

   if (! targetLanguage.isEmpty()) {
      trObj.setLanguageCode(targetLanguage);
   }

   if (! sourceLanguage.isEmpty()) {
      trObj.setSourceLanguageCode(sourceLanguage);
   }

   if (noObsolete) {
      trObj.stripObsoleteMessages();
   }

   if (noFinished) {
      trObj.stripFinishedMessages();
   }

   if (dropTranslations) {
      trObj.dropTranslations();
   }

   if (noUiLines) {
      trObj.dropUiLines();
   }

   if (locations != Translator::LocationType::Default) {
      trObj.setLocationType(locations);
   }

   trObj.normalizeTranslations(cd);

   if (! cd.errors().isEmpty()) {
      std::cerr << csPrintable(cd.error());
      cd.clearErrors();
   }

   if (! trObj.save(outFileName, cd, outFormat)) {
      std::cerr << csPrintable(cd.error());
      return 3;
   }

   return 0;
}
