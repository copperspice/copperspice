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

#include <qline.h>

#include <cs_catch2.h>

TEST_CASE("QLine traits", "[QLine]")
{
   REQUIRE(std::is_copy_constructible_v<QLine> == true);
   REQUIRE(std::is_move_constructible_v<QLine> == true);

   REQUIRE(std::is_copy_assignable_v<QLine> == true);
   REQUIRE(std::is_move_assignable_v<QLine> == true);

   REQUIRE(std::has_virtual_destructor_v<QLine> == false);
}

TEST_CASE("QLine constructor", "[qline]")
{
   QLine data(5, 10, 100, 200);

   REQUIRE(! data.isNull());

   REQUIRE(data.x1() == 5);
   REQUIRE(data.y1() == 10);
   REQUIRE(data.x2() == 100);
   REQUIRE(data.y2() == 200);
}

TEST_CASE("QLine empty", "[qline]")
{
   QLine data;

   REQUIRE(data.isNull() == true);
}

TEST_CASE("QLine dx_dy", "[qline]")
{
   QLine data;

   {
      data.setLine(5, 50, 25, 85);

      REQUIRE(data.dx() == 20);
      REQUIRE(data.dy() == 35 );
   }
}

TEST_CASE("QLine set_line", "[qline]")
{
   QLine data;

   {
      data.setLine(5, 10, 100, 200);

      REQUIRE(data.p1() == QPoint(5, 10));
      REQUIRE(data.p2() == QPoint(100, 200));
   }
}

TEST_CASE("QLine set_p1", "[qline]")
{
   QLine data;

   {
      data.setP1(QPoint(5, 10));

      REQUIRE(data.p1() == QPoint(5, 10));
      REQUIRE(data.p2() == QPoint(0, 0));
   }
}

TEST_CASE("QLine set_p2", "[qline]")
{
   QLine data;

   {
      data.setP2(QPoint(100, 200));

      REQUIRE(data.p1() == QPoint(0, 0));
      REQUIRE(data.p2() == QPoint(100, 200));
   }
}









