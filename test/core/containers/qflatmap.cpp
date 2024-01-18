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

#include <qflatmap.h>

#include <cs_catch2.h>

TEST_CASE("QFlatMap traits", "[qflatmap]")
{
   REQUIRE(std::is_copy_constructible_v<QFlatMap<int, int>> == true);
   REQUIRE(std::is_move_constructible_v<QFlatMap<int, int>> == true);

   REQUIRE(std::is_copy_assignable_v<QFlatMap<int, int>> == true);
   REQUIRE(std::is_move_assignable_v<QFlatMap<int, int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QFlatMap<int, int>> == false);
}

TEST_CASE("QFlatMap clear", "[qflatmap]")
{
   QFlatMap<int, QString> map = { { 1, "watermelon"},
                                  { 2, "apple"},
                                  { 3, "pear"},
                                  { 4, "grapefruit"} };

   map.clear();

   REQUIRE(map.size() == 0);
}

TEST_CASE("QFlatMap contains", "[qflatmap]")
{
   QFlatMap<int, QString> map = { { 1, "watermelon"},
                                  { 2, "apple"},
                                  { 3, "pear"},
                                  { 4, "grapefruit"} };

   REQUIRE(map.contains(2));
   REQUIRE(! map.contains(9));
}

TEST_CASE("QFlatMap count", "[qflatmap]")
{
   QFlatMap<int, QString> map = { { 1, "watermelon"},
                                  { 2, "apple"},
                                  { 3, "pear"},
                                  { 4, "grapefruit"} };

   REQUIRE(map.count() == 4);
   REQUIRE(map.size() == 4);
}

TEST_CASE("QFlatMap empty", "[qflatmap]")
{
   QFlatMap<int, QString> map;

   REQUIRE(map.isEmpty());
}

TEST_CASE("QFlatMap erase", "[qflatmap]")
{
   QFlatMap<int, QString> map = { { 1, "watermelon"},
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

TEST_CASE("QFlatMap equality", "[qflatmap]")
{
   QFlatMap<int, QString> map1 = { { 1, "watermelon"},
                                   { 2, "apple"},
                                   { 3, "pear"},
                                   { 4, "grapefruit"} };

   QFlatMap<int, QString> map2 = { { 1, "watermelon"},
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

TEST_CASE("QFlatMap equal_range", "[qflatmap]")
{
   QFlatMap<int, QString> map = { { 1, "watermelon"},
                                  { 2, "apple"},
                                  { 3, "pear"},
                                  { 4, "grapefruit"} };

   auto range = map.equal_range(2);
   QPair<int, QString> data = range.first.pair();

   REQUIRE(data.first  == 2);
   REQUIRE(data.second == "apple");
}

TEST_CASE("QFlatMap first", "[qflatmap]")
{
   QFlatMap<int, QString> map = { { 1, "watermelon"},
                                  { 2, "apple"},
                                  { 3, "pear"},
                                  { 4, "grapefruit"} };

   REQUIRE(map.firstKey() == 1);
   REQUIRE(map.first() == "watermelon");
}

TEST_CASE("QFlatMap insert", "[qflatmap]")
{
   QFlatMap<int, QString> map = { { 1, "watermelon"},
                                  { 2, "apple"},
                                  { 3, "pear"},
                                  { 4, "grapefruit"} };

   map.insert(6, "mango");

   REQUIRE(map.value(6) == "mango");
   REQUIRE(map.size() == 5);
}

TEST_CASE("QFlatMap insert_hint", "[qflatmap]")
{
   QFlatMap<int, QString> map = { { 1, "watermelon"},
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
   iter = map.upperBound(4);
   map.insert( iter, 4, "peach" );

   REQUIRE(map[4] == "peach");
}

TEST_CASE("QFlatMap last", "[qflatmap]")
{
   QFlatMap<int, QString> map = { { 1, "watermelon"},
                                  { 2, "apple"},
                                  { 3, "pear"},
                                  { 4, "grapefruit"} };

   REQUIRE(map.lastKey() == 4);
   REQUIRE(map.last() == "grapefruit");
}

TEST_CASE("QFlatMap remove", "[qflatmap]")
{
   QFlatMap<int, QString> map = { { 1, "watermelon"},
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

TEST_CASE("QFlatMap swap", "[qflatmap]")
{
   QFlatMap<int, QString> map1 = { { 1, "watermelon"},
                                   { 2, "apple"},
                                   { 3, "pear"},
                                   { 4, "grapefruit"} };

   QFlatMap<int, QString> map2 = { { 1, "grape"},
                                   { 2, "orange"},
                                   { 3, "peach"} };

   map1.swap(map2);

   REQUIRE(map1.value(2) == ("orange"));
   REQUIRE(map2.contains(4));
}

