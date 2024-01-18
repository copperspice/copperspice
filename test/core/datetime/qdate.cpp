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

#include <cs_catch_defines.h>

#include <qstring.h>
#include <qdate.h>
#include <qdatetime.h>

#include <chrono>

#include <cs_catch2.h>

TEST_CASE("QDate traits", "[qdate]")
{
   REQUIRE(std::is_copy_constructible_v<QDate> == true);
   REQUIRE(std::is_move_constructible_v<QDate> == true);

   REQUIRE(std::is_copy_assignable_v<QDate> == true);
   REQUIRE(std::is_move_assignable_v<QDate> == true);

   REQUIRE(std::has_virtual_destructor_v<QDate> == false);
}

TEST_CASE("QDate add_days", "[qdate]")
{
   {
      QDate date = QDate(2012, 10, 31);

      date = date.addDays(5);

      REQUIRE(date == QDate(2012, 11, 05));
   }

   {
      QDate date;

      date = date.addDays(3);

      REQUIRE(date == QDate());
   }
}

TEST_CASE("QDate add_months", "[qdate]")
{
   {
      QDate date = QDate(2012, 10, 31);

      date = date.addMonths(2);

      REQUIRE(date == QDate(2012, 12, 31));
   }

   {
      QDate date = QDate(2012, 10, 31);

      date = date.addMonths(0);

      REQUIRE(date == QDate(2012, 10, 31));
   }

   {
      QDate date = QDate(2012, 10, 31);

      date = date.addMonths(-4);

      REQUIRE(date == QDate(2012, 6, 30));
   }

   {
      QDate date = QDate(2014, 6, 1);

      date = date.addMonths(14);

      REQUIRE(date == QDate(2015, 8, 1));
   }

   {
      QDate date = QDate(2014, 6, 1);

      date = date.addMonths(-14);

      REQUIRE(date == QDate(2013, 4, 1));
   }

   {
      QDate date;

      date = date.addMonths(2);

      REQUIRE(date == QDate());
   }
}

TEST_CASE("QDate add_years", "[qdate]")
{
   {
      QDate date = QDate(2012, 10, 31);

      date = date.addYears(5);

      REQUIRE(date == QDate(2017, 10, 31));
   }

   {
      QDate date;

      date = date.addYears(8);

      REQUIRE(date == QDate());
   }
}

TEST_CASE("QDate currentDate", "[qdate]")
{
   QDate date = QDate::currentDate();

   REQUIRE(date > QDate(2023, 3, 14));
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

   {
      QDate date2;
      REQUIRE(date1.daysTo(date2) == 0);
   }
}

TEST_CASE("QDate fromJulianDay", "[qdate]")
{
   QDate date;

   date = QDate::fromJulianDay(2459765);

   REQUIRE(date.isValid());
   REQUIRE(! date.isNull());
   REQUIRE(date == QDate(2022, 7, 04));
}

TEST_CASE("QDate get_setDate", "[qdate]")
{
   QDate date;

   {
      date.setDate(2023, 6, 1);

      REQUIRE(date.isValid() == true);
      REQUIRE(date.toJulianDay() == 2460097);
   }

   {
      date.setDate(2023, 3, 32);

      REQUIRE(date.isValid() == false);
      REQUIRE(date.toJulianDay() == INVALID_JD);
   }

   {
      date.setDate(2023, 6, 1);

      int m;
      int d;
      int y;

      date.getDate(&y, &m, &d);

      REQUIRE(m == 6);
      REQUIRE(d == 1);
      REQUIRE(y == 2023);
   }
}

TEST_CASE("QDate leap_year", "[qdate]")
{
   QDate date;

   date = QDate(1900, 2, 29);
   REQUIRE(date.isValid() == false);
   REQUIRE(date.isLeapYear(1900) == false);

   date = QDate(2000, 2, 29);
   REQUIRE(date.isValid() == true);
   REQUIRE(date.isLeapYear(2000) == true);

   date = QDate(2008, 2, 29);
   REQUIRE(date.isValid() == true);
   REQUIRE(date.isLeapYear(2008) == true);

   date = QDate(2011, 2, 29);
   REQUIRE(date.isValid() == false);
   REQUIRE(date.isLeapYear(2011) == false);

   date = QDate(2100, 2, 29);
   REQUIRE(date.isValid() == false);
   REQUIRE(date.isLeapYear(2100) == false);

   date = QDate(-1, 2, 29);
   REQUIRE(date.isValid() == true);
   REQUIRE(date.isLeapYear(-1) == true);
}

TEST_CASE("QDate names", "[qdate]")
{
   QLocale locale = QLocale::system();

   if (locale.language() == QLocale::English) {
      REQUIRE(QDate::longDayName(1)     == "Monday");
      REQUIRE(QDate::shortDayName(3)    == "Wed");

      REQUIRE(QDate::longMonthName(8)   == "August");
      REQUIRE(QDate::shortMonthName(10) == "Oct");

      REQUIRE(QDate::longMonthName(8, QDate::StandaloneFormat)   == "August");
      REQUIRE(QDate::shortMonthName(10, QDate::StandaloneFormat) == "Oct");

   } else if (locale.language() == QLocale::German) {

      REQUIRE(QDate::longDayName(1)     == "Montag");
      REQUIRE(QDate::shortDayName(3)    == "Mi");

      REQUIRE(QDate::longMonthName(2)   == "Februar");
      REQUIRE(QDate::shortMonthName(10) == "Okt");

      REQUIRE(QDate::longMonthName(2, QDate::StandaloneFormat)   == "Februar");
      REQUIRE(QDate::shortMonthName(10, QDate::StandaloneFormat) == "Okt");
   }
}

TEST_CASE("QDate null", "[qdate]")
{
   QDate date;

   REQUIRE(date.isNull() == true);
   REQUIRE(date.isValid() == false);
}

TEST_CASE("QDate ofDay", "[qdate]")
{
   QDateTime dt;

   // start
   {
      dt = QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0), QTimeZone::utc());
      REQUIRE(dt == QDate(1970, 1, 1).startOfDay(QTimeZone::utc()));
   }

   {
      dt = QDateTime(QDate(2023, 3, 12), QTime(0, 0, 0), QTimeZone("America/New_York"));
      REQUIRE(dt == QDate(2023, 3, 12).startOfDay(QTimeZone("America/New_York")));
   }

   {
      dt = QDateTime(QDate(2023, 3, 12), QTime(0, 0, 0), QTimeZone("America/New_York"));
      REQUIRE(dt == QDate(2023, 3, 12).startOfDay(QTimeZone("America/New_York")));
   }

   {
      dt = QDateTime();
      REQUIRE(dt == QDate().startOfDay(QTimeZone::utc()));
   }

   {
      // dst starts at 00:00 and moves forward to 01:00 revise this test
      dt = QDateTime(QDate(2008, 10, 19), QTime(0, 0, 0), QTimeZone("America/Sao_Paulo"));
      CHECK(dt == QDate(2008, 10, 19).startOfDay(QTimeZone("America/Sao_Paulo")));
   }

   {
      dt = QDateTime(QDate(2023, 4, 27), QTime(0, 0, 0), QTimeZone("Africa/Cairo"));
      REQUIRE(dt == QDate(2023, 4, 27).startOfDay(QTimeZone("Africa/Cairo")));

      dt = QDateTime(QDate(2023, 4, 28), QTime(0, 0, 0), QTimeZone("Africa/Cairo"));
      REQUIRE(dt == QDate(2023, 4, 28).startOfDay(QTimeZone("Africa/Cairo")));

      dt = QDateTime(QDate(2023, 4, 29), QTime(0, 0, 0), QTimeZone("Africa/Cairo"));
      REQUIRE(dt == QDate(2023, 4, 29).startOfDay(QTimeZone("Africa/Cairo")));
   }

   // end
   {
      dt = QDateTime(QDate(1970, 1, 1), QTime(23, 59, 59, 999), QTimeZone::utc());
      REQUIRE(dt == QDate(1970, 1, 1).endOfDay(QTimeZone::utc()));
   }

   {
      dt = QDateTime(QDate(2023, 11, 5), QTime(23, 59, 59, 999), QTimeZone("America/New_York"));
      REQUIRE(dt == QDate(2023, 11, 5).endOfDay(QTimeZone("America/New_York")));
   }

   {
      dt = QDateTime();
      REQUIRE(dt == QDate().endOfDay(QTimeZone::utc()));
   }

   {
      // dst starts at 00:00 and moves back to 23:00 revise this test
      dt = QDateTime(QDate(2008, 2, 17), QTime(0, 0, 0), QTimeZone("America/Sao_Paulo"));
      CHECK(dt == QDate(2008, 2, 17).startOfDay(QTimeZone("America/Sao_Paulo")));
   }

   {
      dt = QDateTime(QDate(2023, 10, 25), QTime(23, 59, 59, 999), QTimeZone("Africa/Cairo"));
      REQUIRE(dt == QDate(2023, 10, 25).endOfDay(QTimeZone("Africa/Cairo")));

      dt = QDateTime(QDate(2023, 10, 26), QTime(23, 59, 59, 999), QTimeZone("Africa/Cairo"));
      REQUIRE(dt == QDate(2023, 10, 26).endOfDay(QTimeZone("Africa/Cairo")));

      dt = QDateTime(QDate(2023, 10, 27), QTime(23, 59, 59, 999), QTimeZone("Africa/Cairo"));
      REQUIRE(dt == QDate(2023, 10, 27).endOfDay(QTimeZone("Africa/Cairo")));
   }
}

TEST_CASE("QDate operator", "[qdate]")
{
   QDate dt1 = QDate(2022, 8, 25);
   QDate dt2 = QDate(2022, 8, 31);

   REQUIRE(dt1 <  dt2);
   REQUIRE(dt1 <= dt2);
   REQUIRE(dt1 <= dt1);
   REQUIRE(dt1 != dt2);

   REQUIRE(dt2 >  dt1);
   REQUIRE(dt2 >= dt1);
   REQUIRE(dt2 >= dt2);
}

TEST_CASE("QDate fromString", "[qdate]")
{
   {
      QDate date = QDate::fromString("2019-10-31", Qt::ISODate);
      REQUIRE(date == QDate(2019, 10, 31));
   }

   {
      QDate date = QDate::fromString("31 Oct 2019", Qt::RFC2822Date);
      REQUIRE(date == QDate(2019, 10, 31));
   }

   {
      QDate date = QDate::fromString("Thu Oct 31 2019", Qt::TextDate);
      REQUIRE(date == QDate(2019, 10, 31));
   }

   {
      // empty date because string is less than 10 chars

      QDate date = QDate::fromString("2019-04-1", Qt::ISODate);
      REQUIRE(date == QDate());
   }

   {
      QDate date = QDate::fromString("2019-10-31", "yyyy-MM-dd");
      REQUIRE(date == QDate(2019, 10, 31));
   }
}

TEST_CASE("QDate to_string", "[qdate]")
{
   {
      QDate date = QDate();
      REQUIRE(date.toString(Qt::ISODate) == QString());
   }

   {
      QDate date = QDate(2021, 10, 31);
      REQUIRE(date.toString(Qt::ISODate) == "2021-10-31");
   }

   {
      QDate date = QDate(2021, 10, 31);
      REQUIRE(date.toString(Qt::RFC2822Date) == "31 Oct 2021");
   }

   {
      QDate date = QDate(2021, 10, 31);

      QString result = date.toString(Qt::SystemLocaleShortDate);
      REQUIRE((result == "10/31/2021" || result == "10/31/21" || result == "31/10/21"));
   }

   {
      QDate date = QDate(2021, 10, 31);

#if defined(Q_OS_DARWIN)
      REQUIRE(date.toString(Qt::SystemLocaleLongDate) == "October 31, 2021");
#else
      REQUIRE(date.toString(Qt::SystemLocaleLongDate) == "Sunday, October 31, 2021");
#endif
   }

   {
      QDate date = QDate(2021, 10, 31);

      QString result = date.toString(Qt::DefaultLocaleShortDate);
      REQUIRE((result == "10/31/2021" || result == "10/31/21" || result == "31/10/21"));
   }

   {
      QDate date = QDate(2021, 10, 31);

#if defined(Q_OS_DARWIN)
      REQUIRE(date.toString(Qt::DefaultLocaleLongDate) == "October 31, 2021");
#else
      REQUIRE(date.toString(Qt::DefaultLocaleLongDate) == "Sunday, October 31, 2021");
#endif
   }

   {
      QDate date = QDate(2021, 10, 31);
      REQUIRE(date.toString(Qt::TextDate) == "Sun Oct 31 2021");
   }

   {
      QDate date = QDate(2021, 10, 31);
      REQUIRE(date.toString("dd/MM/yyyy") == "31/10/2021");
   }
}

TEST_CASE("QDate various", "[qdate]")
{
   QDate date;

   {
      date = QDate(2012, 10, 31);

      REQUIRE(date.isNull()  == false);
      REQUIRE(date.isValid() == true);

      REQUIRE(date.month() == 10);
      REQUIRE(date.day() == 31);
      REQUIRE(date.year() == 2012);

      REQUIRE(date.toJulianDay() == 2456232);

      REQUIRE(date.dayOfWeek() == 3);
      REQUIRE(date.dayOfYear() == 305);

      REQUIRE(date.daysInMonth() == 31);
      REQUIRE(date.daysInYear() == 366);

      REQUIRE(date.weekNumber() == 44);
   }

   {
      date = QDate(2016, 2, 29);

      REQUIRE(date.isNull()  == false);
      REQUIRE(date.isValid() == true);

      REQUIRE(date.month() == 2);
      REQUIRE(date.day() == 29);
      REQUIRE(date.year() == 2016);

      REQUIRE(date.toJulianDay() == 2457448);

      REQUIRE(date.dayOfWeek() == 1);
      REQUIRE(date.dayOfYear() == 60);

      REQUIRE(date.daysInMonth() == 29);
      REQUIRE(date.daysInYear() == 366);

      REQUIRE(date.weekNumber() == 9);
   }

   {
      date = QDate(2020, 12, 31);

      REQUIRE(date.weekNumber() == 53);

      date = QDate(2021, 1, 1);

      REQUIRE(date.weekNumber() == 53);
   }

   {
      date = QDate();

      REQUIRE(date.isNull()  == true);
      REQUIRE(date.isValid() == false);

      REQUIRE(date.year() == 0);
      REQUIRE(date.month() == 0);
      REQUIRE(date.day() == 0);

      REQUIRE(date.toJulianDay() == INVALID_JD);

      REQUIRE(date.dayOfWeek() == 0);
      REQUIRE(date.dayOfYear() == 0);

      REQUIRE(date.daysInMonth() == 0);
      REQUIRE(date.daysInYear() == 0);

      REQUIRE(date.weekNumber() == 0);
   }
}

TEST_CASE("QDate duration", "[qdate]")
{
#if defined(CS_CHRONO_TYPES_CATCH)
   // C++20 only

   QDate date = QDate(2023, 10, 31);

   SECTION ("days") {
      date = date.addDuration(std::chrono::days(5));

      REQUIRE(date == QDate(2023, 11, 05));
   }
#endif
}

TEST_CASE("QDate std_chrono", "[qdate]")
{
#if defined(CS_CHRONO_TYPES_CATCH_YMD)
   // C++20 only

   QDate date1 = QDate(2021, 10, 31);

   std::chrono::sys_days result_1 = date1.toStdSysDays();

   std::chrono::sys_days result_2 = std::chrono::year_month_day(
      std::chrono::year(2021), std::chrono::month(10), std::chrono::day(31));

   REQUIRE(result_1 == result_2);

   //
   QDate date2 = QDate::fromStdSysDays(result_1);

   REQUIRE(date1 == date2);
#endif
}
