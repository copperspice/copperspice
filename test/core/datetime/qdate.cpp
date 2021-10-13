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

#include <qstring.h>
#include <qdate.h>

#include <cs_catch2.h>

TEST_CASE("QDate traits", "[qdate]")
{
   REQUIRE(std::is_copy_constructible_v<QDate> == true);
   REQUIRE(std::is_move_constructible_v<QDate> == true);

   REQUIRE(std::is_copy_assignable_v<QDate> == true);
   REQUIRE(std::is_move_assignable_v<QDate> == true);

   REQUIRE(std::is_nothrow_move_constructible_v<QDate> == true);
   REQUIRE(std::is_nothrow_move_assignable_v<QDate> == true);

   REQUIRE(std::has_virtual_destructor_v<QDate> == false);
}

TEST_CASE("QDate add_days", "[qdate]")
{
   QDate date = QDate(2012, 10, 31);

   date = date.addDays(5);

   REQUIRE(date == QDate(2012, 11, 05));
}

TEST_CASE("QDate add_months", "[qdate]")
{
   QDate date = QDate(2012, 10, 31);

   date = date.addMonths(2);

   REQUIRE(date == QDate(2012, 12, 31));
}

TEST_CASE("QDate add_years", "[qdate]")
{
   QDate date = QDate(2012, 10, 31);

   date = date.addYears(5);

   REQUIRE(date == QDate(2017, 10, 31));
}

TEST_CASE("QDate days_to", "[qdate]")
{
   QDate date1 = QDate(2017, 10, 31);

   {
      QDate date2 = QDate(2017, 11, 15);
      REQUIRE(date1.daysTo(date2) == 15);
   }

   {
      QDate date2 = QDate(2017, 10, 9);
      REQUIRE(date1.daysTo(date2) == -22);
   }
}

TEST_CASE("QDate null", "[qdate]")
{
   QDate date;

   REQUIRE(date.isNull());
   REQUIRE(! date.isValid());
}

TEST_CASE("QDate to_string", "[qdate]")
{
   QDate date = QDate(2017, 10, 31);

   REQUIRE(date.toString(Qt::ISODate) == "2017-10-31");
}

TEST_CASE("QDate various", "[qdate]")
{
   QDate date = QDate(2012, 10, 31);

   REQUIRE(! date.isNull());
   REQUIRE(date.isValid());

   REQUIRE(date.year() == 2012);
   REQUIRE(date.month() == 10);
   REQUIRE(date.day() == 31);

   REQUIRE(date.toJulianDay() == 2456232);

   REQUIRE(date.dayOfWeek() == 3);
   REQUIRE(date.dayOfYear() == 305);

   REQUIRE(date.daysInMonth() == 31);
   REQUIRE(date.daysInYear() == 366);

   REQUIRE(date.weekNumber() == 44);
}