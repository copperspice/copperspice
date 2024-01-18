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

#include <qstringlist.h>

#include <cs_catch2.h>

TEST_CASE("QStringList traits", "[qstringlist]")
{
   REQUIRE(std::is_copy_constructible_v<QStringList> == true);
   REQUIRE(std::is_move_constructible_v<QStringList> == true);

   REQUIRE(std::is_copy_assignable_v<QStringList> == true);
   REQUIRE(std::is_move_assignable_v<QStringList> == true);

   REQUIRE(std::has_virtual_destructor_v<QStringList> == false);
}

TEST_CASE("QStringList count", "[qstringlist]")
{
   QStringList list;

   REQUIRE(list.count() == 0);
   REQUIRE(list.size() == 0);
   REQUIRE(list.length() == 0);
   REQUIRE(list.isEmpty());
}

TEST_CASE("QStringList join", "[qstringlist]")
{
   QStringList list;

   list.resize(5);
   list[0] = "watermelon";
   list[1] = "orange";
   list[2] = "pear";
   list[3] = "apple";
   list[4] = "plum";

   QString str = list.join(", ");

   REQUIRE(str == "watermelon, orange, pear, apple, plum");
}

TEST_CASE("QStringList split", "[qstringlist]")
{
   QString str = "watermelon, orange, pear, apple, plum";

   QStringList list;
   list = str.split(", ");

   REQUIRE(list.count() == 5);

   REQUIRE(list[1] == "orange");
   REQUIRE(list[4] == "plum");
}

TEST_CASE("QStringList skip_empty_comma", "[qstringlist]")
{
   QString str = "watermelon, orange,, pear, apple, plum";

   QStringList list;
   list = str.split(',', QStringParser::SkipEmptyParts);

   REQUIRE(list.count() == 5);

   REQUIRE(list[1] == " orange");
   REQUIRE(list[4] == " plum");
}

TEST_CASE("QStringList skip_empty_and", "[qstringlist]")
{
   QString str = "watermelon and   orange   and pear and apple and plum";

   QStringList list;
   list = str.split("and", QStringParser::SkipEmptyParts);

   REQUIRE(list.count() == 5);

   REQUIRE(list[1].trimmed() == "orange");
   REQUIRE(list[4].trimmed() == "plum");
}


TEST_CASE("QStringList skip_empty_space", "[qstringlist]")
{
   QString str = "WaterMelon ORANGE   pear apple Plum";

   QStringList list;
   list = str.split(" ", QStringParser::SkipEmptyParts, Qt::CaseInsensitive);

   REQUIRE(list.count() == 5);

   REQUIRE(list[1].trimmed() == "ORANGE");
   REQUIRE(list[4].trimmed() == "Plum");
}

