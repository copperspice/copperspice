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

#include <qlinef.h>

#include <cs_catch2.h>

TEST_CASE("QLineF traits", "[qlinef]")
{
   REQUIRE(std::is_copy_constructible_v<QLineF> == true);
   REQUIRE(std::is_move_constructible_v<QLineF> == true);

   REQUIRE(std::is_copy_assignable_v<QLineF> == true);
   REQUIRE(std::is_move_assignable_v<QLineF> == true);

   REQUIRE(std::has_virtual_destructor_v<QLineF> == false);
}

TEST_CASE("QLineF constructor", "[qlinef]")
{
   QLineF data(5, 10, 100, 200);

   REQUIRE(! data.isNull());

   REQUIRE(data.x1() == 5);
   REQUIRE(data.y1() == 10);
   REQUIRE(data.x2() == 100);
   REQUIRE(data.y2() == 200);
}

TEST_CASE("QLineF empty", "[qlinef]")
{
   QLineF data;

   REQUIRE(data.isNull());
}

TEST_CASE("QLineF dx_dy", "[qlinef]")
{
   QLineF data;

   {
      data.setLine(5.5, 50, 25, 85.5);

      REQUIRE(data.dx() == 19.50);
      REQUIRE(data.dy() == 35.5 );
   }
}

TEST_CASE("QLineF set_line", "[qlinef]")
{
   QLineF data;

   {
      data.setLine(5, 10, 100, 200);

      REQUIRE(data.p1() == QPointF(5, 10));
      REQUIRE(data.p2() == QPointF(100, 200));
   }
}

TEST_CASE("QLineF set_p1", "[qlinef]")
{
   QLineF data;

   {
      data.setP1(QPointF(5, 10));

      REQUIRE(data.p1() == QPoint(5, 10));
      REQUIRE(data.p2() == QPoint(0, 0));
   }
}

TEST_CASE("QLineF set_p2", "[qlinef]")
{
   QLineF data;

   {
      data.setP2(QPointF(100, 200));

      REQUIRE(data.p1() == QPointF(0, 0));
      REQUIRE(data.p2() == QPointF(100, 200));
   }
}
