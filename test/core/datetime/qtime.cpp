/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#include <qtime.h>

#include <cs_catch2.h>

TEST_CASE("QTime traits", "[qtime]")
{
   REQUIRE(std::is_copy_constructible_v<QTime> == true);
   REQUIRE(std::is_move_constructible_v<QTime> == true);

   REQUIRE(std::is_copy_assignable_v<QTime> == true);
   REQUIRE(std::is_move_assignable_v<QTime> == true);

   REQUIRE(std::has_virtual_destructor_v<QTime> == false);
}

TEST_CASE("QTime null", "[qtime]")
{
   QTime time;

   REQUIRE(time.isNull());
   REQUIRE(! time.isValid());
}

TEST_CASE("QTime add_seconds", "[qtime]")
{
   QTime time = QTime(15, 45, 05);

   time = time.addSecs(65);

   REQUIRE(time == QTime(15, 46, 10));
}

TEST_CASE("QTime add_mseconds", "[qtime]")
{
   QTime time = QTime(15, 45, 05);

   time = time.addMSecs(2000);

   REQUIRE(time == QTime(15, 45, 7));
}

TEST_CASE("QTime valid", "[qtime]")
{
   QTime time = QTime(15, 45, 05);

   REQUIRE(! time.isNull());
   REQUIRE(time.isValid());

   REQUIRE(time.hour() == 15);
   REQUIRE(time.minute() == 45);
   REQUIRE(time.second() == 05);

   REQUIRE(time.msecsSinceStartOfDay() == 56705000);

   {
      time = QTime(22, 5, 62);
      REQUIRE(time.isValid() == false);

      time = QTime(11, 61, 5);
      REQUIRE(time.isValid() == false);

      time = QTime(26, 15, 3);
      REQUIRE(time.isValid() == false);
   }
}

TEST_CASE("QTime to_string", "[qtime]")
{
   QTime time = QTime(15, 9, 05);

   {
      QString str = time.toString();         // hh:mm::ss
      REQUIRE(str == QString("15:09:05"));
   }

   {
      QString str = time.toString("h:m:ss AP");
      REQUIRE(str == QString("3:9:05 PM"));
   }

   {
      QString str = time.toString("h:m:s ap");
      REQUIRE(str == QString("3:9:5 pm"));
   }
}


