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

#include <qline.h>

#include <cs_catch2.h>

TEST_CASE("QLine traits", "[qline]")
{
   REQUIRE(std::is_copy_constructible_v<QLine> == true);
   REQUIRE(std::is_move_constructible_v<QLine> == true);

   REQUIRE(std::is_copy_assignable_v<QLine> == true);
   REQUIRE(std::is_move_assignable_v<QLine> == true);

   REQUIRE(std::has_virtual_destructor_v<QLine> == false);
}

TEST_CASE("QLine center", "[qline]")
{
   QLine data(5, 10, 100, 200);

   QPoint result = data.center();

   REQUIRE(result == QPoint(52, 105));
}

TEST_CASE("QLine constructor", "[qline]")
{
   QLine data(5, 10, 100, 200);

   REQUIRE(data.isNull() == false);

   REQUIRE(data.x1() == 5);
   REQUIRE(data.y1() == 10);
   REQUIRE(data.x2() == 100);
   REQUIRE(data.y2() == 200);

   //
   QPoint p1(10, 20);
   QPoint p2(35, 45);

   data = QLine(p1, p2);

   REQUIRE(data.isNull() == false);

   REQUIRE(data.x1() == 10);
   REQUIRE(data.y1() == 20);
   REQUIRE(data.x2() == 35);
   REQUIRE(data.y2() == 45);

   REQUIRE(data.p1() == p1);
   REQUIRE(data.p2() == p2);
}

TEST_CASE("QLine copy_assign", "[qline]")
{
   QLine data_a(10, 20, 30, 40);
   QLine data_b(data_a);

   REQUIRE(data_a == data_b);
   REQUIRE(data_b == QLine{10, 20, 30, 40});

   //
   QLine data_c;
   data_c = data_a;

   REQUIRE(data_a == data_c);
   REQUIRE(data_c == QLine{10, 20, 30, 40});
}

TEST_CASE("QLine dx_dy", "[qline]")
{
   QLine data;
   data.setLine(5, 50, 25, 85);

   REQUIRE(data.x1() == 5);
   REQUIRE(data.y1() == 50);
   REQUIRE(data.x2() == 25);
   REQUIRE(data.y2() == 85);

   REQUIRE(data.dx() == 20);
   REQUIRE(data.dy() == 35);

   REQUIRE(data.p1() == QPoint(5,  50));
   REQUIRE(data.p2() == QPoint(25, 85));

   //
   data = QLine(5, 7, -32, -46);

   REQUIRE(data.dx() == -37);
   REQUIRE(data.dy() == -53);
}

TEST_CASE("QLine isNull", "[qline]")
{
   QLine data;

   REQUIRE(data.isNull() == true);

   REQUIRE(data.x1() == 0);
   REQUIRE(data.y1() == 0);
   REQUIRE(data.x2() == 0);
   REQUIRE(data.y2() == 0);

   //
   data = QLine(15, 15, 15, 15);

   REQUIRE(data.isNull() == true);

   REQUIRE(data.x1() == 15);
   REQUIRE(data.y1() == 15);
   REQUIRE(data.x2() == 15);
   REQUIRE(data.y2() == 15);

   REQUIRE(data.dx() == 0);
   REQUIRE(data.dy() == 0);

   //
   data = QLine(15, 15, 16, 15);

   REQUIRE(data.isNull() == false);

   REQUIRE(data.x1() == 15);
   REQUIRE(data.y1() == 15);
   REQUIRE(data.x2() == 16);
   REQUIRE(data.y2() == 15);
}

TEST_CASE("QLine move_assign", "[qline]")
{
   QLine data_a(10, 20, 30, 40);
   QLine data_b(std::move(data_a));

   REQUIRE(data_b == QLine{10, 20, 30, 40});

   //
   QLine data_c;
   data_c = std::move(data_b);

   REQUIRE(data_c == QLine{10, 20, 30, 40});
}

TEST_CASE("QLine operators", "[qline]")
{
   QLine data_a(10, 20, 30, 40);
   QLine data_b(10, 20, 30, 40);

   REQUIRE(data_a == data_b);

   //
   QLine data_c(10, 20, 30, 50);

   REQUIRE(data_a != data_c);
   REQUIRE(data_c != data_a);
}

TEST_CASE("QLine set_p1_p2", "[qline]")
{
   QLine data(1, 2, 3, 4);

   data.setP1(QPoint(17, 26));
   data.setP2(QPoint(32, 48));

   REQUIRE(data.x1() == 17);
   REQUIRE(data.y1() == 26);
   REQUIRE(data.x2() == 32);
   REQUIRE(data.y2() == 48);

   REQUIRE(data.dx() == 15);
   REQUIRE(data.dy() == 22);

   REQUIRE(data.p1() == QPoint(17, 26));
   REQUIRE(data.p2() == QPoint(32, 48));

   //
   data.setPoints(QPoint(18, 36), QPoint(7, -3));

   REQUIRE(data.p1() == QPoint(18,  36));
   REQUIRE(data.p2() == QPoint(7,  -3));
}

TEST_CASE("QLine set_p1", "[qline]")
{
   QLine data;
   data.setP1(QPoint(5, 10));

   REQUIRE(data.p1() == QPoint(5, 10));
   REQUIRE(data.p2() == QPoint(0, 0));

   //
   data.setP1(QPoint(-5, -10));

   REQUIRE(data.p1() == QPoint(-5, -10));
   REQUIRE(data.p2() == QPoint(0, 0));
}

TEST_CASE("QLine set_p2", "[qline]")
{
   QLine data;
   data.setP2(QPoint(100, 200));

   REQUIRE(data.p1() == QPoint(0, 0));
   REQUIRE(data.p2() == QPoint(100, 200));
}

TEST_CASE("QLine translate", "[qline]")
{
   QLine data(10, 25, 30, 16);

   //
   data.translate(13, -17);

   REQUIRE(data.p1() == QPoint(23,  8));
   REQUIRE(data.p2() == QPoint(43, -1));

   //
   data = QLine(0, 0, 18, 26);
   data.translate(QPoint(9, 8));

   REQUIRE(data == QLine(9, 8, 27, 34));
}

TEST_CASE("QLine translated", "[qline]")
{
   QLine data   = QLine(10, 20, 30, 40);
   QLine result = data.translated(5, 3);

   REQUIRE(result == QLine(15, 23, 35, 43));
   REQUIRE(data   == QLine(10, 20, 30, 40));

   //
   data = QLine(10, 10, 20, 20);

   REQUIRE(data.translated(QPoint(-5, 12)) == QLine(5, 22, 15, 32));
}
