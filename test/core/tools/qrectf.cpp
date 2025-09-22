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

#include <qrectf.h>

#include <cs_catch2.h>

TEST_CASE("QRectF traits", "[qrectf]")
{
   REQUIRE(std::is_copy_constructible_v<QRectF> == true);
   REQUIRE(std::is_move_constructible_v<QRectF> == true);

   REQUIRE(std::is_copy_assignable_v<QRectF> == true);
   REQUIRE(std::is_move_assignable_v<QRectF> == true);

   REQUIRE(std::has_virtual_destructor_v<QRectF> == false);
}

TEST_CASE("QRectF constructor", "[qrectf]")
{
   QRectF data(5, 10, 100, 150);

   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.isNull()  == false);
   REQUIRE(data.isValid() == true );

   REQUIRE(data.x()      == 5);
   REQUIRE(data.y()      == 10);

   REQUIRE(data.left()   == 5);
   REQUIRE(data.top()    == 10);

   REQUIRE(data.width()  == 100);
   REQUIRE(data.height() == 150);

   REQUIRE(data.right()  == 105);      // 5  + 100
   REQUIRE(data.bottom() == 160);      // 10 + 150

   REQUIRE(data.bottomRight() == QPointF(105, 160));
   REQUIRE(data.topLeft()     == QPointF(5, 10));

   //
   data = QRectF( QPointF(10, 20), QPointF(15, 45) );

   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.isNull()  == false);
   REQUIRE(data.isValid() == true );

   REQUIRE(data.x()      == 10);
   REQUIRE(data.y()      == 20);

   REQUIRE(data.left()   == 10);
   REQUIRE(data.top()    == 20);

   REQUIRE(data.width()  == 5);        // 15 - 10
   REQUIRE(data.height() == 25);       // 45 - 20

   REQUIRE(data.right()  == 15);
   REQUIRE(data.bottom() == 45);

   REQUIRE(data.bottomRight() == QPointF(15, 45));
   REQUIRE(data.topLeft()     == QPointF(10, 20));

   //
   data = QRectF( QPointF(10, 20), QSizeF(9, 14) );

   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.isNull()  == false);
   REQUIRE(data.isValid() == true );

   REQUIRE(data.x()      == 10);
   REQUIRE(data.y()      == 20);

   REQUIRE(data.left()   == 10);
   REQUIRE(data.top()    == 20);

   REQUIRE(data.width()  == 9);
   REQUIRE(data.height() == 14);

   REQUIRE(data.right()  == 19);       // 10 +  9
   REQUIRE(data.bottom() == 34);       // 20 + 14

   REQUIRE(data.bottomRight() == QPointF(19, 34));
   REQUIRE(data.topLeft()     == QPointF(10, 20));
}

TEST_CASE("QRectF contains", "[qrectf]")
{
   QRectF data(5, 10, 100, 200);

   REQUIRE(data.contains(5,  10) == true);
   REQUIRE(data.contains(20, 25) == true);

   REQUIRE(data.contains(5, 5) == false);

   REQUIRE(data.contains(data.topLeft())     == true);
   REQUIRE(data.contains(data.bottomRight()) == true);

   // fits inside
   REQUIRE(data.contains(QRectF(15, 25, 5, 5)) == true);

   // overlaps but does not fit
   REQUIRE(data.contains(QRectF(0, 0, 50, 75)) == false);

   //
   data = QRect(5, 5, 1, 1);

   REQUIRE(data.isNull() == false);

   REQUIRE(data.contains(5, 5) == true);
   REQUIRE(data.contains(QPoint(5, 5)) == true);

   //
   data = QRectF(5, 5, 0, 0);

   REQUIRE(data.isNull() == true);

   REQUIRE(data.contains(5, 5) == false);
   REQUIRE(data.contains(QPointF(5, 5)) == false);
}

TEST_CASE("QRectF empty", "[qrectf]")
{
   QRectF data;

   REQUIRE(data.isEmpty() == true);
   REQUIRE(data.isNull()  == true);
   REQUIRE(data.isValid() == false);

   REQUIRE(data.x()      == 0);
   REQUIRE(data.y()      == 0);

   REQUIRE(data.left()   == 0);
   REQUIRE(data.top()    == 0);

   REQUIRE(data.width()  == 0);
   REQUIRE(data.height() == 0);

   REQUIRE(data.right()  == 0);
   REQUIRE(data.bottom() == 0);

   REQUIRE(data.bottomRight() == QPointF( 0, 0));
   REQUIRE(data.topLeft()     == QPointF( 0, 0));
}

TEST_CASE("QRectF equal", "[qrectf]")
{
    QRectF rect_a(1, 2, 3, 4);
    QRectF rect_b(1, 2, 3, 4);
    QRectF rect_c(1, 2, 3, 5);

    REQUIRE(rect_a == rect_b);
    REQUIRE(rect_a != rect_c);
}

TEST_CASE("QRectF intersct", "[qrectf]")
{
   QRectF rect_a(0, 0,  50, 80);
   QRectF rect_b(5, 12, 10, 90);

   QRectF rect_c(100, 100, 20, 20);

   QRectF result;

   REQUIRE(rect_a.top()    == 0);
   REQUIRE(rect_a.bottom() == 80);

   REQUIRE(rect_b.top()    == 12);
   REQUIRE(rect_b.bottom() == 102);

   //
   result = rect_a.intersected(rect_b);

   REQUIRE(result == QRectF(5, 12, 10, 68));

   //
   result = rect_a.united(rect_b);

   REQUIRE(result == QRectF(0, 0, 50, 102));

   //
   result = rect_a.intersected(rect_c);

   REQUIRE(result.isEmpty() == true);
}

TEST_CASE("QRectF margins", "[qrectf]")
{
   QRectF rect_a(10, 50, 20, 70);

   rect_a.adjust(2, 3, 4, 5);

   REQUIRE(rect_a.left()   == 12);
   REQUIRE(rect_a.top()    == 53);

   REQUIRE(rect_a.right()  == 10 + 20 + 4);
   REQUIRE(rect_a.bottom() == 50 + 70 + 5);

   //
   QRectF rect_b = rect_a.adjusted(1, 8, 15, 9);

   REQUIRE(rect_b.left()   == 13);
   REQUIRE(rect_b.top()    == 61);

   REQUIRE(rect_b.right()  == 10 + 20 + 4 + 15);
   REQUIRE(rect_b.bottom() == 50 + 70 + 5 + 9);
}

TEST_CASE("QRectF normalize", "[qrectf]")
{
   QRectF data_a(10, 30, -5, -9);

   REQUIRE(data_a.isEmpty() == true);
   REQUIRE(data_a.isValid() == false);

   REQUIRE(data_a.left()   == 10);      // swap A
   REQUIRE(data_a.top()    == 30);      // swap B

   REQUIRE(data_a.width()  == -5);
   REQUIRE(data_a.height() == -9);

   REQUIRE(data_a.right()  == 5);       // swap A
   REQUIRE(data_a.bottom() == 21);      // swap B

   REQUIRE(data_a.bottomRight() == QPointF(5,  21));
   REQUIRE(data_a.topLeft()     == QPointF(10, 30));

   //
   QRectF data_b = data_a.normalized();

   REQUIRE(data_b.isValid() == true);

   REQUIRE(data_b.left()    ==  5);     // swap A
   REQUIRE(data_b.top()     == 21);     // swap B

   REQUIRE(data_b.width()   ==  5);     // 10 -  5
   REQUIRE(data_b.height()  ==  9);     // 30 - 21

   REQUIRE(data_b.right()   == 10);     // swap A
   REQUIRE(data_b.bottom()  == 30);     // swap B

   REQUIRE(data_b.bottomRight() == QPointF(10, 30));
   REQUIRE(data_b.topLeft()     == QPointF( 5, 21));

   //
   data_a = QRectF(5, 15, 25, 30);
   data_b = data_a.normalized();

   REQUIRE(data_a.isValid() == true);

   REQUIRE(data_a == data_b);
}

TEST_CASE("QRectF set", "[qrectf]")
{
   QRectF data(0, 0, 10, 20);

   data.setWidth(99);
   data.setHeight(88);

   REQUIRE(data.left()  == 0);
   REQUIRE(data.top()   == 0);

   REQUIRE(data.width()  == 99);
   REQUIRE(data.height() == 88);

   REQUIRE(data.right()  == 99);
   REQUIRE(data.bottom() == 88);

   //
   data.setLeft(5);
   data.setRight(15);

   REQUIRE(data.left()  == 5);
   REQUIRE(data.top()   == 0);

   REQUIRE(data.width()  == 10);      // 15 - 5
   REQUIRE(data.height() == 88);

   REQUIRE(data.right()  == 15);
   REQUIRE(data.bottom() == 88);

   //
   data.setTop(3);
   data.setBottom(9);

   REQUIRE(data.left()   == 5);
   REQUIRE(data.top()    == 3);

   REQUIRE(data.width()  == 10);
   REQUIRE(data.height() ==  6);      // 9 - 3

   REQUIRE(data.right()  == 15);
   REQUIRE(data.bottom() ==  9);

   //
   data.setSize( QSizeF(125, 200) );

   REQUIRE(data.left()   == 5);
   REQUIRE(data.top()    == 3);

   REQUIRE(data.width()  == 125);
   REQUIRE(data.height() == 200);

   REQUIRE(data.right()  == 130);
   REQUIRE(data.bottom() == 203);

   //
   data.setTopLeft(QPointF(10, 20));
   data.setBottomRight(QPointF(30, 40));

   REQUIRE(data.left()   == 10);
   REQUIRE(data.top()    == 20);

   REQUIRE(data.width()  == 20);
   REQUIRE(data.height() == 20);

   REQUIRE(data.right()  == 30);
   REQUIRE(data.bottom() == 40);
}

TEST_CASE("QRectF translation", "[qrectf]")
{
   QRectF data(10, 30, 45, 95);

   data.translate(5, -3);

   REQUIRE(data.left()   == 15);
   REQUIRE(data.top()    == 27);

   REQUIRE(data.width()  == 45);
   REQUIRE(data.height() == 95);

   REQUIRE(data.right()  == 60);      // 15 + 45
   REQUIRE(data.bottom() == 122);     // 27 + 95

   //
   data.translate( QPointF(-2, 4) );

   REQUIRE(data.left()   == 13);
   REQUIRE(data.top()    == 31);

   REQUIRE(data.width()  == 45);
   REQUIRE(data.height() == 95);

   REQUIRE(data.right()  == 58);      // 13 + 45
   REQUIRE(data.bottom() == 126);     // 31 + 95

   //
   data.moveTo(100, 200);

   REQUIRE(data.left()   == 100);
   REQUIRE(data.top()    == 200);

   REQUIRE(data.width()  == 45);
   REQUIRE(data.height() == 95);

   REQUIRE(data.right()  == 145);     // 100 + 45
   REQUIRE(data.bottom() == 295);     // 200 + 95

   REQUIRE(data.topLeft()     == QPointF(100, 200));
   REQUIRE(data.bottomRight() == QPointF(145, 295));

   //
   data.moveLeft(50);

   REQUIRE(data.left()   == 50);
   REQUIRE(data.top()    == 200);

   REQUIRE(data.right()  == 95);
   REQUIRE(data.bottom() == 295);

   REQUIRE(data.width()  == 45);
   REQUIRE(data.height() == 95);

   //
   data.moveTop(60);

   REQUIRE(data.left()   == 50);
   REQUIRE(data.top()    == 60);

   REQUIRE(data.right()  == 95);
   REQUIRE(data.bottom() == 155);      // 60 + 95

   REQUIRE(data.width()  == 45);
   REQUIRE(data.height() == 95);

   //
   data.moveRight(80);

   REQUIRE(data.left()   == 35);
   REQUIRE(data.top()    == 60);

   REQUIRE(data.right()  == 80);
   REQUIRE(data.bottom() == 155);

   REQUIRE(data.width()  == 45);
   REQUIRE(data.height() == 95);

   //
   data.moveBottom(90);

   REQUIRE(data.left()   == 35);
   REQUIRE(data.top()    == -5);

   REQUIRE(data.right()  == 80);
   REQUIRE(data.bottom() == 90);

   REQUIRE(data.width()  == 45);
   REQUIRE(data.height() == 95);
}

TEST_CASE("QRectF valid_null", "[qrectf]")
{
   QRectF data(0, 0, 0, 0);

   REQUIRE(data.isEmpty() == true);
   REQUIRE(data.isNull()  == true);
   REQUIRE(data.isValid() == false);

   //
   data = QRectF(0, 0, 5, -1);

   REQUIRE(data.isEmpty() == true);
   REQUIRE(data.isNull()  == false);
   REQUIRE(data.isValid() == false);

   //
   data = QRectF(0, 0, -5, -10);

   REQUIRE(data.isEmpty() == true);
   REQUIRE(data.isNull()  == false);
   REQUIRE(data.isValid() == false);
}
