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

#include <qmap.h>

#include <cs_catch2.h>

TEST_CASE("QMap traits", "[qmap]")
{
   REQUIRE(std::is_copy_constructible_v<QMap<int, int>> == true);
   REQUIRE(std::is_move_constructible_v<QMap<int, int>> == true);

   REQUIRE(std::is_copy_assignable_v<QMap<int, int>> == true);
   REQUIRE(std::is_move_assignable_v<QMap<int, int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QMap<int, int>> == false);
}

TEST_CASE("QMap clear", "[qmap]")
{
   QMap<int, QString> map = { { 1, "watermelon"},
                              { 2, "apple"},
                              { 3, "pear"},
                              { 4, "grapefruit"} };

   map.clear();

   REQUIRE(map.size() == 0);
}

TEST_CASE("QMap contains", "[qmap]")
{
   QMap<int, QString> map = { { 1, "watermelon"},
                              { 2, "apple"},
                              { 3, "pear"},
                              { 4, "grapefruit"} };

   REQUIRE(map.contains(2));
   REQUIRE(! map.contains(9));
}

TEST_CASE("QMap count", "[qmap]")
{
   QMap<int, QString> map = { { 1, "watermelon"},
                              { 2, "apple"},
                              { 3, "pear"},
                              { 4, "grapefruit"} };

   REQUIRE(map.count() == 4);
   REQUIRE(map.size() == 4);
}

TEST_CASE("QMap empty", "[qmap]")
{
   QMap<int, QString> map;

   REQUIRE(map.isEmpty());
}

TEST_CASE("QMap erase", "[qmap]")
{
   QMap<int, QString> map = { { 1, "watermelon"},
                              { 2, "apple"},
                              { 3, "pear"},
                              { 4, "grapefruit"} };

   auto iter = map.find(2);
   map.erase(iter);

   REQUIRE(map.value(2) == "");

   REQUIRE(map.value(1) == "watermelon");
   REQUIRE(map.value(3) == "pear");
   REQUIRE(map.value(4) == "grapefruit");

   REQUIRE(map.size() == 3);
}

TEST_CASE("QMap equality", "[qmap]")
{
   QMap<int, QString> map1 = { { 1, "watermelon"},
                               { 2, "apple"},
                               { 3, "pear"},
                               { 4, "grapefruit"} };

   QMap<int, QString> map2 = { { 1, "watermelon"},
                               { 2, "apple"},
                               { 3, "pear"},
                               { 4, "grapefruit"} };
   {
      REQUIRE(map1 == map2);
      REQUIRE(! (map1 != map2));
   }

   {
      map2.remove(3);

      REQUIRE(! (map1 == map2));
      REQUIRE(map1 != map2);
   }
}

TEST_CASE("QMap equal_range", "[qmap]")
{
   QMap<int, QString> map = { { 1, "watermelon"},
                              { 2, "apple"},
                              { 3, "pear"},
                              { 4, "grapefruit"} };

   auto range = map.equal_range(2);
   QPair<int, QString> data = range.first.pair();

   REQUIRE(data.first  == 2);
   REQUIRE(data.second == "apple");
}

TEST_CASE("QMap first", "[qmap]")
{
   QMap<int, QString> map = { { 1, "watermelon"},
                              { 2, "apple"},
                              { 3, "pear"},
                              { 4, "grapefruit"} };

   REQUIRE(map.firstKey() == 1);
   REQUIRE(map.first() == "watermelon");
}

TEST_CASE("QMap insert", "[qmap]")
{
   QMap<int, QString> map = { { 1, "watermelon"},
                              { 2, "apple"},
                              { 3, "pear"},
                              { 4, "grapefruit"} };

   map.insert( {6, "mango"} );

   REQUIRE(map.value(6) == "mango");
   REQUIRE(map.size() == 5);
}

TEST_CASE("QMap insert_hint", "[qmap]")
{
   QMap<int, QString> map = { { 1, "watermelon"},
                              { 2, "apple"},
                              { 3, "pear"},
                              { 5, "quince"},
                              { 6, "grapefruit"} };

   auto iter = map.upperBound(4);
   map.insert( iter, 4, "mango" );

   REQUIRE(map.size() == 6);

   REQUIRE(map[4] == "mango");
   REQUIRE(map[5] == "quince");

   //
   map.insert( iter, 4, "peach" );
   REQUIRE(map[4] == "peach");
}

TEST_CASE("QMap last", "[qmap]")
{
   QMap<int, QString> map = { { 1, "watermelon"},
                              { 2, "apple"},
                              { 3, "pear"},
                              { 4, "grapefruit"} };

   REQUIRE(map.lastKey() == 4);
   REQUIRE(map.last() == "grapefruit");
}

TEST_CASE("QMap operator_bracket", "[qmap]")
{
   QMap<int, QString> map = { { 1, "watermelon"},
                              { 2, "apple"},
                              { 3, "pear"},
                              { 3, "quince"},
                              { 4, "grapefruit"} };

   REQUIRE(map[4] == "grapefruit");
   REQUIRE(map[5] == "");

   REQUIRE(map.contains(5) == true);
   REQUIRE(map[5] == "");
}

TEST_CASE("QMap remove", "[qmap]")
{
   QMap<int, QString> map = { { 1, "watermelon"},
                              { 2, "apple"},
                              { 3, "pear"},
                              { 4, "grapefruit"} };

   map.remove(3);

   REQUIRE(map.value(3) == "");

   REQUIRE(map.value(1) == "watermelon");
   REQUIRE(map.value(2) == "apple");
   REQUIRE(map.value(4) == "grapefruit");

   REQUIRE(map.size() == 3);
}

TEST_CASE("QMap swap", "[qmap]")
{
   QMap<int, QString> map1 = { { 1, "watermelon"},
                               { 2, "apple"},
                               { 3, "pear"},
                               { 4, "grapefruit"} };

   QMap<int, QString> map2 = { { 1, "grape"},
                               { 2, "orange"},
                               { 3, "peach"} };

   map1.swap(map2);

   REQUIRE(map1.value(2) == ("orange"));
   REQUIRE(map2.contains(4));
}

