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
#include <qlibraryinfo.h>
#include <qregularexpression.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtranslator.h>

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

static void printUsage()
{
   printOut("Usage:\n"
            "    lrelease [options] project-file\n"
            "    lrelease [options] ts-files [-qm qm-file]\n\n"
            "lrelease is part of Qt's Linguist tool chain. It can be used as a\n"
            "stand-alone tool to convert XML-based translations files in the TS\n"
            "format into the 'compiled' QM format used by QTranslator objects.\n\n"
            "Options:\n"
            "    -help  Display this information and exit\n"
            "    -idbased\n"
            "           Use IDs instead of source strings for message keying\n"
            "    -compress\n"
            "           Compress the QM files\n"
            "    -nounfinished\n"
            "           Do not include unfinished translations\n"
            "    -removeidentical\n"
            "           If the translated text is the same as\n"
            "           the source text, do not include the message\n"
            "    -markuntranslated <prefix>\n"
            "           If a message has no real translation, use the source text\n"
            "           prefixed with the given string instead\n"
            "    -silent\n"
            "           Do not explain what is being done\n"
            "    -version\n"
            "           Display the version of lrelease and exit\n");
}

static bool loadTsFile(Translator &tor, const QString &tsFileName, bool /* verbose */)
{
   ConversionData cd;
   bool ok = tor.load(tsFileName, cd, "auto");

   if (!ok) {
      printErr(QString("lrelease error: %1").formatArg(cd.error()));

   } else {
      if (!cd.errors().isEmpty()) {
         printOut(cd.error());
      }
   }

   cd.clearErrors();

   return ok;
}

static bool releaseTranslator(Translator &tor, const QString &qmFileName, ConversionData &cd, bool removeIdentical)
{
   tor.reportDuplicates(tor.resolveDuplicates(), qmFileName, cd.isVerbose());

   if (cd.isVerbose()) {
      printOut(QString("Updating '%1'...\n").formatArg(qmFileName));
   }

   if (removeIdentical) {
      if (cd.isVerbose()) {
         printOut(QString("Removing translations equal to source text in '%1'...\n").formatArg(qmFileName));
      }
      tor.stripIdenticalSourceTranslations();
   }

   QFile file(qmFileName);
   if (! file.open(QIODevice::WriteOnly)) {
      printErr(QString("lrelease error: Unable to create '%1': %2\n").formatArg(qmFileName).formatArg(file.errorString()));
      return false;
   }

   tor.normalizeTranslations(cd);
   bool ok = saveQM(tor, file, cd);
   file.close();

   if (!ok) {
      printErr(QString("lrelease error: Unable to save '%1': %2").formatArg(qmFileName).formatArg(cd.error()));
   } else if (! cd.errors().isEmpty()) {
      printOut(cd.error());
   }

   cd.clearErrors();
   return ok;
}

static bool releaseTsFile(const QString &tsFileName, ConversionData &cd, bool removeIdentical)
{
   Translator tor;
   if (! loadTsFile(tor, tsFileName, cd.isVerbose())) {
      return false;
   }

   QString qmFileName = tsFileName;
   for (const Translator::FileFormat &fmt : Translator::registeredFileFormats()) {
      if (qmFileName.endsWith('.' + fmt.extension)) {
         qmFileName.chop(fmt.extension.length() + 1);
         break;
      }
   }
   qmFileName += ".qm";

   return releaseTranslator(tor, qmFileName, cd, removeIdentical);
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

   ConversionData cd;
   cd.m_verbose = true;
   bool removeIdentical = false;
   Translator tor;

   QStringList inputFiles;
   QString outputFile;

   for (int i = 1; i < argc; ++i) {
      if (! strcmp(argv[i], "-compress")) {
         cd.m_saveMode = TranslatorMessage::SaveMode::Stripped;
         continue;

      } else if (! strcmp(argv[i], "-idbased")) {
         cd.m_idBased = true;
         continue;

      } else if (! strcmp(argv[i], "-nocompress")) {
         cd.m_saveMode = TranslatorMessage::SaveMode::Everything;
         continue;

      } else if (! strcmp(argv[i], "-removeidentical")) {
         removeIdentical = true;
         continue;

      } else if (! strcmp(argv[i], "-nounfinished")) {
         cd.m_ignoreUnfinished = true;
         continue;

      } else if (! strcmp(argv[i], "-markuntranslated")) {
         if (i == argc - 1) {
            printUsage();
            return 1;
         }
         cd.m_unTrPrefix = QString::fromUtf8(argv[++i]);

      } else if (! strcmp(argv[i], "-silent")) {
         cd.m_verbose = false;
         continue;

      } else if (! strcmp(argv[i], "-verbose")) {
         cd.m_verbose = true;
         continue;

      } else if (! strcmp(argv[i], "-version")) {
         printOut(QString("lrelease version %1\n").formatArg(CS_VERSION_STR));
         return 0;

      } else if (! strcmp(argv[i], "-qm")) {
         if (i == argc - 1) {
            printUsage();
            return 1;
         }
         outputFile = QString::fromUtf8(argv[++i]);

      } else if (! strcmp(argv[i], "-help")) {
         printUsage();
         return 0;

      } else if (argv[i][0] == '-') {
         printUsage();
         return 1;

      } else {
         inputFiles << QString::fromUtf8(argv[i]);
      }
   }

   if (inputFiles.isEmpty()) {
      printUsage();
      return 1;
   }

   for (const QString &inputFile : inputFiles) {

      if (outputFile.isEmpty()) {
         if (!releaseTsFile(inputFile, cd, removeIdentical)) {
            return 1;
         }

      } else {
         if (!loadTsFile(tor, inputFile, cd.isVerbose())) {
            return 1;
         }
      }
   }

   if (! outputFile.isEmpty()) {
      return releaseTranslator(tor, outputFile, cd, removeIdentical) ? 0 : 1;
   }

   return 0;
}
