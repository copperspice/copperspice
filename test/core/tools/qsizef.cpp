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

#include <qsizef.h>

#include <cs_catch2.h>

TEST_CASE("QSizeF traits", "[qsizef]")
{
   REQUIRE(std::is_copy_constructible_v<QSizeF> == true);
   REQUIRE(std::is_move_constructible_v<QSizeF> == true);

   REQUIRE(std::is_copy_assignable_v<QSizeF> == true);
   REQUIRE(std::is_move_assignable_v<QSizeF> == true);

   REQUIRE(std::has_virtual_destructor_v<QSizeF> == false);
}

TEST_CASE("QSizeF bound_expandedTo", "[qsizef]")
{
   QSizeF data_a(53, 60);
   QSizeF data_b(80, 35);

   QSizeF result;

   //
   result = data_a.expandedTo(data_b);

   REQUIRE(result == QSizeF(80, 60));

   //
   result = data_a.boundedTo(data_b);

   REQUIRE(result == QSizeF(53, 35));
}

TEST_CASE("QSizeF constructor", "[qsizef]")
{
   QSizeF data(50, 125.5);

   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.isNull()  == false);
   REQUIRE(data.isValid() == true);

   REQUIRE(data.width()  == 50);
   REQUIRE(data.height() == 125.5);

   //
   data = QSizeF(-25, -10);

   REQUIRE(data.isEmpty() == true);
   REQUIRE(data.isNull()  == false);
   REQUIRE(data.isValid() == false);

   REQUIRE(data.width()  == -25);
   REQUIRE(data.height() == -10 );
}

TEST_CASE("QSizeF copy_assign", "[qsizef]")
{
   QSizeF data_a(10, 20.3);
   QSizeF data_b(data_a);

   REQUIRE(data_a == data_b);
   REQUIRE(data_b == QSizeF{10, 20.3});

   //
   QSizeF data_c;
   data_c = data_a;

   REQUIRE(data_a == data_c);
   REQUIRE(data_c == QSizeF{10, 20.3});
}

TEST_CASE("QSizeF empty", "[qsizef]")
{
   QSizeF data;

   REQUIRE(data.isEmpty() == true);
   REQUIRE(data.isNull()  == false);
   REQUIRE(data.isValid() == false);

   REQUIRE(data.width()  == -1);
   REQUIRE(data.height() == -1);

   REQUIRE(QSizeF(0, 5).isEmpty() == true);
   REQUIRE(QSizeF(5, 0).isEmpty() == true);

   REQUIRE(QSizeF(0, 5).isNull()  == false);
   REQUIRE(QSizeF(5, 0).isNull()  == false);

   REQUIRE(QSizeF(0, 5).isValid() == true);
   REQUIRE(QSizeF(5, 0).isValid() == true);
}

TEST_CASE("QSizeF move_assign", "[qsizef]")
{
   QSizeF data_a(38.9, 72);
   QSizeF data_b(std::move(data_a));

   REQUIRE(data_b == QSizeF{38.9, 72});

   //
   QSizeF data_c;
   data_c = std::move(data_b);

   REQUIRE(data_c == QSizeF{38.9, 72});
}

TEST_CASE("QSizeF operators", "[qsizef]")
{
   QSizeF data_a(10, 20);
   QSizeF data_b(10, 20);
   QSizeF data_c(50, 20);

   //
   REQUIRE(data_a == data_b);
   REQUIRE(data_a != data_c);

   REQUIRE((data_a != data_b) == false);
   REQUIRE((data_a == data_c) == false);

   //
   data_a += QSizeF(15, 20);

   REQUIRE(data_a == QSizeF(25, 40));
   REQUIRE(data_a.width()  == 25);
   REQUIRE(data_a.height() == 40);

   //
   data_a -= QSizeF(5, 10);

   REQUIRE(data_a == QSizeF(20, 30));

   //
   data_a *= 2;

   REQUIRE(data_a == QSizeF(40, 60));
   REQUIRE(data_a.width()  == 40);
   REQUIRE(data_a.height() == 60);

   //
   data_a /= 4;

   REQUIRE(data_a == QSizeF(10, 15));
   REQUIRE(data_a.width()  == 10);
   REQUIRE(data_a.height() == 15);

   //
   data_a = QSizeF(10, 20);
   data_b = QSizeF(30, 40);

   QSizeF result;

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
   REQUIRE_THAT(result.height(), Catch::Matchers::WithinAbs(13.33, 0.01));
}

TEST_CASE("QSizeF set", "[qsizef]")
{
   QSizeF data(30, 40);

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

TEST_CASE("QSizeF transpose", "[qsizef]")
{
   QSizeF data_a(40.1, 75);

   //
   REQUIRE(data_a.width()  == 40.1);
   REQUIRE(data_a.height() == 75);

   data_a.transpose();

   REQUIRE(data_a.width()  == 75);
   REQUIRE(data_a.height() == 40.1);

   //
   QSizeF data_b = data_a.transposed();

   REQUIRE(data_a.width()  == 75);
   REQUIRE(data_a.height() == 40.1);

   REQUIRE(data_b.width()  == 40.1);
   REQUIRE(data_b.height() == 75);
}
