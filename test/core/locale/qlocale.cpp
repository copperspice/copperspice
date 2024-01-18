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

#include <qlocale.h>

#include <cs_catch2.h>

TEST_CASE("QLocale traits", "[qlocale]")
{
   REQUIRE(std::is_copy_constructible_v<QLocale> == true);
   REQUIRE(std::is_move_constructible_v<QLocale> == true);

   REQUIRE(std::is_copy_assignable_v<QLocale> == true);
   REQUIRE(std::is_move_assignable_v<QLocale> == true);

   REQUIRE(std::has_virtual_destructor_v<QLocale> == false);
}

TEST_CASE("QLocale constructor_enum", "[qlocale]")
{
   QLocale data  = QLocale(QLocale::English, QLocale::UnitedStates);

   REQUIRE(data.name() == "en_US");
   REQUIRE(data.nativeCountryName() == "United States");
   REQUIRE(data.nativeLanguageName() == "American English");
   REQUIRE(data.country()  == QLocale::UnitedStates);
   REQUIRE(data.language() == QLocale::English);
   REQUIRE(data.currencySymbol() == "$");
   REQUIRE(data.groupSeparator() == ",");
   REQUIRE(data.decimalPoint() == ".");
   REQUIRE(data.dateFormat(QLocale::ShortFormat) == "M/d/yy");
   REQUIRE(data.timeFormat(QLocale::ShortFormat) == QString("h:mm") + QChar(0x202f) + "AP");
   REQUIRE(data.dayName(1) == "Monday");

   //
   data = QLocale(QLocale::French, QLocale::France);

   REQUIRE(data.name() == "fr_FR");
   REQUIRE(data.nativeCountryName() == "France");
   REQUIRE(data.nativeLanguageName() == "français");
   REQUIRE(data.country()  == QLocale::France);
   REQUIRE(data.language() == QLocale::French);
   REQUIRE(data.currencySymbol() == "€");
   REQUIRE(data.groupSeparator().unicode() == 0x202F);  // narrow non-breaking
   REQUIRE(data.decimalPoint() == ",");
   REQUIRE(data.dateFormat(QLocale::ShortFormat) == "dd/MM/yyyy");
   REQUIRE(data.timeFormat(QLocale::ShortFormat) == "HH:mm");
   REQUIRE(data.dayName(1) == "lundi");

   //
   data = QLocale(QLocale::German, QLocale::Germany);

   REQUIRE(data.name() == "de_DE");
   REQUIRE(data.nativeCountryName() == "Deutschland");
   REQUIRE(data.nativeLanguageName() == "Deutsch");
   REQUIRE(data.country()  == QLocale::Germany);
   REQUIRE(data.language() == QLocale::German);
   REQUIRE(data.currencySymbol() == "€");
   REQUIRE(data.groupSeparator() == ".");
   REQUIRE(data.decimalPoint() == ",");
   REQUIRE(data.dateFormat(QLocale::ShortFormat) == "dd.MM.yy");
   REQUIRE(data.timeFormat(QLocale::ShortFormat) == "HH:mm");
   REQUIRE(data.dayName(1) == "Montag");

   //
   data = QLocale(QLocale::Dutch, QLocale::Netherlands);

   REQUIRE(data.name() == "nl_NL");
   REQUIRE(data.nativeCountryName() == "Nederland");
   REQUIRE(data.nativeLanguageName() == "Nederlands");
   REQUIRE(data.country()  == QLocale::Netherlands);
   REQUIRE(data.language() == QLocale::Dutch);
   REQUIRE(data.currencySymbol() == "€");
   REQUIRE(data.groupSeparator() == ".");
   REQUIRE(data.decimalPoint() == ",");
   REQUIRE(data.dateFormat(QLocale::ShortFormat) == "dd-MM-yyyy");
   REQUIRE(data.timeFormat(QLocale::ShortFormat) == "HH:mm");

   //
   data  = QLocale(QLocale::English, QLocale::UnitedKingdom);

   REQUIRE(data.name() == "en_GB");
   REQUIRE(data.nativeCountryName() == "United Kingdom");
   REQUIRE(data.nativeLanguageName() == "British English");
   REQUIRE(data.country()  == QLocale::UnitedKingdom);
   REQUIRE(data.language() == QLocale::English);
   REQUIRE(data.currencySymbol() == "£");
   REQUIRE(data.groupSeparator() == ",");
   REQUIRE(data.decimalPoint() == ".");
   REQUIRE(data.dateFormat(QLocale::ShortFormat) == "dd/MM/yyyy");
   REQUIRE(data.timeFormat(QLocale::ShortFormat) == "HH:mm");

   //
   data = QLocale(QLocale::Swedish, QLocale::Sweden);

   REQUIRE(data.name() == "sv_SE");
   REQUIRE(data.nativeCountryName() == "Sverige");
   REQUIRE(data.nativeLanguageName() == "svenska");
   REQUIRE(data.country()  == QLocale::Sweden);
   REQUIRE(data.language() == QLocale::Swedish);
   REQUIRE(data.currencySymbol() == "kr");
   REQUIRE(data.groupSeparator().unicode() == 160);      // non-breaking
   REQUIRE(data.decimalPoint() == ",");
   REQUIRE(data.dateFormat(QLocale::ShortFormat) == "yyyy-MM-dd");
   REQUIRE(data.timeFormat(QLocale::ShortFormat) == "HH:mm");

   //
   data = QLocale(QLocale::Italian, QLocale::Switzerland);

   REQUIRE(data.name() == "it_CH");
   REQUIRE(data.nativeCountryName() == "Svizzera");
   REQUIRE(data.nativeLanguageName() == "italiano");
   REQUIRE(data.country()  == QLocale::Switzerland);
   REQUIRE(data.language() == QLocale::Italian);
   REQUIRE(data.groupSeparator().unicode() == 0x2019);  // right quotation
   REQUIRE(data.decimalPoint() == ".");
   REQUIRE(data.dateFormat(QLocale::ShortFormat) == "dd.MM.yy");
   REQUIRE(data.timeFormat(QLocale::ShortFormat) == "HH:mm");

   //
   data = QLocale(QLocale::French, QLocale::Switzerland);

   REQUIRE(data.name() == "fr_CH");
   REQUIRE(data.nativeCountryName() == "Suisse");
   REQUIRE(data.nativeLanguageName() == "français suisse");
   REQUIRE(data.country()  == QLocale::Switzerland);
   REQUIRE(data.language() == QLocale::French);
   REQUIRE(data.currencySymbol() == "CHF");
   REQUIRE(data.groupSeparator().unicode() == 0x202F);  // narrow non-breaking
   REQUIRE(data.decimalPoint() == ",");
   REQUIRE(data.dateFormat(QLocale::ShortFormat) == "dd.MM.yy");
   REQUIRE(data.timeFormat(QLocale::ShortFormat) == "HH:mm");

   //
   data = QLocale(QLocale::German, QLocale::Switzerland);

   REQUIRE(data.name() == "de_CH");
   REQUIRE(data.nativeCountryName() == "Schweiz");
   REQUIRE(data.nativeLanguageName() == "Schweizer Hochdeutsch");
   REQUIRE(data.country()  == QLocale::Switzerland);
   REQUIRE(data.language() == QLocale::German);
   REQUIRE(data.currencySymbol() == "CHF");
   REQUIRE(data.groupSeparator().unicode() == 0x2019);  // right quotation
   REQUIRE(data.decimalPoint() == ".");
   REQUIRE(data.dateFormat(QLocale::ShortFormat) == "dd.MM.yy");
   REQUIRE(data.timeFormat(QLocale::ShortFormat) == "HH:mm");

   //
   data = QLocale(QLocale::Indonesian, QLocale::Indonesia);

   REQUIRE(data.name() == "id_ID");
   REQUIRE(data.nativeCountryName() == "Indonesia");
   REQUIRE(data.nativeLanguageName() == "Indonesia");
   REQUIRE(data.country()  == QLocale::Indonesia);
   REQUIRE(data.language() == QLocale::Indonesian);
   REQUIRE(data.currencySymbol() == "Rp");
   REQUIRE(data.groupSeparator() == ".");
   REQUIRE(data.decimalPoint() == ",");
   REQUIRE(data.dateFormat(QLocale::ShortFormat) == "dd/MM/yy");
   REQUIRE(data.timeFormat(QLocale::ShortFormat) == "HH.mm");

   //
   data = QLocale(QLocale::Turkish, QLocale::Turkey);

   REQUIRE(data.name() == "tr_TR");
   REQUIRE(data.nativeCountryName() == "Türkiye");
   REQUIRE(data.nativeLanguageName() == "Türkçe");
   REQUIRE(data.country()  == QLocale::Turkey);
   REQUIRE(data.language() == QLocale::Turkish);
   REQUIRE(data.currencySymbol() == "\u20ba");
   REQUIRE(data.groupSeparator() == ".");
   REQUIRE(data.decimalPoint() == ",");
   REQUIRE(data.dateFormat(QLocale::ShortFormat) == "d.MM.yyyy");
   REQUIRE(data.timeFormat(QLocale::ShortFormat) == "HH:mm");

   //
   data  = QLocale(QLocale::Portuguese, QLocale::Brazil);

   REQUIRE(data.name() == "pt_BR");
   REQUIRE(data.nativeCountryName() == "Brasil");
   REQUIRE(data.nativeLanguageName() == "português");
   REQUIRE(data.country()  == QLocale::Brazil);
   REQUIRE(data.language() == QLocale::Portuguese);
   REQUIRE(data.currencySymbol() == "R$");
   REQUIRE(data.groupSeparator() == ".");
   REQUIRE(data.decimalPoint() == ",");
   REQUIRE(data.dateFormat(QLocale::ShortFormat) == "dd/MM/yyyy");
   REQUIRE(data.timeFormat(QLocale::ShortFormat) == "HH:mm");

   //
   data = QLocale(QLocale::Vietnamese, QLocale::Vietnam);

   REQUIRE(data.name() == "vi_VN");
   REQUIRE(data.nativeCountryName() == "Việt Nam");
   REQUIRE(data.nativeLanguageName() == "Tiếng Việt");
   REQUIRE(data.country()  == QLocale::Vietnam);
   REQUIRE(data.language() == QLocale::Vietnamese);
   REQUIRE(data.currencySymbol() == "\u20AB");
   REQUIRE(data.groupSeparator() == ".");
   REQUIRE(data.decimalPoint() == ",");
   REQUIRE(data.dateFormat(QLocale::ShortFormat) == "dd/MM/yyyy");
   REQUIRE(data.timeFormat(QLocale::ShortFormat) == "HH:mm");
}

TEST_CASE("QLocale constructor_str", "[qlocale]")
{
   QLocale data = QLocale("en_US");

   REQUIRE(data.name() == "en_US");
   REQUIRE(data.nativeCountryName()  == "United States");
   REQUIRE(data.nativeLanguageName() == "American English");
   REQUIRE(data.country()  == QLocale::UnitedStates);
   REQUIRE(data.language() == QLocale::English);

   //
   data  = QLocale("fr_FR");

   REQUIRE(data.name() == "fr_FR");
   REQUIRE(data.nativeCountryName() == "France");
   REQUIRE(data.nativeLanguageName() == "français");
   REQUIRE(data.country()  == QLocale::France);
   REQUIRE(data.language() == QLocale::French);

   //
   data = QLocale("de_DE");

   REQUIRE(data.name() == "de_DE");
   REQUIRE(data.nativeCountryName() == "Deutschland");
   REQUIRE(data.nativeLanguageName() == "Deutsch");
   REQUIRE(data.country()  == QLocale::Germany);
   REQUIRE(data.language() == QLocale::German);

   //
   data = QLocale("nl_NL");

   REQUIRE(data.name() == "nl_NL");
   REQUIRE(data.nativeCountryName() == "Nederland");
   REQUIRE(data.nativeLanguageName() == "Nederlands");
   REQUIRE(data.country()  == QLocale::Netherlands);
   REQUIRE(data.language() == QLocale::Dutch);

   //
   data  = QLocale("en_GB");

   REQUIRE(data.name() == "en_GB");
   REQUIRE(data.nativeCountryName() == "United Kingdom");
   REQUIRE(data.nativeLanguageName() == "British English");
   REQUIRE(data.country()  == QLocale::UnitedKingdom);
   REQUIRE(data.language() == QLocale::English);

   //
   data = QLocale("sv_SE");

   REQUIRE(data.name() == "sv_SE");
   REQUIRE(data.nativeCountryName() == "Sverige");
   REQUIRE(data.nativeLanguageName() == "svenska");
   REQUIRE(data.country()  == QLocale::Sweden);
   REQUIRE(data.language() == QLocale::Swedish);

   //
   data = QLocale("it_CH");

   REQUIRE(data.name() == "it_CH");
   REQUIRE(data.nativeCountryName() == "Svizzera");
   REQUIRE(data.nativeLanguageName() == "italiano");
   REQUIRE(data.country()  == QLocale::Switzerland);
   REQUIRE(data.language() == QLocale::Italian);

   data = QLocale("fr_CH");

   REQUIRE(data.name() == "fr_CH");
   REQUIRE(data.nativeCountryName() == "Suisse");
   REQUIRE(data.nativeLanguageName() == "français suisse");
   REQUIRE(data.country()  == QLocale::Switzerland);
   REQUIRE(data.language() == QLocale::French);

   data = QLocale("de_CH");

   REQUIRE(data.name() == "de_CH");
   REQUIRE(data.nativeCountryName() == "Schweiz");
   REQUIRE(data.nativeLanguageName() == "Schweizer Hochdeutsch");
   REQUIRE(data.country()  == QLocale::Switzerland);
   REQUIRE(data.language() == QLocale::German);

   //
   data = QLocale("id_ID");

   REQUIRE(data.name() == "id_ID");
   REQUIRE(data.nativeCountryName() == "Indonesia");
   REQUIRE(data.nativeLanguageName() == "Indonesia");
   REQUIRE(data.country()  == QLocale::Indonesia);
   REQUIRE(data.language() == QLocale::Indonesian);

   //
   data = QLocale("tr_TR");

   REQUIRE(data.name() == "tr_TR");
   REQUIRE(data.nativeCountryName() == "Türkiye");
   REQUIRE(data.nativeLanguageName() == "Türkçe");
   REQUIRE(data.country()  == QLocale::Turkey);
   REQUIRE(data.language() == QLocale::Turkish);

   //
   data  = QLocale("pt_BR");

   REQUIRE(data.name() == "pt_BR");
   REQUIRE(data.nativeCountryName() == "Brasil");
   REQUIRE(data.nativeLanguageName() == "português");
   REQUIRE(data.country()  == QLocale::Brazil);
   REQUIRE(data.language() == QLocale::Portuguese);

   //
   data = QLocale("vi_VN");

   REQUIRE(data.name() == "vi_VN");
   REQUIRE(data.nativeCountryName() == "Việt Nam");
   REQUIRE(data.nativeLanguageName() == "Tiếng Việt");
   REQUIRE(data.country()  == QLocale::Vietnam);
   REQUIRE(data.language() == QLocale::Vietnamese);

/*
   data = QLocale("nv_US");

   REQUIRE(data.name() == "nv_US");
   REQUIRE(data.nativeCountryName()  == "United States");
   REQUIRE(data.nativeLanguageName() == "Navaho");
   REQUIRE(data.country()  == QLocale::UnitedStates);
   REQUIRE(data.language() == QLocale::Navaho);
*/

}

TEST_CASE("QLocale constructor_any_country", "[qlocale]")
{
   QLocale data = QLocale(QLocale::Arabic, QLocale::AnyCountry);

   REQUIRE(data.country()  == QLocale::Egypt);
   REQUIRE(data.language() == QLocale::Arabic);

   //
   data = QLocale(QLocale::Dutch, QLocale::AnyCountry);

   REQUIRE(data.country()  == QLocale::Netherlands);
   REQUIRE(data.language() == QLocale::Dutch);

   //
   data = QLocale(QLocale::German, QLocale::AnyCountry);

   REQUIRE(data.country()  == QLocale::Germany);
   REQUIRE(data.language() == QLocale::German);

   //
   data = QLocale(QLocale::Swedish, QLocale::AnyCountry);

   REQUIRE(data.country()  == QLocale::Sweden);
   REQUIRE(data.language() == QLocale::Swedish);

   //
   data = QLocale(QLocale::Spanish, QLocale::AnyCountry);

   REQUIRE(data.country()  == QLocale::Spain);
   REQUIRE(data.language() == QLocale::Spanish);

   //
   data = QLocale(QLocale::Uzbek, QLocale::AnyCountry);

   REQUIRE(data.country()  == QLocale::Uzbekistan);
   REQUIRE(data.language() == QLocale::Uzbek);
}

TEST_CASE("QLocale constructor_english", "[qlocale]")
{
   QList<QLocale> list_locale = QLocale::matchingLocales(QLocale::English, QLocale::AnyScript, QLocale::AnyCountry);

   QList<QLocale::Country> list_countries;
   for (auto item : list_locale) {
      list_countries.append(item.country());
   }

   REQUIRE(list_countries.contains(QLocale::Australia) == true);
   REQUIRE(list_countries.contains(QLocale::Belize) == true);
   REQUIRE(list_countries.contains(QLocale::Guam) == true);
   REQUIRE(list_countries.contains(QLocale::Fiji) == true);
   REQUIRE(list_countries.contains(QLocale::Jamaica) == true);
   REQUIRE(list_countries.contains(QLocale::UnitedKingdom) == true);
   REQUIRE(list_countries.contains(QLocale::UnitedStates) == true);
   REQUIRE(list_countries.contains(QLocale::UnitedStatesVirginIslands) == true);

   REQUIRE(list_countries.contains(QLocale::France) == false);
   REQUIRE(list_countries.contains(QLocale::Poland) == false);
   REQUIRE(list_countries.contains(QLocale::Russia) == false);

   //
   list_locale = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::Switzerland);

   QList<QLocale::Language> list_lang;
   for (auto item : list_locale) {
      list_lang.append(item.language());
   }

   REQUIRE(list_lang.contains(QLocale::French) == true);
   REQUIRE(list_lang.contains(QLocale::German) == true);
   REQUIRE(list_lang.contains(QLocale::Italian) == true);

   REQUIRE(list_lang.contains(QLocale::Russian) == false);
}

TEST_CASE("QLocale ampm", "[qlocale]")
{
   QLocale data  = QLocale(QLocale::C);

   REQUIRE(data.amText() == "AM");
   REQUIRE(data.pmText() == "PM");

   // german
   data = QLocale("de_DE");

   REQUIRE(data.amText() == "AM");
   REQUIRE(data.pmText() == "PM");

   // indonesia
   data  = QLocale("id_ID");

   REQUIRE(data.amText() == "AM");
   REQUIRE(data.pmText() == "PM");

   // sweden
   data  = QLocale("sv_SE");

   REQUIRE(data.amText() == "fm");
   REQUIRE(data.pmText() == "em");

   // netherlands
   data  = QLocale("nl_NL");

   REQUIRE(data.amText() == "a.m.");
   REQUIRE(data.pmText() == "p.m.");

   // sri lanka
   data = QLocale("ta_LK");
   REQUIRE(data.amText() == "முற்பகல்");
   REQUIRE(data.pmText() == "பிற்பகல்");
}

TEST_CASE("QLocale day_name", "[qlocale]")
{
   QLocale data(QLocale::C);

   REQUIRE(data.dayName(1) == "Monday");
   REQUIRE(data.dayName(6) == "Saturday");

   data = QLocale(QLocale::English, QLocale::UnitedStates);
   REQUIRE(data.dayName(1) == "Monday");
   REQUIRE(data.dayName(6) == "Saturday");

   data = QLocale(QLocale::German, QLocale::Germany);
   REQUIRE(data.dayName(1) == "Montag");
   REQUIRE(data.dayName(6) == "Samstag");
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

TEST_CASE("QLocale empty", "[qlocale]")
{
   QLocale data;

   REQUIRE(data == QLocale::system());
}

TEST_CASE("QLocale first_day_of_week", "[qlocale]")
{
   QLocale data(QLocale::C);

   REQUIRE(data.firstDayOfWeek() == Qt::Monday);

   data = QLocale(QLocale::English, QLocale::UnitedStates);
   REQUIRE(data.firstDayOfWeek() == Qt::Sunday);

   data = QLocale(QLocale::German, QLocale::Germany);
   REQUIRE(data.firstDayOfWeek() == Qt::Monday);

   data = QLocale(QLocale::Hebrew, QLocale::Israel);
   REQUIRE(data.firstDayOfWeek() == Qt::Sunday);
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

TEST_CASE("QLocale month_name", "[qlocale]")
{
   QLocale data(QLocale::C);

   REQUIRE(data.monthName(1)  == "January");
   REQUIRE(data.monthName(12) == "December");

   data = QLocale(QLocale::German, QLocale::Germany);
   REQUIRE(data.monthName(1)  == "Januar");
   REQUIRE(data.monthName(12) == "Dezember");
}

TEST_CASE("QLocale weekdays", "[qlocale]")
{
   QLocale data(QLocale::C);

   REQUIRE(data.weekdays().contains(Qt::Tuesday)  == true);
   REQUIRE(data.weekdays().contains(Qt::Saturday) == false);
   REQUIRE(data.weekdays().contains(Qt::Sunday)   == false);

   data = QLocale(QLocale::English, QLocale::UnitedStates);
   REQUIRE(data.weekdays().contains(Qt::Tuesday)  == true);
   REQUIRE(data.weekdays().contains(Qt::Saturday) == false);
   REQUIRE(data.weekdays().contains(Qt::Sunday)   == false);

   data = QLocale(QLocale::German, QLocale::Germany);
   REQUIRE(data.weekdays().contains(Qt::Tuesday)  == true);
   REQUIRE(data.weekdays().contains(Qt::Saturday) == false);
   REQUIRE(data.weekdays().contains(Qt::Sunday)   == false);

   data = QLocale(QLocale::Hebrew, QLocale::Israel);
   REQUIRE(data.weekdays().contains(Qt::Tuesday)  == true);
   REQUIRE(data.weekdays().contains(Qt::Friday)   == false);
   REQUIRE(data.weekdays().contains(Qt::Saturday) == false);
   REQUIRE(data.weekdays().contains(Qt::Sunday)   == true);
}
