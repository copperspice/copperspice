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

#include <qrect.h>

#include <cs_catch2.h>

TEST_CASE("QRect traits", "[QRect]")
{
   REQUIRE(std::is_copy_constructible_v<QRect> == true);
   REQUIRE(std::is_move_constructible_v<QRect> == true);

   REQUIRE(std::is_copy_assignable_v<QRect> == true);
   REQUIRE(std::is_move_assignable_v<QRect> == true);

   REQUIRE(std::has_virtual_destructor_v<QRect> == false);
}

TEST_CASE("QRect constructor", "[qrect]")
{
   QRect data(5, 10, 100, 200);

   REQUIRE(! data.isEmpty());
   REQUIRE(! data.isNull());

   REQUIRE(data.isValid());

   REQUIRE(data.left()   == 5);
   REQUIRE(data.top()    == 10);

   REQUIRE(data.width()  == 100);
   REQUIRE(data.height() == 200);

   REQUIRE(data.right()  == 104);
   REQUIRE(data.bottom() == 209);
}

TEST_CASE("QRect contains", "[qrect]")
{
   QRect data(5, 10, 100, 200);

   REQUIRE(data.contains(5, 10));
   REQUIRE(data.contains(20, 20));

   REQUIRE(! data.contains(5, 5));
}

TEST_CASE("QRect empty", "[qrect]")
{
   QRect data;

   REQUIRE(data.isEmpty());
   REQUIRE(data.isNull());

   REQUIRE(! data.isValid());
}
