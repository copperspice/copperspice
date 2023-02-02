/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#include <qfuturewatcher.h>

#include <cs_catch2.h>

TEST_CASE("QFutureWatcher traits", "[qfuturewatcher]")
{
   REQUIRE(std::is_copy_constructible_v<QFutureWatcher<int>> == false);
   REQUIRE(std::is_move_constructible_v<QFutureWatcher<int>> == false);

   REQUIRE(std::is_copy_assignable_v<QFutureWatcher<int>> == false);
   REQUIRE(std::is_move_assignable_v<QFutureWatcher<int>> == false);

   REQUIRE(std::has_virtual_destructor_v<QFutureWatcher<int>> == true);
}
