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

#include <qmargins.h>

#include <cs_catch2.h>

TEST_CASE("QMargins traits", "[qmargins]")
{
   REQUIRE(std::is_copy_constructible_v<QMargins> == true);
   REQUIRE(std::is_move_constructible_v<QMargins> == true);

   REQUIRE(std::is_copy_assignable_v<QMargins> == true);
   REQUIRE(std::is_move_assignable_v<QMargins> == true);

   REQUIRE(std::has_virtual_destructor_v<QMargins> == false);
}

TEST_CASE("QMargins constructor", "[qmargins]")
{
   QMargins data(5, 10, 100, 200);

   REQUIRE(! data.isNull());

   REQUIRE(data.left()   == 5);
   REQUIRE(data.top()    == 10);

   REQUIRE(data.right()  == 100);
   REQUIRE(data.bottom() == 200);
}

TEST_CASE("QMargin is_null", "[qmargins]")
{
   QMargins data;

   REQUIRE(data.isNull());
}

TEST_CASE("QMargin operators", "[qmargins]")
{
   QMargins data1(25, 14, 100, 50);
   QMargins data2(2, 3, 4, 5);

   SECTION ("add")
   {
      QMargins data3 =  data1 + data2;
      QMargins result(27, 17, 104, 55);

      REQUIRE(data3 == result);

      QMargins data4 = data1;
      data4 += data2;

      REQUIRE(data4 == data3);
   }

   SECTION ("subtract")
   {
      QMargins data3 =  data1 - data2;
      QMargins result(23, 11, 96, 45);

      REQUIRE(data3 == result);

      QMargins data4 = data1;
      data4 -= data2;

      REQUIRE(data4 == data3);
   }

   SECTION ("plus_minus_equals")
   {
      QMargins data3 = data1;
      QMargins result(27, 16, 102, 52);

      data3 += 2;
      REQUIRE(data3 == result);

      data3 -= 2;
      REQUIRE(data3 == data1);
   }

   SECTION ("multiple")
   {
      QMargins data3 = data1 * 2;
      QMargins result(50, 28, 200, 100);

      REQUIRE(data3 == result);
      REQUIRE(2 * data1 == data3);
      REQUIRE(qreal(2) * data1 == data3);
      REQUIRE(data1 * qreal(2) == data3);

      QMargins data4 = data1;
      data4 *= 2;

      REQUIRE(data4 == data3);

      data4 = data1;
      data4 *= qreal(2);
      REQUIRE(data4 == data3);
   }

   SECTION ("divide")
   {
      QMargins data3 = data1 / 2;
      QMargins result(12, 7, 50, 25);      // rounding for x

      REQUIRE(data3 == result);

      QMargins data4 = data1;
      data4 /= 2;
      REQUIRE(data4 == data3);

      data4 = data1;
      data4 /= 2.0;

      QMargins result2(13, 7, 50, 25);      // rounding for x
      REQUIRE(data4 == result2);
   }
}

TEST_CASE("QMargin set", "[qmargins]")
{
   QMargins data;

   data.setLeft(1000);
   data.setTop(-20000);
   data.setBottom(40000);
   data.setRight(50000);

   REQUIRE(data.left()   == 1000);
   REQUIRE(data.top()    == -20000);

   REQUIRE(data.right()  == 50000);
   REQUIRE(data.bottom() == 40000);
}
