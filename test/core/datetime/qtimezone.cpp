/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#include <qtimezone.h>

#include <cs_catch2.h>

TEST_CASE("QTimeZone equality", "[qtimezone]")
{
   QTimeZone data1("America/Denver");
   QTimeZone data2("America/New_York");

   REQUIRE(data1 != data2);
   REQUIRE(! (data1 == data2));

   REQUIRE(data1.hasDaylightTime());
   REQUIRE(data2.hasDaylightTime());

   REQUIRE(data1.country() == data2.country());
}

TEST_CASE("QTimeZone isvalid", "[qtimezone]")
{
   QTimeZone data("");

   REQUIRE(data.isValid());
}

TEST_CASE("QTimeZone offset", "[qtimezone]")
{
   QDateTime day;
   QTimeZone data("America/New_York");

   {
      day = QDateTime(QDate(2017, 1, 1), QTime(0, 0, 0), Qt::UTC);
      REQUIRE(data.offsetFromUtc(day) == -18000);
   }

   {
      day = QDateTime(QDate(2017, 8, 17), QTime(0, 0, 0), Qt::UTC);
      REQUIRE(data.offsetFromUtc(day) == -14400);
   }

   {
      day = QDateTime(QDate(2017, 10, 31), QTime(0, 0, 0), Qt::UTC);
      REQUIRE(data.offsetFromUtc(day) == -14400);
   }

   {
      day = QDateTime(QDate(2017, 11, 15), QTime(0, 0, 0), Qt::UTC);
      REQUIRE(data.offsetFromUtc(day) == -18000);
   }
}

TEST_CASE("QTimeZone id_by_country", "[qtimezone]")
{
   QList<QByteArray> list = QTimeZone::availableTimeZoneIds(QLocale::UnitedStates);

   REQUIRE(list.contains("America/Boise") == true);
   REQUIRE(list.contains("America/Denver") == true);
   REQUIRE(list.contains("America/TeddyBear") == false);

   list = QTimeZone::availableTimeZoneIds(QLocale::Canada);
   REQUIRE(list.contains("America/Toronto") == true);
   REQUIRE(list.contains("America/Vancouver") == true);
   REQUIRE(list.contains("America/Denver") == false);
}


