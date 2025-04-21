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

TEST_CASE("QSet begin_end", "[qset]")
{
   QSet<QString> setA = { "watermelon", "apple", "pear", "grapefruit" };
   QSet<QString> setB;

   for (auto iter = setA.begin(); iter != setA.end(); ++iter)  {
      setB.insert(*iter);
   }

   REQUIRE(setA.size() == setB.size());
   REQUIRE(setA == setB);
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

   REQUIRE(set.contains("pear")  == true);
   REQUIRE(set.contains("mango") == false);
}

TEST_CASE("QSet contains_b", "[qset]")
{
   QSet<QString> set1 = { "watermelon", "apple", "pear", "grapefruit" };
   QSet<QString> set2 = { "grape", "orange", "apple" };

   REQUIRE(set1.contains(set1) == true);
   REQUIRE(set1.contains(set2) == false);

   set1.insert("orange");
   set1.insert("grape");

   REQUIRE(set1.contains(set2) == true);
}

TEST_CASE("QSet duplicate", "[qset]")
{
   QSet<QString> set = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(set.contains("apple"));
   REQUIRE(set.size() == 4);

   set.insert("apple");

   REQUIRE(set.contains("apple"));
   REQUIRE(set.size() == 4);
}

TEST_CASE("QSet empty", "[qset]")
{
   QSet<QString> set;

   REQUIRE(set.isEmpty() == true);
   REQUIRE(set.size() == 0);
}

TEST_CASE("QSet comparison", "[qset]")
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

TEST_CASE("QSet intersect", "[qset]")
{
   QSet<QString> setA;
   setA << "apple" << "banana" << "cherry";

   QSet<QString> setB;
   setB << "banana" << "cherry" << "date";

   {
      QSet<QString> result = setA.intersect(setB);

      REQUIRE(result.size() == 2);
      REQUIRE(result.contains("banana"));
      REQUIRE(result.contains("cherry"));

      REQUIRE(result.contains("apple") == false);
      REQUIRE(result.contains("date")  == false);

      REQUIRE(result == setA);

      REQUIRE(setA.size() == 2);
      REQUIRE(setB.size() == 3);
   }

   {
      QSet<QString> setC;
      setC << "kiwi" << "mango";

      QSet<QString> result = setA.intersect(setC);

      REQUIRE(setA.size() == 0);

      REQUIRE(result.size() == 0);
      REQUIRE(result.isEmpty() == true);
   }

   {
      // reset values
      setA << "apple" << "banana" << "cherry";

      QSet<QString> result = setA.intersect(QSet<QString>());

      REQUIRE(setA.size() == 0);

      REQUIRE(result.size() == 0);
      REQUIRE(result.isEmpty() == true);
   }

   {
      // reset values
      setA << "apple" << "banana" << "cherry";

      QSet<QString> setD = setA;

      REQUIRE(setA.size() == 3);
      REQUIRE(setD.size() == 3);

      REQUIRE(setD.contains("apple")  == true);
      REQUIRE(setD.contains("banana") == true);
      REQUIRE(setD.contains("cherry") == true);
   }

   {
      QSet<QString> setE;
      setE << "apple" << "date" << "cherry";

      setE &= setB;

      REQUIRE(setE.size() == 2);
      REQUIRE(setE.contains("date"));
      REQUIRE(setE.contains("cherry"));
   }
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

TEST_CASE("QSet toList", "[qset]")
{
    QSet<QString> set = { "watermelon", "apple", "pear", "grapefruit" };

    SECTION("Convert to QList") {
        QList<QString> list = set.toList();

        REQUIRE(list.size() == 4);

        REQUIRE(list.contains("watermelon"));
        REQUIRE(list.contains("apple"));
        REQUIRE(list.contains("pear"));
        REQUIRE(list.contains("grapefruit"));
    }

    SECTION("Modify QList") {
        QList<QString> list = set.toList();
        list.removeAll("pear");

        REQUIRE(list.size() == 3);
        REQUIRE(list.contains("apple") == true);
        REQUIRE(list.contains("pear") == false);

        REQUIRE(set.size() == 4);
        REQUIRE(set.contains("pear") == true);
    }
}

TEST_CASE("QSet unite", "[qset]")
{
   QSet<QString> setA = { "watermelon", "apple", "pear", "grapefruit" };
   QSet<QString> setB = { "grape", "orange", "pear"};

   {
      QSet<QString> result = setA.unite(setB);

      REQUIRE(result.size() == 6);
      REQUIRE(result.contains("watermelon") == true);
      REQUIRE(result.contains("apple") == true);
      REQUIRE(result.contains("pear") == true);
      REQUIRE(result.contains("grapefruit") == true);

      REQUIRE(result.contains("grape") == true);
      REQUIRE(result.contains("orange") == true);
   }

   {
      QSet<QString> result = setA.unite(QSet<QString>());

      REQUIRE(result == setA);
   }

   {
      QSet<QString> setC;
      setC << "peach" << "apple";

      QSet<QString> result = setA.unite(setB).unite(setC);

      REQUIRE(result.size() == 7);
      REQUIRE(result.contains("peach"));
      REQUIRE(result.contains("apple"));
   }
}
