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

#include <qlist.h>

#include <cs_catch2.h>

TEST_CASE("QList traits", "[qlist]")
{
   REQUIRE(std::is_copy_constructible_v<QList<int>> == true);
   REQUIRE(std::is_move_constructible_v<QList<int>> == true);

   REQUIRE(std::is_copy_assignable_v<QList<int>> == true);
   REQUIRE(std::is_move_assignable_v<QList<int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QList<int>> == false);
}

TEST_CASE("QList append", "[qlist]")
{
   QList<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   v.append("quince");

   REQUIRE(v.contains("quince"));
   REQUIRE(v[4] == "quince");
   REQUIRE(v.length() == 5);
}

TEST_CASE("QList clear", "[qlist]")
{
   QList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   list.clear();

   REQUIRE(list.size() == 0);
}

TEST_CASE("QList contains", "[qlist]")
{
   QList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(list.contains("pear"));
   REQUIRE(! list.contains("orange"));
}

TEST_CASE("QList empty", "[qlist]")
{
   QList<QString> list;

   REQUIRE(list.isEmpty());
}

TEST_CASE("QList equality", "[qlist]")
{
   QList<QString> list1 = { "watermelon", "apple", "pear", "grapefruit" };
   QList<QString> list2 = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(list1 == list2);
   REQUIRE(! (list1 != list2));
}

TEST_CASE("QList erase", "[qlist]")
{
   QList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   list.erase(list.begin() + 1);

   REQUIRE(! list.contains("apple"));

   REQUIRE(list.contains("watermelon"));
   REQUIRE(list.contains("pear"));
   REQUIRE(list.contains("grapefruit"));

   REQUIRE(list.length() == 3);
}

TEST_CASE("QList insert", "[qlist]")
{
   QList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   list.insert(1, "mango");

   REQUIRE(list.contains("mango"));
   REQUIRE(list[1] == "mango");
   REQUIRE(list.length() == 5);
}

TEST_CASE("QList length", "[qlist]")
{
   QList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(list.length() == 4);
   REQUIRE(list.size() == 4);
}

TEST_CASE("QList position", "[qlist]")
{
   QList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(list.first() == "watermelon");
   REQUIRE(list.last()  == "grapefruit");

   REQUIRE(list.front() == "watermelon");
   REQUIRE(list.back()  == "grapefruit");
}

TEST_CASE("QList prepend", "[qlist]")
{
   QList<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   v.prepend("quince");

   REQUIRE(v.contains("quince"));
   REQUIRE(v[0] == "quince");
   REQUIRE(v.length() == 5);
}

TEST_CASE("QList remove", "[qlist]")
{
   QList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   list.removeOne("apple");
   list.remove(0);

   REQUIRE(! list.contains("apple"));
   REQUIRE(! list.contains("watermelon"));

   REQUIRE(list.contains("pear"));
   REQUIRE(list.contains("grapefruit"));

   REQUIRE(list.length() == 2);
}