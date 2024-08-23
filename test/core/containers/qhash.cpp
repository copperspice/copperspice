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

#include <qhash.h>

#include <cs_catch2.h>

TEST_CASE("QHash traits", "[qhash]")
{
   REQUIRE(std::is_copy_constructible_v<QHash<int, int>> == true);
   REQUIRE(std::is_move_constructible_v<QHash<int, int>> == true);

   REQUIRE(std::is_copy_assignable_v<QHash<int, int>> == true);
   REQUIRE(std::is_move_assignable_v<QHash<int, int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QHash<int, int>> == false);
}

TEST_CASE("QHash clear", "[qhash]")
{
   QHash<int, QString> hash = { { 1, "watermelon"},
                                { 2, "apple"},
                                { 3, "pear"},
                                { 4, "grapefruit"} };

   hash.clear();

   REQUIRE(hash.size() == 0);
}

TEST_CASE("QHash contains", "[qhash]")
{
   QHash<int, QString> hash = { { 1, "watermelon"},
                                { 2, "apple"},
                                { 3, "pear"},
                                { 4, "grapefruit"} };

   REQUIRE(hash.contains(2));
   REQUIRE(! hash.contains(9));
}

TEST_CASE("QHash count", "[qhash]")
{
   QHash<int, QString> hash = { { 1, "watermelon"},
                                { 2, "apple"},
                                { 3, "pear"},
                                { 4, "grapefruit"} };

   REQUIRE(hash.count() == 4);
   REQUIRE(hash.size() == 4);
}

TEST_CASE("QHash empty", "[qhash]")
{
   QHash<int, QString> hash;

   REQUIRE(hash.isEmpty());
}

TEST_CASE("QHash erase", "[qhash]")
{
   QHash<int, QString> hash = { { 1, "watermelon"},
                                { 2, "apple"},
                                { 3, "pear"},
                                { 4, "grapefruit"} };

   SECTION ("iterator") {
      auto iter = hash.find(2);
      hash.erase(iter);

      REQUIRE(hash.value(2) == "");

      REQUIRE(hash.value(1) == "watermelon");
      REQUIRE(hash.value(3) == "pear");
      REQUIRE(hash.value(4) == "grapefruit");

      REQUIRE(hash.size() == 3);
   }

   SECTION ("key") {
      auto count = hash.erase(2);

      REQUIRE(hash.value(2) == "");
      REQUIRE(count == 1);

      REQUIRE(hash.value(1) == "watermelon");
      REQUIRE(hash.value(3) == "pear");
      REQUIRE(hash.value(4) == "grapefruit");

      REQUIRE(hash.size() == 3);

      count = hash.erase(2);
      REQUIRE(count == 0);
   }
}

TEST_CASE("QHash equality", "[qhash]")
{
   QHash<int, QString> hash1 = { { 1, "watermelon"},
                                 { 2, "apple"},
                                 { 3, "pear"},
                                 { 4, "grapefruit"} };

   QHash<int, QString> hash2 = { { 1, "watermelon"},
                                 { 2, "apple"},
                                 { 3, "pear"},
                                 { 4, "grapefruit"} };
   {
      REQUIRE(hash1 == hash2);
      REQUIRE(! (hash1 != hash2));
   }

   {
      hash2.remove(3);

      REQUIRE(! (hash1 == hash2));
      REQUIRE(hash1 != hash2);
   }
}

TEST_CASE("QHash equal_range", "[qhash]")
{
   QHash<int, QString> hash = { { 1, "watermelon"},
                                { 2, "apple"},
                                { 3, "pear"},
                                { 4, "grapefruit"} };

   auto range = hash.equal_range(2);
   QPair<int, QString> data = range.first.pair();

   REQUIRE(data.first  == 2);
   REQUIRE(data.second == "apple");
}

TEST_CASE("QHash insert_copy", "[qhash]")
{
   QHash<int, QString> hash = { { 1, "watermelon"},
                                { 2, "apple"},
                                { 3, "pear"},
                                { 4, "grapefruit"} };

   hash.insert( {6, "mango"} );

   REQUIRE(hash.value(6) == "mango");
   REQUIRE(hash.size() == 5);
}

TEST_CASE("QHash insert_move", "[qhash]")
{
   QHash<int, QUniquePointer<QString> > hash;

   hash.insert(1, QMakeUnique<QString>("watermelon"));
   hash.insert(2, QMakeUnique<QString>("apple"));
   hash.insert(3, QMakeUnique<QString>("pear"));
   hash.insert(4, QMakeUnique<QString>("grapefruit"));

   REQUIRE(*(hash[3]) == "pear");
   REQUIRE(hash.size() == 4);
}

TEST_CASE("QHash operator_bracket", "[qhash]")
{
   QHash<int, QString> hash = { { 1, "watermelon"},
                                { 2, "apple"},
                                { 3, "pear"},
                                { 4, "grapefruit"} };

   REQUIRE(hash[4] == "grapefruit");
   REQUIRE(hash[5] == "");

   REQUIRE(hash.contains(5) == true);
   REQUIRE(hash[5] == "");
}

TEST_CASE("QHash remove", "[qhash]")
{
   QHash<int, QString> hash = { { 1, "watermelon"},
                                { 2, "apple"},
                                { 3, "pear"},
                                { 4, "grapefruit"} };

   hash.remove(3);

   REQUIRE(hash.value(3) == "");

   REQUIRE(hash.value(1) == "watermelon");
   REQUIRE(hash.value(2) == "apple");
   REQUIRE(hash.value(4) == "grapefruit");

   REQUIRE(hash.size() == 3);
}

TEST_CASE("QHash swap", "[qhash]")
{
   QHash<int, QString> hash1 = { { 1, "watermelon"},
                                 { 2, "apple"},
                                 { 3, "pear"},
                                 { 4, "grapefruit"} };

   QHash<int, QString> hash2 = { { 1, "grape"},
                                 { 2, "orange"},
                                 { 3, "peach"} };

   hash1.swap(hash2);

   REQUIRE(hash1.value(2) == ("orange"));
   REQUIRE(hash2.contains(4));
}
