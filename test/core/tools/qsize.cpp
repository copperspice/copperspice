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

   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.isNull()  == false);
   REQUIRE(data.isValid() == true);

   REQUIRE(data.width()  == 50);
   REQUIRE(data.height() == 125);

   //
   data = QSize(-25, -10);

   REQUIRE(data.isEmpty() == true);
   REQUIRE(data.isNull()  == false);
   REQUIRE(data.isValid() == false);

   REQUIRE(data.width()  == -25);
   REQUIRE(data.height() == -10 );
}

TEST_CASE("QSize copy_assign", "[qsize]")
{
   QSize data_a(38, 72);
   QSize data_b(data_a);

   REQUIRE(data_a == data_b);
   REQUIRE(data_b == QSize{38, 72});

   //
   QSize data_c;
   data_c = data_a;

   REQUIRE(data_a == data_c);
   REQUIRE(data_c == QSize{38, 72});
}

TEST_CASE("QSize empty", "[qsize]")
{
   QSize data;

   REQUIRE(data.isEmpty() == true);
   REQUIRE(data.isNull()  == false);
   REQUIRE(data.isValid() == false);

   REQUIRE(data.width()  == -1);
   REQUIRE(data.height() == -1);

   REQUIRE(QSize(0, 5).isEmpty() == true);
   REQUIRE(QSize(5, 0).isEmpty() == true);

   REQUIRE(QSize(0, 5).isNull()  == false);
   REQUIRE(QSize(5, 0).isNull()  == false);

   REQUIRE(QSize(0, 5).isValid() == true);
   REQUIRE(QSize(5, 0).isValid() == true);
}

TEST_CASE("QSize bound_expandedTo", "[qsize]")
{
   QSize data_a(53, 60);
   QSize data_b(80, 35);

   QSize result;

   //
   result = data_a.expandedTo(data_b);

   REQUIRE(result == QSize(80, 60));

   //
   result = data_a.boundedTo(data_b);

   REQUIRE(result == QSize(53, 35));
}

TEST_CASE("QSize move_assign", "[qsize]")
{
   QSize data_a(38, 72);
   QSize data_b(std::move(data_a));

   REQUIRE(data_b == QSize{38, 72});

   //
   QSize data_c;
   data_c = std::move(data_b);

   REQUIRE(data_c == QSize{38, 72});
}

TEST_CASE("QSize operators", "[qsize]")
{
   QSize data_a(10, 20);
   QSize data_b(10, 20);
   QSize data_c(50, 20);

   //
   REQUIRE(data_a == data_b);
   REQUIRE(data_a != data_c);

   REQUIRE((data_a != data_b) == false);
   REQUIRE((data_a == data_c) == false);

   //
   data_a += QSize(15, 20);

   REQUIRE(data_a == QSize(25, 40));
   REQUIRE(data_a.width()  == 25);
   REQUIRE(data_a.height() == 40);

   //
   data_a -= QSize(5, 10);

   REQUIRE(data_a == QSize(20, 30));

   //
   data_a *= 2;

   REQUIRE(data_a == QSize(40, 60));
   REQUIRE(data_a.width()  == 40);
   REQUIRE(data_a.height() == 60);

   //
   data_a /= 4;

   REQUIRE(data_a == QSize(10, 15));
   REQUIRE(data_a.width()  == 10);
   REQUIRE(data_a.height() == 15);

   //
   data_a = QSize(10, 20);
   data_b = QSize(30, 40);

   QSize result;

   //
   result = data_a + data_b;

   REQUIRE(result.width()  == 40);
   REQUIRE(result.height() == 60);

   //
   result = data_b - data_a;

   REQUIRE(result.width()  == 20);
   REQUIRE(result.height() == 20);

   //
   result = data_a * 2;

   REQUIRE(result.width()  == 20);
   REQUIRE(result.height() == 40);

   //
   result = data_b / 3;

   REQUIRE(result.width()  == 10);
   REQUIRE(result.height() == 13);
}

TEST_CASE("QSize set", "[qsize]")
{
   QSize data(30, 40);

   //
   data.setWidth(100);

   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.isValid() == true);

   REQUIRE(data.width()  == 100);
   REQUIRE(data.height() == 40);

   //
   data.setHeight(75);

   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.isValid() == true);

   REQUIRE(data.width()  == 100);
   REQUIRE(data.height() == 75);

   //
   data.setWidth(-1);

   REQUIRE(data.isEmpty() == true);
   REQUIRE(data.isValid() == false);

   REQUIRE(data.width()  == -1);
   REQUIRE(data.height() == 75);
}

TEST_CASE("QSize transpose", "[qsize]")
{
   QSize data_a(40, 75);

   //
   REQUIRE(data_a.width()  == 40);
   REQUIRE(data_a.height() == 75);

   data_a.transpose();

   REQUIRE(data_a.width()  == 75);
   REQUIRE(data_a.height() == 40);

   //
   QSize data_b = data_a.transposed();

   REQUIRE(data_a.width()  == 75);
   REQUIRE(data_a.height() == 40);

   REQUIRE(data_b.width()  == 40);
   REQUIRE(data_b.height() == 75);
}
