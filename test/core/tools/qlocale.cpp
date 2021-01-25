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

#include <qlocale.h>

#include <cs_catch2.h>

TEST_CASE("QLocale constructor", "[qlocale]")
{
   QLocale data1 = QLocale(QLocale::English, QLocale::UnitedStates);
   QLocale data2 = QLocale(QLocale::French,  QLocale::France);
   QLocale data3 = QLocale(QLocale::German,  QLocale::Germany);
   QLocale data4 = QLocale(QLocale::Dutch,  QLocale::Netherlands);
   QLocale data5 = QLocale(QLocale::English, QLocale::UnitedKingdom);

   REQUIRE(data1.name() == "en_US");
   REQUIRE(data1.nativeCountryName() == "United States");

   REQUIRE(data2.name() == "fr_FR");
   REQUIRE(data2.nativeCountryName() == "France");

   REQUIRE(data3.name() == "de_DE");
   REQUIRE(data3.nativeCountryName() == "Deutschland");

   REQUIRE(data4.name() == "nl_NL");
   REQUIRE(data4.nativeCountryName() == "Nederland");

   REQUIRE(data5.name() == "en_GB");
   REQUIRE(data5.nativeCountryName() == "United Kingdom");
}

TEST_CASE("QLocale empty", "[qlocale]")
{
   QLocale data;

   REQUIRE(data == QLocale::system());
}

TEST_CASE("QLocale day_of_week", "[qlocale]")
{
   QDate date;
   QString str;

   QLocale data(QLocale::C);

   {
      date = QDate(2017, 1, 1);

      str  = data.toString(date, "ddd");
      REQUIRE(str == "Sun");

      str  = data.toString(date, "dddd");
      REQUIRE(str == "Sunday");
   }

   {
      date = QDate(2017, 1, 2);

      str  = data.toString(date, "ddd");
      REQUIRE(str == "Mon");

      str  = data.toString(date, "dddd");
      REQUIRE(str == "Monday");
   }

   {
      date = QDate(2017, 1, 3);

      str  = data.toString(date, "ddd");
      REQUIRE(str == "Tue");

      str  = data.toString(date, "dddd");
      REQUIRE(str == "Tuesday");
   }

   {
      date = QDate(2017, 1, 4);

      str  = data.toString(date, "ddd");
      REQUIRE(str == "Wed");

      str  = data.toString(date, "dddd");
      REQUIRE(str == "Wednesday");
   }

   {
      date = QDate(2017, 1, 5);

      str  = data.toString(date, "ddd");
      REQUIRE(str == "Thu");

      str  = data.toString(date, "dddd");
      REQUIRE(str == "Thursday");
   }

   {
      date = QDate(2017, 1, 6);

      str  = data.toString(date, "ddd");
      REQUIRE(str == "Fri");

      str  = data.toString(date, "dddd");
      REQUIRE(str == "Friday");
   }

   {
      date = QDate(2017, 1, 7);

      str  = data.toString(date, "ddd");
      REQUIRE(str == "Sat");

      str  = data.toString(date, "dddd");
      REQUIRE(str == "Saturday");
   }
}

TEST_CASE("QLocale format_date", "[qlocale]")
{
   QDate date;
   QString str;

   QLocale data(QLocale::C);

   {
      date = QDate(2017, 4, 1);

      str  = data.toString(date, "d/M/yyyy");
      REQUIRE(str == "1/4/2017");

      str  = data.toString(date, "d/M/yy");
      REQUIRE(str == "1/4/17");

      str  = data.toString(date, "dd/MM/yyyy");
      REQUIRE(str == "01/04/2017");

      str  = data.toString(date, "ddd/MMM/yyyy");
      REQUIRE(str == "Sat/Apr/2017");

      str  = data.toString(date, "dddd/MMMM/yyyy");
      REQUIRE(str == "Saturday/April/2017");

      str  = data.toString(date, "ddddd/MMMMM/yyyy");
      REQUIRE(str == "Saturday1/April4/2017");

      str  = data.toString(date, "dddd d/MMMM MM yy");
      REQUIRE(str == "Saturday 1/April 04 17");
   }
}

TEST_CASE("QLocale format_time", "[qlocale]")
{
   QTime time;
   QString str;

   QLocale data(QLocale::C);

   {
      time = QTime(2, 5, 9);

      str  = data.toString(time, "h:m:s");
      REQUIRE(str == "2:5:9");

      str  = data.toString(time, "hh:mm:ss");
      REQUIRE(str == "02:05:09");

      str  = data.toString(time, "HH:mm:ss ap");
      REQUIRE(str == "02:05:09 am");

      str  = data.toString(time, "HH:mm:ss AP");
      REQUIRE(str == "02:05:09 AM");
   }

   {
      time = QTime(10, 2, 9);

      str  = data.toString(time, "h:m:s");
      REQUIRE(str == "10:2:9");

      str  = data.toString(time, "hh:mm:ss");
      REQUIRE(str == "10:02:09");

      str  = data.toString(time, "HH:mm:ss ap");
      REQUIRE(str == "10:02:09 am");

      str  = data.toString(time, "HH:mm:ss AP");
      REQUIRE(str == "10:02:09 AM");
   }

   {
      time = QTime(2, 0, 8, 501);

      str  = data.toString(time, "h:m:s.z");
      REQUIRE(str == "2:0:8.501");

      str  = data.toString(time, "hh:mm:ss.zzz");
      REQUIRE(str == "02:00:08.501");

      str  = data.toString(time, "HH:mm:ss.z ap");
      REQUIRE(str == "02:00:08.501 am");

      str  = data.toString(time, "HH:mm:ss.z AP");
      REQUIRE(str == "02:00:08.501 AM");
   }
}

TEST_CASE("QLocale names", "[qlocale]")
{
   QLocale data(QLocale::C);

   REQUIRE(data.monthName(1)  == "January");
   REQUIRE(data.monthName(12) == "December");

   REQUIRE(data.dayName(1) == "Monday");
   REQUIRE(data.dayName(6) == "Saturday");

   REQUIRE(data.weekdays().contains(Qt::Tuesday)  == true);
   REQUIRE(data.weekdays().contains(Qt::Saturday) == false);
}


