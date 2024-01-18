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

#include <qpoint.h>
#include <limits>

#include <cs_catch2.h>

TEST_CASE("QPoint traits", "[QPoint]")
{
   REQUIRE(std::is_copy_constructible_v<QPoint> == true);
   REQUIRE(std::is_move_constructible_v<QPoint> == true);

   REQUIRE(std::is_copy_assignable_v<QPoint> == true);
   REQUIRE(std::is_move_assignable_v<QPoint> == true);

   REQUIRE(std::has_virtual_destructor_v<QPoint> == false);
}

TEST_CASE("QPoint constructor", "[qpoint]")
{
   QPoint data(75, 125);

   REQUIRE(! data.isNull());

   REQUIRE(data.x() == 75);
   REQUIRE(data.y() == 125);
}

TEST_CASE("QPoint empty", "[qpoint")
{
   QPoint data;

   REQUIRE(data.isNull());
}

TEST_CASE("QPoint manhattanLength", "[qpoint]")
{
   QPoint data;
   int result;

   {
      data   = QPoint(0, 0);
      result = 0;

      REQUIRE(data.manhattanLength() == result);
   }

   {
      data   = QPoint(5, 0);
      result = 5;

      REQUIRE(data.manhattanLength() == result);
   }

   {
      data   = QPoint(0, 5);
      result = 5;

      REQUIRE(data.manhattanLength() == result);
   }

   {
      data   = QPoint(15, 15);
      result = 30;

      REQUIRE(data.manhattanLength() == result);
   }

   {
      data   = QPoint(-5, -15);
      result = 20;

      REQUIRE(data.manhattanLength() == result);
   }
}

TEST_CASE("QPoint operators", "[qpoint]")
{
   QPoint data1(50, 125);
   QPoint data2(10, 20);

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

TEST_CASE("QPoint multiply", "[qpoint]")
{
   QPoint data;
   double factor;

   {
      data   = QPoint(5, 10);
      factor = 0.0;

      data *= factor;

      REQUIRE(data.x() == 0);
      REQUIRE(data.y() == 0);
   }

   {
      data   = QPoint(5, 10);
      factor = 0.501;

      data *= factor;

      REQUIRE(data.x() == 3);      // rounded up
      REQUIRE(data.y() == 5);
   }
}

TEST_CASE("QPoint divide", "[qpoint]")
{
   QPoint data;
   double factor;

   {
      data   = QPoint(5, 10);
      factor = 1;

      data /= factor;

      REQUIRE(data.x() == 5);
      REQUIRE(data.y() == 10);
   }

   {
      data   = QPoint(5, 10);
      factor = -2;

      data /= factor;

      REQUIRE(data.x() == -2);      // rounded up
      REQUIRE(data.y() == -5);
   }
}

TEST_CASE("QPoint rx_ry", "[qpoint")
{
   QPoint data1(-1, 0);

   {
      QPoint data2(data1);

      ++data2.rx();
      REQUIRE(data2.x() == data1.x() + 1);
   }

   {
      QPoint data2(data1);

      ++data2.ry();
      REQUIRE(data2.y() == data1.y() + 1);
   }
}

TEST_CASE("QPoint set_get", "[qpoint")
{
   QPoint data;

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



