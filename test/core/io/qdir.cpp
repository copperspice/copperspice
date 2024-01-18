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

TEST_CASE("QDir traits", "[qdir]")
{
   REQUIRE(std::is_copy_constructible_v<QDir> == true);
   REQUIRE(std::is_move_constructible_v<QDir> == true);

   REQUIRE(std::is_copy_assignable_v<QDir> == true);
   REQUIRE(std::is_move_assignable_v<QDir> == true);

   REQUIRE(std::has_virtual_destructor_v<QDir> == false);
}

TEST_CASE("QDir absolute_path", "[qdir]")
{

#if defined(Q_OS_WIN)
   QString str("c:/machine/dir1/file1");
   QString result("c:/machine/dir1/file1");

#else
   QString str("/machine/dir1/file1");
   QString result("/machine/dir1/file1");

#endif

   QDir path(str);

   REQUIRE(path.absolutePath() == result);
}

TEST_CASE("QDir absolute_file_path", "[qdir]")
{
   QString str("/machine/dir1");
   QString fileName("file1");

   QString result("/machine/dir1/file1");

   QDir path(str);
   QString absFilePath = path.absoluteFilePath(fileName);

   REQUIRE(absFilePath == result);
}

TEST_CASE("QDir current", "[qdir]")
{
   QString str = QDir::currentPath();
   QDir path   = QDir::current();

   REQUIRE(path.path() == str);
}

TEST_CASE("QDir dir_name", "[qdir]")
{
   QString str("c:/machine/dir1");
   QString result("dir1");

   QDir path(str);

   REQUIRE(path.dirName() == result);
}

TEST_CASE("QDir clean_path", "[qdir]")
{
   {
      QDir path = QDir::cleanPath("c:\\machine");

#if defined(Q_OS_WIN)
      REQUIRE(path.path() == "c:/machine");
#else
      REQUIRE(path.path() == "c:\\machine");
#endif
   }

   {
      QDir path = QDir::cleanPath("c:\\");
#if defined(Q_OS_WIN)
      REQUIRE(path.path() == "c:/");
#else
      REQUIRE(path.path() == "c:\\");
#endif
   }

   {
      QDir path = QDir::cleanPath("c:");
      REQUIRE(path.path() == "c:");
   }

   {
      QDir path = QDir::cleanPath("./");
      REQUIRE(path.path() == ".");
   }

   {
      QDir path = QDir::cleanPath("..");
      REQUIRE(path.path() == "..");
   }
}

TEST_CASE("QDir entry_list", "[qdir]")
{
   QString fileName = "MyFileName.cfg";
   QString str(QDir::tempPath() + "/" + fileName);

   QTemporaryFile tmpFile(str);
   tmpFile.open();

   str = fileName + tmpFile.fileName().right(7);

   QDir path(QDir::temp());

   REQUIRE(path.entryList().contains(str) == true);
   REQUIRE(path.entryList(QDir::AllDirs).contains(str) == false);
   REQUIRE(path.entryList(QDir::Dirs).contains(str) == false);
   REQUIRE(path.entryList(QDir::Files).contains(str) == true);
}

TEST_CASE("QDir exists", "[qdir]")
{
   QDir path;

   REQUIRE(path.exists());
}

TEST_CASE("QDir set_path", "[qdir]")
{
   QString str1(".");
   QString str2("..");

   QDir path1;

   {
      QDir path2(str1);
      path1.setPath(str1);

      QStringList list1 = path1.entryList();

      REQUIRE(path1.entryList() == list1);
      REQUIRE(path2.entryList() == list1);
   }

   {
      QDir path3(str2);
      QStringList list3 = path3.entryList();

      path1.setPath(str2);
      REQUIRE(path3.entryList() == list3);
   }
}

TEST_CASE("QDir temp_path", "[qdir]")
{
   QString str = QDir::tempPath();
   QDir path   = QDir::temp();

   REQUIRE(str == path.absolutePath());
}

