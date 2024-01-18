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

#include <qqueue.h>

#include <cs_catch2.h>

TEST_CASE("QQueue traits", "[qqueue]")
{
   REQUIRE(std::is_copy_constructible_v<QQueue<int>> == true);
   REQUIRE(std::is_move_constructible_v<QQueue<int>> == true);

   REQUIRE(std::is_copy_assignable_v<QQueue<int>> == true);
   REQUIRE(std::is_move_assignable_v<QQueue<int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QQueue<int>> == false);
}

TEST_CASE("QQueue empty", "[qqueue]")
{
   QQueue<QString> list;

   REQUIRE(list.isEmpty());
}

TEST_CASE("QQueue length", "[qqueue]")
{
   QQueue<QString> list;

   list.enqueue("watermelon");
   list.enqueue("apple");
   list.enqueue("pear");
   list.enqueue("grapefruit");

   REQUIRE(list.length() == 4);
   REQUIRE(list.size() == 4);
}

TEST_CASE("QQueue clear", "[qqueue]")
{
   QQueue<QString> list;

   list.enqueue("watermelon");
   list.enqueue("apple");
   list.enqueue("pear");
   list.enqueue("grapefruit");

   list.clear();

   REQUIRE(list.size() == 0);
}

TEST_CASE("QQueue contains", "[qqueue]")
{
   QQueue<QString> list;

   list.enqueue("watermelon");
   list.enqueue("apple");
   list.enqueue("pear");
   list.enqueue("grapefruit");

   REQUIRE(list.contains("pear"));
   REQUIRE(! list.contains("orange"));
}

TEST_CASE("QQueue erase", "[qqueue]")
{
   QQueue<QString> list;

   list.enqueue("watermelon");
   list.enqueue("apple");
   list.enqueue("pear");
   list.enqueue("grapefruit");

   list.erase(list.begin() + 1);

   REQUIRE(! list.contains("apple"));

   REQUIRE(list.contains("watermelon"));
   REQUIRE(list.contains("pear"));
   REQUIRE(list.contains("grapefruit"));

   REQUIRE(list.length() == 3);
}

TEST_CASE("QQueue remove", "[qqueue]")
{
   QQueue<QString> list;

   list.enqueue("watermelon");
   list.enqueue("apple");
   list.enqueue("pear");
   list.enqueue("grapefruit");

   list.removeOne("apple");
   list.remove(0);

   REQUIRE(! list.contains("apple"));
   REQUIRE(! list.contains("watermelon"));

   REQUIRE(list.contains("pear"));
   REQUIRE(list.contains("grapefruit"));

   REQUIRE(list.length() == 2);
}

TEST_CASE("QQueue insert", "[qqueue]")
{
   QQueue<QString> list;

   list.enqueue("watermelon");
   list.enqueue("apple");
   list.enqueue("pear");
   list.enqueue("grapefruit");

   list.insert(1, "mango");

   REQUIRE(list.contains("mango"));
   REQUIRE(list[1] == "mango");
   REQUIRE(list.length() == 5);
}

TEST_CASE("QQueue position", "[qqueue]")
{
   QQueue<QString> list;

   list.enqueue("watermelon");
   list.enqueue("apple");
   list.enqueue("pear");
   list.enqueue("grapefruit");

   REQUIRE(list.first() == "watermelon");
   REQUIRE(list.last()  == "grapefruit");

   REQUIRE(list.front() == "watermelon");
   REQUIRE(list.back()  == "grapefruit");
}
