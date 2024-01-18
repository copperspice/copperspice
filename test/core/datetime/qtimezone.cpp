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

#include <qtimezone.h>

#include <cs_catch2.h>

TEST_CASE("QTimeZone traits", "[qtimezone]")
{
   REQUIRE(std::is_copy_constructible_v<QTimeZone> == true);
   REQUIRE(std::is_move_constructible_v<QTimeZone> == true);

   REQUIRE(std::is_copy_assignable_v<QTimeZone> == true);
   REQUIRE(std::is_move_assignable_v<QTimeZone> == true);

   REQUIRE(std::has_virtual_destructor_v<QTimeZone> == false);
}

TEST_CASE("QTimeZone daylightTime", "[qtimezone]")
{
   QTimeZone timeZone("America/New_York");

   QDateTime dt1 = QDateTime(QDate(2022, 1, 1), QTime(0, 0), QTimeZone::utc());
   QDateTime dt6 = QDateTime(QDate(2022, 6, 1), QTime(0, 0), QTimeZone::utc());

   {
      REQUIRE(timeZone.daylightTimeOffset(dt1) == 0);
      REQUIRE(timeZone.isDaylightTime(dt1) == false);
      REQUIRE(timeZone.standardTimeOffset(dt1) == -18000);

      REQUIRE(timeZone.hasTransitions() == true);
      REQUIRE(timeZone.nextTransition(dt1).atUtcMSecs == 1647154800000);
      REQUIRE(timeZone.nextTransition(dt6).atUtcMSecs == 1667714400000);
      REQUIRE(timeZone.previousTransition(dt1).atUtcMSecs == 1636264800000);
      REQUIRE(timeZone.previousTransition(dt6).atUtcMSecs == 1647154800000);
   }

   {
      REQUIRE(timeZone.daylightTimeOffset(dt6) == 3600);
      REQUIRE(timeZone.isDaylightTime(dt6) == true);
      REQUIRE(timeZone.standardTimeOffset(dt6) == -18000);

      REQUIRE(timeZone.offsetData(dt6).atUtcMSecs == 1654041600000);

      REQUIRE(timeZone.hasTransitions() == true);
      REQUIRE(timeZone.nextTransition(dt1).atUtcMSecs == 1647154800000);
      REQUIRE(timeZone.nextTransition(dt6).atUtcMSecs == 1667714400000);
      REQUIRE(timeZone.previousTransition(dt1).atUtcMSecs == 1636264800000);
      REQUIRE(timeZone.previousTransition(dt6).atUtcMSecs == 1647154800000);

      auto list = timeZone.transitions(dt1, dt6);
      REQUIRE(list.count() == 1);
      REQUIRE(list[0].atUtcMSecs == 1647154800000);
   }

   timeZone = QTimeZone::utc();

   REQUIRE(timeZone.abbreviation(dt1) == "UTC");
   REQUIRE(timeZone.isDaylightTime(dt1) == false);
   REQUIRE(timeZone.offsetData(dt1).atUtcMSecs == (std::numeric_limits<qint64>::min)());
   REQUIRE(timeZone.standardTimeOffset(dt1) == 0);

   timeZone = QTimeZone();

   REQUIRE(timeZone.abbreviation(dt1) == "");
   REQUIRE(timeZone.isDaylightTime(dt1) == false);
   REQUIRE(timeZone.offsetData(dt1).atUtcMSecs == (std::numeric_limits<qint64>::min)());
   REQUIRE(timeZone.standardTimeOffset(dt1) == 0);

   REQUIRE(QTimeZone::isTimeZoneIdAvailable("America/Los_Angeles") == true);
   REQUIRE(QTimeZone::isTimeZoneIdAvailable("America/MyHouse") == false);
}

TEST_CASE("QTimeZone equality", "[qtimezone]")
{
   QTimeZone timeZone1("America/Denver");
   QTimeZone timeZone2("America/New_York");

   REQUIRE(timeZone1 != timeZone2);
   REQUIRE(! (timeZone1 == timeZone2));

   REQUIRE(timeZone1.hasDaylightTime() == true);
   REQUIRE(timeZone2.hasDaylightTime() == true);

   REQUIRE(timeZone1.country() == timeZone2.country());
}

TEST_CASE("QTimeZone isValid", "[qtimezone]")
{
   // construct with an empty iana id
   QTimeZone timeZone1("");
   REQUIRE(timeZone1.isValid() == true);

   // default constructor
   QTimeZone timeZone2;
   REQUIRE(timeZone2.isValid() == false);
}

TEST_CASE("QTimeZone id_by_country", "[qtimezone]")
{
   QList<QByteArray> list = QTimeZone::availableTimeZoneIds(QLocale::UnitedStates);

   REQUIRE(list.size() > 5);

   REQUIRE(list.contains("America/Boise") == true);
   REQUIRE(list.contains("America/Chicago") == true);
   REQUIRE(list.contains("America/Denver") == true);
   REQUIRE(list.contains("America/Indiana/Petersburg") == true);
   REQUIRE(list.contains("America/Indiana/Winamac") == true);

   REQUIRE(list.contains("America/TeddyBear") == false);

   //
   list = QTimeZone::availableTimeZoneIds(QLocale::Canada);

   REQUIRE(list.size() > 5);

   REQUIRE(list.contains("America/Toronto") == true);
   REQUIRE(list.contains("America/Halifax") == true);
   REQUIRE(list.contains("America/Vancouver") == true);

   REQUIRE(list.contains("America/Denver") == false);

   //
   list = QTimeZone::availableTimeZoneIds(QLocale::Germany);

   REQUIRE(list.size() == 2);

   REQUIRE(list.contains("Europe/Berlin")   == true);
   REQUIRE(list.contains("Europe/Busingen") == true);

   //
   list = QTimeZone::availableTimeZoneIds(QLocale::Netherlands);

   REQUIRE(list.size() == 1);
   REQUIRE(list.contains("Europe/Amsterdam") == true);

   //
   list = QTimeZone::availableTimeZoneIds(QLocale::Sweden);

   REQUIRE(list.size() == 1);
   REQUIRE(list.contains("Europe/Stockholm") == true);
}

TEST_CASE("QTimeZone null", "[qtimezone]")
{
   QDateTime dt = QDateTime(QDate(2022, 1, 1), QTime(0, 0), QTimeZone::utc());
   QTimeZone timeZone;

   REQUIRE(timeZone.id() == QByteArray());
   REQUIRE(timeZone.comment() == QString());
   REQUIRE(timeZone.country() == QLocale::AnyCountry);
   REQUIRE(timeZone.displayName(dt) == QString());
   REQUIRE(timeZone.displayName(QTimeZone::StandardTime) == QString());

   REQUIRE(timeZone.offsetFromUtc(dt) == 0);

   REQUIRE(timeZone.daylightTimeOffset(dt) == 0);
   REQUIRE(timeZone.hasDaylightTime() == false);
   REQUIRE(timeZone.hasTransitions() == false);
   REQUIRE(timeZone.nextTransition(dt).atUtcMSecs == (std::numeric_limits<qint64>::min)());
   REQUIRE(timeZone.previousTransition(dt).atUtcMSecs == (std::numeric_limits<qint64>::min)());
}

TEST_CASE("QTimeZone offset", "[qtimezone]")
{
   QDateTime dt;
   QTimeZone timeZone("America/New_York");

   {
      dt = QDateTime(QDate(2017, 1, 1), QTime(0, 0, 0), QTimeZone::utc());
      REQUIRE(timeZone.offsetFromUtc(dt) == -18000);
   }

   {
      dt = QDateTime(QDate(2017, 8, 17), QTime(0, 0, 0), QTimeZone::utc());
      REQUIRE(timeZone.offsetFromUtc(dt) == -14400);
   }

   {
      dt = QDateTime(QDate(2017, 10, 31), QTime(0, 0, 0), QTimeZone::utc());
      REQUIRE(timeZone.offsetFromUtc(dt) == -14400);
   }

   {
      dt = QDateTime(QDate(2017, 11, 15), QTime(0, 0, 0), QTimeZone::utc());
      REQUIRE(timeZone.offsetFromUtc(dt) == -18000);
   }
}

TEST_CASE("QTimeZone windowsID", "[qtimezone]")
{
   REQUIRE(QTimeZone::ianaIdToWindowsId("America/Chicago")  == "Central Standard Time");
   REQUIRE(QTimeZone::ianaIdToWindowsId("America/Winnipeg") == "Central Standard Time");
   REQUIRE(QTimeZone::ianaIdToWindowsId("InvalidZone")      == "");

   REQUIRE(QTimeZone::windowsIdToDefaultIanaId("Central Standard Time") == "America/Chicago");
   REQUIRE(QTimeZone::windowsIdToDefaultIanaId("Central Standard Time", QLocale::Canada) == "America/Winnipeg");
}

TEST_CASE("QTimeZone various", "[qtimezone]")
{
   QByteArray id = "America/Los_Angeles";
   QDateTime date = QDateTime(QDate(2022, 6, 1), QTime(0, 0), QTimeZone::utc());
   QTimeZone zone = QTimeZone(id);

   QString result;

   result = zone.displayName(date);
   REQUIRE((result == "Pacific Daylight Time" || result == "PDT"));

   result = zone.displayName(QTimeZone::DaylightTime);
   REQUIRE((result == "Pacific Daylight Time" || result == "PDT"));

   result = zone.displayName(QTimeZone::StandardTime);
   REQUIRE((result == "Pacific Standard Time" || result == "PST"));

   result = zone.abbreviation(date);
   REQUIRE((result == "Pacific Daylight Time" || result == "PDT"));
}
