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

#include <qpointf.h>

#include <cs_catch2.h>

TEST_CASE("QPointF traits", "[QPointF]")
{
   REQUIRE(std::is_copy_constructible_v<QPointF> == true);
   REQUIRE(std::is_move_constructible_v<QPointF> == true);

   REQUIRE(std::is_copy_assignable_v<QPointF> == true);
   REQUIRE(std::is_move_assignable_v<QPointF> == true);

   REQUIRE(std::has_virtual_destructor_v<QPointF> == false);
}

TEST_CASE("QPointF constructor", "[qpointf]")
{
   QPointF data(75, 125);

   REQUIRE(! data.isNull());

   REQUIRE(data.x() == 75);
   REQUIRE(data.y() == 125);
}

TEST_CASE("QPointF empty", "[qpointf]")
{
   QPointF data;

   REQUIRE(data.isNull());
}

TEST_CASE("QPointF manhattanLength", "[qpointf]")
{
   QPointF data;
   int result;

   {
      data   = QPointF(0, 0);
      result = 0;

      REQUIRE(data.manhattanLength() == result);
   }

   {
      data   = QPointF(5, 0);
      result = 5;

      REQUIRE(data.manhattanLength() == result);
   }

   {
      data   = QPointF(0, 5);
      result = 5;

      REQUIRE(data.manhattanLength() == result);
   }

   {
      data   = QPointF(15, 15);
      result = 30;

      REQUIRE(data.manhattanLength() == result);
   }

   {
      data   = QPointF(-5, -15);
      result = 20;

      REQUIRE(data.manhattanLength() == result);
   }
}

TEST_CASE("QPointF operators", "[qpointf]")
{
   QPointF data1(50, 125);
   QPointF data2(10, 20);

   SECTION("plus") {
      data1 += data2;

      REQUIRE(data1.x()  == 60);
      REQUIRE(data1.y() == 145);
   }

   SECTION("minus") {
      data1 -= data2;

      REQUIRE(data1.x()  == 40);
      REQUIRE(data1.y() == 105);
   }
}

TEST_CASE("QPointF multiply", "[qpointf]")
{
   QPointF data;
   double factor;

   {
      data   = QPointF(5, 10);
      factor = 0.0;

      data *= factor;

      REQUIRE(data.x() == 0);
      REQUIRE(data.y() == 0);
   }

   {
      data   = QPointF(5, 10);
      factor = 0.50;

      data *= factor;

      REQUIRE(data.x() == 2.5);
      REQUIRE(data.y() == 5.0);
   }
}

TEST_CASE("QPointF divide", "[qpointf]")
{
   QPointF data;
   double factor;

   {
      data   = QPointF(5, 10);
      factor = 1;

      data /= factor;

      REQUIRE(data.x() == 5);
      REQUIRE(data.y() == 10);
   }

   {
      data   = QPointF(5, 10);
      factor = -2;

      data /= factor;

      REQUIRE(data.x() == -2.5);
      REQUIRE(data.y() == -5);
   }
}

TEST_CASE("QPointF rx_ry", "[qpoint")
{
   QPointF data1(-1, 0);

   {
      QPointF data2(data1);

      ++data2.rx();
      REQUIRE(data2.x() == data1.x() + 1);
   }

   {
      QPointF data2(data1);

      ++data2.ry();
      REQUIRE(data2.y() == data1.y() + 1);
   }
}

TEST_CASE("QPointF set_get", "[qpointf]")
{
   QPointF data;

   {
      data.setX(0);
      REQUIRE(data.x() == 0);

      data.setY(0);
      REQUIRE(data.y() == 0);
   }

   {
      int minValue = std::numeric_limits<int>::min();

      data.setX(minValue);
      REQUIRE(data.x() == minValue);

      data.setY(minValue);
      REQUIRE(data.y() ==  minValue);
   }

   {
      int maxValue = std::numeric_limits<int>::max();

      data.setX(maxValue);
      REQUIRE(data.x() == maxValue);

      data.setY(maxValue);
      REQUIRE(data.y() ==  maxValue);
   }
}

