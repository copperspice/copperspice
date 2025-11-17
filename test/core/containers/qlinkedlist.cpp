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

#include <qlinkedlist.h>

#include <cs_catch2.h>

TEST_CASE("QLinkedList traits", "[qlinkedlist]")
{
   REQUIRE(std::is_copy_constructible_v<QLinkedList<int>> == true);
   REQUIRE(std::is_move_constructible_v<QLinkedList<int>> == true);

   REQUIRE(std::is_copy_assignable_v<QLinkedList<int>> == true);
   REQUIRE(std::is_move_assignable_v<QLinkedList<int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QLinkedList<int>> == false);
}

TEST_CASE("QLinkedList empty", "[qlinkedlist]")
{
   QLinkedList<QString> list;

   REQUIRE(list.isEmpty() == true);
}

TEST_CASE("QLinkedList clear", "[qlinkedlist]")
{
   QLinkedList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   list.clear();

   REQUIRE(list.size() == 0);
}

TEST_CASE("QLinkedList contains", "[qlinkedlist]")
{
   QLinkedList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(list.contains("pear") == true);
   REQUIRE(list.contains("orange") == false);
}

TEST_CASE("QLinkedList copy_assign", "[qlinkedlist]")
{
   QLinkedList<QString> list_a = { "watermelon", "apple", "pear", "grapefruit" };
   QLinkedList<QString> list_b = list_a;

   REQUIRE(list_a == list_b);

   REQUIRE(list_a == QLinkedList<QString>{"watermelon", "apple", "pear", "grapefruit"});
   REQUIRE(list_b == QLinkedList<QString>{"watermelon", "apple", "pear", "grapefruit"});

   //
   QLinkedList<QString> list_c;
   list_c = list_a;

   REQUIRE(list_a == list_c);

   REQUIRE(list_a == QLinkedList<QString>{"watermelon", "apple", "pear", "grapefruit"});
   REQUIRE(list_c == QLinkedList<QString>{"watermelon", "apple", "pear", "grapefruit"});
}

TEST_CASE("QLinkedList erase", "[qlinkedlist]")
{
   QLinkedList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   auto iter = list.begin();
   ++iter;

   list.erase(iter);

   REQUIRE(list.contains("apple") == false);

   REQUIRE(list.contains("watermelon") == true);
   REQUIRE(list.contains("pear") == true);
   REQUIRE(list.contains("grapefruit") == true);

   REQUIRE(list.length() == 3);
}

TEST_CASE("QLinkedList insert", "[qlinkedlist]")
{
   QLinkedList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   auto iter = list.begin();
   ++iter;

   // part 1
   list.insert(iter, "mango");

   REQUIRE(list.contains("mango") == true);
   REQUIRE(list.first()  == "watermelon");
   REQUIRE(list.length() == 5);

   // part 2
   list.erase(list.begin());

   REQUIRE(list.contains("mango") == true);
   REQUIRE(list.first()  == "mango");
   REQUIRE(list.length() == 4);
}

TEST_CASE("QLinkedList length", "[qlinkedlist]")
{
   QLinkedList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(list.length() == 4);
   REQUIRE(list.size() == 4);
}

TEST_CASE("QLinkedList move_assign", "[qlinkedlist]")
{
   QLinkedList<QString> list_a = { "watermelon", "apple", "pear", "grapefruit" };
   QLinkedList<QString> list_b = std::move(list_a);

   REQUIRE(list_b == QLinkedList<QString>{"watermelon", "apple", "pear", "grapefruit"});

   //
   QLinkedList<QString> list_c;
   list_c = std::move(list_b);

   REQUIRE(list_c == QLinkedList<QString>{"watermelon", "apple", "pear", "grapefruit"});
}

TEST_CASE("QLinkedList remove", "[qlinkedlist]")
{
   QLinkedList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   list.removeOne("apple");

   REQUIRE(list.contains("apple") == false);

   REQUIRE(list.contains("watermelon") == true);
   REQUIRE(list.contains("pear") == true);
   REQUIRE(list.contains("grapefruit") == true);

   REQUIRE(list.length() == 3);
}

TEST_CASE("QLinkedList position", "[qlinkedlist]")
{
   QLinkedList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(list.first() == "watermelon");
   REQUIRE(list.last()  == "grapefruit");

   REQUIRE(list.front() == "watermelon");
   REQUIRE(list.back()  == "grapefruit");
}
