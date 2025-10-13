/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

TEST_CASE("QPoint traits", "[qpoint]")
{
   REQUIRE(std::is_copy_constructible_v<QPoint> == true);
   REQUIRE(std::is_move_constructible_v<QPoint> == true);

   REQUIRE(std::is_copy_assignable_v<QPoint> == true);
   REQUIRE(std::is_move_assignable_v<QPoint> == true);

   REQUIRE(std::has_virtual_destructor_v<QPoint> == false);
}

TEST_CASE("QPoint comparison", "[qpoint]")
{
   QPoint data_a(5, 5);
   QPoint data_b(5, 5);

   REQUIRE(data_a == data_b);
   REQUIRE((data_a != data_b) == false);

   //
   data_b = QPoint(5, 20);

   REQUIRE(data_a != data_b);
   REQUIRE((data_a == data_b) == false);
}

TEST_CASE("QPoint constructor", "[qpoint]")
{
   QPoint data(75, 125);

   REQUIRE(data.isNull() == false);

   REQUIRE(data.x() == 75);
   REQUIRE(data.y() == 125);

   //
   data = QPoint(5, -50);

   REQUIRE(data.isNull() == false);

   REQUIRE(data.x() ==  5);
   REQUIRE(data.y() == -50);
}

TEST_CASE("QPoint copy_assign", "[qpoint]")
{
   QPoint data_a(43, 72);
   QPoint data_b(data_a);

   REQUIRE(data_b.x() == 43);
   REQUIRE(data_b.y() == 72);

   //
   data_b = data_a;

   REQUIRE(data_a == data_b);
   REQUIRE(data_b == data_a);
}

TEST_CASE("QPoint dotProduct", "[qpoint]")
{
   QPoint data_a(3, 4);
   QPoint data_b(5, 6);

   REQUIRE(QPoint::dotProduct(data_a, data_b) == (3*5 + 4*6));

   //
   data_a = QPoint(-2, 5);
   data_b = QPoint(4, -3);

   REQUIRE(QPoint::dotProduct(data_a, data_b) == (-2*4 + 5*(-3)));
}

TEST_CASE("QPoint isNull", "[qpoint")
{
   QPoint data;

   REQUIRE(data.isNull() == true);

   REQUIRE(data.x() == 0);
   REQUIRE(data.y() == 0);

   //
   data = QPoint(0, 0);

   REQUIRE(data.isNull() == true);

   //
   data = QPoint(10, 0);

   REQUIRE(data.isNull() == false);
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

TEST_CASE("QPoint math", "[qpoint]")
{
   QPoint data_a(50, 125);
   QPoint data_b(10, 20);

   SECTION("plus") {
      data_a += data_b;

      REQUIRE(data_a.x() == 60);
      REQUIRE(data_a.y() == 145);

      //
      QPoint result = data_a + data_b;

      REQUIRE(result.x() == 70);
      REQUIRE(result.y() == 165);
   }

   SECTION("minus") {
      data_a -= data_b;

      REQUIRE(data_a.x() == 40);
      REQUIRE(data_a.y() == 105);

      //
      QPoint result = data_a - data_b;

      REQUIRE(result.x() == 30);
      REQUIRE(result.y() == 85);
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

   {
      REQUIRE(data * 4 == QPoint(12, 20));
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

   {
      REQUIRE(data / 2 == QPoint(-1, -2));
   }
}

TEST_CASE("QPoint rx_ry", "[qpoint")
{
   QPoint data1(-1, 0);

   {
      QPoint data2(data1);
      ++data2.rx();

      REQUIRE(data2.x() == 0);
   }

   {
      QPoint data2(data1);
      ++data2.ry();

      REQUIRE(data2.y() == 1);
   }
}

TEST_CASE("QPoint set", "[qpoint")
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
