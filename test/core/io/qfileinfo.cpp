/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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
      fname = "";

      fi = QFile(str);


      REQUIRE(fi.absolutePath() == path);
      REQUIRE(fi.fileName() == fname);
   }

   {
      str   = "/machine/share/dir1";
      path  = drivePrefix + "/machine/share";
      fname = "dir1";

      fi = QFile(str);

      REQUIRE(fi.absolutePath() == path);
      REQUIRE(fi.fileName() == fname);
   }
}

TEST_CASE("QFileInfo base_name", "[qfileinfo]")
{
   QTemporaryFile tmpFile(QDir::tempPath() + "/cs_temp.XXXXXX.cfg");
   tmpFile.open();

   QFileInfo fi(tmpFile.fileName());

   REQUIRE(fi.baseName().endsWith("cs_temp"));
   REQUIRE(fi.suffix().endsWith("cfg"));
}

TEST_CASE("QFileInfo file", "[qfileinfo]")
{
   QTemporaryFile tmpFile;
   tmpFile.open();

   QFileInfo fi(tmpFile.fileName());

   REQUIRE(fi.exists()   == true);
   REQUIRE(fi.isDir()    == false);
   REQUIRE(fi.isFile()   == true);
   REQUIRE(fi.isHidden() == false);
}

TEST_CASE("QFileInfo is_file", "[qfileinfo]")
{
   QString str;
   QFileInfo fi;

   {
      str = QDir::currentPath();
      fi  = QFileInfo(str);

      REQUIRE(fi.exists() == true);
      REQUIRE(fi.isDir()  == true);
      REQUIRE(fi.isFile() == false);
   }
}

