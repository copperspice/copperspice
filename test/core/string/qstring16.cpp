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
#include <qstring16.h>

#include <cs_catch2.h>

TEST_CASE("QString16 traits", "[qstring16]")
{
   REQUIRE(std::is_copy_constructible_v<QString16> == true);
   REQUIRE(std::is_move_constructible_v<QString16> == true);

   REQUIRE(std::is_copy_assignable_v<QString16> == true);
   REQUIRE(std::is_move_assignable_v<QString16> == true);

   REQUIRE(std::has_virtual_destructor_v<QString16> == false);
}

TEST_CASE("QString16 append", "[qstring16]")
{
   QString16 str = "A wacky fox and sizeable pig";

   str.append(" went to lunch");

   REQUIRE(str == "A wacky fox and sizeable pig went to lunch");
}

TEST_CASE("QString16 begin_end", "[qstring16]")
{
   QString16 str = "On a clear day you can see forever";

   {
      auto iterBegin = str.begin();
      auto iterEnd   = str.end();

      REQUIRE(*iterBegin == 'O');
      REQUIRE(*(iterEnd - 1) == 'r');
   }

   {
      auto iterBegin = str.constBegin();
      auto iterEnd   = str.constEnd();

      REQUIRE(*iterBegin == 'O');
      REQUIRE(*(iterEnd - 1) == 'r');
   }

   {
      auto iterBegin = str.cbegin();
      auto iterEnd   = str.cend();

      REQUIRE(*iterBegin == 'O');
      REQUIRE(*(iterEnd - 1) == 'r');
   }
}

TEST_CASE("QString16 clear", "[qstring16]")
{
   QString16 str = "On a clear day you can see forever";

   str.clear();

   REQUIRE(str.length() == 0);
}

TEST_CASE("QString16 chop", "[qstring16]")
{
   QString16 str = "A wacky fox and sizeable pig";

   SECTION ("chop_a") {
      str.chop(8);
      REQUIRE(str == "A wacky fox and size");
   }

   SECTION ("chop_b") {
      str.chop(25);
      REQUIRE(str == "A w");
   }

   SECTION ("chop_c") {
      str.chop(27);
      REQUIRE(str == "A");
   }

   SECTION ("chop_d") {
      str.chop(28);
      REQUIRE(str == "");
   }

   SECTION ("chop_e") {
      str.chop(50);
      REQUIRE(str == "");
   }
}

TEST_CASE("QString16 contains", "[qstring16]")
{
   QString16 str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   REQUIRE(str.contains("jumped"));
   REQUIRE(! str.contains("lunch"));

   REQUIRE(str.contains('x'));
   REQUIRE(! str.contains('q'));

   REQUIRE(str.contains("jUmpeD", Qt::CaseInsensitive));
}

TEST_CASE("QString16 compare", "[qstring16]")
{
   QString16 str1 = "apple";
   QString16 str2 = "APPLE";

   REQUIRE(str1.compare(str2, Qt::CaseInsensitive) == 0);
   REQUIRE(str1.compare(str2, Qt::CaseSensitive) == 1);
}
TEST_CASE("QString16 count", "[qstring16]")
{
   QString16 str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   REQUIRE(str.count("o") == 4);
   REQUIRE(str.count("q") == 0);

   REQUIRE(str.count('a', Qt::CaseInsensitive) == 7);
}

TEST_CASE("QString16 empty", "[qstring16]")
{
   QString16 str;

   REQUIRE(str.isEmpty());
   REQUIRE(str.constData()[0] == '\0');

   REQUIRE(str.constBegin() == str.constEnd());
   REQUIRE(str.cbegin() == str.cend());
   REQUIRE(str.begin() == str.end());
}

TEST_CASE("QString16 ends_with", "[qstring16]")
{
   QString16 str1 = "apple";
   QString16 str2 = "APPLE";

   {
      REQUIRE(str1.endsWith('e', Qt::CaseInsensitive));
      REQUIRE(str1.endsWith('e', Qt::CaseSensitive));
      REQUIRE(! str1.endsWith('E', Qt::CaseSensitive));

      REQUIRE(! str1.endsWith('t', Qt::CaseInsensitive));

      REQUIRE(str2.endsWith('e', Qt::CaseInsensitive));
      REQUIRE(! str2.endsWith('e', Qt::CaseSensitive));
      REQUIRE(str2.endsWith('E', Qt::CaseSensitive));
   }

   {
      REQUIRE(str1.endsWith("le", Qt::CaseInsensitive));
      REQUIRE(str1.endsWith("le", Qt::CaseSensitive));
      REQUIRE(! str1.endsWith("LE", Qt::CaseSensitive));

      REQUIRE(str2.endsWith("le", Qt::CaseInsensitive));
      REQUIRE(! str2.endsWith("le", Qt::CaseSensitive));
      REQUIRE(str2.endsWith("LE", Qt::CaseSensitive));
   }
}

TEST_CASE("QString16 find", "[qstring16]")
{
   QString16 str1 = "On a clear day you can see forever";

   {
      int index = str1.find("day");
      QString16 str2  = str1.mid(index);

      REQUIRE(str2 == "day you can see forever");
   }

   {
      auto iter  = str1.find_fast("day");
      QString16 str2(iter, str1.end());

      REQUIRE(str2 == "day you can see forever");
   }
}

TEST_CASE("QString16 index_of_fast", "[qstring16]")
{
   QString16 str1 = "On a clear Day\0 you can see forever";

   {
      auto iter = str1.indexOfFast("dAY", str1.cbegin(), Qt::CaseInsensitive);
      QString16 str2(iter, str1.end());

      REQUIRE(str2 == "Day\0 you can see forever");
   }
}

TEST_CASE("QString16 last_index_of_fast", "[qstring16]")
{
   QString16 str1 = "On a clear Day\0 you can see forever";

   {
      auto iter = str1.lastIndexOfFast("dAY", str1.cbegin(), Qt::CaseInsensitive);
      QString16 str2(iter, str1.end());

      REQUIRE(str2 == "Day\0 you can see forever");
   }
}

TEST_CASE("QString16 index", "[qstring16]")
{
   QString16 str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   int position;

   {
      position = str.indexOf('w');
      REQUIRE(position == 2);
   }

   {
      position = str.indexOf("size");
      REQUIRE(position == 16);
   }

   {
      position = str.lastIndexOf('w');
      REQUIRE(position == 40);
   }

   {
      position = str.lastIndexOf("blue");
      REQUIRE(position == 51);
   }
}

TEST_CASE("QString16 insert", "[qstring16]")
{
   QString16 str1 = "Sunday Tuesday";
   QString16 str2 = "Monday ";

   SECTION ("insert_a") {
      str1.insert(7, str2);
      REQUIRE(str1 == "Sunday Monday Tuesday");
   }

   SECTION ("insert_b") {
      str1.insert(str1.begin() + 7, str2);
      REQUIRE(str1 == "Sunday Monday Tuesday");
   }

   SECTION ("insert_c") {
      str1.insert(str1.begin() + 7, str2.begin(), str2.end());
      REQUIRE(str1 == "Sunday Monday Tuesday");
   }

   SECTION ("insert_d") {
      str1.insert(6, QChar('!'));
      REQUIRE(str1 == "Sunday! Tuesday");
   }

   SECTION ("insert_e") {
      QChar32 data[] = {'M', 'o', 'n', 'd', 'a', 'y', ' '};

      str1.insert(7, data, std::size(data));
      REQUIRE(str1 == "Sunday Monday Tuesday");
   }
}

TEST_CASE("QString16 justify", "[qstring16]")
{
   QString16 str = "grapefruit";

   SECTION ("left justify") {
      str = str.leftJustified(15, 'X');
      REQUIRE(str == "grapefruitXXXXX");

      str = str.leftJustified(12, 'X', true);
      REQUIRE(str == "grapefruitXX");
   }

   SECTION ("right justify") {
      str = str.rightJustified(15, 'Y');
      REQUIRE(str == "YYYYYgrapefruit");
   }
}

TEST_CASE("QString16 left_right", "[qstring16]")
{
   QString16 str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   REQUIRE(str.left(11)   == "A wacky fox");
   REQUIRE(str.mid(16, 8) == "sizeable");
   REQUIRE(str.right(11)  == "a blue moon");
}

TEST_CASE("QString16 length", "[qstring16]")
{
   QString16 str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   REQUIRE(str.length() == 60);
   REQUIRE(str.size() == 60);
}

TEST_CASE("QString16 normalized", "[qstring16]")
{
   QString16 str1;
   QString16 str2;

   {
      // A with a circle over it, o with a hat   (composed)
      str1 = "\u00C5 \u00F4";
      str2 = str1.normalized( QString16::NormalizationForm_C );

      REQUIRE(str2 == "\u00C5 \u00F4");
   }

   {
      // A with a circle over it, o with a hat  (composed)
      str1 = "\u00C5 \u00F4";
      str2 = str1.normalized( QString16::NormalizationForm_D );

      REQUIRE(str2 == "A\u030a o\u0302");
   }

   {
      // A with a circle over it, o with a hat  (decomposed)
      str1 = "A\u030a o\u0302";
      str2 = str1.normalized( QString16::NormalizationForm_C );

      REQUIRE(str2 == "\u00C5 \u00F4");
   }

   {
      // A with a circle over it, o with a hat  (decomposed)
      str1 = "A\u030a o\u0302";
      str2 = str1.normalized( QString16::NormalizationForm_D );

      REQUIRE(str2 == "A\u030a o\u0302");
   }
}

TEST_CASE("QString16 prepend", "[qstring16]")
{
   QString16 str = "a wacky fox and sizeable pig";

   str.prepend("One day, ");

   REQUIRE(str == "One day, a wacky fox and sizeable pig");
}

TEST_CASE("QString16 remove", "[qstring16]")
{
   QString16 str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   SECTION ("remove a") {
      str = str.remove(2, 14);
      REQUIRE(str == "A sizeable pig jumped halfway over a blue moon");
   }

   SECTION ("remove b") {
      str = str.remove("halfway ");
      REQUIRE(str == "A wacky fox and sizeable pig jumped over a blue moon");
   }

   SECTION ("remove c") {
      str = str.remove("SizeAble", Qt::CaseInsensitive);
      REQUIRE(str == "A wacky fox and  pig jumped halfway over a blue moon");
   }

   SECTION ("remove d") {
      str = str.remove(QChar32('a'));
      REQUIRE(str == "A wcky fox nd sizeble pig jumped hlfwy over  blue moon");
   }

   SECTION ("remove e") {
      str = str.remove(QRegularExpression16("[aeiou]."));
      REQUIRE(str == "A wky f d sblp jp hfw  bl mn");
   }
}

TEST_CASE("QString16 repeated", "[qstring16]")
{
   QString16 str = "A wacky fox and sizeable pig";

   str = str.left(12).repeated(3);

   REQUIRE(str == "A wacky fox A wacky fox A wacky fox ");
}

TEST_CASE("QString16 replace_a", "[qstring16]")
{
   QString16 str = "A wacky fox went to lunch";

   str.replace(12, 13, "took a nap");

   REQUIRE(str == "A wacky fox took a nap");
}

TEST_CASE("QString16 replace_b", "[qstring16]")
{
   QString16 str1 = "ß";

   QString16 str2 = str1;
   str2.replace("ß", "Found eszett");

   REQUIRE(str2 == "Found eszett");
}

TEST_CASE("QString16 replace_c", "[qstring16]")
{
   QString16 str = "cow";

   str.replace('c', "cr");

   REQUIRE(str == "crow");
}

TEST_CASE("QString16 replace_regex_a", "[qstring16]")
{
   QString16 str = "Sunday Monday Tuesday";

   QRegularExpression16 regExp("(da)y");

   {
      str.replace(regExp, "<\\0>");
      REQUIRE(str == "Sun<day> Mon<day> Tues<day>");
   }

   {
      str.replace(regExp, "\\1-\\1-\\1");
      REQUIRE(str == "Sun<da-da-da> Mon<da-da-da> Tues<da-da-da>");
   }
}

TEST_CASE("QString16 replace_regex_b", "[qstring16]")
{
   QString16 str = "Sunday Monday Tuesday";

   QRegularExpression16 regExp("\\b(\\w*)([ou])(\\w*)");

   str.replace(regExp, "(\\1--\\2--\\3)");
   REQUIRE(str == "(S--u--nday) (M--o--nday) (T--u--esday)");
}

TEST_CASE("QString16 rfind", "[qstring16]")
{
   QString16 str1 = "On a clear day you can see forever";

   {
      int index = str1.rfind("day");
      QString16 str2  = str1.mid(index);

      REQUIRE(str2 == "day you can see forever");
   }

   {
      auto iter  = str1.rfind_fast("day");
      QString16 str2(iter, str1.end());

      REQUIRE(str2 == "day you can see forever");
   }
}

TEST_CASE("QString16 section", "[qstring16]")
{
   QString16 str1 = "/usr/local/bin/myapp";

   {
      QString16 str2 = QStringParser::section(str1, ' ', 1, 2);
      REQUIRE(str2 == "" );
   }

   {
      QString16 str2 = QStringParser::section(str1, '/', 3, 4);
      REQUIRE(str2 == "bin/myapp" );
   }

   {
      QString16 str2 = QStringParser::section(str1, "/", 3, 4);
      REQUIRE(str2 == "bin/myapp" );
   }

   {
      QString16 str2 = str1.section('/', 3, 4);
      REQUIRE(str2 == "bin/myapp" );
   }

   {
      QString16 str2 = str1.section("/", 3, 4);
      REQUIRE(str2 == "bin/myapp" );
   }
}

TEST_CASE("QString16 starts_with", "[qstring16]")
{
   QString16 str1 = "apple";
   QString16 str2 = "APPLE";

  {
      REQUIRE(str1.startsWith('a', Qt::CaseInsensitive));
      REQUIRE(str1.startsWith('a', Qt::CaseSensitive));
      REQUIRE(! str1.startsWith('A', Qt::CaseSensitive));

      REQUIRE(! str1.startsWith('t', Qt::CaseInsensitive));

      REQUIRE(str2.startsWith('a', Qt::CaseInsensitive));
      REQUIRE(! str2.startsWith('a', Qt::CaseSensitive));
      REQUIRE(str2.startsWith('A', Qt::CaseSensitive));
   }

   {
      REQUIRE(str1.startsWith("ap", Qt::CaseInsensitive));
      REQUIRE(str1.startsWith("ap", Qt::CaseSensitive));
      REQUIRE(! str1.startsWith("AP", Qt::CaseSensitive));

      REQUIRE(str2.startsWith("ap", Qt::CaseInsensitive));
      REQUIRE(! str2.startsWith("ap", Qt::CaseSensitive));
      REQUIRE(str2.startsWith("AP", Qt::CaseSensitive));
   }
}

TEST_CASE("QString16 string_view", "[qstring16]")
{
   QString16 str("A wacky fox and sizeable pig jumped halfway over a blue moon");

   {
      QStringView16 view = str.leftView(20);
      REQUIRE(view == u"A wacky fox and size");
   }

   {
      QStringView16 view = str.rightView(11);
      REQUIRE(view == u"a blue moon");
   }
}

TEST_CASE("QString16 swap", "[qstring16]")
{
   QString16 str1 = "string one";
   QString16 str2 = "string two";

   swap(str1, str2);

   REQUIRE(str1 == "string two");
   REQUIRE(str2 == "string one");
}

TEST_CASE("QString16 truncate", "[qstring16]")
{
   QString16 str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   str.truncate(20);

   REQUIRE(str == "A wacky fox and size");
}

TEST_CASE("QString16 toLower", "[qstring16]")
{
   QString16 str = "Mango";

   SECTION ("toLower") {
      str = str.toLower();
      REQUIRE(str == "mango");
   }

   SECTION ("toUppper") {
      str = str.toUpper();
      REQUIRE(str == "MANGO");
   }

   SECTION ("toCaseFolded") {
      str = str.toCaseFolded();
      REQUIRE(str == "mango");
   }
}


TEST_CASE("QString16 to_integer", "[qstring16]")
{
   QString16 str = "FF";

   bool ok;

   {
      int value = str.toInteger<int>(&ok, 16);
      REQUIRE(ok == true);
      REQUIRE(value == 255);
   }

   {
      int value = str.toInteger<int>(&ok, 10);
      REQUIRE(ok == false);
      REQUIRE(value == 0);
   }

   {
      str = "-73";

      int value = str.toInteger<int>();
      REQUIRE(value == -73);
   }

   {
      str = "85";

      short value = str.toInteger<short>();
      REQUIRE(value == 85);
   }

   {
      str = "1234.56";

      double value = str.toDouble();
      REQUIRE(value == 1234.56);
   }

   {
      str = "1234.56";

      float value = str.toFloat();
      REQUIRE(value == 1234.56006f);
   }
}

TEST_CASE("QString16 utf8", "[qstring16]")
{
   QString16 str = "!ä";

   REQUIRE(str.length() ==  2);

   REQUIRE(str[0].unicode() == char32_t(33));
   REQUIRE(str[1].unicode() == char32_t(228));
}

TEST_CASE("QString16 toLower_german", "[qstring16]")
{
   QString16 str = QString16::fromUtf8( "HeizölrückstoßabdämpFung \u20A8 \u2122" );

   SECTION ("toLower") {
      str = str.toLower();

      REQUIRE(str == QString16("heizölrückstoßabdämpfung \u20A8 \u2122"));

      REQUIRE(str.length() == 28);
      REQUIRE(str[4].unicode()  == char32_t(246));
      REQUIRE(str[13].unicode() == char32_t(223));
   }

   SECTION ("toUppper") {
      str = str.toUpper();
      REQUIRE(str == QString16("HEIZÖLRÜCKSTOSSABDÄMPFUNG \u20A8 \u2122"));
   }

   SECTION ("toCaseFolded") {
      str = str.toCaseFolded();
      REQUIRE(str == QString16("heizölrückstossabdämpfung \u20A8 \u2122"));
   }
}

TEST_CASE("QString16 trim", "[qstring16]")
{
   QString16 str = "    A wacky fox and sizeable pig \n\t ";

   str = str.trimmed();

   REQUIRE(str == "A wacky fox and sizeable pig");
}

TEST_CASE("QString16 comparison", "[qstring16]")
{
   QString16 str = "grapes";

   REQUIRE("apples" < str);
   REQUIRE(! (str < "apples"));

   REQUIRE("apples" <= str);
   REQUIRE(! (str <= "apples"));

   REQUIRE(! ("apples" > str));
   REQUIRE(str > "apples");

   REQUIRE(! ("apples" >= str));
   REQUIRE(str >= "apples");
}

TEST_CASE("QString16 storage_iterators", "[qstring16]")
{
   QString16 str;

   REQUIRE(str.storage_begin()  == str.storage_end());
   REQUIRE(str.storage_rbegin() == str.storage_rend());

   str = "grape";
   REQUIRE(*str.storage_begin()  == 'g');
   REQUIRE(*str.storage_rbegin() == 'e');
   REQUIRE(str.storage_end() - str.storage_begin() == 5);

   // unicode 1F600, two storage units
   QString16 face = QString16(UCHAR('\U0001F600'));
   str = "grape" + face;

   REQUIRE(*(str.storage_end() - 3) == 'e');
   REQUIRE(*(str.storage_rbegin() + 2) == 'e');
}