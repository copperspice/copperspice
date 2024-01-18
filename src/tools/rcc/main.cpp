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

#define RCC_VERSION_STR "1.0.0"

#include <rcc.h>

#include <qdebug.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

#include <qcorecmdlineargs_p.h>

void showHelp(const QString &argv0, const QString &error)
{
   fprintf(stderr, "CopperSpice resource compiler\n");

   if (! error.isEmpty()) {
      fprintf(stderr, "%s: %s\n", csPrintable(argv0), csPrintable(error));
   }

   fprintf(stderr, "Usage: %s  [options] <inputs>\n\n"
      "Options:\n"
      "  -o file              write output to file rather than stdout\n"
      "  -name name           create an external initialization function with name\n"
      "  -threshold level     threshold to consider compressing files\n"
      "  -compress level      compress input files by level\n"
      "  -root path           prefix resource access path with root path\n"
      "  -no-compress         disable all compression\n"
      "  -binary              output a binary file for use as a dynamic resource\n"
      "  -project             generate resource file containing all files from the current directory\n"
      "  -version             display rcc version\n"
      "  -help                display this information\n",
      csPrintable(argv0));
}

void dumpRecursive(const QDir &dir, QTextStream &out)
{
   QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

   for (QFileInfo entry : entries) {

      if (entry.isDir()) {
         dumpRecursive(entry.filePath(), out);

      } else {
         out << "<file>" << entry.filePath() << "</file>\n";
      }
   }
}

int createProject(const QString &outFileName)
{
   QDir currentDir = QDir::current();
   QString currentDirName = currentDir.dirName();

   if (currentDirName.isEmpty()) {
      currentDirName = "root";
   }

   QFile file;
   bool isOk = false;

   if (outFileName.isEmpty()) {
      isOk = file.open(stdout, QFile::WriteOnly | QFile::Text);

   } else {
      file.setFileName(outFileName);
      isOk = file.open(QFile::WriteOnly | QFile::Text);
   }

   if (! isOk) {
      fprintf(stderr, "Unable to open %s: %s\n",
         outFileName.isEmpty() ? csPrintable(outFileName) : "standard output", csPrintable(file.errorString()));
      return 1;
   }

   QTextStream out(&file);
   out << "<!DOCTYPE RCC><RCC version=\"1.0\">\n" "<qresource>\n";

   // use "." as dir to get relative file pathes
   dumpRecursive(QDir("."), out);

   out << "</qresource>\n" "</RCC>\n";

   return 0;
}

int runRcc(int argc, char *argv[])
{
   QString outFilename;
   QStringList filenamesIn;

   bool helpRequested    = false;
   bool list             = false;
   bool projectRequested = false;

   QStringList args = qCmdLineArgs(argc, argv);

   RCCResourceLibrary library;

   // parse options
   QString errorMsg;

   for (int i = 1; i < args.count() && errorMsg.isEmpty(); i++) {
      if (args[i].isEmpty()) {
         continue;
      }

      if (args[i][0] == '-') {
         // option
         QString opt = args[i];

         if (opt == "-o") {
            if (! (i < argc - 1)) {
               errorMsg = "Missing output name";
               break;
            }
            outFilename = args[++i];

         } else if (opt == "-name") {
            if (! (i < argc - 1)) {
               errorMsg = "Missing target name";
               break;
            }
            library.setInitName(args[++i]);

         } else if (opt == "-root") {
            if (! (i < argc - 1)) {
               errorMsg = "Missing root path";
               break;
            }

            library.setResourceRoot(QDir::cleanPath(args[++i]));
            if (library.resourceRoot().isEmpty() || library.resourceRoot().at(0) != '/') {
               errorMsg = "Root must start with a /";
            }

         } else if (opt == "-compress") {
            if (! (i < argc - 1)) {
               errorMsg = "Missing compression level";
               break;
            }
            library.setCompressLevel(args[++i].toInteger<int>());

         } else if (opt == "-threshold") {
            if (!(i < argc - 1)) {
               errorMsg = "Missing compression threshold";
               break;
            }
            library.setCompressThreshold(args[++i].toInteger<int>());

         } else if (opt == "-binary") {
            library.setFormat(RCCResourceLibrary::Binary);

         } else if (opt == "-verbose") {
            library.setVerbose(true);

         } else if (opt == "-list") {
            list = true;

         } else if (opt == "-version" || opt == "-v") {
            fprintf(stderr, "CopperSpice Resource Compiler version %s\n", RCC_VERSION_STR);
            return 1;

         } else if (opt == "-help" || opt == "-h") {
            helpRequested = true;

         } else if (opt == "-no-compress") {
            library.setCompressLevel(-2);

         } else if (opt == "-project") {
            projectRequested = true;

         } else {
            errorMsg = QString("Unknown option: '%1'").formatArg(args[i]);
         }

      } else {
         if (! QFile::exists(args[i])) {
            qWarning("%s: File does not exist '%s'", csPrintable(args[0]), csPrintable(args[i]));
            return 1;
         }

         filenamesIn.append(args[i]);
      }
   }

   if (projectRequested && ! helpRequested) {
      return createProject(outFilename);
   }

   if (! filenamesIn.size() || ! errorMsg.isEmpty() || helpRequested) {
      showHelp(args[0], errorMsg);
      return 1;
   }

   QFile errorDevice;
   errorDevice.open(stderr, QIODevice::WriteOnly | QIODevice::Text);

   if (library.verbose()) {
      errorDevice.write("CopperSpice resource compiler\n");
   }

   library.setInputFiles(filenamesIn);

   if (! library.readFiles(list, errorDevice)) {
      return 1;
   }

   // open output
   QFile out;
   QIODevice::OpenMode mode = QIODevice::WriteOnly;

   if (library.format() == RCCResourceLibrary::C_Code) {
      mode |= QIODevice::Text;
   }

   if (outFilename.isEmpty() || outFilename == "-") {
      // using this overload close() only flushes.
      out.open(stdout, mode);

   } else {
      out.setFileName(outFilename);

      if (! out.open(mode)) {
         const QString msg = QString("Unable to open %1 for writing: %2\n").formatArg(outFilename).formatArg(out.errorString());
         errorDevice.write(msg.toUtf8());

         return 1;
      }
   }

   // do the task

   if (list) {
      for (const auto &item : library.dataFiles()) {
         out.write(QDir::cleanPath(item).toUtf8());
         out.write("\n");
      }

      return 0;
   }

   return library.output(out, errorDevice) ? 0 : 1;
}

int main(int argc, char *argv[])
{
   return runRcc(argc, argv);
}
