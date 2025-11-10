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
   QList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   list.append("quince");

   REQUIRE(list.at(2) == "pear");
   REQUIRE(list[2]    == "pear");

   REQUIRE(list.contains("quince"));
   REQUIRE(list[4]    == "quince");

   REQUIRE(list.length() == 5);

   //
   list.append(list);

   REQUIRE(list[4]    == "quince");
   REQUIRE(list[9]    == "quince");

   REQUIRE(list.length() == 10);
}

TEST_CASE("QList begin_end", "[qlist]")
{
   {
      QList<QString> list;
      auto iter = list.begin();

      REQUIRE(iter == list.end());
   }

   {
      QList<QString> list = {"apple"};
      auto iter = list.begin();

      REQUIRE(iter != list.end());
      REQUIRE(*iter == "apple");

      ++iter;

      REQUIRE(iter == list.end());
   }

   {
      const QList<QString> list = {"apple", "orange"};
      QList<QString>::const_iterator iter = list.cbegin();

      REQUIRE(*iter == "apple");
   }

   {
      QList<int> list_a = {10, 20, 30, 40};
      QList<int> list_b;

      for (auto iter = list_a.crbegin(); iter != list_a.crend(); ++iter) {
         list_b.append(*iter);
      }

      REQUIRE(list_b == QList<int>{40, 30, 20, 10});
   }
}

TEST_CASE("QList clear", "[qlist]")
{
   QList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   list.clear();

   REQUIRE(list.isEmpty());
   REQUIRE(list.size() == 0);
}

TEST_CASE("QList contains", "[qlist]")
{
   QList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(list.contains("pear") == true);
   REQUIRE(list.contains("orange") == false);
}

TEST_CASE("QList copy_assign", "[qlist]")
{
   QList<QString> list_a = { "watermelon", "apple", "pear", "grapefruit" };
   QList<QString> list_b = list_a;

   REQUIRE(list_a == list_b);

   list_b[0] = "quince";

   REQUIRE(list_a[0] == "watermelon");
   REQUIRE(list_b[0] == "quince");

   QList<QString> list_c;
   list_c = list_a;

   REQUIRE(list_c == list_a);
}

TEST_CASE("QList empty", "[qlist]")
{
   QList<QString> list;

   REQUIRE(list.isEmpty() == true);
   REQUIRE(list.size() == 0);
}

TEST_CASE("QList comparison", "[qlist]")
{
   QList<QString> list1 = { "watermelon", "apple", "pear", "grapefruit" };
   QList<QString> list2 = { "watermelon", "apple", "pear", "grapefruit" };

   QList<QString> list3(list1);

   QList<QString> list4;
   list4 = list2;

   REQUIRE(list1 == list2);
   REQUIRE(! (list1 != list2));

   REQUIRE(list1 == list3);
   REQUIRE(list1 == list4);
}

TEST_CASE("QList erase", "[qlist]")
{
   QList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   {
      list.erase(list.begin() + 1);

      REQUIRE(list.contains("apple") == false);

      REQUIRE(list.contains("watermelon"));
      REQUIRE(list.contains("pear"));
      REQUIRE(list.contains("grapefruit"));

      REQUIRE(list.length() == 3);
   }

   {
      list.erase(list.end() - 1);

      REQUIRE(list.contains("watermelon"));
      REQUIRE(list.contains("pear"));

      REQUIRE(list.length() == 2);
   }

   {
      list = { "watermelon", "apple", "pear", "grapefruit", "orange", "berry" };

      list.erase(list.begin() + 2, list.begin() + 5);

      REQUIRE(list.contains("watermelon"));
      REQUIRE(list.contains("apple"));
      REQUIRE(list.contains("berry"));

      REQUIRE(list.length() == 3);
   }

   {
      list.erase(list.begin(), list.begin());
      REQUIRE(list.length() == 3);
   }
}

TEST_CASE("QList indexOf", "[qlist]")
{
   QList<QString> list = { "watermelon", "apple", "pear", "grapefruit", "pear" };

   REQUIRE(list.indexOf("pear")   == 2);
   REQUIRE(list.indexOf("orange") == -1);

   REQUIRE(list.lastIndexOf("pear") == 4);
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

TEST_CASE("QList mid", "[qlist]")
{
   QList<QString> list_a = {"a", "b", "c", "d", "e"};
   QList<QString> list_b = {"c", "d", "e"} ;

   REQUIRE(list_a.mid(2) == list_b);

   //
   list_b = {"b", "c", "d"};

   REQUIRE(list_a.mid(1, 3) == list_b);

   //
   list_b = {"d", "e"};

   REQUIRE(list_a.mid(3, 10) == list_b);
   REQUIRE(list_a.mid(10).isEmpty() == true);

   //
   REQUIRE(list_a.mid(2, 0).isEmpty() == true);
}

TEST_CASE("QList move", "[qlist]")
{
   QList<QString> list = {"a", "b", "c", "d"};

   {
      list.move(1, 3);
      QList<QString> tmpList = {"a", "c", "d", "b"};

      REQUIRE(list == tmpList);
   }

   {
      list.move(3, 0);
      QList<QString> tmpList = {"b", "a", "c", "d"};

      REQUIRE(list == tmpList);
   }

   {
      list.move(1, 1);
      QList<QString> tmpList = {"b", "a", "c", "d"};

      REQUIRE(list == tmpList);
   }

   {
      list.move(0, 2);
      QList<QString> tmpList = {"a", "c", "b", "d"};

      REQUIRE(list == tmpList);
   }

   {
      list.move(2, 0);
      QList<QString> tmpList = {"b", "a", "c", "d"};

      REQUIRE(list == tmpList);
   }
}

TEST_CASE("QList move_assign", "[qlist]")
{
   QList<QString> list_a = { "watermelon", "apple", "pear", "grapefruit" };
   QList<QString> list_b = std::move(list_a);

   REQUIRE(list_b == QList<QString>{"watermelon", "apple", "pear", "grapefruit"});

   //
   QList<QString> list_c;
   list_c = std::move(list_b);

   REQUIRE(list_c == QList<QString>{"watermelon", "apple", "pear", "grapefruit"});
}

TEST_CASE("QList operators", "[qlist]")
{
   QList<int> list_a = { 10, 20 };
   QList<int> list_b = { 30, 40 };

   list_a += list_b;

   REQUIRE(list_a.size() == 4);
   REQUIRE(list_a.last() == 40);

   //
   QList<int> list_c = list_a + list_b;

   REQUIRE(list_c.size() == 6);
   REQUIRE(list_c.at(2)  == 30);

   //
   list_a << 50;
   list_a << 60;

   REQUIRE(list_a.size() == 6);
   REQUIRE(list_a.last() == 60);
}

TEST_CASE("QList position", "[qlist]")
{
   QList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(list.at(0)  == "watermelon");
   REQUIRE(list.at(3)  == "grapefruit");

   REQUIRE(list.value(0) == "watermelon");
   REQUIRE(list.value(10, "default") == "default");

   REQUIRE(list.first() == "watermelon");
   REQUIRE(list.last()  == "grapefruit");

   REQUIRE(list.front() == "watermelon");
   REQUIRE(list.back()  == "grapefruit");
}

TEST_CASE("QList prepend", "[qlist]")
{
   QList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   list.prepend("quince");

   REQUIRE(list.contains("quince"));
   REQUIRE(list[0] == "quince");
   REQUIRE(list.length() == 5);
}

TEST_CASE("QList remove", "[qlist]")
{
   QList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   {
      list.removeOne("apple");
      list.remove(0);

      REQUIRE(list.contains("apple") == false);
      REQUIRE(list.contains("watermelon") == false);

      REQUIRE(list.contains("pear"));
      REQUIRE(list.contains("grapefruit"));

      REQUIRE(list.length() == 2);
   }

   {
      list.removeAt(1);

      REQUIRE(list.contains("pear"));
      REQUIRE(list.contains("grapefruit") == false);

      REQUIRE(list.length() == 1);
   }

   {
      list.prepend("apple");
      list.prepend("watermelon");

      REQUIRE(list.length() == 3);

      list.removeFirst();
      list.removeLast();

      REQUIRE(list.contains("apple"));
      REQUIRE(list.length() == 1);
   }
}

TEST_CASE("QList replace", "[qlist]")
{
   QList<QString> list = { "watermelon", "apple", "pear" };
   list.replace(1, "quince");

   REQUIRE(list == QList<QString>{ "watermelon", "quince", "pear" });
}

TEST_CASE("QList swap", "[qlist]")
{
   QList<QString> list_a = { "watermelon", "apple" };
   QList<QString> list_b = { "pear", "grapefruit"  };

   list_a.swap(list_b);

   REQUIRE(list_a.first() == "pear");
   REQUIRE(list_b.first() == "watermelon");
}

TEST_CASE("QList take", "[qlist]")
{
   QList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(list.takeFirst() == "watermelon");
   REQUIRE(list.takeLast()  == "grapefruit");
   REQUIRE(list == QList<QString>{ "apple", "pear" });

   list.prepend("watermelon");
   list.append("grapefruit");

   //
   QString value = list.takeAt(2);

   REQUIRE(value == "pear");

   REQUIRE(list.contains("pear") == false);
   REQUIRE(list.size() == 3);
}
