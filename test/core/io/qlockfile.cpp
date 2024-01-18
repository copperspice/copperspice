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

#include <qlockfile.h>

#include <cs_catch2.h>

TEST_CASE("QLockFile traits", "[qlockfile]")
{
   REQUIRE(std::is_copy_constructible_v<QLockFile> == false);
   REQUIRE(std::is_move_constructible_v<QLockFile> == false);

   REQUIRE(std::is_copy_assignable_v<QLockFile> == false);
   REQUIRE(std::is_move_assignable_v<QLockFile> == false);

   REQUIRE(std::has_virtual_destructor_v<QLockFile> == false);
}

TEST_CASE("QLockFile basic", "[qlockfile]")
{
  QLockFile f("testfile");

  REQUIRE(f.error() == QLockFile::NoError);
  REQUIRE(f.isLocked() == false);

  REQUIRE(f.lock() == true);
  REQUIRE(f.isLocked() == true);

  REQUIRE(f.tryLock() == false);
  REQUIRE(f.isLocked() == true);

  f.unlock();

  REQUIRE(f.isLocked() == false);
  REQUIRE(f.tryLock() == true);
  REQUIRE(f.isLocked() == true);
}

TEST_CASE("QLockFile stale_lock_time", "[qlockfile]")
{
   QLockFile f("testfile");

   f.setStaleLockTime(45);

   REQUIRE(f.staleLockTime() == 45);
}
