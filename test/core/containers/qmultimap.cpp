/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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
   REQUIRE(map.isEmpty() == true);
}

TEST_CASE("QMultiMap comparison", "[qmultimap]")
{
   QMultiMap<int, QString> map_a = { { 1, "watermelon"},
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
      REQUIRE(map_a == map2);
      REQUIRE(! (map_a != map2));
   }

   {
      map2.remove(3);

      REQUIRE(! (map_a == map2));
      REQUIRE(map_a != map2);
   }
}

TEST_CASE("QMultiMap contains", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                   { 2, "apple"},
                                   { 3, "pear"},
                                   { 3, "quince"},
                                   { 4, "grapefruit"} };

   REQUIRE(map.contains(2) == true);
   REQUIRE(map.contains(9) == false);
}

TEST_CASE("QMultiMap copy_assign", "[qmultimap]")
{
   QMultiMap<int, QString> map_a = { { 1, "watermelon"},
                                     { 2, "apple"},
                                     { 3, "pear"},
                                     { 4, "grapefruit"} };

   QMultiMap<int, QString> map_b(map_a);

   REQUIRE(map_a.size() == 4);
   REQUIRE(map_b.size() == 4);

   REQUIRE(map_a.value(1) == "watermelon");
   REQUIRE(map_a.value(4) == "grapefruit");

   REQUIRE(map_b.value(1) == "watermelon");
   REQUIRE(map_b.value(4) == "grapefruit");

   REQUIRE(map_a == map_b);

   //
   QMultiMap<int, QString> map_c;
   map_c = map_a;

   REQUIRE(map_a.size() == 4);
   REQUIRE(map_c.size() == 4);

   REQUIRE(map_a.value(1) == "watermelon");
   REQUIRE(map_a.value(4) == "grapefruit");

   REQUIRE(map_c.value(1) == "watermelon");
   REQUIRE(map_c.value(4) == "grapefruit");

   REQUIRE(map_a == map_c);
}

TEST_CASE("QMultiMap count", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                   { 2, "apple"},
                                   { 3, "pear"},
                                   { 3, "quince"},
                                   { 4, "grapefruit"} };

   REQUIRE(map.count() == 5);
   REQUIRE(map.size()  == 5);
}

TEST_CASE("QMultiMap empty", "[qmultimap]")
{
   QMultiMap<int, QString> map;

   REQUIRE(map.isEmpty() == true);
   REQUIRE(map.size() == 0);
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

TEST_CASE("QMultiMap erase", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                   { 2, "apple"},
                                   { 3, "pear"},
                                   { 3, "quince"},
                                   { 4, "grapefruit"} };

   SECTION ("iterator") {
      auto iter = map.find(2);
      map.erase(iter);

      REQUIRE(map.size() == 4);

      REQUIRE(map.value(1) == "watermelon");
      REQUIRE(map.value(2) == QString());
      REQUIRE(map.value(3) == "quince");
      REQUIRE(map.value(4) == "grapefruit");
   }

   SECTION ("key") {
      auto count = map.erase(3);

      REQUIRE(count == 2);
      REQUIRE(map.size() == 3);

      REQUIRE(map.value(1) == "watermelon");
      REQUIRE(map.value(2) == "apple");
      REQUIRE(map.value(3) == QString());
      REQUIRE(map.value(4) == "grapefruit");

      //
      count = map.erase(3);

      REQUIRE(count == 0);
   }
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

TEST_CASE("QMultiMap insert_copy", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                    { 2, "apple"},
                                    { 3, "pear"},
                                    { 3, "quince"},
                                    { 4, "grapefruit"} };

   map.insert( {6, "mango"} );

   REQUIRE(map.size() == 6);
   REQUIRE(map.value(6) == "mango");
}

TEST_CASE("QMultiMap insert_move", "[qmultimap]")
{
   QMultiMap<int, QUniquePointer<QString> > map;

   map.insert(1, QMakeUnique<QString>("watermelon"));
   map.insert(2, QMakeUnique<QString>("apple"));
   map.insert(3, QMakeUnique<QString>("pear"));
   map.insert(4, QMakeUnique<QString>("grapefruit"));

   REQUIRE(map.size() == 4);
   REQUIRE(*(map[3]) == "pear");
}

TEST_CASE("QMultiMap insert_hint", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                   { 3, "apple"},
                                   { 3, "pear"},
                                   { 3, "quince"},
                                   { 5, "grapefruit"} };

   auto iter = map.upperBound(3);
   map.insert(iter, 3, "mango");

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

TEST_CASE("QMultiMap move_assign", "[qmultimap]")
{
   QMultiMap<int, QString> map_a = { { 1, "watermelon"},
                                     { 2, "apple"},
                                     { 3, "pear"},
                                     { 3, "quince"},
                                     { 4, "grapefruit"} };

   QMultiMap<int, QString> map_b(std::move(map_a));

   REQUIRE(map_b.size() == 5);

   REQUIRE(map_b.value(1) == "watermelon");
   REQUIRE(map_b.value(4) == "grapefruit");

   //
   QMultiMap<int, QString> map_c;
   map_c = std::move(map_b);

   REQUIRE(map_c.size() == 5);

   REQUIRE(map_c.value(1) == "watermelon");
   REQUIRE(map_c.value(4) == "grapefruit");
}

TEST_CASE("QMultiMap operator_bracket", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                   { 2, "apple"},
                                   { 3, "pear"},
                                   { 4, "grapefruit"} };

   REQUIRE(map.size() == 4);
   REQUIRE(map[4] == "grapefruit");

   // creates new element
   REQUIRE(map[5] == QString());

   REQUIRE(map.size() == 5);

   REQUIRE(map.contains(5) == true);
   REQUIRE(map[5] == QString());
}

TEST_CASE("QMultiMap remove", "[qmultimap]")
{
   QMultiMap<int, QString> map = { { 1, "watermelon"},
                                   { 2, "apple"},
                                   { 3, "pear"},
                                   { 3, "quince"},
                                   { 4, "grapefruit"} };

   REQUIRE(map.size() == 5);

   map.remove(3);

   REQUIRE(map.size() == 3);

   REQUIRE(map.value(1) == "watermelon");
   REQUIRE(map.value(2) == "apple");
   REQUIRE(map.value(3) == QString());
   REQUIRE(map.value(4) == "grapefruit");
}

TEST_CASE("QMultiMap swap", "[qmultimap]")
{
   QMultiMap<int, QString> map_a = { { 1, "watermelon"},
                                     { 2, "apple"},
                                     { 3, "pear"},
                                     { 3, "quince"},
                                     { 4, "grapefruit"} };

   QMultiMap<int, QString> map_b = { { 1, "grape"},
                                     { 2, "orange"},
                                     { 3, "peach"} };

   REQUIRE(map_a.size() == 5);
   REQUIRE(map_b.size() == 3);

   map_a.swap(map_b);

   REQUIRE(map_a.size() == 3);
   REQUIRE(map_b.size() == 5);

   REQUIRE(map_a.value(2) == ("orange"));
   REQUIRE(map_b.contains(4) == true);
}
