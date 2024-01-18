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

#include <qdatetime.h>
#include <qtimezone.h>

#include <chrono>

#include <cs_catch2.h>

TEST_CASE("QDateTime traits", "[qdatetime]")
{
   REQUIRE(std::is_copy_constructible_v<QDateTime> == true);
   REQUIRE(std::is_move_constructible_v<QDateTime> == true);

   REQUIRE(std::is_copy_assignable_v<QDateTime> == true);
   REQUIRE(std::is_move_assignable_v<QDateTime> == true);

   REQUIRE(std::has_virtual_destructor_v<QDateTime> == false);
}

TEST_CASE("QDateTime add_days", "[qdatetime]")
{
   QDateTime dt = QDateTime(QDate(2017, 10, 31));

   dt = dt.addDays(5);

   REQUIRE(dt == QDateTime(QDate(2017, 11, 05)));
}

TEST_CASE("QDateTime add_months", "[qdatetime]")
{
   QDateTime dt = QDateTime(QDate(2017, 10, 31));

   dt = dt.addMonths(2);

   REQUIRE(dt == QDateTime(QDate(2017, 12, 31)));
}

TEST_CASE("QDateTime add_years", "[qdatetime]")
{
   QDateTime dt = QDateTime(QDate(2017, 10, 31));

   dt = dt.addYears(5);

   REQUIRE(dt == QDateTime(QDate(2022, 10, 31)));
}

TEST_CASE("QDateTime add_seconds", "[qdatetime]")
{
   QDateTime dt;

   {
      dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 05));

      dt = dt.addSecs(65);

      REQUIRE(dt == QDateTime(QDate(2017, 10, 31), QTime(15, 46, 10)));
   }

   {
      dt = QDateTime();

      dt = dt.addSecs(65);

      REQUIRE(dt == QDateTime());
   }
}

TEST_CASE("QDateTime add_mseconds", "[qdatetime]")
{
   QDateTime dt;

   {
      dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 05));

      dt = dt.addMSecs(2000);

      REQUIRE(dt == QDateTime(QDate(2017, 10, 31), QTime(15, 45, 07)));
   }

   {
      dt = QDateTime();

      dt = dt.addMSecs(1000);

      REQUIRE(dt == QDateTime());
   }
}

TEST_CASE("QDateTime assign", "[qdatetime]")
{
   QDateTime dt1 = QDateTime(QDate(2019, 8, 17), QTime(14, 20, 0));
   QDateTime dt2;

   dt2 = dt1;

   REQUIRE(dt1 == dt2);
}

TEST_CASE("QDateTime currentDateTime", "[qdatetime]")
{
   QDateTime dt;

   {
      dt = QDateTime::currentDateTime();
      REQUIRE(dt.timeZone() == QTimeZone::systemTimeZone());
   }

   {
      dt = QDateTime::currentDateTime(QTimeZone("America/New_York"));
      REQUIRE(dt.timeZone() == QTimeZone("America/New_York"));
   }

   {
      dt = QDateTime::currentDateTimeUtc();
      REQUIRE(dt.timeZone() == QTimeZone::utc());
   }
}

TEST_CASE("QDateTime daylightTime", "[qdatetime]")
{
   QDateTime dt;

   {
      dt = QDateTime(QDate(2021, 1, 1), QTime(0, 0), QTimeZone::utc());
      REQUIRE(dt.isDaylightTime() == false);
   }

   {
      dt = QDateTime(QDate(2021, 7, 1), QTime(0, 0), QTimeZone::utc());
      REQUIRE(dt.isDaylightTime() == false);
   }

   {
      dt = QDateTime(QDate(2021, 1, 1), QTime(0, 0), QTimeZone("America/New_York"));
      REQUIRE(dt.isDaylightTime() == false);
   }

   {
      dt = QDateTime(QDate(2021, 7, 1), QTime(0, 0), QTimeZone("America/New_York"));
      REQUIRE(dt.isDaylightTime() == true);
   }
}

TEST_CASE("QDateTime days_to", "[qdatetime]")
{
   QDateTime dt1 = QDateTime(QDate(2017, 10, 31));

   {
      QDateTime dt2 = QDateTime(QDate(2017, 11, 15));
      REQUIRE(dt1.daysTo(dt2) == 15);
   }

   {
      QDateTime dt3 = QDateTime(QDate(2017, 10, 9));
      REQUIRE(dt1.daysTo(dt3) == -22);
   }
}

TEST_CASE("QDateTime fromString", "[qdatetime]")
{
   QDateTime dt;

   {
      dt = QDateTime(QDate(2015, 11, 30), QTime(18, 30, 40));

      QDateTime result = QDateTime::fromString(QString("2015-11-30T18:30:40"), Qt::ISODate);

      REQUIRE(result == dt);
   }

   {
      dt = QDateTime(QDate(2015, 11, 30), QTime(18, 30, 40));

      QDateTime result = QDateTime::fromString(QString("2015-11-30T18:30:40"), "yyyy-MM-ddThh:mm:ss");

//      REQUIRE(result == dt);
   }
}

TEST_CASE("QDateTime toEpoch", "[qdatetime]")
{
   QDateTime dt1;
   QDateTime dt2;

   dt1 = QDateTime(QDate(1969, 12, 31), QTime(23, 59, 58), QTimeZone::utc());
   REQUIRE(dt1.toMSecsSinceEpoch() == -2000);
   REQUIRE(dt1.toSecsSinceEpoch()  == -2);

   dt1 = QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0), QTimeZone::utc());
   REQUIRE(dt1.toMSecsSinceEpoch() == 0);
   REQUIRE(dt1.toSecsSinceEpoch()  == 0);

   dt1 = QDateTime(QDate(2020, 2, 29), QTime(18, 0, 0), QTimeZone::utc());
   REQUIRE(dt1.toMSecsSinceEpoch() == 1582999200000);
   REQUIRE(dt1.toSecsSinceEpoch()  == 1582999200);

   //
   dt1 = QDateTime(QDate(2005, 11, 15), QTime(15, 15, 30), QTimeZone::utc());
   dt2 = QDateTime::fromSecsSinceEpoch(1132067730, QTimeZone::utc());

   REQUIRE(dt2.date() == dt1.date());
   REQUIRE(dt2.time() == dt1.time());
   REQUIRE(dt2.timeZone() == dt1.timeZone());
   REQUIRE(dt2.timeRepresentation() == dt1.timeRepresentation());
}

TEST_CASE("QDateTime msecsTo", "[qdatetime]")
{
   QDateTime dt1 = QDateTime(QDate(2022, 8, 31), QTime(15, 40, 5));
   QDateTime dt2 = QDateTime(QDate(2022, 8, 31), QTime(15, 45, 5));

   REQUIRE(dt1.msecsTo(dt2) == 300000);
   REQUIRE(dt2.msecsTo(dt1) == -300000);

   //
   dt1 = QDateTime();
   dt2 = QDateTime();

   REQUIRE(dt1.msecsTo(dt2) == 0);
   REQUIRE(dt2.msecsTo(dt1) == 0);
}

TEST_CASE("QDateTime null", "[qdatetime]")
{
   QDateTime dt;

   REQUIRE(dt.isNull() == true);
   REQUIRE(dt.isValid() == false);

   REQUIRE(dt.date() == QDate());
   REQUIRE(dt.time() == QTime());

   REQUIRE(dt.toOffsetFromUtc(3600) == QDateTime(QDate(), QTime(), QTimeZone(3600)));
}

TEST_CASE("QDateTime offset", "[qdatetime]")
{
   QDateTime dt;

   {
      dt = QDateTime(QDate(2019, 10, 31), QTime(15, 45, 05), QTimeZone::utc());
      REQUIRE(dt.offsetFromUtc() == 0);
   }

   {
      dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 5), QTimeZone("UTC-07:00"));
      REQUIRE(dt.offsetFromUtc() == -25200);
   }

   {
      dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 5), QTimeZone("UTC-07:00"));

      QDateTime result = QDateTime(QDate(2017, 10, 31), QTime(17, 45, 5), QTimeZone("UTC-05:00"));
      REQUIRE(dt.toOffsetFromUtc(-18000) == result);
   }

   {
      dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 5), QTimeZone("UTC-07:00"));

      QDateTime result = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 5), QTimeZone("UTC-07:00"));
      REQUIRE(dt.toOffsetFromUtc(-25200) == result);
   }
}

TEST_CASE("QDateTime operator", "[qdatetime]")
{
   QDateTime dt1 = QDateTime(QDate(2022, 8, 31), QTime(15, 40, 5));
   QDateTime dt2 = QDateTime(QDate(2022, 8, 31), QTime(15, 45, 5));

   REQUIRE(dt1 <  dt2);
   REQUIRE(dt1 <= dt2);
   REQUIRE(dt1 <= dt1);
   REQUIRE(dt1 != dt2);

   REQUIRE(dt2 >  dt1);
   REQUIRE(dt2 >= dt1);
   REQUIRE(dt2 >= dt2);

   {
      dt1 = QDateTime();
      REQUIRE(dt1.isValid() == false);

      dt2 = QDate(1970, 1, 1).startOfDay();
      REQUIRE(dt2.isValid() == true);

      REQUIRE(dt1 != dt2);
   }
}

TEST_CASE("QDateTime secsTo", "[qdatetime]")
{
   QDateTime dt1 = QDateTime(QDate(2022, 8, 31), QTime(15, 40, 5));
   QDateTime dt2 = QDateTime(QDate(2022, 8, 31), QTime(15, 45, 5));

   REQUIRE(dt1.secsTo(dt2) == 300);
   REQUIRE(dt2.secsTo(dt1) == -300);
}

TEST_CASE("QDateTime set", "[qdatetime]")
{
   QDateTime dt = QDateTime(QDate(2022, 8, 31), QTime(15, 40, 5));

   dt.setDate(QDate(2023, 6, 10));
   REQUIRE(dt.date() == QDate(2023, 6, 10));

   dt.setTime(QTime(11, 30, 00));
   REQUIRE(dt.time() == QTime(11, 30, 00));
}

TEST_CASE("QDateTime setEpoch", "[qdatetime]")
{
   QDateTime dt1;
   QDateTime dt2;

   {
      dt1.setMSecsSinceEpoch(0);
      dt2 = QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0), QTimeZone::utc());

      REQUIRE(dt1 == dt2);

      dt1.setMSecsSinceEpoch(300000);
      dt2 = QDateTime(QDate(1970, 1, 1), QTime(0, 5, 0), QTimeZone::utc());

      REQUIRE(dt1 == dt2);
   }

   {
      dt1.setSecsSinceEpoch(0);
      dt2 = QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0), QTimeZone::utc());

      REQUIRE(dt1 == dt2);

      dt1.setSecsSinceEpoch(600);
      dt2 = QDateTime(QDate(1970, 1, 1), QTime(0, 10, 0), QTimeZone::utc());

      REQUIRE(dt1 == dt2);
   }
}

TEST_CASE("QDateTime time_t", "[qdatetime]")
{
   QDateTime dt1;
   QDateTime dt2;

   dt1.setTime_t(0);
   dt2 = QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0), QTimeZone::utc() );

   REQUIRE(dt1 == dt2);
   REQUIRE(dt1.toTime_t() == 0);
}

TEST_CASE("QDateTime to_string", "[qdatetime]")
{
   QDateTime dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 5));
   REQUIRE(dt.toString(Qt::ISODate) == "2017-10-31T15:45:05");

   dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 5), QTimeZone("UTC-07:00"));
   REQUIRE(dt.toString(Qt::ISODate) == "2017-10-31T15:45:05-07:00");

   dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 05), QTimeZone::utc());
   REQUIRE(dt.toString(Qt::ISODate) == "2017-10-31T15:45:05Z");

   dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 05), QTimeZone::utc());
   REQUIRE(dt.toString(Qt::TextDate) == "Tue Oct 31 15:45:05 2017 UTC");

   QString str = dt.toString("yyyy-MM-dd hh:mm:ss t");
   REQUIRE(str == QString("2017-10-31 15:45:05 UTC"));
}

TEST_CASE("QDateTime toLocalTime", "[qdatetime]")
{
   QDateTime dt_utc;
   QDateTime dt_local;

   {
      dt_utc   = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 30), QTimeZone::utc());

      dt_local = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 30), QTimeZone(0));
      dt_local.toTimeZone(QTimeZone::systemTimeZone());

      REQUIRE(dt_utc.toLocalTime() == dt_local);
   }

   {
      dt_utc   = QDateTime(QDate(2017, 12, 15), QTime(15, 45, 30), QTimeZone::utc());

      dt_local = QDateTime(QDate(2017, 12, 15), QTime(15, 45, 30), QTimeZone(0));
      dt_local.toTimeZone(QTimeZone::systemTimeZone());

      REQUIRE(dt_utc.toLocalTime() == dt_local);
   }
}

TEST_CASE("QDateTime to_utc", "[qdatetime]")
{
   QDateTime dt;

   {
      dt = QDateTime(QDate(2021, 10, 31), QTime(15, 45, 5), QTimeZone(-25200));

      REQUIRE(dt.toUTC() == QDateTime(QDate(2021, 10, 31), QTime(22, 45, 5), QTimeZone::utc()));
      REQUIRE(dt.toUTC().toString(Qt::ISODate) == "2021-10-31T22:45:05Z");
   }

   {
      dt = QDateTime(QDate(1969, 12, 31), QTime(23, 59, 59), QTimeZone(0));

      REQUIRE(dt.toUTC() == QDateTime(QDate(1969, 12, 31), QTime(23, 59, 59), QTimeZone::utc()));
      REQUIRE(dt.toUTC().toString(Qt::ISODate) == "1969-12-31T23:59:59Z");
   }
}

TEST_CASE("QDateTime set_timezone", "[qdatetime]")
{
   QDateTime dt   = QDateTime(QDate(2023, 10, 31), QTime(9, 15, 0), QTimeZone::utc());
   QTimeZone zone = QTimeZone(3600);      // Germany

   const QDate hold_Date(dt.date());
   const QTime hold_Time(dt.time());

   dt.setTimeZone(zone);

   REQUIRE(dt.date() == hold_Date);
   REQUIRE(dt.time() == hold_Time);
   REQUIRE(dt.timeRepresentation() == zone);
}

TEST_CASE("QDateTime secs_to", "[qdatetime]")
{
   QDateTime dt1 = QDateTime(QDate(2017, 10, 31), QTime(9, 15, 0), QTimeZone::utc());

   {
      QDateTime dt2 = QDateTime(QDate(2017, 11, 23), QTime(13, 0, 0), QTimeZone::utc());
      REQUIRE(dt1.secsTo(dt2) == 2000700);
   }

   {
      QDateTime dt2 = QDateTime(QDate(2017, 10, 18), QTime(13, 0, 0), QTimeZone::utc());
      REQUIRE(dt1.secsTo(dt2) == -1109700);
   }
}

TEST_CASE("QDateTime swap", "[qdatetime]")
{
   QDateTime dt1 = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 5));
   QDateTime dt2 = QDateTime(QDate(2017, 2, 14),  QTime(9, 23, 18));

   dt1.swap(dt2);

   REQUIRE(dt1.date() == QDate(2017, 2, 14));
   REQUIRE(dt1.time() == QTime(9, 23, 18));

   REQUIRE(dt2.date() == QDate(2017, 10, 31));
   REQUIRE(dt2.time() == QTime(15, 45, 5));
}

TEST_CASE("QDateTime toTimeZone", "[qdatetime]")
{
   QTimeZone tz_ny = QTimeZone("America/New_York");
   QTimeZone tz_ca = QTimeZone("America/Los_Angeles");

   {
      QDateTime dt_ny(QDate(2022, 6, 1), QTime(15, 30, 0), tz_ny);

      QDateTime dt_ca = dt_ny.toTimeZone(tz_ca);

      REQUIRE(dt_ca.date() == QDate(2022, 6, 1));
      REQUIRE(dt_ca.time() == QTime(12, 30, 0));
      REQUIRE(dt_ca.timeZone() == tz_ca);
   }

   {
      QDateTime dt_ny(QDate(2022, 12, 1), QTime(15, 30, 0), tz_ny);

      QDateTime dt_ca = dt_ny.toTimeZone(tz_ca);

      REQUIRE(dt_ca.date() == QDate(2022, 12, 1));
      REQUIRE(dt_ca.time() == QTime(12, 30, 0));
      REQUIRE(dt_ca.timeZone() == tz_ca);
   }
}

TEST_CASE("QDateTime timezone_abbr", "[qdatetime]")
{
   QDateTime dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 05), QTimeZone::utc());
   REQUIRE(dt.timeZoneAbbreviation() == "UTC");

   dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 05), QTimeZone("UTC-07:00"));
   REQUIRE(dt.timeZoneAbbreviation() == "UTC-07:00");

   dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 05), QTimeZone(-25200));
   REQUIRE(dt.timeZoneAbbreviation() == "UTC-07:00");
}

TEST_CASE("QDateTime valid", "[qdatetime]")
{
   QDateTime dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 5));

   REQUIRE(dt.isNull() == false);
   REQUIRE(dt.isValid() == true);

   REQUIRE(dt.date() == QDate(2017, 10, 31));
   REQUIRE(dt.time() == QTime(15, 45, 05));
}

TEST_CASE("QDateTime duration", "[qdatetime]")
{
   QDateTime dt = QDateTime(QDate(2023, 10, 31), QTime(15, 45, 5), QTimeZone("America/Los_Angeles"));

   SECTION ("secs") {
      dt = dt.addDuration(std::chrono::seconds(20));

      REQUIRE(dt.date() == QDate(2023, 10, 31));
      REQUIRE(dt.time() == QTime(15, 45, 25));
   }

#if defined(CS_CHRONO_TYPES_CATCH)
   // C++20 only

   SECTION ("days") {
      dt = dt.addDuration(std::chrono::days(5));

      REQUIRE(dt.date() == QDate(2023, 11, 05));
      REQUIRE(dt.time() == QTime(14, 45, 05));      // dst occured on 11/04/23
   }
#endif
}

TEST_CASE("QDateTime std_chrono", "[qdatetime]")
{
   QDateTime dt;

   {
      dt = QDateTime(QDate(2023, 10, 31), QTime(15, 45, 5), QTimeZone::utc());

#if defined(CS_CHRONO_TYPES_CATCH)
      // C++20 only

      std::chrono::sys_time<std::chrono::milliseconds> dt_msec = dt.toStdSysMilliseconds();

      std::chrono::sys_time<std::chrono::milliseconds> chronoTime =
         std::chrono::sys_time<std::chrono::milliseconds>(std::chrono::milliseconds(1698767105000));

#else
      std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> dt_msec = dt.toStdSysMilliseconds();

      // 10/31/1923 15:45:05
      std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> chronoTime =
         std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(std::chrono::milliseconds(1698767105000));

#endif

      REQUIRE(dt_msec == chronoTime);
   }

   {
      dt = QDateTime(QDate(2023, 10, 31), QTime(15, 45, 5), QTimeZone::utc());

#if defined(CS_CHRONO_TYPES_CATCH)
      // C++20 only

      std::chrono::sys_time<std::chrono::seconds> dt_sec = dt.toStdSysSeconds();

      std::chrono::sys_time<std::chrono::seconds> chronoTime =
         std::chrono::sys_time<std::chrono::seconds>(std::chrono::seconds(1698767105));

#else
      std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> dt_sec = dt.toStdSysSeconds();

      // 10/31/1923 15:45:05
      std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> chronoTime =
         std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>(std::chrono::seconds(1698767105));
#endif

      REQUIRE(dt_sec == chronoTime);
   }

#if defined(CS_CHRONO_TYPES_CATCH)
   // c++20

   {
      dt = QDateTime(QDate(2023, 10, 31), QTime(15, 45, 5), QTimeZone("America/Los_Angeles"));

      // UTC-0700
      std::chrono::local_time<std::chrono::seconds> chronoTime =
         std::chrono::local_time<std::chrono::seconds>(std::chrono::seconds(1698763505));

      QDateTime localTime = QDateTime::fromStdLocalTime(chronoTime);

      REQUIRE(dt == localTime);
   }
#endif

}
