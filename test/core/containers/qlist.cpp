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
   REQUIRE(list.size() == 0);
}

TEST_CASE("QList equality", "[qlist]")
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

      REQUIRE(! list.contains("apple"));

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
   QList<QString> list = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(list.indexOf("pear")   == 2);
   REQUIRE(list.indexOf("orange") == -1);
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
   QList<QString> list = {"a", "b", "c", "d", "e"};

   QList<QString> tmpList = {"c", "d", "e"} ;
   REQUIRE(list.mid(2) == tmpList);

   tmpList = {"b", "c", "d"};
   REQUIRE(list.mid(1, 3) == tmpList);

   tmpList = {"d", "e"};
   REQUIRE(list.mid(3, 10) == tmpList);

   REQUIRE(list.mid(10).isEmpty() == true);
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

   {
      list.removeOne("apple");
      list.remove(0);

      REQUIRE(! list.contains("apple"));
      REQUIRE(! list.contains("watermelon"));

      REQUIRE(list.contains("pear"));
      REQUIRE(list.contains("grapefruit"));

      REQUIRE(list.length() == 2);
   }

   {
      list.removeAt(1);

      REQUIRE(list.contains("pear"));
      REQUIRE(! list.contains("grapefruit"));

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
