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

#include <qtemporarydir.h>

#include <cs_catch2.h>

TEST_CASE("QTemporaryDir traits", "[qtemporarydir]")
{
   REQUIRE(std::is_copy_constructible_v<QTemporaryDir> == false);
   REQUIRE(std::is_move_constructible_v<QTemporaryDir> == false);

   REQUIRE(std::is_copy_assignable_v<QTemporaryDir> == false);
   REQUIRE(std::is_move_assignable_v<QTemporaryDir> == false);

   REQUIRE(std::has_virtual_destructor_v<QTemporaryDir> == false);
}

TEST_CASE("QTemporaryDir auto_remove", "[qtemporarydir]")
{
   QTemporaryDir tmpDir;

   tmpDir.setAutoRemove(false);
   REQUIRE(tmpDir.autoRemove() == false);

   tmpDir.setAutoRemove(true);
   REQUIRE(tmpDir.autoRemove() == true);
}

TEST_CASE("QTemporaryDir constructor", "[qtemporarydir]")
{
   QTemporaryDir tmpDir;
   QString tmpPath = QDir::tempPath();

   REQUIRE(tmpDir.path().left(tmpPath.size()) == tmpPath);
   REQUIRE(QFileInfo(tmpDir.path()).isDir() == true);
   REQUIRE(tmpDir.errorString() == QString());
}

TEST_CASE("QTemporaryDir remove", "[qtemporarydir]")
{
   QTemporaryDir tmpDir;

   REQUIRE(tmpDir.isValid() == true);
   REQUIRE(tmpDir.remove() == true);
}
