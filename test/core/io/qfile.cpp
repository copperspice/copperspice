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

#include <qbytearray.h>
#include <qdir.h>
#include <qfile.h>
#include <qtemporaryfile.h>

#include <cs_catch2.h>

TEST_CASE("QFile traits", "[qfile]")
{
   REQUIRE(std::is_copy_constructible_v<QFile> == false);
   REQUIRE(std::is_move_constructible_v<QFile> == false);

   REQUIRE(std::is_copy_assignable_v<QFile> == false);
   REQUIRE(std::is_move_assignable_v<QFile> == false);

   REQUIRE(std::has_virtual_destructor_v<QFile> == true);
}

TEST_CASE("QFile exists", "[qfile]")
{
   QFile file;

   REQUIRE(file.exists() == false);
   REQUIRE(file.open(QIODevice::ReadOnly) == false);

   REQUIRE(file.pos() == 0);
   REQUIRE(file.size() == 0);
   REQUIRE(file.resize(10) == false);
}

TEST_CASE("QFile file_name", "[qfile]")
{
   QFile file("c:/machine/path1/dir1/file1");

   REQUIRE(file.fileName() == "c:/machine/path1/dir1/file1");

   REQUIRE(file.pos() == 0);
   REQUIRE(file.size() == 0);
   REQUIRE(file.resize(10) == false);
}

TEST_CASE("QFile size", "[qfile]")
{
   QString fileName = "MyFileName.cfg";
   QString str(QDir::tempPath() + "/" + fileName);

   QTemporaryFile tmpFile(str);
   tmpFile.open();

   qint64 size = tmpFile.write("a", 1);

   REQUIRE(size == 1);
   REQUIRE(tmpFile.size() == 1);

   tmpFile.resize(10);
   REQUIRE(tmpFile.size() == 10);
}

TEST_CASE("QFile read", "[qfile]")
{
   QString fileName = "MyFileName.cfg";
   QString str(QDir::tempPath() + "/" + fileName);

   QTemporaryFile tmpFile(str);
   tmpFile.open();

   tmpFile.write("CopperSpice", 11);

   REQUIRE((tmpFile.permissions() & QFileDevice::ReadOwner)  == QFileDevice::ReadOwner);
   REQUIRE((tmpFile.permissions() & QFileDevice::WriteOwner) == QFileDevice::WriteOwner);

   {
      tmpFile.seek(0);
      QByteArray data = tmpFile.readAll();

      REQUIRE(data == "CopperSpice");
   }

   {
      tmpFile.seek(0);
      QByteArray data = tmpFile.read(4096);

      REQUIRE(data == "CopperSpice");
   }
}

