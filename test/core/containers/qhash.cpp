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

TEST_CASE("QHash assign", "[qhash]")
{
   QHash<int, double> hash { {1, 15.8}, {2, 26.2} };

   hash = hash;

   REQUIRE(hash.size() == 2);
   REQUIRE(hash.value(1) == 15.8);
   REQUIRE(hash.value(2) == 26.2);
}

TEST_CASE("QHash clear", "[qhash]")
{
   QHash<int, QString> hash = { { 1, "watermelon"},
                                { 2, "apple"},
                                { 3, "pear"},
                                { 4, "grapefruit"} };

   hash.clear();

   REQUIRE(hash.size() == 0);
   REQUIRE(hash.isEmpty() == true);

   REQUIRE(hash.count() == 0);
}

TEST_CASE("QHash contains", "[qhash]")
{
   QHash<int, QString> hash = { { 1, "watermelon"},
                                { 2, "apple"},
                                { 3, "pear"},
                                { 4, "grapefruit"} };

   REQUIRE(hash.size() == 4);
   REQUIRE(hash.isEmpty() == false);

   REQUIRE(hash.count() == 4);

   REQUIRE(hash.contains(2) == true);
   REQUIRE(hash.contains(9) == false);
}

TEST_CASE("QHash count", "[qhash]")
{
   QHash<int, QString> hash = { { 1, "watermelon"},
                                { 2, "apple"},
                                { 3, "pear"},
                                { 4, "grapefruit"} };

   REQUIRE(hash.size()  == 4);
   REQUIRE(hash.isEmpty() == false);

   REQUIRE(hash.count() == 4);
}

TEST_CASE("QHash empty", "[qhash]")
{
   QHash<int, QString> hash;

   REQUIRE(hash.size() == 0);
   REQUIRE(hash.isEmpty() == true);

   REQUIRE(hash.count() == 0);
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

      REQUIRE(hash.size()  == 3);
      REQUIRE(hash.count() == 3);

      REQUIRE(hash.value(1) == "watermelon");
      REQUIRE(hash.value(2) == "");
      REQUIRE(hash.value(3) == "pear");
      REQUIRE(hash.value(4) == "grapefruit");

      REQUIRE(hash.value(8) == "");
      REQUIRE(hash.value(8, "na") == "na");

      REQUIRE(hash[1] == "watermelon");
      REQUIRE(hash[3] == "pear");
      REQUIRE(hash[4] == "grapefruit");
   }

   SECTION ("key") {
      auto removed = hash.erase(2);

      REQUIRE(hash.size() == 3);
      REQUIRE(removed == 1);

      REQUIRE(hash.value(1) == "watermelon");
      REQUIRE(hash.value(2) == "");
      REQUIRE(hash.value(3) == "pear");
      REQUIRE(hash.value(4) == "grapefruit");

      //
      removed = hash.erase(2);

      REQUIRE(hash.size() == 3);
      REQUIRE(hash.isEmpty() == false);

      REQUIRE(removed == 0);
   }
}

TEST_CASE("QHash comparison", "[qhash]")
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

TEST_CASE("QHash copy_assign", "[qhash]")
{
   QHash<QString, int> hash_a = { { "watermelon", 10},
                                  { "apple",      20},
                                  { "pear",       30},
                                  { "grapefruit", 40} };

   QHash<QString, int> hash_b(hash_a);

   REQUIRE(hash_a.size() == 4);
   REQUIRE(hash_b.size() == 4);

   REQUIRE(hash_a["watermelon"] == 10);
   REQUIRE(hash_a["grapefruit"] == 40);

   REQUIRE(hash_b["watermelon"] == 10);
   REQUIRE(hash_b["grapefruit"] == 40);

   REQUIRE(hash_a == hash_b);

   //
   QHash<QString, int> hash_c;
   hash_c = hash_a;

   REQUIRE(hash_a.size() == 4);
   REQUIRE(hash_c.size() == 4);

   REQUIRE(hash_a["watermelon"] == 10);
   REQUIRE(hash_a["grapefruit"] == 40);

   REQUIRE(hash_c["watermelon"] == 10);
   REQUIRE(hash_c["grapefruit"] == 40);

   REQUIRE(hash_a == hash_c);
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

   REQUIRE(hash.size() == 5);
   REQUIRE(hash.value(6) == "mango");

   //
   hash.insert(3, "orange");

   REQUIRE(hash.size() == 5);
   REQUIRE(hash[3] == "orange");
}

TEST_CASE("QHash insert_move", "[qhash]")
{
   QHash<int, QUniquePointer<QString> > hash;

   hash.insert(1, QMakeUnique<QString>("watermelon"));
   hash.insert(2, QMakeUnique<QString>("apple"));
   hash.insert(3, QMakeUnique<QString>("pear"));
   hash.insert(4, QMakeUnique<QString>("grapefruit"));

   REQUIRE(hash.size() == 4);
   REQUIRE(*(hash[3]) == "pear");
}

TEST_CASE("QHash lookup", "[qhash]")
{
   QHash<QString, int> hash = { { "watermelon", 10 },
                                { "apple",      20 },
                                { "pear",       30 },
                                { "grapefruit", 40 } };

   //
   QList<QString> keys = hash.keys();

   REQUIRE(keys.size() == 4);
   REQUIRE(keys.contains("apple"));

   //
   QList<int> values = hash.values();

   REQUIRE(values.size() == 4);
   REQUIRE(values.contains(30) == true);

   //
   hash.insert("", 50);

   REQUIRE(hash.contains(""));
   REQUIRE(hash.value("") == 50);
}

TEST_CASE("QHash operator_bracket", "[qhash]")
{
   QHash<int, QString> hash = { { 1, "watermelon"},
                                { 2, "apple"},
                                { 3, "pear"},
                                { 4, "grapefruit"} };

   REQUIRE(hash.size() == 4);

   REQUIRE(hash[4] == "grapefruit");
   REQUIRE(hash[5] == "");

   //
   REQUIRE(hash.size() == 5);

   REQUIRE(hash.contains(5) == true);
   REQUIRE(hash.value(5) == "");
   REQUIRE(hash[5] == "");

   //
   hash[7] = "quince";

   REQUIRE(hash.size() == 6);

   REQUIRE(hash.contains(7) == true);
   REQUIRE(hash.value(7) == "quince");
   REQUIRE(hash[7] == "quince");

   // will not add another element
   REQUIRE(hash.contains(6) == false);
   REQUIRE(hash.value(6) == "");
}

TEST_CASE("QHash move_assign", "[qhash]")
{
   QHash<QString, int> hash_a = { { "watermelon", 10},
                                  { "apple",      20},
                                  { "pear",       30},
                                  { "grapefruit", 40} };

   QHash<QString, int> hash_b = std::move(hash_a);

   REQUIRE(hash_b.size() == 4);

   REQUIRE(hash_b["watermelon"] == 10);
   REQUIRE(hash_b["grapefruit"] == 40);

   //
   QHash<QString, int> hash_c;
   hash_c = std::move(hash_b);

   REQUIRE(hash_c.size() == 4);

   REQUIRE(hash_c["watermelon"] == 10);
   REQUIRE(hash_c["grapefruit"] == 40);
}

TEST_CASE("QHash remove", "[qhash]")
{
   QHash<int, QString> hash = { { 1, "watermelon"},
                                { 2, "apple"},
                                { 3, "pear"},
                                { 4, "grapefruit"} };

   REQUIRE(hash.size() == 4);

   //
   hash.remove(3);

   REQUIRE(hash.size() == 3);

   REQUIRE(hash.value(1) == "watermelon");
   REQUIRE(hash.value(2) == "apple");
   REQUIRE(hash.value(3) == "");
   REQUIRE(hash.value(4) == "grapefruit");

   //
   hash.insert(10, "orange");

   REQUIRE(hash.size() == 4);

   REQUIRE(hash.remove(10)   == 1);
   REQUIRE(hash.contains(10) == false);

   REQUIRE(hash.size() == 3);

   //
   REQUIRE(hash.remove(10) == 0);
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
