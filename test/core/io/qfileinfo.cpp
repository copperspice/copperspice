/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#include <qdir.h>
#include <qfileinfo.h>
#include <qtemporaryfile.h>

#include <cs_catch2.h>

TEST_CASE("QFileInfo traits", "[qfileinfo]")
{
   REQUIRE(std::is_copy_constructible_v<QFileInfo> == true);
   REQUIRE(std::is_move_constructible_v<QFileInfo> == true);

   REQUIRE(std::is_copy_assignable_v<QFileInfo> == true);
   REQUIRE(std::is_move_assignable_v<QFileInfo> == true);

   REQUIRE(std::has_virtual_destructor_v<QFileInfo> == false);
}

TEST_CASE("QFileInfo absolutePath", "[qfileinfo]")
{
   QString str;
   QString path;
   QString fname;

   QString drivePrefix;

   QFileInfo fi;

#if defined(Q_OS_WIN)
   drivePrefix = QDir::currentPath().left(2);
#endif

   {
      str   = "/machine/share/dir1/";
      path  = drivePrefix + "/machine/share/dir1";
      fname = QString();

      fi = QFile(str);

      REQUIRE(fi.absolutePath() == path);
      REQUIRE(fi.fileName() == fname);

#if defined(Q_OS_FREEBSD)
      REQUIRE(fi.canonicalPath() == "/machine/share");
      REQUIRE(fi.canonicalFilePath() == "/machine/share/dir1");
#else
      REQUIRE(fi.canonicalPath() == "/");
      REQUIRE(fi.canonicalFilePath() == "");
#endif

   }

   {
      str   = "/machine/share/dir1";
      path  = drivePrefix + "/machine/share";
      fname = "dir1";

      fi = QFile(str);

      REQUIRE(fi.absolutePath() == path);
      REQUIRE(fi.fileName() == fname);

#if defined(Q_OS_FREEBSD)
      REQUIRE(fi.canonicalPath() == "/machine/share");
      REQUIRE(fi.canonicalFilePath() == "/machine/share/dir1");
#else
      REQUIRE(fi.canonicalPath() == "/");
      REQUIRE(fi.canonicalFilePath() == "");
#endif

   }
}

TEST_CASE("QFileInfo file_empty", "[qfileinfo]")
{
   QString str = QDir::currentPath();

   QFileInfo fi;
   fi  = QFileInfo(str);

   REQUIRE(fi.exists() == true);
   REQUIRE(fi.isDir()  == true);
   REQUIRE(fi.isFile() == false);
}

TEST_CASE("QFileInfo file_exists", "[qfileinfo]")
{
   QTemporaryFile tmpFile;
   tmpFile.open();

   QFileInfo fi(tmpFile.fileName());

   REQUIRE(fi.exists()   == true);
   REQUIRE(fi.isDir()    == false);
   REQUIRE(fi.isFile()   == true);
   REQUIRE(fi.isHidden() == false);

   REQUIRE(fi.filePath().isEmpty() == false);
}

TEST_CASE("QFileInfo file_name", "[qfileinfo]")
{
   QTemporaryFile tmpFile(QDir::tempPath() + "/cs_temp.XXXXXX.cfg");
   tmpFile.open();

   QString fName = tmpFile.fileName();
   QFileInfo fi(fName);

   REQUIRE(fi.filePath() == fName);

   REQUIRE(fi.baseName() == "cs_temp");
   REQUIRE(fi.completeBaseName() == "cs_temp." + fName.right(10).left(6));

   REQUIRE(fi.completeSuffix() == fName.right(10));      // XXXXXX.cfg
   REQUIRE(fi.suffix() == "cfg");
}

TEST_CASE("QFileInfo file_no_extentsion", "[qfileinfo]")
{
   QString fName = QDir::tempPath() + "/no_extension_XXXXXX";

   QTemporaryFile tmpFile(fName);
   tmpFile.open();

   QFileInfo fi(tmpFile.fileName());

   REQUIRE(fi.exists() == true);

   REQUIRE(fi.suffix().isEmpty() == true);
   REQUIRE(fi.completeSuffix().isEmpty() == true);
}

TEST_CASE("QFileInfo file_not_pressent", "[qfileinfo]")
{
   QFileInfo fi("nonexistent.txt");

   REQUIRE(fi.exists() == false);
   REQUIRE(fi.fileName() == "nonexistent.txt");
}

TEST_CASE("QFileInfo permissions_flags", "[qfileinfo]")
{
   QTemporaryFile tmpFile;
   tmpFile.open();

   QString fName = tmpFile.fileName();
   QFileInfo fi(fName);

   REQUIRE(fi.permission(QFileDevice::ReadUser)  == true);
   REQUIRE(fi.permission(QFileDevice::WriteUser) == true);

   REQUIRE(fi.isReadable()   == true);
   REQUIRE(fi.isWritable()   == true);
   REQUIRE(fi.isExecutable() == false);
}

TEST_CASE("QFileInfo symbolic_link", "[qfileinfo]")
{
#ifndef Q_OS_WIN
   QTemporaryFile tmpFile;
   tmpFile.open();

   QString fName = tmpFile.fileName();
   QFileInfo fi(fName);

   //
   QString symlink = QDir::tempPath() + "/link.txt";

   // removed for safety
   QFile::remove(symlink);

   QFile::link(fName, symlink);

   //
   QFileInfo linkInfo(symlink);

   REQUIRE(linkInfo.exists() == true);
   REQUIRE(linkInfo.isSymLink() == true);
   REQUIRE(linkInfo.symLinkTarget() == QFileInfo(fName).absoluteFilePath());

   QFile::remove(symlink);
#endif
}
