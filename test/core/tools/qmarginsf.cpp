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

#include <qmarginsf.h>

#include <cs_catch2.h>

TEST_CASE("QMarginsF traits", "[qmarginsf]")
{
   REQUIRE(std::is_copy_constructible_v<QMarginsF> == true);
   REQUIRE(std::is_move_constructible_v<QMarginsF> == true);

   REQUIRE(std::is_copy_assignable_v<QMarginsF> == true);
   REQUIRE(std::is_move_assignable_v<QMarginsF> == true);

   REQUIRE(std::has_virtual_destructor_v<QMarginsF> == false);
}

TEST_CASE("QMarginsF constructor", "[qmarginsf]")
{
   QMarginsF data(5, 10, 100, 200);

   REQUIRE(! data.isNull());

   REQUIRE(data.left()   == 5);
   REQUIRE(data.top()    == 10);

   REQUIRE(data.right()  == 100);
   REQUIRE(data.bottom() == 200);
}

TEST_CASE("QMarginsF is_null", "[qmarginsf]")
{
   QMarginsF data;

   REQUIRE(data.isNull());
}

TEST_CASE("QMarginF operators", "[qmarginsf]")
{
   QMarginsF data1(25, 14, 100, 50);
   QMarginsF data2(2, 3, 4, 5);

   SECTION ("add")
   {
      QMarginsF data3 =  data1 + data2;
      QMarginsF result(27, 17, 104, 55);

      REQUIRE(data3 == result);

      QMarginsF data4 = data1;
      data4 += data2;

      REQUIRE(data4 == data3);
   }

   SECTION ("subtract")
   {
      QMarginsF data3 =  data1 - data2;
      QMarginsF result(23, 11, 96, 45);

      REQUIRE(data3 == result);

      QMarginsF data4 = data1;
      data4 -= data2;

      REQUIRE(data4 == data3);
   }

   SECTION ("plus_minus_equals")
   {
      QMarginsF data3 = data1;
      QMarginsF result(27, 16, 102, 52);

      data3 += 2;
      REQUIRE(data3 == result);

      data3 -= 2;
      REQUIRE(data3 == data1);
   }

   SECTION ("multiple")
   {
      QMarginsF data3 = data1 * 2;
      QMarginsF result(50, 28, 200, 100);

      REQUIRE(data3 == result);
      REQUIRE(2 * data1 == data3);
      REQUIRE(qreal(2) * data1 == data3);
      REQUIRE(data1 * qreal(2) == data3);

      QMarginsF data4 = data1;
      data4 *= 2;

      REQUIRE(data4 == data3);

      data4 = data1;
      data4 *= qreal(2);
      REQUIRE(data4 == data3);
   }

   SECTION ("divide")
   {
      QMarginsF data3 = data1 / 2;
      QMarginsF result(12.5, 7, 50, 25);

      REQUIRE(data3 == result);

      QMarginsF data4 = data1;
      data4 /= 2;
      REQUIRE(data4 == data3);

      data4 = data1;
      data4 /= 2.0;

      QMarginsF result2(12.5, 7, 50, 25);
      REQUIRE(data4 == result2);
   }
}

TEST_CASE("QMarginF set", "[qmarginsf]")
{
   QMarginsF data;

   data.setLeft(1000);
   data.setTop(-20000);
   data.setBottom(40000);
   data.setRight(50000);

   REQUIRE(data.left()   == 1000);
   REQUIRE(data.top()    == -20000);

   REQUIRE(data.right()  == 50000);
   REQUIRE(data.bottom() == 40000);
}
