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

#include <qset.h>

#include <cs_catch2.h>

TEST_CASE("QSet traits", "[qset]")
{
   REQUIRE(std::is_copy_constructible_v<QSet<int>> == true);
   REQUIRE(std::is_move_constructible_v<QSet<int>> == true);

   REQUIRE(std::is_copy_assignable_v<QSet<int>> == true);
   REQUIRE(std::is_move_assignable_v<QSet<int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QSet<int>> == false);
}

TEST_CASE("QSet clear", "[qset]")
{
   QSet<QString> set = { "watermelon", "apple", "pear", "grapefruit" };

   set.clear();

   REQUIRE(set.size() == 0);
}

TEST_CASE("QSet contains_a", "[qset]")
{
   QSet<QString> set = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(set.contains("pear"));
   REQUIRE(! set.contains("mango"));
}

TEST_CASE("QSet contains_b", "[qset]")
{
   QSet<QString> set1 = { "watermelon", "apple", "pear", "grapefruit" };
   QSet<QString> set2 = { "grape", "orange", "apple" };

   REQUIRE(set1.contains(set1) == true);
   REQUIRE(set1.contains(set2) == false);      // FAILS

   set1.insert("orange");
   set1.insert("grape");

   REQUIRE(set1.contains(set2) == true);
}

TEST_CASE("QSet empty", "[qset]")
{
   QSet<QString> set;

   REQUIRE(set.isEmpty());
}

TEST_CASE("QSet equality", "[qset]")
{
   QSet<QString> set1 = { "watermelon", "apple", "pear", "grapefruit" };
   QSet<QString> set2 = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(set1 == set2);
   REQUIRE(! (set1 != set2));
}

TEST_CASE("QSet erase", "[qset]")
{
   QSet<QString> set = { "watermelon", "apple", "pear", "grapefruit" };

   auto iter = set.find("apple");
   set.erase(iter);

   REQUIRE(! set.contains("apple"));

   REQUIRE(set.contains("watermelon"));
   REQUIRE(set.contains("pear"));
   REQUIRE(set.contains("grapefruit"));

   REQUIRE(set.size() == 3);
}

TEST_CASE("QSet insert", "[qset]")
{
   QSet<QString> set = { "watermelon", "apple", "pear", "grapefruit" };

   set.insert("mango");

   REQUIRE(set.contains("mango"));
   REQUIRE(set.size() == 5);
}

TEST_CASE("QSet length", "[qset]")
{
   QSet<QString> set = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(set.size() == 4);
}

TEST_CASE("QSet remove", "[qset]")
{
   QSet<QString> set = { "watermelon", "apple", "pear", "grapefruit" };

   set.remove("pear");

   REQUIRE(! set.contains("pear"));

   REQUIRE(set.contains("watermelon"));
   REQUIRE(set.contains("apple"));
   REQUIRE(set.contains("grapefruit"));

   REQUIRE(set.size() == 3);
}

TEST_CASE("QSet swap", "[qset]")
{
   QSet<QString> set1 = { "watermelon", "apple", "pear", "grapefruit" };
   QSet<QString> set2 = { "grape", "orange", "peach"};

   set1.swap(set2);

   REQUIRE(set1.contains("orange"));
   REQUIRE(! set2.contains("orange"));
}

