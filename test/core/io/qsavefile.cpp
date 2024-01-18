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

#include <qsavefile.h>

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

TEST_CASE("QSaveFile filename", "[qsavefile]")
{
   QSaveFile file("c:/machine/path1/dir1/file1");

   REQUIRE(file.fileName() == "c:/machine/path1/dir1/file1");

   REQUIRE(file.pos() == 0);
   REQUIRE(file.size() == 0);
   REQUIRE(file.resize(10) == false);
}
