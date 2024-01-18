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

#include <qstack.h>

#include <cs_catch2.h>

TEST_CASE("QStack traits", "[qstack]")
{
   REQUIRE(std::is_copy_constructible_v<QStack<int>> == true);
   REQUIRE(std::is_move_constructible_v<QStack<int>> == true);

   REQUIRE(std::is_copy_assignable_v<QStack<int>> == true);
   REQUIRE(std::is_move_assignable_v<QStack<int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QStack<int>> == false);
}

TEST_CASE("QStack empty", "[qstack]")
{
   QStack<QString> stack;

   REQUIRE(stack.isEmpty());
}

TEST_CASE("QStack length", "[qstack]")
{
   QStack<QString> stack;
   stack.push("watermelon");
   stack.push("apple");
   stack.push("pear");
   stack.push("grapefruit");

   REQUIRE(stack.length() == 4);
   REQUIRE(stack.size() == 4);
}

TEST_CASE("QStack clear", "[qstack]")
{
   QStack<QString> stack;
   stack.push("watermelon");
   stack.push("apple");
   stack.push("pear");
   stack.push("grapefruit");

   stack.clear();

   REQUIRE(stack.size() == 0);
}

TEST_CASE("QStack contains", "[qstack]")
{
   QStack<QString> stack;
   stack.push("watermelon");
   stack.push("apple");
   stack.push("pear");
   stack.push("grapefruit");

   REQUIRE(stack.contains("pear"));
   REQUIRE(! stack.contains("orange"));
}

TEST_CASE("QStack erase", "[qstack]")
{
   QStack<QString> stack;
   stack.push("watermelon");
   stack.push("apple");
   stack.push("pear");
   stack.push("grapefruit");;

   stack.erase(stack.begin() + 1);

   REQUIRE(! stack.contains("apple"));

   REQUIRE(stack.contains("watermelon"));
   REQUIRE(stack.contains("pear"));
   REQUIRE(stack.contains("grapefruit"));

   REQUIRE(stack.length() == 3);
}

TEST_CASE("QStack remove", "[qstack]")
{
   QStack<QString> stack;
   stack.push("watermelon");
   stack.push("apple");
   stack.push("pear");
   stack.push("grapefruit");

   stack.removeOne("apple");
   stack.remove(0);

   REQUIRE(! stack.contains("apple"));
   REQUIRE(! stack.contains("watermelon"));

   REQUIRE(stack.contains("pear"));
   REQUIRE(stack.contains("grapefruit"));

   REQUIRE(stack.length() == 2);
}

TEST_CASE("QStack insert", "[qstack]")
{
   QStack<QString> stack;
   stack.push("watermelon");
   stack.push("apple");
   stack.push("pear");
   stack.push("grapefruit");

   stack.insert(1, "mango");

   REQUIRE(stack.contains("mango"));
   REQUIRE(stack[1] == "mango");
   REQUIRE(stack.length() == 5);
}

TEST_CASE("QStack position", "[qstack]")
{
   QStack<QString> stack;
   stack.push("watermelon");
   stack.push("apple");
   stack.push("pear");
   stack.push("grapefruit");

   REQUIRE(stack.first() == "watermelon");
   REQUIRE(stack.last()  == "grapefruit");

   REQUIRE(stack.front() == "watermelon");
   REQUIRE(stack.back()  == "grapefruit");
}