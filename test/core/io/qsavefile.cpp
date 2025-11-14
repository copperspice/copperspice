/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qsavefile.h>
#include <qtemporarydir.h>

#include <cs_catch2.h>

TEST_CASE("QSaveFile traits", "[qsavefile]")
{
   REQUIRE(std::is_copy_constructible_v<QSaveFile> == false);
   REQUIRE(std::is_move_constructible_v<QSaveFile> == false);

   REQUIRE(std::is_copy_assignable_v<QSaveFile> == false);
   REQUIRE(std::is_move_assignable_v<QSaveFile> == false);

   REQUIRE(std::has_virtual_destructor_v<QSaveFile> == true);
}

TEST_CASE("QSaveFile open", "[qsavefile]")
{
   QSaveFile file;

   REQUIRE(file.open(QIODevice::WriteOnly) == true);

   REQUIRE(file.pos() == 0);
   REQUIRE(file.size() == 0);
   REQUIRE(file.resize(10) == true);
}

TEST_CASE("QSaveFile errors", "[qsavefile]")
{
   QTemporaryDir tmpDir;
   REQUIRE(tmpDir.isValid() == true);

   QString fileName = tmpDir.path() + "/error.txt";

   SECTION("a")
   {
      QSaveFile file(fileName);

      REQUIRE(file.commit() == false);
   }

   SECTION("b")
   {
      QSaveFile file(fileName);

      REQUIRE(file.open(QIODevice::WriteOnly) == true);

      auto s1 = file.write("ABC");
      REQUIRE(s1 == 3);

      //
      REQUIRE(file.commit() == true);

      auto s2 = file.write("ABC");
      REQUIRE(s2 == -1);
   }
}

TEST_CASE("QSaveFile fileName", "[qsavefile]")
{
   QSaveFile file("c:/machine/path1/dir1/file1");
   REQUIRE(file.fileName() == "c:/machine/path1/dir1/file1");

   REQUIRE(file.pos() == 0);
   REQUIRE(file.size() == 0);
   REQUIRE(file.resize(10) == false);
}

TEST_CASE("QSaveFile write", "[qsavefile]")
{
   QTemporaryDir tmpDir;
   REQUIRE(tmpDir.isValid() == true);

   //
   QByteArray data_in("CopperSpice");
   QString fileName = tmpDir.path() + "/test.txt";

   QSaveFile file_A(fileName);

   REQUIRE(file_A.open(QIODevice::WriteOnly | QIODevice::Text) == true);
   REQUIRE(file_A.write(data_in) == data_in.size());
   REQUIRE(file_A.commit() == true);

   QFile file_B(fileName);

   REQUIRE(file_B.open(QIODevice::ReadOnly | QIODevice::Text) == true);

   QByteArray data_out = file_B.readAll();
   REQUIRE(data_in == data_out);
}

TEST_CASE("QSaveFile write_multiple", "[qsavefile]")
{
   QTemporaryDir tmpDir;
   REQUIRE(tmpDir.isValid() == true);

   //
   QString fileName = tmpDir.path() + "/test.txt";

   QSaveFile file_A(fileName);
   REQUIRE(file_A.open(QIODevice::WriteOnly) == true);

   REQUIRE(file_A.write("pear\n") > 0);
   REQUIRE(file_A.write("apple\n") > 0);
   REQUIRE(file_A.write("watermelon\n") > 0);

   REQUIRE(file_A.commit() == true);

   QFile file_B(fileName);
   REQUIRE(file_B.open(QIODevice::ReadOnly) == true);

   QByteArray data_in("pear\napple\nwatermelon\n");
   QByteArray data_out = file_B.readAll();

   REQUIRE(data_in == data_out);
}
