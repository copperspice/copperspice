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

#include <qstring8.h>
#include <qstringview.h>

#include <catch2/catch.hpp>

TEST_CASE("QStringView8 traits", "[qstringview8]")
{
   REQUIRE(std::is_copy_constructible_v<QStringView8> == true);
   REQUIRE(std::is_move_constructible_v<QStringView8> == true);

   REQUIRE(std::is_copy_assignable_v<QStringView8> == true);
   REQUIRE(std::is_move_assignable_v<QStringView8> == true);

   REQUIRE(std::has_virtual_destructor_v<QStringView8> == false);
}

TEST_CASE("QStringView8 begin_end", "[qstringview8]")
{
   QString str = "On a clear day you can see forever";
   QStringView8 view = str;

   {
      auto iterBegin = view.begin();
      auto iterEnd   = view.end();

      REQUIRE(*iterBegin == 'O');
      REQUIRE(*(iterEnd - 1) == 'r');
   }

   {
      auto iterBegin = view.constBegin();
      auto iterEnd   = view.constEnd();

      REQUIRE(*iterBegin == 'O');
      REQUIRE(*(iterEnd - 1) == 'r');
   }

   {
      auto iterBegin = view.cbegin();
      auto iterEnd   = view.cend();

      REQUIRE(*iterBegin == 'O');
      REQUIRE(*(iterEnd - 1) == 'r');
   }
}

TEST_CASE("QStringView8 chop", "[qstringview8]")
{
   QString str = "A wacky fox and sizeable pig";
   QStringView8 view = str;

   SECTION ("chop_a") {
      view.chop(8);
      REQUIRE(view == "A wacky fox and size");
   }

   SECTION ("chop_b") {
      view.chop(25);
      REQUIRE(view == "A w");
   }

   SECTION ("chop_c") {
      view.chop(27);
      REQUIRE(view == "A");
   }

   SECTION ("chop_d") {
      view.chop(28);
      REQUIRE(view == "");
   }

   SECTION ("chop_e") {
      view.chop(50);
      REQUIRE(view == "");
   }
}

TEST_CASE("QStringView8 contains", "[qstringview8]")
{
   QString8 str      = "A wacky fox and sizeable pig jumped halfway over a blue moon";
   QStringView8 view = str;

   REQUIRE(view.contains("jumped"));
   REQUIRE(! view.contains("lunch"));

   REQUIRE(view.contains('x'));
   REQUIRE(! view.contains('q'));

   REQUIRE(view.contains("jUmpeD", Qt::CaseInsensitive));
}

TEST_CASE("QStringView8 compare", "[qstringview8]")
{
   QString str1 = "apple";
   QString str2 = "APPLE";

   QStringView view1 = str1;
   QStringView view2 = str2;

   REQUIRE(view1.compare(view2, Qt::CaseInsensitive) == 0);
   REQUIRE(view1.compare(view2, Qt::CaseSensitive) == 1);
}

TEST_CASE("QStringView8 count", "[qstringview8]")
{
   QString8 str      = "A wacky fox and sizeable pig jumped halfway over a blue moon";
   QStringView8 view = str;

   REQUIRE(view.count('a') == 6);
   REQUIRE(view.count("a") == 6);

   REQUIRE(view.count('a', Qt::CaseInsensitive) == 7);
}

TEST_CASE("QStringView8 empty", "[qstringview8]")
{
   QStringView8 view;

   REQUIRE(view.isEmpty());

   REQUIRE(view.constBegin() == view.constEnd());
   REQUIRE(view.cbegin() == view.cend());
   REQUIRE(view.begin() == view.end());
}

TEST_CASE("QStringView8 ends_with", "[qstringview8]")
{
   QString str1 = "apple";
   QString str2 = "APPLE";

   QStringView8 view1 = str1;
   QStringView8 view2 = str2;

   {
      REQUIRE(view1.endsWith('e', Qt::CaseInsensitive));
      REQUIRE(view1.endsWith('e', Qt::CaseSensitive));
      REQUIRE(! view1.endsWith('E', Qt::CaseSensitive));

      REQUIRE(! view1.endsWith('t', Qt::CaseInsensitive));

      REQUIRE(view2.endsWith('e', Qt::CaseInsensitive));
      REQUIRE(! view2.endsWith('e', Qt::CaseSensitive));
      REQUIRE(view2.endsWith('E', Qt::CaseSensitive));
   }

   {
      REQUIRE(view1.endsWith("le", Qt::CaseInsensitive));
      REQUIRE(view1.endsWith("le", Qt::CaseSensitive));
      REQUIRE(! view1.endsWith("LE", Qt::CaseSensitive));

      REQUIRE(view2.endsWith("le", Qt::CaseInsensitive));
      REQUIRE(! view2.endsWith("le", Qt::CaseSensitive));
      REQUIRE(view2.endsWith("LE", Qt::CaseSensitive));
   }
}

TEST_CASE("QStringView8 index_of_fast", "[qstringview8]")
{
   QString8 str1 = "On a clear Day\0 you can see forever";
   QStringView8 view = str1;

   {
      auto iter = view.indexOfFast("dAY", view.cbegin(), Qt::CaseInsensitive);
      QString8 str2(iter, view.end());

      REQUIRE(str2 == "Day\0 you can see forever");
   }
}

TEST_CASE("QStringView8 last_index_of_fast", "[qstringview8]")
{
   QString8 str1 = "On a clear Day\0 you can see forever";
   QStringView8 view = str1;

   {
      auto iter = view.lastIndexOfFast("dAY", view.cbegin(), Qt::CaseInsensitive);
      QString8 str2(iter, view.end());

      REQUIRE(str2 == "Day\0 you can see forever");
   }
}

TEST_CASE("QStringView8 left", "[qstringview8]")
{
   QString8 str      = "A wacky fox and sizeable pig jumped halfway over a blue moon";
   QStringView8 view = str;

   REQUIRE(view.left(11) == "A wacky fox");
}

TEST_CASE("QStringView8 length", "[qstringview8]")
{
   QString8 str      = "A wacky fox and sizeable pig jumped halfway over a blue moon";
   QStringView8 view = str;

   REQUIRE(view.length() == str.length());
}

TEST_CASE("QStringView8 mid", "[qstringview8]")
{
   QString8 str      = "On a clear day you can see forever";
   QStringView8 view = str;

   REQUIRE(view.mid(11, 3) == "day");
}

TEST_CASE("QStringView8 operator", "[qstringview8]")
{
   QString8 str      = "On a clear day you can see forever";
   QStringView8 view = str;

   REQUIRE(view[11] == "d");
}

TEST_CASE("QStringView8 right", "[qstringview8]")
{
   QString8 str      = "A wacky fox and sizeable pig jumped halfway over a blue moon";
   QStringView8 view = str;

   REQUIRE(view.right(9) == "blue moon");
}

TEST_CASE("QStringView8 starts_with", "[qstringview8]")
{
   QString8 str      = "On a clear day you can see forever";
   QStringView8 view = str;

   REQUIRE(view.startsWith("On a clear") == true);
   REQUIRE(view.startsWith("on a clear") == false);

   REQUIRE(view.startsWith("on a clear", Qt::CaseInsensitive) == true);
}

TEST_CASE("QStringView8 trimmed", "[qstringview8]")
{
   QString8 str      = " orange  apple pear  quince  ";
   QStringView8 view = str;

   REQUIRE(view.trimmed() == "orange  apple pear  quince");
}
