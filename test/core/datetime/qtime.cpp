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

TEST_CASE("QTime add_seconds", "[qtime]")
{
   QTime time = QTime(15, 45, 05);

   {
      time = time.addSecs(65);

      REQUIRE(time == QTime(15, 46, 10));
   }

   {
      time = time.addSecs(-50);

      REQUIRE(time == QTime(15, 45, 20));
   }
}

TEST_CASE("QTime add_mseconds", "[qtime]")
{
   QTime time = QTime(15, 45, 05);

   {
      time = time.addMSecs(2000);

      REQUIRE(time == QTime(15, 45, 7));
   }

   {
      time = time.addMSecs(-3000);

      REQUIRE(time == QTime(15, 45, 4));
   }
}

TEST_CASE("QTime fromString", "[qtime]")
{
   {
      QTime time = QTime::fromString("15:32:05", Qt::ISODate);
      REQUIRE(time == QTime(15, 32, 05));
   }

   {
      QTime time = QTime::fromString(" 28 Feb 2000 15:32:05 +0000", Qt::RFC2822Date);
      REQUIRE(time == QTime(15, 32, 05));
   }

   {
      QTime time = QTime::fromString("15:32:05", "hh:m:ss");
      REQUIRE(time == QTime(15, 32, 05));
   }
}

TEST_CASE("QTime msec", "[qtime]")
{
   QTime time;

   {
      time = QTime(11, 30, 25);

      REQUIRE(time.msec() == 0);
   }

   {
      time = QTime(11, 30, 25, 100);

      REQUIRE(time.msec() == 100);
   }

   {
      time = QTime();

      REQUIRE(time.msec() == -1);
   }
}

TEST_CASE("QTime msecs_to", "[qtime]")
{
   QTime time1 = QTime(11, 30, 25);

   {
      QTime time2 = QTime(14, 42, 18);
      REQUIRE(time1.msecsTo(time2) == 11513 * 1000);
   }

   {
      QTime time2 = QTime(9, 12, 00);
      REQUIRE(time1.msecsTo(time2) == -8305 * 1000);
   }

   {
      QTime time2;
      REQUIRE(time1.msecsTo(time2) == 0);
   }
}

TEST_CASE("QTime null", "[qtime]")
{
   QTime time;

   REQUIRE(time.isNull() == true);
   REQUIRE(time.isValid() == false);
}

TEST_CASE("QTime secs_to", "[qtime]")
{
   QTime time1 = QTime(11, 30, 25);

   {
      QTime time2 = QTime(14, 42, 18);
      REQUIRE(time1.secsTo(time2) == 11513);
   }

   {
      QTime time2 = QTime(9, 12, 00);
      REQUIRE(time1.secsTo(time2) == -8305);
   }

   {
      QTime time2;
      REQUIRE(time1.secsTo(time2) == 0);
   }
}

TEST_CASE("QTime to_string", "[qtime]")
{
   QTime time = QTime(15, 9, 05);

   {
      QString str = time.toString();         // hh:mm::ss
      REQUIRE(str == QString("15:09:05"));
   }

   if (QLocale::system().language() == QLocale::English) {

      {
         QString str = time.toString("h:m:ss AP");
         REQUIRE(str == QString("3:9:05 PM"));
      }

      {
         QString str = time.toString("h:m:s ap");
         REQUIRE(str == QString("3:9:5 pm"));
      }

   } else {

      {
         QString str = time.toString("h:m:ss");
         REQUIRE(str == QString("15:9:05"));
      }

      {
         QString str = time.toString("h:m:s");
         REQUIRE(str == QString("15:9:5"));
      }

   }
}

TEST_CASE("QTime valid", "[qtime]")
{
   QTime time = QTime(15, 45, 05);

   REQUIRE(time.isNull()  == false);
   REQUIRE(time.isValid() == true);

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

   {
      time = QTime();

      REQUIRE(time.isNull()  == true);
      REQUIRE(time.isValid() == false);

      REQUIRE(time.hour() == -1);
      REQUIRE(time.minute() == -1);
      REQUIRE(time.second() == -1);

      REQUIRE(time.msecsSinceStartOfDay() == 0);
   }
}
