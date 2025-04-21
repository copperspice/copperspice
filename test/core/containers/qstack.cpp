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

TEST_CASE("QStack begin_end", "[qstack]")
{
   QStack<QString> stack;
   stack.push("watermelon");
   stack.push("apple");
   stack.push("pear");
   stack.push("grapefruit");

   {
      auto iterBegin = stack.begin();
      auto iterEnd   = stack.end();

      REQUIRE(*iterBegin == "watermelon");
      REQUIRE(*(iterEnd - 1) == "grapefruit");
   }

   {
      auto iterBegin = stack.constBegin();
      auto iterEnd   = stack.constEnd();

      REQUIRE(*iterBegin == "watermelon");
      REQUIRE(*(iterEnd - 1) == "grapefruit");
   }

   {
      auto iterBegin = stack.cbegin();
      auto iterEnd   = stack.cend();

      REQUIRE(*iterBegin == "watermelon");
      REQUIRE(*(iterEnd - 1) == "grapefruit");
   }

   {
      QStack<QString>::const_iterator iter = stack.begin();

      REQUIRE(iter == stack.cbegin());
      REQUIRE(iter != stack.cend());

      REQUIRE(iter == stack.begin());
   }
}

TEST_CASE("QStack empty", "[qstack]")
{
   QStack<QString> stack;

   REQUIRE(stack.isEmpty());
   REQUIRE(stack.size() == 0);
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

#if ! defined(Q_CC_MSVC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif

   stack.insert(1, "mango");

#if ! defined(Q_CC_MSVC)
#pragma GCC diagnostic pop
#endif

   REQUIRE(stack.contains("mango"));
   REQUIRE(stack[1] == "mango");
   REQUIRE(stack.length() == 5);
}

TEST_CASE("QStack pop", "[qstack]")
{
   QStack<QString> stack;
   stack.push("watermelon");
   stack.push("apple");
   stack.push("pear");
   stack.push("grapefruit");

   REQUIRE(stack.pop() == "grapefruit");
   REQUIRE(stack.pop() == "pear");
   REQUIRE(stack.size() == 2);

   REQUIRE(stack.contains("pear") == false);
   REQUIRE(stack.contains("apple") == true);
}

TEST_CASE("QStack position", "[qstack]")
{
   QStack<QString> stack;
   stack.push("watermelon");
   stack.push("apple");
   stack.push("pear");
   stack.push("grapefruit");

   REQUIRE(stack.top()   == "grapefruit");

   REQUIRE(stack.first() == "watermelon");
   REQUIRE(stack.last()  == "grapefruit");

   REQUIRE(stack.front() == "watermelon");
   REQUIRE(stack.back()  == "grapefruit");
}

TEST_CASE("QStack toList", "[qstack]")
{
   QStack<QString> stack;
   stack.push("watermelon");
   stack.push("apple");
   stack.push("pear");
   stack.push("grapefruit");

   SECTION("Convert to QList") {
        QList<QString> list = stack.toList();

        REQUIRE(list.size() == 4);

        REQUIRE(list.contains("watermelon"));
        REQUIRE(list.contains("apple"));
        REQUIRE(list.contains("pear"));
        REQUIRE(list.contains("grapefruit"));
    }

    SECTION("Modify QList") {
        QList<QString> list = stack.toList();
        list.removeAll("pear");

        REQUIRE(list.size() == 3);
        REQUIRE(list.contains("apple") == true);
        REQUIRE(list.contains("pear") == false);

        REQUIRE(stack.size() == 4);
        REQUIRE(stack.contains("pear") == true);
    }
}

TEST_CASE("QStack swap", "[qstack]")
{
   QStack<QString> stack1;
   stack1.push("watermelon");
   stack1.push("apple");
   stack1.push("pear");
   stack1.push("grapefruit");

   QStack<QString> stack2;
   stack2.push("grape");
   stack2.push("orange");
   stack2.push("peach");

   stack1.swap(stack2);

   REQUIRE(stack1.contains("orange") == true);
   REQUIRE(stack2.contains("orange") == false);
}
