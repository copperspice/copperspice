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

#include <qregularexpression.h>
#include <qstring8.h>
#include <qstringlist.h>
#include <qstringparser.h>

#include <cs_catch2.h>

TEST_CASE("QStringParser formatArg_str", "[qstringparser]")
{
   QString str1 = "Hello %1";

   SECTION("formatArg a") {
      str1 = str1.formatArg("CopperSpice");
      REQUIRE(str1 == "Hello CopperSpice");
   }

   SECTION ("formatArg b") {
      QString str2 = QStringParser::formatArg(str1, "CopperSpice");
      REQUIRE(str2 == "Hello CopperSpice");
   }

   SECTION("formatArg c") {
      str1 = str1.formatArg(17);
      REQUIRE(str1 == "Hello 17");
   }

   SECTION ("formatArg d") {
      QString str2 = QStringParser::formatArg(str1, 17);
      REQUIRE(str2 == "Hello 17");
   }
}

TEST_CASE("QStringParser formatArg_int", "[qstringparser]")
{
   QString str = "Value %1";
   int data    = 5400;

   SECTION ("formatArg a") {
      str = QStringParser::formatArg(str, data, 0, 8);
      REQUIRE(str == "Value 12430");
   }

   SECTION ("formatArg b") {
      str = QStringParser::formatArg(str, data, 0, 16);
      REQUIRE(str == "Value 1518");
   }
}

TEST_CASE("QStringParser formatArg16", "[qstringparser]")
{
   QString16 str = "Value %1";

   SECTION ("formatArg a") {
      str = QStringParser::formatArg(str, 5400, 0, 16);
      REQUIRE(str == "Value 1518");
   }

   SECTION ("formatArg b") {
      str = QStringParser::formatArg(str, 3.14, 0);
      REQUIRE(str == "Value 3.14");
   }
}

TEST_CASE("QStringParser number_a", "[qstringparser]")
{
   QString str;

   str = QStringParser::number(73);
   REQUIRE(str == "73");

   str = QStringParser::number(3.1415);
   REQUIRE(str == "3.1415");
}

TEST_CASE("QStringParser number_b", "[qstringparser]")
{
   QString str1 = "Numeric Value %1";

   {
      long value = -7654321;
      QString str2 = QStringParser::formatArg(str1, value);

      REQUIRE(str2 == "Numeric Value -7654321");
   }

   {
      ulong value = 7654321;
      QString str2 = QStringParser::formatArg(str1, value);

      REQUIRE(str2 == "Numeric Value 7654321");
   }

   {
      qint64 value = -7654321;
      QString str2 = QStringParser::formatArg(str1, value);

      REQUIRE(str2 == "Numeric Value -7654321");
   }

   {
      quint64 value = 7654321;
      QString str2 = QStringParser::formatArg(str1, value);

      REQUIRE(str2 == "Numeric Value 7654321");
   }
}

TEST_CASE("QStringParser section", "[qstringparser]")
{
   QString str = "apple,pear,grape,orange";
   QString result;

   //
   result = QStringParser::section(str, ',', 2, 2);
   REQUIRE(result == "grape");

   result = QStringParser::section(str, ',', -3, -2);
   REQUIRE(result == "pear,grape");

   result = QStringParser::section(str, ',', -3, 3);
   REQUIRE(result == "pear,grape,orange");

   //
   str = "/usr/local/bin/uic";

   result = QStringParser::section(str, '/', 3, 4);
   REQUIRE(result == "bin/uic");

   result = QStringParser::section(str, '/', 3, 3);
   REQUIRE(result == "bin");

   result = QStringParser::section(str, '/', 3, 3, QStringParser::SectionFlag::SectionSkipEmpty);
   REQUIRE(result == "uic");

   result = QStringParser::section(str, '/', 2, 2, QStringParser::SectionFlag::SectionIncludeLeadingSep);
   REQUIRE(result == "/local");

   result = QStringParser::section(str, '/', 5, 5, QStringParser::SectionFlag::SectionIncludeLeadingSep);
   REQUIRE(result == "");

   result = QStringParser::section(str, '/', 2, 2, QStringParser::SectionFlag::SectionIncludeTrailingSep);
   REQUIRE(result == "local/");

   result = QStringParser::section(str, '/', 2, 2,
         QStringParser::SectionFlag::SectionIncludeLeadingSep | QStringParser::SectionFlag::SectionIncludeTrailingSep);
   REQUIRE(result == "/local/");

   result = QStringParser::section(str, '/', -1);
   REQUIRE(result == "uic");

   //
   str = "*one*two*";

   result = QStringParser::section(str, '*', 1, 2, QStringParser::SectionFlag::SectionIncludeLeadingSep);
   REQUIRE(result == "*one*two");

   result = QStringParser::section(str, '*', 2, 2, QStringParser::SectionFlag::SectionIncludeLeadingSep);
   REQUIRE(result == "*two");

   result = QStringParser::section(str, '*', 3, 3, QStringParser::SectionFlag::SectionIncludeLeadingSep);
   REQUIRE(result == "*");

   result = QStringParser::section(str, '*', 0, 0, QStringParser::SectionFlag::SectionIncludeTrailingSep);
   REQUIRE(result == "*");
}

TEST_CASE("QStringParser section_regex", "[qstringparser]")
{
   QString str = "apple   pear grape    orange";
   QString result;

   QRegularExpression regExp("(\\s)+");

   result = QStringParser::section(str, regExp, 0, 0);
   REQUIRE(result == "apple");

   result = QStringParser::section(str, regExp, 0, 1);
   REQUIRE(result == "apple   pear");

   result = QStringParser::section(str, regExp, 0, 10);
   REQUIRE(result == "apple   pear grape    orange");

   result = QStringParser::section(str, regExp, 10, 10);
   REQUIRE(result == "");

   result = QStringParser::section(str, regExp, -1, -1);
   REQUIRE(result == "orange");

   result = QStringParser::section(str, regExp, -2, -1);
   REQUIRE(result == "grape    orange");

   result = QStringParser::section(str, regExp, -1, -2);
   REQUIRE(result == "");

   result = QStringParser::section(str, regExp, 2, 2, QStringParser::SectionFlag::SectionIncludeLeadingSep);
   REQUIRE(result == " grape");

   result = QStringParser::section(str, regExp, 2, 2, QStringParser::SectionFlag::SectionIncludeTrailingSep);
   REQUIRE(result == "grape    ");

   result = QStringParser::section(str, regExp, 2, 2,
         QStringParser::SectionFlag::SectionIncludeLeadingSep | QStringParser::SectionFlag::SectionIncludeTrailingSep);
   REQUIRE(result == " grape    ");
}

TEST_CASE("QStringParser split", "[qstringparser]")
{
   QString str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   QStringList data = QStringParser::split(str, ' ');

   REQUIRE(data[3] == "and");
   REQUIRE(data[11] == "moon");
}

TEST_CASE("QStringParser split_regex", "[qstringparser]")
{
   QString str = "apple#pear@grape#orange";

   QRegularExpression regExp("[#@]");
   QList<QString> list = QStringParser::split(str, regExp);

   REQUIRE(list.size() == 4);

   REQUIRE(list[0] == "apple");
   REQUIRE(list[1] == "pear");
   REQUIRE(list[2] == "grape");
   REQUIRE(list[3] == "orange");
}

TEST_CASE("QStringParser toInteger", "[qstringparser]")
{
   bool ok;

   {
      QString str = "0x80";

      int value = QStringParser::toInteger<int>(str, &ok, 0);
      REQUIRE(value == 128);
      REQUIRE(ok == true);
   }

   {
      QString str = "0xffff0000";

      int value = QStringParser::toInteger<int>(str, &ok, 0);
      REQUIRE(value == 0);
      REQUIRE(ok == false);
   }

   {
      QString str = "0xffff0000";

      uint value = QStringParser::toInteger<uint>(str, &ok, 0);
      REQUIRE(value == 0xffff0000);
      REQUIRE(ok == true);
   }
}

TEST_CASE("QStringParser formatArgs_str", "[qstringparser]")
{
   QString count = "1";
   QString total = "5";
   QString fName = "somefile.cpp";

   QString status = QString("Processing file %1 of %2: %3");
   status = QStringParser::formatArgs(status, count, total, fName);

   REQUIRE(status == "Processing file 1 of 5: somefile.cpp");
}

TEST_CASE("QStringParser formatArg_chain", "[qstringparser]")
{
   QString str = "Values (A):  %1  %2  %3  %4  %5  %6  %7  %8  %9  %3  %2  %10  %11  %12";

   str = str.formatArg("apple").formatArg("pear").formatArg(5).formatArg(3.14).formatArg("grape").formatArg(0);

   REQUIRE(str == "Values (A):  apple  pear  5  3.14  grape  0  %7  %8  %9  5  pear  %10  %11  %12");
}

TEST_CASE("QStringParser formatArg_no_chain", "[qstringparser]")
{
   QString str = "Values (B):  %1  %2  %3  %4  %5  %6  %7  %8  %9  %3  %2  %10  %11  %12";

   str = QStringParser::formatArg(str, "apple");
   str = QStringParser::formatArg(str, "pear");
   str = QStringParser::formatArg(str, 5);
   str = QStringParser::formatArg(str, 3.14);
   str = QStringParser::formatArg(str, "grape");
   str = QStringParser::formatArg(str, 0);

   REQUIRE(str == "Values (B):  apple  pear  5  3.14  grape  0  %7  %8  %9  5  pear  %10  %11  %12");
}
