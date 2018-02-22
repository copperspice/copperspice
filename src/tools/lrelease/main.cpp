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
#include <QtCore/QTranslator>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
#include <QtCore/QLibraryInfo>

QT_USE_NAMESPACE

class LR
{
   Q_DECLARE_TR_FUNCTIONS(LRelease)
};

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
   printOut(LR::tr(
               "Usage:\n"
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
               "           Display the version of lrelease and exit\n"
            ));
}

static bool loadTsFile(Translator &tor, const QString &tsFileName, bool /* verbose */)
{
   ConversionData cd;
   bool ok = tor.load(tsFileName, cd, QLatin1String("auto"));
   if (!ok) {
      printErr(LR::tr("lrelease error: %1").arg(cd.error()));
   } else {
      if (!cd.errors().isEmpty()) {
         printOut(cd.error());
      }
   }
   cd.clearErrors();
   return ok;
}

static bool releaseTranslator(Translator &tor, const QString &qmFileName,
                              ConversionData &cd, bool removeIdentical)
{
   tor.reportDuplicates(tor.resolveDuplicates(), qmFileName, cd.isVerbose());

   if (cd.isVerbose()) {
      printOut(LR::tr("Updating '%1'...\n").arg(qmFileName));
   }
   if (removeIdentical) {
      if (cd.isVerbose()) {
         printOut(LR::tr("Removing translations equal to source text in '%1'...\n").arg(qmFileName));
      }
      tor.stripIdenticalSourceTranslations();
   }

   QFile file(qmFileName);
   if (!file.open(QIODevice::WriteOnly)) {
      printErr(LR::tr("lrelease error: cannot create '%1': %2\n")
               .arg(qmFileName, file.errorString()));
      return false;
   }

   tor.normalizeTranslations(cd);
   bool ok = saveQM(tor, file, cd);
   file.close();

   if (!ok) {
      printErr(LR::tr("lrelease error: cannot save '%1': %2")
               .arg(qmFileName, cd.error()));
   } else if (!cd.errors().isEmpty()) {
      printOut(cd.error());
   }
   cd.clearErrors();
   return ok;
}

static bool releaseTsFile(const QString &tsFileName,
                          ConversionData &cd, bool removeIdentical)
{
   Translator tor;
   if (!loadTsFile(tor, tsFileName, cd.isVerbose())) {
      return false;
   }

   QString qmFileName = tsFileName;
   for (const Translator::FileFormat & fmt: Translator::registeredFileFormats()) {
      if (qmFileName.endsWith(QLatin1Char('.') + fmt.extension)) {
         qmFileName.chop(fmt.extension.length() + 1);
         break;
      }
   }
   qmFileName += QLatin1String(".qm");

   return releaseTranslator(tor, qmFileName, cd, removeIdentical);
}

static void print(const QString &fileName, int lineNo, const QString &msg)
{
   if (lineNo) {
      printErr(QString::fromLatin1("%2(%1): %3").arg(lineNo).arg(fileName, msg));
   } else {
      printErr(msg);
   }
}

int main(int argc, char **argv)
{
   QCoreApplication app(argc, argv);

#ifndef Q_OS_WIN32
   QTranslator translator;
   QTranslator qtTranslator;
   QString sysLocale = QLocale::system().name();
   QString resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
   if (translator.load(QLatin1String("linguist_") + sysLocale, resourceDir)
         && qtTranslator.load(QLatin1String("qt_") + sysLocale, resourceDir)) {
      app.installTranslator(&translator);
      app.installTranslator(&qtTranslator);
   }
#endif

   ConversionData cd;
   cd.m_verbose = true; // the default is true starting with Qt 4.2
   bool removeIdentical = false;
   Translator tor;
   QStringList inputFiles;
   QString outputFile;

   for (int i = 1; i < argc; ++i) {
      if (!strcmp(argv[i], "-compress")) {
         cd.m_saveMode = SaveStripped;
         continue;
      } else if (!strcmp(argv[i], "-idbased")) {
         cd.m_idBased = true;
         continue;
      } else if (!strcmp(argv[i], "-nocompress")) {
         cd.m_saveMode = SaveEverything;
         continue;
      } else if (!strcmp(argv[i], "-removeidentical")) {
         removeIdentical = true;
         continue;
      } else if (!strcmp(argv[i], "-nounfinished")) {
         cd.m_ignoreUnfinished = true;
         continue;
      } else if (!strcmp(argv[i], "-markuntranslated")) {
         if (i == argc - 1) {
            printUsage();
            return 1;
         }
         cd.m_unTrPrefix = QString::fromLocal8Bit(argv[++i]);
      } else if (!strcmp(argv[i], "-silent")) {
         cd.m_verbose = false;
         continue;
      } else if (!strcmp(argv[i], "-verbose")) {
         cd.m_verbose = true;
         continue;
      } else if (!strcmp(argv[i], "-version")) {
         printOut(LR::tr("lrelease version %1\n").arg(QLatin1String(CS_VERSION_STR)));
         return 0;
      } else if (!strcmp(argv[i], "-qm")) {
         if (i == argc - 1) {
            printUsage();
            return 1;
         }
         outputFile = QString::fromLocal8Bit(argv[++i]);
      } else if (!strcmp(argv[i], "-help")) {
         printUsage();
         return 0;
      } else if (argv[i][0] == '-') {
         printUsage();
         return 1;
      } else {
         inputFiles << QString::fromLocal8Bit(argv[i]);
      }
   }

   if (inputFiles.isEmpty()) {
      printUsage();
      return 1;
   }

   for (const QString & inputFile : inputFiles) {

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

   if (!outputFile.isEmpty()) {
      return releaseTranslator(tor, outputFile, cd, removeIdentical) ? 0 : 1;
   }

   return 0;
}
