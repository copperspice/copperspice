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

#include <qdatetime.h>

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
   QDateTime dt = QDateTime( QDate(2017, 10, 31), QTime(15, 45, 05));

   dt = dt.addSecs(65);

   REQUIRE(dt == QDateTime( QDate(2017, 10, 31), QTime(15, 46, 10)));
}

TEST_CASE("QDateTime add_mseconds", "[qdatetime]")
{
   QDateTime dt = QDateTime( QDate(2017, 10, 31), QTime(15, 45, 05));

   dt = dt.addMSecs(2000);

   REQUIRE(dt == QDateTime( QDate(2017, 10, 31), QTime(15, 45, 07)));
}

TEST_CASE("QDateTime days_to", "[qdatetime]")
{
   QDateTime dt1 = QDateTime(QDate(2017, 10, 31));

   {
      QDateTime dt2 = QDateTime(QDate(2017, 11, 15));
      REQUIRE(dt1.daysTo(dt2) == 15);
   }

   {
      QDateTime dt2 = QDateTime(QDate(2017, 10, 9));
      REQUIRE(dt1.daysTo(dt2) == -22);
   }
}

TEST_CASE("QDateTime null", "[qdatetime]")
{
   QDateTime dt;

   REQUIRE(dt.isNull());
   REQUIRE(! dt.isValid());
}

TEST_CASE("QDateTime secs_to", "[qdatetime]")
{
   QDateTime dt1 = QDateTime(QDate(2017, 10, 31), QTime(9, 15, 0), Qt::UTC);

   {
      QDateTime dt2 = QDateTime(QDate(2017, 11, 23), QTime(13, 0, 0), Qt::UTC);
      REQUIRE(dt1.secsTo(dt2) == 2000700);
   }

   {
      QDateTime dt2 = QDateTime(QDate(2017, 10, 18), QTime(13, 0, 0), Qt::UTC);
      REQUIRE(dt1.secsTo(dt2) == -1109700);
   }
}

TEST_CASE("QDateTime to_string", "[qdatetime]")
{
   QDateTime dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 5));
   REQUIRE(dt.toString(Qt::ISODate) == "2017-10-31T15:45:05");

   dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 5), Qt::OffsetFromUTC, -25200);
   REQUIRE(dt.toString(Qt::ISODate) == "2017-10-31T15:45:05-07:00");

   dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 05), Qt::UTC);
   REQUIRE(dt.toString(Qt::ISODate) == "2017-10-31T15:45:05Z");
}

TEST_CASE("QDateTime to_utc", "[qdatetime]")
{
   QDateTime dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 5), Qt::OffsetFromUTC, -25200);

   QDateTime result = dt.toUTC();

   REQUIRE(result.toString(Qt::ISODate) == "2017-10-31T22:45:05Z");
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

TEST_CASE("QDateTime timezone_abbr", "[qdatetime]")
{
   QDateTime dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 05), Qt::UTC);
   REQUIRE(dt.timeZoneAbbreviation() == "UTC");

   dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 05), Qt::OffsetFromUTC);
   REQUIRE(dt.timeZoneAbbreviation() == "UTC");

   dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 05), Qt::OffsetFromUTC, -25200);
   REQUIRE(dt.timeZoneAbbreviation() == "UTC-07:00");
}

TEST_CASE("QDateTime valid", "[qdatetime]")
{
   QDateTime dt = QDateTime(QDate(2017, 10, 31), QTime(15, 45, 5));

   REQUIRE(! dt.isNull());
   REQUIRE(dt.isValid());

   REQUIRE(dt.date() == QDate(2017, 10, 31));
   REQUIRE(dt.time() == QTime(15, 45, 05));
}



