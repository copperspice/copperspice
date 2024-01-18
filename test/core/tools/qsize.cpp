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

#include <qsize.h>

#include <cs_catch2.h>

TEST_CASE("QSize traits", "[qsize]")
{
   REQUIRE(std::is_copy_constructible_v<QSize> == true);
   REQUIRE(std::is_move_constructible_v<QSize> == true);

   REQUIRE(std::is_copy_assignable_v<QSize> == true);
   REQUIRE(std::is_move_assignable_v<QSize> == true);

   REQUIRE(std::has_virtual_destructor_v<QSize> == false);
}

TEST_CASE("QSize constructor", "[qsize]")
{
   QSize data(50, 125);

   REQUIRE(! data.isEmpty());
   REQUIRE(! data.isNull());

   REQUIRE(data.isValid());

   REQUIRE(data.width()  == 50);
   REQUIRE(data.height() == 125);
}

TEST_CASE("QSize empty", "[qsize]")
{
   QSize data;

   REQUIRE(data.isEmpty());
   REQUIRE(! data.isNull());     // default constructed (-1, -1)

   REQUIRE(! data.isValid());
}

TEST_CASE("QSize operators", "[qsize]")
{
   QSize data1(50, 125);
   QSize data2(10, 20);

   SECTION("plus") {
      data1 += data2;

      REQUIRE(data1.width()  == 60);
      REQUIRE(data1.height() == 145);
   }

   SECTION("minus") {
      data1 -= data2;

      REQUIRE(data1.width()  == 40);
      REQUIRE(data1.height() == 105);
   }
}
