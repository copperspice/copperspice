/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#include <qformat.h>

#include <catch2/catch.hpp>

TEST_CASE("qformat formatDebug-a", "[qformat]")
{
   static QString output;
   output.clear();

   csInstallMsgHandler([](QtMsgType, QStringView msg) {output = msg;});

   formatDebug("Display numeric value: {:d}", 42);
   REQUIRE(output == "Display numeric value: 42");

   csInstallMsgHandler(nullptr);
}

TEST_CASE("qformat formatDebug-b", "[qformat]")
{
   static QString output;
   output.clear();

   csInstallMsgHandler([](QtMsgType, QStringView msg) {output = msg;});

   formatDebug("Display numeric value: {:d}", 105LL);
   REQUIRE(output == "Display numeric value: 105");

   csInstallMsgHandler(nullptr);
}

TEST_CASE("qformat formatDebug-c", "[qformat]")
{
   static QString output;
   output.clear();

   csInstallMsgHandler([](QtMsgType, QStringView msg) {output = msg;});

   formatDebug("Display numeric value: {:x}", static_cast<std::size_t>(95));
   REQUIRE(output == "Display numeric value: 5f");

   formatDebug("Display numeric value: {:X}", static_cast<std::size_t>(95));
   REQUIRE(output == "Display numeric value: 5F");

   formatDebug("Display numeric value: {:#x}", static_cast<std::size_t>(95));
   REQUIRE(output == "Display numeric value: 0x5f");

   formatDebug("Display numeric value: {:#X}", static_cast<std::size_t>(95));
   REQUIRE(output == "Display numeric value: 0X5F");

   csInstallMsgHandler(nullptr);
}

TEST_CASE("qformat formatDebug-d", "[qformat]")
{
   static QString output;
   output.clear();

   csInstallMsgHandler([](QtMsgType, QStringView msg) {output = msg;});

   formatDebug("Display numeric value: {:b}", static_cast<std::ptrdiff_t>(17));
   REQUIRE(output == "Display numeric value: 10001");

   formatDebug("Display numeric value: {:#b}", static_cast<std::ptrdiff_t>(17));
   REQUIRE(output == "Display numeric value: 0b10001");

   csInstallMsgHandler(nullptr);
}

TEST_CASE("qformat formatDebug-e", "[qformat]")
{
   static QString output;
   output.clear();

   csInstallMsgHandler([](QtMsgType, QStringView msg) {output = msg;});

   formatDebug("Sunday Monday {:s}", QString("Tuesday"));
   REQUIRE(output == "Sunday Monday Tuesday");

   formatDebug("Saturday is day {:d} in {:s}", 6, QString("Germany"));
   REQUIRE(output == "Saturday is day 6 in Germany");

   csInstallMsgHandler(nullptr);
}

TEST_CASE("qformat qdebug-lld", "[qformat]")
{
   // generates a warning: format '%lld' expects argument of type 'long long int', but argument 2 has type 'int' [-Wformat=]
   // qDebug("Display numeric value: %lld", 42);
}

TEST_CASE("qformat formatPrint-warnings", "[.][qformat]")
{
   // hidden test


   // formatPrint("Display result: {:s} {:d} {:s} {:s}\n",
   //    QString("pear and grape"), 32, QString("apple"), "and plum");

   // ** remaining calls will display on the screen, enable for temporary testing
   // formatPrint("Display numeric value: {:s}\n", "apple and orange");

   // show text only
   // formatPrint("Display numeric value: %d\n", 42);

   // display --> "%d"
   // formatPrint("%d\n", 18);

   // display --> "%d"
   // formatPrint("%d\n");

   // formatDebug("Debug message, display numeric value: %d", 42);
   // formatWarning("Warning message, display numeric value: %s", "warning");
}

