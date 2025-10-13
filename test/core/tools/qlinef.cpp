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

TEST_CASE("QLineF angle", "[qlinef]")
{
   QLineF data_a(0, 0,  0, 1);
   QLineF data_b(0, 0, -1, 0);
   QLineF data_c(0, 0,  1, 1);

   REQUIRE(data_a.angle() == 270.0);             // counter clock wise
   REQUIRE(data_b.angle() == 180.0);
   REQUIRE(data_c.angle() == 315.0);

   //
   QLineF data(0, 0, 1, 0);

   data.setAngle(90);

   REQUIRE(data.p1() == QPointF(0, 0));

   REQUIRE_THAT(data.p2().x(), Catch::Matchers::WithinAbs(0.0, 0.01));
   REQUIRE(data.p2().y() == -1.0);
}

TEST_CASE("QLineF angleTo", "[qlinef]")
{
   QLineF data_a(0, 0, 1, 0);
   QLineF data_b(0, 0, 0, 1);

   REQUIRE(data_a.angleTo(data_b) == 270.0);     // counter clock wise

   //
   data_a = QLineF(0, 0,  1, 0);
   data_b = QLineF(0, 0, -1, 0);

   REQUIRE(data_a.angleTo(data_b) == 180.0);
}

TEST_CASE("QLineF center", "[qline]")
{
   QLineF data(5, 10, 100, 200);

   QPointF result = data.center();

   REQUIRE(result == QPointF(52.5, 105));
}

TEST_CASE("QLineF constructor", "[qlinef]")
{
   QLineF data(5, 10, 100.3, 200);

   REQUIRE(data.isNull() == false);

   REQUIRE(data.x1() == 5);
   REQUIRE(data.y1() == 10);
   REQUIRE(data.x2() == 100.3);
   REQUIRE(data.y2() == 200);

   //
   QPointF p1(10.8, 20);
   QPointF p2(35,   45.6);

   data = QLineF(p1, p2);

   REQUIRE(data.isNull() == false);

   REQUIRE(data.x1() == 10.8);
   REQUIRE(data.y1() == 20);
   REQUIRE(data.x2() == 35);
   REQUIRE(data.y2() == 45.6);

   REQUIRE(data.p1() == p1);
   REQUIRE(data.p2() == p2);
}

TEST_CASE("QLineF copy_assign", "[qlinef]")
{
   QLineF data_a(10, 20, 30, 40);
   QLineF data_b(data_a);

   REQUIRE(data_b == data_a);

   //
   QLineF data_c;

   data_c = data_a;

   REQUIRE(data_c == data_a);
}

TEST_CASE("QLineF dx_dy", "[qlinef]")
{
   QLineF data;
   data.setLine(5, 50, 25, 85);

   REQUIRE(data.x1() == 5);
   REQUIRE(data.y1() == 50);
   REQUIRE(data.x2() == 25);
   REQUIRE(data.y2() == 85);

   REQUIRE(data.dx() == 20);
   REQUIRE(data.dy() == 35);

   REQUIRE(data.p1() == QPointF(5,  50));
   REQUIRE(data.p2() == QPointF(25, 85));

   //
   data = QLineF(5, 7, -32, -46);

   REQUIRE(data.dx() == -37);
   REQUIRE(data.dy() == -53);
}

TEST_CASE("QLineF intersect", "[qlinef]")
{
   QLineF data_a(0,  0, 10, 0);
   QLineF data_b(5, -5,  5, 5);

   QPointF result;

   REQUIRE(data_a.intersect(data_b, &result) == QLineF::BoundedIntersection);

   REQUIRE_THAT(result.x(), Catch::Matchers::WithinAbs(5.0, 0.01));
   REQUIRE_THAT(result.y(), Catch::Matchers::WithinAbs(0.0, 0.01));

   //
   data_a = QLineF(0, 0, 5, 5);
   data_b = QLineF(0, 1, 5, 6);

   REQUIRE(data_a.intersect(data_b, &result) == QLineF::NoIntersection);

   //
   data_a = QLineF(0, 0, 10, 10);
   data_b = QLineF(5, 5, 20, 20);

   REQUIRE(data_a.intersect(data_b, &result) == QLineF::NoIntersection);

   //
   data_a = QLineF(0, 0,  5, 5);
   data_b = QLineF(5, 5, 10, 0);

   REQUIRE(data_a.intersect(data_b, &result) == QLineF::BoundedIntersection);
   REQUIRE(result == QPointF(5, 5));

   //
   data_a = QLineF(0, 0, 1,  1);
   data_b = QLineF(2, 0, 3, -1);

   REQUIRE(data_a.intersect(data_b, &result) == QLineF::UnboundedIntersection);
}

TEST_CASE("QLineF isNull", "[qlinef]")
{
   QLineF data;

   REQUIRE(data.isNull() == true);

   REQUIRE(data.x1() == 0);
   REQUIRE(data.y1() == 0);
   REQUIRE(data.x2() == 0);
   REQUIRE(data.y2() == 0);

   //
   data = QLineF(15, 15, 15, 15);

   REQUIRE(data.isNull() == true);

   REQUIRE(data.x1() == 15);
   REQUIRE(data.y1() == 15);
   REQUIRE(data.x2() == 15);
   REQUIRE(data.y2() == 15);

   REQUIRE(data.dx() == 0);
   REQUIRE(data.dy() == 0);

   //
   data = QLineF(15, 15, 16, 15);

   REQUIRE(data.isNull() == false);

   REQUIRE(data.x1() == 15);
   REQUIRE(data.y1() == 15);
   REQUIRE(data.x2() == 16);
   REQUIRE(data.y2() == 15);
}

TEST_CASE("QLineF length", "[qlinef]")
{
   QLineF data(0, 0, 3, 4);

   REQUIRE(data.length() == 5.0);

   //
   data.setLength(10);

   REQUIRE(data.length() == 10.0);

   REQUIRE(data.p1()     == QPointF(0, 0));
   REQUIRE(data.p2()     == QPointF(6, 8));

   REQUIRE(data.p2().x() == 6.0);
   REQUIRE(data.p2().y() == 8.0);
}

TEST_CASE("QLineF normalVector", "[qlinef]")
{
   QLineF data(0, 0, 3, 0);
   QLineF result = data.normalVector();

   REQUIRE(result.p1() == QPointF(0,  0));
   REQUIRE(result.p2() == QPointF(0, -3));    // rotated 90° counter clock wise
}

TEST_CASE("QLineF operators", "[qlinef]")
{
   QLineF data_a(10, 20, 30, 40);
   QLineF data_b(10, 20, 30, 40);

   REQUIRE(data_a == data_b);

   //
   QLineF data_c(10, 20, 30, 50);

   REQUIRE(data_a != data_c);
   REQUIRE(data_c != data_a);
}

TEST_CASE("QLineF pointAt", "[qlinef]")
{
   QLineF data(0, 0, 10, 10);

   REQUIRE(data.pointAt(0.0) == QPointF(0,  0));
   REQUIRE(data.pointAt(1.0) == QPointF(10,10));
   REQUIRE(data.pointAt(0.5) == QPointF(5,  5));
   REQUIRE(data.pointAt(1.5) == QPointF(15,15));
}

TEST_CASE("QLineF set_p1_p2", "[qlinef]")
{
   QLineF data(1, 2, 3, 4);

   data.setP1(QPointF(17, 26));
   data.setP2(QPointF(32, 48));

   REQUIRE(data.x1() == 17);
   REQUIRE(data.y1() == 26);
   REQUIRE(data.x2() == 32);
   REQUIRE(data.y2() == 48);

   REQUIRE(data.dx() == 15);
   REQUIRE(data.dy() == 22);

   REQUIRE(data.p1() == QPointF(17, 26));
   REQUIRE(data.p2() == QPointF(32, 48));

   //
   data.setPoints(QPointF(1.5, 2.5), QPointF(7.5, -1.5));

   REQUIRE(data.p1() == QPointF(1.5,  2.5));
   REQUIRE(data.p2() == QPointF(7.5, -1.5));
}

TEST_CASE("QLineF set_p1", "[qlinef]")
{
   QLineF data;
   data.setP1(QPointF(5, 10));

   REQUIRE(data.p1() == QPointF(5, 10));
   REQUIRE(data.p2() == QPointF(0, 0));

   //
   data.setP1(QPointF(-5, -10));

   REQUIRE(data.p1() == QPointF(-5, -10));
   REQUIRE(data.p2() == QPointF(0, 0));
}

TEST_CASE("QLineF set_p2", "[qlinef]")
{
   QLineF data;
   data.setP2(QPointF(100, 200));

   REQUIRE(data.p1() == QPointF(0, 0));
   REQUIRE(data.p2() == QPointF(100, 200));
}

TEST_CASE("QLineF translate", "[qlinef]")
{
   QLineF data(10, 25, 30, 16);

   //
   data.translate(13, -17);

   REQUIRE(data.p1() == QPointF(23,  8));
   REQUIRE(data.p2() == QPointF(43, -1));

   //
   data = QLineF(0, 0, 18, 26);
   data.translate(QPointF(9, 8));

   REQUIRE(data == QLineF(9, 8, 27, 34));
}

TEST_CASE("QLineF translated", "[qlinef]")
{
   QLineF data   = QLineF(10, 20, 30, 40);
   QLineF result = data.translated(5, 3);

   REQUIRE(result == QLineF(15, 23, 35, 43));
   REQUIRE(data   == QLineF(10, 20, 30, 40));

   //
   data = QLineF(10, 10, 20, 20);

   REQUIRE(data.translated(QPointF(-5, 12)) == QLineF(5, 22, 15, 32));
}
