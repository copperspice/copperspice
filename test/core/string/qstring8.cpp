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

#include <cs_catch2.h>

TEST_CASE("QString8 traits", "[qstring]")
{
   REQUIRE(std::is_copy_constructible_v<QString8> == true);
   REQUIRE(std::is_move_constructible_v<QString8> == true);

   REQUIRE(std::is_copy_assignable_v<QString8> == true);
   REQUIRE(std::is_move_assignable_v<QString8> == true);

   REQUIRE(std::has_virtual_destructor_v<QString8> == false);
}

TEST_CASE("QString8 append", "[qstring]")
{
   QString str = "A wacky fox and sizeable pig";

   str.append(" went to lunch");

   REQUIRE(str == "A wacky fox and sizeable pig went to lunch");
}

TEST_CASE("QString8 begin_end", "[qstring]")
{
   QString str = "On a clear day you can see forever";

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

TEST_CASE("QString8 clear", "[qstring]")
{
   QString str = "On a clear day you can see forever";

   str.clear();

   REQUIRE(str.length() == 0);
}

TEST_CASE("QString8 chop", "[qstring]")
{
   QString str = "A wacky fox and sizeable pig";

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

TEST_CASE("QString8 contains", "[qstring]")
{
   QString str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   REQUIRE(str.contains("jumped"));
   REQUIRE(! str.contains("lunch"));

   REQUIRE(str.contains('x'));
   REQUIRE(! str.contains('q'));

   REQUIRE(str.contains("jUmpeD", Qt::CaseInsensitive));
}

TEST_CASE("QString8 compare", "[qstring]")
{
   QString str1 = "apple";
   QString str2 = "APPLE";

   REQUIRE(str1.compare(str2, Qt::CaseInsensitive) == 0);
   REQUIRE(str1.compare(str2, Qt::CaseSensitive) == 1);
}

TEST_CASE("QString8 count", "[qstring]")
{
   QString str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   REQUIRE(str.count("o") == 4);
   REQUIRE(str.count("q") == 0);

   REQUIRE(str.count('a', Qt::CaseInsensitive) == 7);
}

TEST_CASE("QString8 empty", "[qstring]")
{
   QString str;

   REQUIRE(str.isEmpty());
   REQUIRE(str.constData()[0] == '\0');

   REQUIRE(str.constBegin() == str.constEnd());
   REQUIRE(str.cbegin() == str.cend());
   REQUIRE(str.begin() == str.end());
}

TEST_CASE("QString8 ends_with", "[qstring]")
{
   QString str1 = "apple";
   QString str2 = "APPLE";

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

TEST_CASE("QString8 find", "[qstring]")
{
   QString str1 = "On a clear day you can see forever";

   {
      int index = str1.find("day");
      QString str2  = str1.mid(index);

      REQUIRE(str2 == "day you can see forever");
   }

   {
      auto iter = str1.find_fast("day");
      QString str2(iter, str1.end());

      REQUIRE(str2 == "day you can see forever");
   }
}

TEST_CASE("QString8 index_of_fast", "[qstring]")
{
   QString str1 = "On a clear Day\0 you can see forever";

   {
      auto iter = str1.indexOfFast("dAY", str1.cbegin(), Qt::CaseInsensitive);
      QString str2(iter, str1.end());

      REQUIRE(str2 == "Day\0 you can see forever");
   }
}

TEST_CASE("QString8 last_index_of_fast", "[qstring]")
{
   QString str1 = "On a clear Day\0 you can see forever";

   {
      auto iter = str1.lastIndexOfFast("dAY", str1.cbegin(), Qt::CaseInsensitive);
      QString str2(iter, str1.end());

      REQUIRE(str2 == "Day\0 you can see forever");
   }
}

TEST_CASE("QString8 index", "[qstring]")
{
   QString str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

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

TEST_CASE("QString8 index_ignore_case", "[qstring]")
{
   QString str = "A wacky FOX and sizeable pig Jumped halfway over a blue moon";

   int position;

   {
      position = str.indexOf("fox", 0, Qt::CaseInsensitive);
      CHECK(position == 8);
   }

   {
      position = str.indexOf("FOX", 0, Qt::CaseInsensitive);
      CHECK(position == 8);
   }

   {
      position = str.indexOf("pig", 0, Qt::CaseInsensitive);
      REQUIRE(position == 25);
   }

   {
      position = str.indexOf("PIG", 0, Qt::CaseInsensitive);
      REQUIRE(position == 25);
   }

   {
      position = str.indexOf("jumped", 0, Qt::CaseInsensitive);
      REQUIRE(position == 29);
   }

   {
      position = str.indexOf("Jumped", 0, Qt::CaseInsensitive);
      REQUIRE(position == 29);
   }

   {
      position = str.indexOf("JUMPED", 0, Qt::CaseInsensitive);
      REQUIRE(position == 29);
   }
}

TEST_CASE("QString8 insert", "[qstring]")
{
   QString str1 = "Sunday Tuesday";
   QString str2 = "Monday ";

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

TEST_CASE("QString8 justify", "[qstring]")
{
   QString str = "grapefruit";

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

TEST_CASE("QString8 left_right", "[qstring]")
{
   QString str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   REQUIRE(str.left(11)   == "A wacky fox");
   REQUIRE(str.mid(16, 8) == "sizeable");
   REQUIRE(str.right(11)  == "a blue moon");
}

TEST_CASE("QString8 length", "[qstring]")
{
   QString str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   REQUIRE(str.length() == 60);
   REQUIRE(str.size() == 60);
}

TEST_CASE("QString8 normalized", "[qstring]")
{
   QString str1;
   QString str2;

   {
      // A with a circle over it, o with a hat   (composed)
      str1 = "\u00C5 \u00F4";
      str2 = str1.normalized( QString8::NormalizationForm_C );

      REQUIRE(str2.size() == 3);
      REQUIRE(str2.size_storage() == 5);
      REQUIRE(str2.storage_end() - str2.storage_begin() == 5);

      REQUIRE(str2[0] == U'\u00C5');
      REQUIRE(str2[1] == ' ');
      REQUIRE(str2[2] == U'\u00F4');

      REQUIRE(sizeof("\u00C5 \u00F4") == 6);

      REQUIRE(str2 == "\u00C5 \u00F4");
   }

   {
      // A with a circle over it, o with a hat  (composed)
      str1 = "\u00C5 \u00F4";
      str2 = str1.normalized( QString8::NormalizationForm_D );

      REQUIRE(str2 == "A\u030a o\u0302");
   }

   {
      // A with a circle over it, o with a hat  (decomposed)
      str1 = "A\u030a o\u0302";
      str2 = str1.normalized( QString8::NormalizationForm_C );

      REQUIRE(str2 == "\u00C5 \u00F4");
   }

   {
      // A with a circle over it, o with a hat  (decomposed)
      str1 = "A\u030a o\u0302";
      str2 = str1.normalized( QString8::NormalizationForm_D );

      REQUIRE(str2 == "A\u030a o\u0302");
   }
}

TEST_CASE("QString8 prepend", "[qstring]")
{
   QString str = "a wacky fox and sizeable pig";

   str.prepend("One day, ");

   REQUIRE(str == "One day, a wacky fox and sizeable pig");
}

TEST_CASE("QString8 remove", "[qstring]")
{
   QString str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

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
      str = str.remove(QRegularExpression("[aeiou]."));
      REQUIRE(str == "A wky f d sblp jp hfw  bl mn");
   }
}

TEST_CASE("QString8 repeated", "[qstring]")
{
   QString str = "A wacky fox and sizeable pig";

   str = str.left(12).repeated(3);

   REQUIRE(str == "A wacky fox A wacky fox A wacky fox ");
}

TEST_CASE("QString8 replace_a", "[qstring]")
{
   QString str = "A wacky fox went to lunch";

   str.replace(12, 13, "took a nap");

   REQUIRE(str == "A wacky fox took a nap");
}

TEST_CASE("QString8 replace_b", "[qstring]")
{
   QString8 str1 = "ß";

   QString8 str2 = str1;
   str2.replace("ß", "Found eszett");

   REQUIRE(str2 == "Found eszett");
}

TEST_CASE("QString8 replace_c", "[qstring]")
{
   QString str = "cow";

   str.replace('c', "cr");

   REQUIRE(str == "crow");
}

TEST_CASE("QString8 replace_regex_a", "[qstring]")
{
   QString8 str = "Sunday Monday Tuesday";

   QRegularExpression regExp("(da)y");

   {
      str.replace(regExp, "<\\0>");
      REQUIRE(str == "Sun<day> Mon<day> Tues<day>");
   }

   {
      str.replace(regExp, "\\1-\\1-\\1");
      REQUIRE(str == "Sun<da-da-da> Mon<da-da-da> Tues<da-da-da>");
   }
}

TEST_CASE("QString8 replace_regex_b", "[qstring]")
{
   QString str = "Sunday Monday Tuesday";

   QRegularExpression8 regExp("\\b(\\w*)([ou])(\\w*)");

   str.replace(regExp, "(\\1--\\2--\\3)");
   REQUIRE(str == "(S--u--nday) (M--o--nday) (T--u--esday)");
}

TEST_CASE("QString8 rfind", "[qstring]")
{
   QString str1 = "On a clear day you can see forever";

   {
      int index = str1.rfind("day");
      QString str2  = str1.mid(index);

      REQUIRE(str2 == "day you can see forever");
   }

   {
      auto iter  = str1.rfind_fast("day");
      QString str2(iter, str1.end());

      REQUIRE(str2 == "day you can see forever");
   }
}

TEST_CASE("QString8 section", "[qstring]")
{
   QString str1 = "/usr/local/bin/myapp";

   {
      QString str2 = QStringParser::section(str1, ' ', 1, 2);
      REQUIRE(str2 == "");
   }

   {
      QString str2 = QStringParser::section(str1, '/', 3, 4);
      REQUIRE(str2 == "bin/myapp");
   }

   {
      QString str2 = QStringParser::section(str1, "/", 3, 4);
      REQUIRE(str2 == "bin/myapp");
   }

   {
      QString str2 = str1.section('/', 3, 4);
      REQUIRE(str2 == "bin/myapp");
   }

   {
      QString str2 = str1.section("/", 3, 4);
      REQUIRE(str2 == "bin/myapp");
   }
}

TEST_CASE("QString8 starts_with", "[qstring]")
{
   QString str1 = "apple";
   QString str2 = "APPLE";

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

TEST_CASE("QString8 string_view", "[qstring]")
{
   QString str("A wacky fox and sizeable pig jumped halfway over a blue moon");

   {
      QStringView view = str.leftView(20);
      REQUIRE(view == "A wacky fox and size");
   }

   {
      QStringView view = str.rightView(11);
      REQUIRE(view == "a blue moon");
   }
}

TEST_CASE("QString8 swap", "[qstring]")
{
   QString str1 = "string one";
   QString str2 = "string two";

   swap(str1, str2);

   REQUIRE(str1 == "string two");
   REQUIRE(str2 == "string one");
}

TEST_CASE("QString8 truncate", "[qstring]")
{
   QString str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   str.truncate(20);

   REQUIRE(str == "A wacky fox and size");
}

TEST_CASE("QString8 toLower", "[qstring]")
{
   QString str = "Mango";

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

TEST_CASE("QString8 to_integer", "[qstring]")
{
   QString str = "FF";

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

TEST_CASE("QString8 utf8", "[qstring]")
{
   QString8 str = "!ä";

   REQUIRE(str.length() ==  2);

   REQUIRE(str[0].unicode() == char32_t(33));
   REQUIRE(str[1].unicode() == char32_t(228));
}

TEST_CASE("QString8 toLower_german", "[qstring]")
{
   QString8 str = QString::fromUtf8( "HeizölrückstoßabdämpFung \u20A8 \u2122" );

   SECTION ("toLower") {
      str = str.toLower();

      REQUIRE(str == QString8("heizölrückstoßabdämpfung \u20A8 \u2122"));

      REQUIRE(str.length() == 28);
      REQUIRE(str[4].unicode()  == char32_t(246));
      REQUIRE(str[13].unicode() == char32_t(223));
   }

   SECTION ("toUppper") {
      str = str.toUpper();
      REQUIRE(str == QString8("HEIZÖLRÜCKSTOSSABDÄMPFUNG \u20A8 \u2122"));
   }

   SECTION ("toCaseFolded") {
      str = str.toCaseFolded();
      REQUIRE(str == QString8("heizölrückstossabdämpfung \u20A8 \u2122"));
   }
}

TEST_CASE("QString8 trim", "[qstring]")
{
   QString str = "    A wacky fox and sizeable pig \n\t ";

   str = str.trimmed();

   REQUIRE(str == "A wacky fox and sizeable pig");
}

TEST_CASE("QString8 comparison", "[qstring]")
{
   QString str = "grapes";

   REQUIRE("apples" < str);
   REQUIRE(! (str < "apples"));

   REQUIRE("apples" <= str);
   REQUIRE(! (str <= "apples"));

   REQUIRE(! ("apples" > str));
   REQUIRE(str > "apples");

   REQUIRE(! ("apples" >= str));
   REQUIRE(str >= "apples");
}

TEST_CASE("QString8 storage_iterators", "[qstring]")
{
   QString str;

   REQUIRE(str.storage_begin()  == str.storage_end());
   REQUIRE(str.storage_rbegin() == str.storage_rend());

   str = "grape";
   REQUIRE(*str.storage_begin()  == 'g');
   REQUIRE(*str.storage_rbegin() == 'e');
   REQUIRE(str.storage_end() - str.storage_begin() == 5);

   // unicode 21B4, three storage units
   QString arrow = QString(UCHAR('↴'));
   str = "grape" + arrow;

   REQUIRE(*(str.storage_end() - 4) == 'e');
   REQUIRE(*(str.storage_rbegin() + 3) == 'e');
}

// test with u8

TEST_CASE("QString8 u8_contains", "[qstring]")
{
   QString str = u8"A wacky fox and sizeable pig jumped halfway over a blue moon";

   REQUIRE(str.contains(u8"jumped"));
   REQUIRE(! str.contains(u8"lunch"));

   REQUIRE(str.contains(u8'x'));
   REQUIRE(! str.contains(u8'q'));
}

TEST_CASE("QString8 u8_count", "[qstring]")
{
   QString str = u8"A wacky fox and sizeable pig jumped halfway over a blue moon";

   REQUIRE(str.count(u8"o") == 4);
   REQUIRE(str.count(u8"q") == 0);
}

TEST_CASE("QString8 u8_prepend", "[qstring]")
{
   QString str = u8"a wacky fox and sizeable pig";

   str.prepend(u8"One day, ");

   REQUIRE(str == u8"One day, a wacky fox and sizeable pig");
}

TEST_CASE("QString8 u8_replace", "[qstring]")
{
   QString str1 = u8"ß";

   QString str2 = str1;
   str2.replace(u8"ß", u8"Found eszett");

   REQUIRE(str2 == u8"Found eszett");
}


// C++20 only

TEST_CASE("QString8 char8_t_constructor", "[qstring]")
{
#if defined(__cpp_char8_t)
   const char8_t *data = u8"A wacky fox and sizeable pig";

   QString str = data;

   REQUIRE(str == "A wacky fox and sizeable pig");
#endif
}


