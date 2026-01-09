/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

TEST_CASE("QLinkedList append", "[qlinkedlist]")
{
   QLinkedList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   list.append("quince");

   REQUIRE(list.first() == "watermelon");
   REQUIRE(list.last()  == "quince");

   REQUIRE(list.contains("quince") == true);

   REQUIRE(list.length() == 5);

   //
   list.append(list);

   auto iter = list.begin();
   ++iter;
   ++iter;
   ++iter;
   ++iter;

   REQUIRE(*iter == "quince");

   //
   ++iter;
   ++iter;
   ++iter;
   ++iter;
   ++iter;

   REQUIRE(*iter == "quince");

   REQUIRE(list.length() == 10);
}

TEST_CASE("QLinkedList begin_end", "[qlinkedlist]")
{
   {
      QLinkedList<QString> list;
      auto iter = list.begin();

      REQUIRE(iter == list.end());
   }

   {
      QLinkedList<QString> list = {"apple"};
      auto iter = list.begin();

      REQUIRE(iter != list.end());
      REQUIRE(*iter == "apple");

      ++iter;

      REQUIRE(iter == list.end());
   }

   {
      const QLinkedList<QString> list = {"apple", "orange"};
      QLinkedList<QString>::const_iterator iter = list.cbegin();

      REQUIRE(*iter == "apple");
   }

   {
      QLinkedList<int> list_a = {10, 20, 30, 40};
      QLinkedList<int> list_b;

      for (auto iter = list_a.crbegin(); iter != list_a.crend(); ++iter) {
         list_b.append(*iter);
      }

      REQUIRE(list_b == QLinkedList<int>{40, 30, 20, 10});
   }
}

TEST_CASE("QLinkedList clear", "[qlinkedlist]")
{
   QLinkedList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   list.clear();

   REQUIRE(list.size() == 0);

   REQUIRE(list.isEmpty() == true);
   REQUIRE(list.size() == 0);
}

TEST_CASE("QLinkedList comparison", "[qlinkedlist]")
{
   QLinkedList<QString> list1 = { "watermelon", "apple", "pear", "grapefruit" };
   QLinkedList<QString> list2 = { "watermelon", "apple", "pear", "grapefruit" };

   QLinkedList<QString> list3(list1);

   QLinkedList<QString> list4;
   list4 = list2;

   REQUIRE(list1 == list2);
   REQUIRE(! (list1 != list2));

   REQUIRE(list1 == list3);
   REQUIRE(list1 == list4);
}

TEST_CASE("QLinkedList constructor", "[qlinkedlist]")
{
   QLinkedList<int> list_a{15, 30, 45};

   REQUIRE(list_a.size()  == 3);

   REQUIRE(list_a.first() == 15);
   REQUIRE(list_a.last()  == 45);

   //
   QLinkedList<QLinkedList<int>> list_b;
   list_b.append(QLinkedList<int>({15, 30, 45}));
   list_b.append(QLinkedList<int>({55, 82}));

   REQUIRE(list_b.size() == 2);

   REQUIRE(list_b.first().size() == 3);
   REQUIRE(list_b.last().size()  == 2);

   REQUIRE(list_b.last().first() == 55);
   REQUIRE(list_b.last().last()  == 82);
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

TEST_CASE("QLinkedList empty", "[qlinkedlist]")
{
   QLinkedList<QString> list;

   REQUIRE(list.isEmpty() == true);
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

   {
      list.erase(--list.end());

      REQUIRE(list.contains("watermelon") == true);
      REQUIRE(list.contains("pear") == true);

      REQUIRE(list.length() == 2);
   }

   {
      list.erase(list.begin(), list.begin());
      REQUIRE(list.length() == 2);
   }
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

TEST_CASE("QLinkedList operators", "[qlinkedlist]")
{
   QLinkedList<int> list_a = { 10, 20 };
   QLinkedList<int> list_b = { 30, 40 };

   list_a += list_b;

   REQUIRE(list_a.size() == 4);
   REQUIRE(list_a.last() == 40);

   //
   QLinkedList<int> list_c = list_a + list_b;

   REQUIRE(list_c.size() == 6);
   REQUIRE(list_c.contains(30) == true);

   //
   list_a << 50;
   list_a << 60;

   REQUIRE(list_a.size() == 6);
   REQUIRE(list_a.last() == 60);
}

TEST_CASE("QLinkedList position", "[qlinkedlist]")
{
   QLinkedList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(list.first() == "watermelon");
   REQUIRE(list.last()  == "grapefruit");

   REQUIRE(list.front() == "watermelon");
   REQUIRE(list.back()  == "grapefruit");

   REQUIRE(list.startsWith("watermelon") == true);
   REQUIRE(list.endsWith("grapefruit")   == true);

   REQUIRE(list.startsWith("grape") == false);
   REQUIRE(list.endsWith("orange")  == false);
}

TEST_CASE("QLinkedList prepend", "[qlinkedlist]")
{
   QLinkedList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   list.prepend("quince");

   REQUIRE(list.contains("quince") == true);
   REQUIRE(list.first() == "quince");

   REQUIRE(list.length() == 5);
}

TEST_CASE("QLinkedList remove", "[qlinkedlist]")
{
   QLinkedList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   {
      list.removeOne("apple");

      REQUIRE(list.contains("apple") == false);

      REQUIRE(list.contains("watermelon") == true);
      REQUIRE(list.contains("pear") == true);
      REQUIRE(list.contains("grapefruit") == true);

      REQUIRE(list.length() == 3);
   }

   {
      list.prepend("apple");
      list.prepend("watermelon");

      REQUIRE(list.length() == 5);

      list.removeFirst();
      list.removeLast();

      REQUIRE(list.contains("apple") == true);
      REQUIRE(list.length() == 3);
   }
}

TEST_CASE("QLinkedList removeAll", "[qlinkedlist]")
{
   QLinkedList<int> list = { 5, 12, 3, 10, 5, 1, 0, 6, 5 };

   REQUIRE(list.count() == 9);
   REQUIRE(list.contains(5) == true);

   //
   int tally_1 = list.count(5);
   int tally_2 = list.removeAll(5);

   REQUIRE(tally_1 == 3);
   REQUIRE(tally_2 == 3);

   REQUIRE(list.count()     == 6);
   REQUIRE(list.contains(5) == false);
}

TEST_CASE("QLinkedList stdList", "[qlinkedlist]")
{
   QLinkedList<double> list_a = { 1.2, 0.5, 3.14 };

   //
   std::list<double> stdlist = list_a.toStdList();

   //
   QLinkedList<double> list_b = QLinkedList<double>::fromStdList(stdlist);

   REQUIRE(list_a == list_b);
}

TEST_CASE("QLinkedList swap", "[qlinkedlist]")
{
   QLinkedList<QString> list_a = { "watermelon", "apple" };
   QLinkedList<QString> list_b = { "pear", "grapefruit"  };

   list_a.swap(list_b);

   REQUIRE(list_a.first() == "pear");
   REQUIRE(list_b.first() == "watermelon");
}

TEST_CASE("QLinkedList take", "[qlinkedlist]")
{
   QLinkedList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(list.takeFirst() == "watermelon");
   REQUIRE(list.takeLast()  == "grapefruit");
   REQUIRE(list == QLinkedList<QString>{ "apple", "pear" });

   list.prepend("watermelon");
   list.append("grapefruit");
}
