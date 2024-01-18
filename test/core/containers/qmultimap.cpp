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

#include <qmultimap.h>

#include <cs_catch2.h>

TEST_CASE("QMultiMap traits", "[qmultimap]")
{
   REQUIRE(std::is_copy_constructible_v<QMultiMap<int, int>> == true);
   REQUIRE(std::is_move_constructible_v<QMultiMap<int, int>> == true);

   REQUIRE(std::is_copy_assignable_v<QMultiMap<int, int>> == true);
   REQUIRE(std::is_move_assignable_v<QMultiMap<int, int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QMultiMap<int, int>> == false);
}

TEST_CASE("QMultiMap clear", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                   { 2, "apple"},
                                   { 3, "pear"},
                                   { 3, "quince"},
                                   { 4, "grapefruit"} };

   map.clear();

   REQUIRE(map.size() == 0);
}

TEST_CASE("QMultiMap contains", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                   { 2, "apple"},
                                   { 3, "pear"},
                                   { 3, "quince"},
                                   { 4, "grapefruit"} };

   REQUIRE(map.contains(2));
   REQUIRE(! map.contains(9));
}

TEST_CASE("QMultiMap count", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                   { 2, "apple"},
                                   { 3, "pear"},
                                   { 3, "quince"},
                                   { 4, "grapefruit"} };

   REQUIRE(map.count() == 5);
   REQUIRE(map.size() == 5);
}

TEST_CASE("QMultiMap empty", "[qmultimap]")
{
   QMultiMap<int, QString> map;

   REQUIRE(map.isEmpty());
}

TEST_CASE("QMultiMap erase", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                   { 2, "apple"},
                                   { 3, "pear"},
                                   { 3, "quince"},
                                   { 4, "grapefruit"} };

   auto iter = map.find(2);
   map.erase(iter);

   REQUIRE(map.value(2) == "");

   REQUIRE(map.value(1) == "watermelon");
   REQUIRE(map.value(3) == "quince");
   REQUIRE(map.value(4) == "grapefruit");

   REQUIRE(map.size() == 4);
}

TEST_CASE("QMultiMap equality", "[qmultimap]")
{
   QMultiMap<int, QString> map1 = { { 1, "watermelon"},
                                    { 2, "apple"},
                                    { 3, "pear"},
                                    { 3, "quince"},
                                    { 4, "grapefruit"} };

   QMultiMap<int, QString> map2 = { { 1, "watermelon"},
                                    { 2, "apple"},
                                    { 3, "pear"},
                                    { 3, "quince"},
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

TEST_CASE("QMultiMap equal_range", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                   { 2, "apple"},
                                   { 3, "pear"},
                                   { 3, "quince"},
                                   { 4, "grapefruit"} };

   auto range = map.equal_range(2);
   QPair<int, QString> data = range.first.pair();

   REQUIRE(data.first  == 2);
   REQUIRE(data.second == "apple");
}

TEST_CASE("QMultiMap first", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                   { 2, "apple"},
                                   { 3, "pear"},
                                   { 3, "quince"},
                                   { 4, "grapefruit"} };

   REQUIRE(map.firstKey() == 1);
   REQUIRE(map.first() == "watermelon");
}

TEST_CASE("QMultiMap insert", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                   { 2, "apple"},
                                   { 3, "pear"},
                                   { 3, "quince"},
                                   { 4, "grapefruit"} };
   map.insert( {6, "mango"} );

   REQUIRE(map.value(6) == "mango");
   REQUIRE(map.size() == 6);
}

TEST_CASE("QMultiMap insert_hint", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                   { 3, "apple"},
                                   { 3, "pear"},
                                   { 3, "quince"},
                                   { 5, "grapefruit"} };

   auto iter = map.upperBound(3);
   map.insert( iter, 3, "mango" );

   REQUIRE(map.size() == 6);

   //
   QList<QString> values = map.values(3);
   REQUIRE(values.size() == 4);

   REQUIRE(values[0] == "apple");
   REQUIRE(values[1] == "pear");
   REQUIRE(values[2] == "quince");
   REQUIRE(values[3] == "mango");
}

TEST_CASE("QMultiMap last", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                   { 2, "apple"},
                                   { 3, "pear"},
                                   { 3, "quince"},
                                   { 4, "grapefruit"} };

   REQUIRE(map.lastKey() == 4);
   REQUIRE(map.last() == "grapefruit");
}

TEST_CASE("QMultiMap operator_bracket", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                   { 2, "apple"},
                                   { 3, "pear"},
                                   { 3, "quince"},
                                   { 4, "grapefruit"} };

   REQUIRE(map[4] == "grapefruit");
   REQUIRE(map[5] == "");

   REQUIRE(map.contains(5) == true);
   REQUIRE(map[5] == "");
}

TEST_CASE("QMultiMap remove", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                   { 2, "apple"},
                                   { 3, "pear"},
                                   { 3, "quince"},
                                   { 4, "grapefruit"} };

   map.remove(3);

   REQUIRE(map.value(3) == "");

   REQUIRE(map.value(1) == "watermelon");
   REQUIRE(map.value(2) == "apple");
   REQUIRE(map.value(4) == "grapefruit");

   REQUIRE(map.size() == 3);
}

TEST_CASE("QMultiMap swap", "[qmultimap]")
{
   QMultiMap<int, QString> map1 = { { 1, "watermelon"},
                                    { 2, "apple"},
                                    { 3, "pear"},
                                    { 3, "quince"},
                                    { 4, "grapefruit"} };

   QMultiMap<int, QString> map2 = { { 1, "grape"},
                                    { 2, "orange"},
                                    { 3, "peach"} };

   map1.swap(map2);

   REQUIRE(map1.value(2) == ("orange"));
   REQUIRE(map2.contains(4));
}

