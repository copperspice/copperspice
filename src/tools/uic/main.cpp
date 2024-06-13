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

#define UIC_VERSION_STR "1.0.0"

#include <driver.h>
#include <option.h>
#include <qdir.h>
#include <qfile.h>
#include <qtextcodec.h>
#include <qtextstream.h>
#include <uic.h>

static const char *error = nullptr;

void showHelp(const char *appName)
{
   fprintf(stderr, "CopperSpice User Interface Compiler version %s\n", UIC_VERSION_STR);

   if (error) {
      fprintf(stderr, "%s: %s\n", appName, error);
   }

   fprintf(stderr, "Usage: %s [options] <uifile>\n\n"
      "  -h, -help                 display this help and exit\n"
      "  -v, -version              display version\n"
      "  -d, -dependencies         display the dependencies\n"
      "  -o <file>                 place the output into <file>\n"
      "  -tr <func>                use func() for i18n\n"
      "  -p, -no-protection        disable header protection\n"
      "  -n, -no-implicit-includes disable generation of #include-directives\n"
      "  -g <name>                 change generator\n"
      "\n", appName);
}

int runUic(int argc, char *argv[])
{
   const char *fileName = nullptr;

   int arg = 1;
   Driver driver;

   while (arg < argc) {
      QString opt = QString::fromUtf8(argv[arg]);

      if (opt == "-h" || opt == "-help" || opt == "--help") {
         showHelp(argv[0]);
         return 0;

      } else if (opt == "-d" || opt == "-dependencies") {
         driver.option().dependencies = true;

      } else if (opt == "-v" || opt == "-version" || opt == "--version") {
         fprintf(stderr, "CopperSpice User Interface Compiler version %s\n", UIC_VERSION_STR);
         return 0;

      } else if (opt == "-o" || opt == "-output") {
         ++arg;

         if (! argv[arg]) {
            showHelp(argv[0]);
            return 1;
         }
         driver.option().outputFile = QFile::decodeName(argv[arg]);

      } else if (opt == "-p" || opt == "-no-protection") {
         driver.option().headerProtection = false;

      } else if (opt == "-n" || opt == "-no-implicit-includes") {
         driver.option().implicitIncludes = false;

      } else if (opt == "-postfix") {
         ++arg;

         if (! argv[arg]) {
            showHelp(argv[0]);
            return 1;
         }
         driver.option().postfix = QString::fromUtf8(argv[arg]);

      } else if (opt == "-tr" || opt == "-translate") {
         ++arg;
         if (!argv[arg]) {
            showHelp(argv[0]);
            return 1;
         }

         driver.option().translateFunction = QString::fromUtf8(argv[arg]);

      } else if (opt == "-g" || opt == "-generator") {
         ++arg;

         if (! argv[arg]) {
            showHelp(argv[0]);
            return 1;
         }

         QString name = QString::fromUtf8(argv[arg]).toLower();
         driver.option().generator = (name == "java") ? Option::JavaGenerator : Option::CppGenerator;

      } else if (! fileName) {
         fileName = argv[arg];

      } else {
         showHelp(argv[0]);
         return 1;
      }

      ++arg;
   }

   QString inputFile;

   if (fileName) {
      inputFile = QString::fromUtf8(fileName);

   } else {
      // no file name provide so there is nothing to do
      showHelp(argv[0]);
      return 1;
   }

   if (driver.option().dependencies) {
      return ! driver.printDependencies(inputFile);
   }

   QTextStream *out = nullptr;
   QFile f;

   if (driver.option().outputFile.size()) {
      f.setFileName(driver.option().outputFile);

      if (! f.open(QIODevice::WriteOnly | QFile::Text)) {
         fprintf(stderr, "Could not create output file %s\n", csPrintable(f.errorString()));
         return 1;
      }

      out = new QTextStream(&f);
      out->setCodec(QTextCodec::codecForName("UTF-8"));
   }

   bool retval = driver.uic(inputFile, out);
   delete out;

   if (! retval) {
      if (driver.option().outputFile.size()) {
         f.close();
         f.remove();
      }

      if (inputFile.isEmpty()) {
         fprintf(stderr, "File '<stdin>' is not valid\n");
      } else {
         fprintf(stderr, "File '%s' is not valid\n", csPrintable(inputFile));
      }
   }

   return ! retval;
}

int main(int argc, char *argv[])
{
   return runUic(argc, argv);
}
