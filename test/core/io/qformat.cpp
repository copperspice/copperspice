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

TEST_CASE("qformat formatter-qbytearray", "[qformat]")
{
   static QString output;
   output.clear();

   csInstallMsgHandler([](QtMsgType, QStringView msg) {output = msg;});

   formatDebug("Strawberries and {:s}", QByteArray("watermelon"));
   REQUIRE(output == "Strawberries and watermelon");

   formatDebug("{} {:s} and {} {}", 7, QByteArray("strawberries"), 1, QString("watermelon") );
   REQUIRE(output == "7 strawberries and 1 watermelon");

   csInstallMsgHandler(nullptr);
}

TEST_CASE("qformat formatter-qstring", "[qformat]")
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

TEST_CASE("qformat formatToQString", "[qformat]")
{
   QString output;

   output = formatToQString("There are {:d} {}, confirm you want to {}", 5, "unsaved files", "exit the program");
   REQUIRE(output == "There are 5 unsaved files, confirm you want to exit the program");

   output = formatToQString("There are {:d} {}, confirm you want to {}", 2, "files selected", "DELETE these files");
   REQUIRE(output == "There are 2 files selected, confirm you want to DELETE these files");

   output = formatToQString("You have {:s} {:d} {}", QString("modified"), 8, "files");
   REQUIRE(output == "You have modified 8 files");

   output = formatToQString("{:*<6d} files", 42);
   REQUIRE(output == "42**** files");

   output = formatToQString("{:*^6d} files", 42);
   REQUIRE(output == "**42** files");

   output = formatToQString("{:*>6d} files", 42);
   REQUIRE(output == "****42 files");

   output = formatToQString("|{:5}|", "ABC");
   REQUIRE(output == "|ABC  |");

   output = formatToQString("{:+}", 150);
   REQUIRE(output == "+150");

   output = formatToQString("{:.2f}", 10.5739);
   REQUIRE(output == "10.57");

   output = formatToQString("{:04}", 15);
   REQUIRE(output == "0015");

   output = formatToQString("{:04}", 127);
   REQUIRE(output == "0127");
}

TEST_CASE("qformat formatter-qpair", "[qformat]")
{
   // A
   QPair<int, int> pair_A(23, 37);

   std::string outputA1 = std::format("Values for QPair are: {}", pair_A);
   REQUIRE(outputA1 == "Values for QPair are: [23, 37]");

   QString outputB1 = formatToQString("Values for QPair are: {}", pair_A);
   REQUIRE(outputB1 == "Values for QPair are: [23, 37]");

   QString outputC1 = formatToQString("Show QPair values with # padding: {:#>10}", pair_A);
   REQUIRE(outputC1 == "Show QPair values with # padding: ##[23, 37]");

   // B
   QString data("apple");
   QPair<QString, int> pair_B(data, 37);

   std::string outputA2 = std::format("Values for QPair are: {}", pair_B);
   REQUIRE(outputA2 == "Values for QPair are: [apple, 37]");

   QString outputB2 = formatToQString("Values for QPair are: {}", pair_B);
   REQUIRE(outputB2 == "Values for QPair are: [apple, 37]");

   QString outputC2 = formatToQString("Show QPair values with padding: {:#>15}", pair_B);
   REQUIRE(outputC2 == "Show QPair values with padding: ####[apple, 37]");
}

// tools
TEST_CASE("qformat formatter-qline", "[qformat]")
{
   QLine line(18, 5, 106, 82);

   std::string outputA = std::format("Values for QLine are: {}", line);
   REQUIRE(outputA == "Values for QLine are: [18, 5, 106, 82]");

   QString outputB = formatToQString("Values for QLine are: {}", line);
   REQUIRE(outputB == "Values for QLine are: [18, 5, 106, 82]");

   QString outputC = formatToQString("Show QLine values with padding: {:>20}", line);
   REQUIRE(outputC == "Show QLine values with padding:     [18, 5, 106, 82]");
}

TEST_CASE("qformat formatter-qlinef", "[qformat]")
{
   QLineF line(39.02, 3.90, 90.52, 22.50);

   std::string outputA = std::format("Values for QLineF are: {}", line);
   REQUIRE(outputA == "Values for QLineF are: [39.02, 3.9, 90.52, 22.5]");

   QString outputB = formatToQString("Values for QLineF are: {}", line);
   REQUIRE(outputB == "Values for QLineF are: [39.02, 3.9, 90.52, 22.5]");

   QString outputC = formatToQString("Show QLineF values with padding: {:>30}", line);
   REQUIRE(outputC == "Show QLineF values with padding:      [39.02, 3.9, 90.52, 22.5]");
}

TEST_CASE("qformat formatter-qmargins", "[qformat]")
{
   QMargins data(25, 14, 200, 50);

   std::string outputA = std::format("Values for this QMargins: {}", data);
   REQUIRE(outputA == "Values for this QMargins: [25, 14, 200, 50]");

   QString outputB = formatToQString("Values for this QMargins: {}", data);
   REQUIRE(outputB == "Values for this QMargins: [25, 14, 200, 50]");

   QString outputC = formatToQString("Show QMargins values with stars: {:*>25}", data);
   REQUIRE(outputC == "Show QMargins values with stars: ********[25, 14, 200, 50]");
}

TEST_CASE("qformat formatter-qmarginsf", "[qformat]")
{
   QMarginsF data(29.73, 14, 203.61, 40.35);

   std::string outputA = std::format("Values for this QMarginsF: {}", data);
   REQUIRE(outputA == "Values for this QMarginsF: [29.73, 14, 203.61, 40.35]");

   QString outputB = formatToQString("Values for this QMarginsF: {}", data);
   REQUIRE(outputB == "Values for this QMarginsF: [29.73, 14, 203.61, 40.35]");

   QString outputC = formatToQString("Show QMarginsF values with stars: {:*>30}", data);
   REQUIRE(outputC == "Show QMarginsF values with stars: ****[29.73, 14, 203.61, 40.35]");
}

TEST_CASE("qformat formatter-qrect", "[qformat]")
{
   QRect rect(5, 10, 100, 150);

   std::string outputA = std::format("Values for this QRect: {}", rect);
   REQUIRE(outputA == "Values for this QRect: [5, 10, 100, 150]");

   QString outputB = formatToQString("Values for this QRect: {}", rect);
   REQUIRE(outputB == "Values for this QRect: [5, 10, 100, 150]");

   QString outputC = formatToQString("Show QRect values with stars: {:*>25}", rect);
   REQUIRE(outputC == "Show QRect values with stars: ********[5, 10, 100, 150]");
}

TEST_CASE("qformat formatter-qrectf", "[qformat]")
{
   QRectF rect(5.3, 10.91, 100.59, 150.01);

   std::string outputA = std::format("Values for this QRectF: {}", rect);
   REQUIRE(outputA == "Values for this QRectF: [5.3, 10.91, 100.59, 150.01]");

   QString outputB = formatToQString("Values for this QRectF: {}", rect);
   REQUIRE(outputB == "Values for this QRectF: [5.3, 10.91, 100.59, 150.01]");

   QString outputC = formatToQString("Show QRectF values with stars: {:*>30}", rect);
   REQUIRE(outputC == "Show QRectF values with stars: **[5.3, 10.91, 100.59, 150.01]");
}

