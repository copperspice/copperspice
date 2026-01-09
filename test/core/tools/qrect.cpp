/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#include <qrect.h>

#include <cs_catch2.h>

TEST_CASE("QRect traits", "[qrect]")
{
   REQUIRE(std::is_copy_constructible_v<QRect> == true);
   REQUIRE(std::is_move_constructible_v<QRect> == true);

   REQUIRE(std::is_copy_assignable_v<QRect> == true);
   REQUIRE(std::is_move_assignable_v<QRect> == true);

   REQUIRE(std::has_virtual_destructor_v<QRect> == false);
}

TEST_CASE("QRect constructor", "[qrect]")
{
   QRect data(5, 10, 100, 150);

   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.isNull()  == false);
   REQUIRE(data.isValid() == true );

   REQUIRE(data.x()      == 5);
   REQUIRE(data.y()      == 10);

   REQUIRE(data.left()   == 5);
   REQUIRE(data.top()    == 10);

   REQUIRE(data.width()  == 100);
   REQUIRE(data.height() == 150);

   REQUIRE(data.right()  == 104);      // 5  + 100 - 1
   REQUIRE(data.bottom() == 159);      // 10 + 150 - 1

   REQUIRE(data.bottomRight() == QPoint(104, 159));
   REQUIRE(data.topLeft()     == QPoint(5, 10));

   //
   data = QRect( QPoint(10, 20), QPoint(15, 45) );

   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.isNull()  == false);
   REQUIRE(data.isValid() == true );

   REQUIRE(data.x()      == 10);
   REQUIRE(data.y()      == 20);

   REQUIRE(data.left()   == 10);
   REQUIRE(data.top()    == 20);

   REQUIRE(data.width()  == 6);      // 15 - 10 + 1
   REQUIRE(data.height() == 26);     // 45 - 20 + 1

   REQUIRE(data.right()  == 15);
   REQUIRE(data.bottom() == 45);

   REQUIRE(data.bottomRight() == QPoint(15, 45));
   REQUIRE(data.topLeft()     == QPoint(10, 20));

   //
   data = QRect( QPoint(10, 20), QSize(9, 14) );

   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.isNull()  == false);
   REQUIRE(data.isValid() == true );

   REQUIRE(data.x()      == 10);
   REQUIRE(data.y()      == 20);

   REQUIRE(data.left()   == 10);
   REQUIRE(data.top()    == 20);

   REQUIRE(data.width()  == 9);
   REQUIRE(data.height() == 14);

   REQUIRE(data.right()  == 18);      // 10 +  9 - 1
   REQUIRE(data.bottom() == 33);      // 20 + 14 - 1

   REQUIRE(data.bottomRight() == QPoint(18, 33));
   REQUIRE(data.topLeft()     == QPoint(10, 20));
}

TEST_CASE("QRect contains", "[qrect]")
{
   QRect data(5, 10, 100, 200);

   REQUIRE(data.contains(5,  10) == true);
   REQUIRE(data.contains(20, 25) == true);

   REQUIRE(data.contains(5, 5) == false);

   REQUIRE(data.contains(data.topLeft())     == true);
   REQUIRE(data.contains(data.bottomRight()) == true);

   // fits inside
   REQUIRE(data.contains(QRect(15, 25, 5, 5)) == true);

   // overlaps but does not fit
   REQUIRE(data.contains(QRect(0, 0, 50, 75)) == false);

   //
   data = QRect(5, 5, 1, 1);

   REQUIRE(data.isNull() == false);

   REQUIRE(data.contains(5, 5) == true);
   REQUIRE(data.contains(5, 5, false) == true);

   // point is on the border, correct to return false
   REQUIRE(data.contains(5, 5, true) == false);

   REQUIRE(data.contains(QPoint(5, 5)) == true);

   //
   data = QRect(5, 5, 0, 0);

   REQUIRE(data.isNull() == true);

   REQUIRE(data.contains(5, 5) == false);
   REQUIRE(data.contains(5, 5, false) == false);

   REQUIRE(data.contains(5, 5, true) == false);

   REQUIRE(data.contains(QPoint(5, 5)) == false);
}

TEST_CASE("QRect copy_assign", "[qrect]")
{
   QRect data_a(10, 30, -5, -9);
   QRect data_b(data_a);

   REQUIRE(data_a == data_b);
   REQUIRE(data_b == QRect{10, 30, -5, -9});

   //
   QRect data_c;
   data_c = data_a;

   REQUIRE(data_a == data_c);
   REQUIRE(data_c == QRect{10, 30, -5, -9});
}

TEST_CASE("QRect empty", "[qrect]")
{
   QRect data;

   REQUIRE(data.isEmpty() == true);
   REQUIRE(data.isNull()  == true);
   REQUIRE(data.isValid() == false);

   REQUIRE(data.x()      == 0);
   REQUIRE(data.y()      == 0);

   REQUIRE(data.left()   == 0);
   REQUIRE(data.top()    == 0);

   REQUIRE(data.width()  == 0);
   REQUIRE(data.height() == 0);

   REQUIRE(data.right()  == -1);
   REQUIRE(data.bottom() == -1);

   REQUIRE(data.bottomRight() == QPoint(-1, -1));
   REQUIRE(data.topLeft()     == QPoint( 0,  0));
}

TEST_CASE("QRect equal", "[qrect]")
{
    QRect rect_a(1, 2, 3, 4);
    QRect rect_b(1, 2, 3, 4);
    QRect rect_c(1, 2, 3, 5);

    REQUIRE(rect_a == rect_b);
    REQUIRE(rect_a != rect_c);
}

TEST_CASE("QRect intersct", "[qrect]")
{
   QRect rect_a(0, 0,  50, 80);
   QRect rect_b(5, 12, 10, 90);

   QRect rect_c(100, 100, 20, 20);

   QRect result;

   REQUIRE(rect_a.top()    == 0);
   REQUIRE(rect_a.bottom() == 79);

   REQUIRE(rect_b.top()    == 12);
   REQUIRE(rect_b.bottom() == 101);

   //
   result = rect_a.intersected(rect_b);

   REQUIRE(result == QRect(5, 12, 10, 68));

   //
   result = rect_a.united(rect_b);

   REQUIRE(result == QRect(0, 0, 50, 102));

   //
   result = rect_a.intersected(rect_c);

   REQUIRE(result.isEmpty() == true);
}

TEST_CASE("QRect margins", "[qrect]")
{
   QRect rect_a(10, 50, 20, 70);

   rect_a.adjust(2, 3, 4, 5);

   REQUIRE(rect_a.left()   == 12);
   REQUIRE(rect_a.top()    == 53);

   REQUIRE(rect_a.right()  == 10 + 20 - 1 + 4);
   REQUIRE(rect_a.bottom() == 50 + 70 - 1 + 5);

   //
   QRect rect_b = rect_a.adjusted(1, 8, 15, 9);

   REQUIRE(rect_b.left()   == 13);
   REQUIRE(rect_b.top()    == 61);

   REQUIRE(rect_b.right()  == 10 + 20 - 1 + 4 + 15);
   REQUIRE(rect_b.bottom() == 50 + 70 - 1 + 5 + 9);
}

TEST_CASE("QRect move_assign", "[qrect]")
{
   QRect data_a(10, 30, -5, -9);
   QRect data_b(std::move(data_a));

   REQUIRE(data_b == QRect{10, 30, -5, -9});

   //
   QRect data_c;
   data_c = std::move(data_b);

   REQUIRE(data_c == QRect{10, 30, -5, -9});
}

TEST_CASE("QRect normalize", "[qrect]")
{
   QRect data_a(10, 30, -5, -9);

   REQUIRE(data_a.isEmpty() == true);
   REQUIRE(data_a.isValid() == false);

   REQUIRE(data_a.left()   == 10);      // swap A
   REQUIRE(data_a.top()    == 30);      // swap B

   REQUIRE(data_a.width()  == -5);
   REQUIRE(data_a.height() == -9);

   REQUIRE(data_a.right()  == 4);       // swap A
   REQUIRE(data_a.bottom() == 20);      // swap B

   REQUIRE(data_a.bottomRight() == QPoint(4,  20));
   REQUIRE(data_a.topLeft()     == QPoint(10, 30));

   //
   QRect data_b = data_a.normalized();

   REQUIRE(data_b.isValid() == true);

   REQUIRE(data_b.left()    == 4);      // swap A
   REQUIRE(data_b.top()     == 20);     // swap B

   REQUIRE(data_b.width()   == 7);      // 10 - 4  + 1
   REQUIRE(data_b.height()  == 11);     // 30 - 20 + 1

   REQUIRE(data_b.right()   == 10);     // swap A
   REQUIRE(data_b.bottom()  == 30);     // swap B

   REQUIRE(data_b.bottomRight() == QPoint(10, 30));
   REQUIRE(data_b.topLeft()     == QPoint(4,  20));

   //
   data_a = QRect(5, 15, 25, 30);
   data_b = data_a.normalized();

   REQUIRE(data_a.isValid() == true);

   REQUIRE(data_a == data_b);
}

TEST_CASE("QRect set", "[qrect]")
{
   QRect data(0, 0, 10, 20);

   data.setWidth(99);
   data.setHeight(88);

   REQUIRE(data.left()  == 0);
   REQUIRE(data.top()   == 0);

   REQUIRE(data.width()  == 99);
   REQUIRE(data.height() == 88);

   REQUIRE(data.right()  == 98);
   REQUIRE(data.bottom() == 87);

   //
   data.setLeft(5);
   data.setRight(15);

   REQUIRE(data.left()  == 5);
   REQUIRE(data.top()   == 0);

   REQUIRE(data.width()  == 11);      // 15 - 5 + 1
   REQUIRE(data.height() == 88);

   REQUIRE(data.right()  == 15);
   REQUIRE(data.bottom() == 87);

   //
   data.setTop(3);
   data.setBottom(9);

   REQUIRE(data.left()   == 5);
   REQUIRE(data.top()    == 3);

   REQUIRE(data.width()  == 11);
   REQUIRE(data.height() ==  7);      // 9 - 3 + 1

   REQUIRE(data.right()  == 15);
   REQUIRE(data.bottom() ==  9);

   //
   data.setSize( QSize(125, 200) );

   REQUIRE(data.left()   == 5);
   REQUIRE(data.top()    == 3);

   REQUIRE(data.width()  == 125);
   REQUIRE(data.height() == 200);

   REQUIRE(data.right()  == 129);
   REQUIRE(data.bottom() == 202);

   //
   data.setTopLeft(QPoint(10, 20));
   data.setBottomRight(QPoint(30, 40));

   REQUIRE(data.left()   == 10);
   REQUIRE(data.top()    == 20);

   REQUIRE(data.width()  == 21);
   REQUIRE(data.height() == 21);

   REQUIRE(data.right()  == 30);
   REQUIRE(data.bottom() == 40);
}

TEST_CASE("QRect translation", "[qrect]")
{
   QRect data(10, 30, 45, 95);

   data.translate(5, -3);

   REQUIRE(data.left()   == 15);
   REQUIRE(data.top()    == 27);

   REQUIRE(data.width()  == 45);
   REQUIRE(data.height() == 95);

   REQUIRE(data.right()  == 59);      // 15 + 45 - 1
   REQUIRE(data.bottom() == 121);     // 27 + 95 - 1

   //
   data.translate( QPoint(-2, 4) );

   REQUIRE(data.left()   == 13);
   REQUIRE(data.top()    == 31);

   REQUIRE(data.width()  == 45);
   REQUIRE(data.height() == 95);

   REQUIRE(data.right()  == 57);      // 13 + 45 - 1
   REQUIRE(data.bottom() == 125);     // 31 + 95 - 1

   //
   data.moveTo(100, 200);

   REQUIRE(data.left()   == 100);
   REQUIRE(data.top()    == 200);

   REQUIRE(data.width()  == 45);
   REQUIRE(data.height() == 95);

   REQUIRE(data.right()  == 144);     // 100 + 45 - 1
   REQUIRE(data.bottom() == 294);     // 200 + 95 - 1

   REQUIRE(data.topLeft()     == QPoint(100, 200));
   REQUIRE(data.bottomRight() == QPoint(144, 294));

   //
   data.moveLeft(50);

   REQUIRE(data.left()   == 50);
   REQUIRE(data.top()    == 200);

   REQUIRE(data.right()  == 94);
   REQUIRE(data.bottom() == 294);

   REQUIRE(data.width()  == 45);
   REQUIRE(data.height() == 95);

   //
   data.moveTop(60);

   REQUIRE(data.left()   == 50);
   REQUIRE(data.top()    == 60);

   REQUIRE(data.right()  == 94);
   REQUIRE(data.bottom() == 154);      // 60 + 95 - 1

   REQUIRE(data.width()  == 45);
   REQUIRE(data.height() == 95);

   //
   data.moveRight(80);

   REQUIRE(data.left()   == 36);
   REQUIRE(data.top()    == 60);

   REQUIRE(data.right()  == 80);
   REQUIRE(data.bottom() == 154);

   REQUIRE(data.width()  == 45);
   REQUIRE(data.height() == 95);

   //
   data.moveBottom(90);

   REQUIRE(data.left()   == 36);
   REQUIRE(data.top()    == -4);

   REQUIRE(data.right()  == 80);
   REQUIRE(data.bottom() == 90);

   REQUIRE(data.width()  == 45);
   REQUIRE(data.height() == 95);
}

TEST_CASE("QRect valid_null", "[qrect]")
{
   QRect data(0, 0, 0, 0);

   REQUIRE(data.isEmpty() == true);
   REQUIRE(data.isNull()  == true);
   REQUIRE(data.isValid() == false);

   //
   data = QRect(0, 0, 5, -1);

   REQUIRE(data.isEmpty() == true);
   REQUIRE(data.isNull()  == false);
   REQUIRE(data.isValid() == false);

   //
   data = QRect(0, 0, -5, -10);

   REQUIRE(data.isEmpty() == true);
   REQUIRE(data.isNull()  == false);
   REQUIRE(data.isValid() == false);
}
