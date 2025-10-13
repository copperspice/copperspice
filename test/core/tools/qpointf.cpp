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

#include <qpointf.h>

#include <cs_catch2.h>

TEST_CASE("QPointF traits", "[qpointf]")
{
   REQUIRE(std::is_copy_constructible_v<QPointF> == true);
   REQUIRE(std::is_move_constructible_v<QPointF> == true);

   REQUIRE(std::is_copy_assignable_v<QPointF> == true);
   REQUIRE(std::is_move_assignable_v<QPointF> == true);

   REQUIRE(std::has_virtual_destructor_v<QPointF> == false);
}

TEST_CASE("QPointF comparison", "[qpointf]")
{
   QPointF data_a(5, 5);
   QPointF data_b(5, 5);

   REQUIRE(data_a == data_b);
   REQUIRE((data_a != data_b) == false);

   //
   data_b = QPointF(5, 20);

   REQUIRE(data_a != data_b);
   REQUIRE((data_a == data_b) == false);
}

TEST_CASE("QPointF constructor", "[qpointf]")
{
   QPointF data(75, 125);

   REQUIRE(data.isNull() == false);

   REQUIRE(data.x() == 75);
   REQUIRE(data.y() == 125);

   //
   data = QPointF(5, -50);

   REQUIRE(data.isNull() == false);

   REQUIRE(data.x() ==  5);
   REQUIRE(data.y() == -50);
}

TEST_CASE("QPointF copy_assign", "[qpointf]")
{
   QPointF data_a(43, 72);
   QPointF data_b(data_a);

   REQUIRE(data_b.x() == 43);
   REQUIRE(data_b.y() == 72);

   //
   data_b = data_a;

   REQUIRE(data_a == data_b);
   REQUIRE(data_b == data_a);
}

TEST_CASE("QPointF dotProduct", "[qpointf]")
{
   QPointF data_a(3, 4);
   QPointF data_b(5, 6);

   REQUIRE(QPointF::dotProduct(data_a, data_b) == (3*5 + 4*6));

   //
   data_a = QPointF(8.3,  7.9);
   data_b = QPointF(-1.2, 12.5);

   qreal result_1 = QPointF::dotProduct(data_a, data_b);
   qreal result_2 = (8.3 * (-1.2) + 7.9 * 12.5);

   // 88.79
   REQUIRE_THAT(result_1, Catch::Matchers::WithinAbs(result_2, 0.01));
}

TEST_CASE("QPointF isNull", "[qpoint")
{
   QPointF data;

   REQUIRE(data.isNull() == true);

   REQUIRE(data.x() == 0);
   REQUIRE(data.y() == 0);

   //
   data = QPointF(0, 0);

   REQUIRE(data.isNull() == true);

   //
   data = QPointF(10, 0);

   REQUIRE(data.isNull() == false);
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

TEST_CASE("QPointF math", "[qpointf]")
{
   QPointF data_a(50, 125);
   QPointF data_b(10, 20);

   SECTION("plus") {
      data_a += data_b;

      REQUIRE(data_a.x() == 60);
      REQUIRE(data_a.y() == 145);

      //
      QPointF result = data_a + data_b;

      REQUIRE(result.x() == 70);
      REQUIRE(result.y() == 165);
   }

   SECTION("minus") {
      data_a -= data_b;

      REQUIRE(data_a.x() == 40);
      REQUIRE(data_a.y() == 105);

      //
      QPointF result = data_a - data_b;

      REQUIRE(result.x() == 30);
      REQUIRE(result.y() == 85);
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
      factor = 0.501;

      data *= factor;

      REQUIRE(data.x() == 2.505);
      REQUIRE(data.y() == 5.010);
   }

   {
      REQUIRE(data * 4 == QPointF(10.02, 20.04));
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

   {
      REQUIRE(data / 2 == QPointF(-1.25, -2.5));
   }
}

TEST_CASE("QPointF rx_ry", "[qpoint")
{
   QPointF data1(-1, 0);

   {
      QPointF data2(data1);
      ++data2.rx();

      REQUIRE(data2.x() == 0);
   }

   {
      QPointF data2(data1);
      ++data2.ry();

      REQUIRE(data2.y() == 1);
   }
}

TEST_CASE("QPointF set", "[qpoint")
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
