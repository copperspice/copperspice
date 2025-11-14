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

TEST_CASE("QSet constructor", "[qset]")
{
   QSet<int> set{ 10, 20, 30, 30, 20};

   REQUIRE(set.size() == 3);

   REQUIRE(set.contains(10) == true);
   REQUIRE(set.contains(20) == true);
   REQUIRE(set.contains(30) == true);
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

   REQUIRE(set.size() == 4);

   //
   set.clear();

   REQUIRE(set.size() == 0);
}

TEST_CASE("QSet contains_a", "[qset]")
{
   QSet<QString> set = { "watermelon", "apple", "pear", "grapefruit", "pear" };

   REQUIRE(set.size() == 4);

   REQUIRE(set.contains("pear")  == true);
   REQUIRE(set.contains("mango") == false);
}

TEST_CASE("QSet contains_b", "[qset]")
{
   QSet<QString> set1 = { "watermelon", "apple", "pear", "grapefruit" };
   QSet<QString> set2 = { "grape", "orange", "apple" };

   REQUIRE(set1.contains(set1) == true);
   REQUIRE(set1.contains(set2) == false);

   //
   set1.insert("orange");
   set1.insert("grape");

   REQUIRE(set1.size() == 6);
   REQUIRE(set1.contains(set2) == true);

   //
   set1.insert("orange");

   REQUIRE(set1.size() == 6);
   REQUIRE(set1.contains(set2) == true);
}

TEST_CASE("QSet copy_assign", "[qset]")
{
   QSet<QString> set_a = { "watermelon", "apple", "pear", "grapefruit", "pear" };
   QSet<QString> set_b = set_a;

   REQUIRE(set_b.size() == 4);
   REQUIRE(set_b.size() == 4);

   REQUIRE(set_a == set_b);

   REQUIRE(set_a.contains("watermelon") == true);
   REQUIRE(set_b.contains("watermelon") == true);

   //
   QSet<QString> set_c;
   set_c = set_a;

   REQUIRE(set_a.size() == 4);
   REQUIRE(set_c.size() == 4);

   REQUIRE(set_a == set_c);

   REQUIRE(set_a.contains("watermelon") == true);
   REQUIRE(set_c.contains("watermelon") == true);
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
   QSet<QString> set3 = { "watermelon", "quince" };

   REQUIRE(set1 == set2);
   REQUIRE(! (set1 != set2));

   REQUIRE(!( set1 == set3));
   REQUIRE(set1 != set3);
}

TEST_CASE("QSet erase", "[qset]")
{
   QSet<QString> set = { "watermelon", "apple", "pear", "grapefruit" };

   auto iter = set.find("apple");
   set.erase(iter);

   REQUIRE(set.size() == 3);

   REQUIRE(set.contains("apple")      == false);

   REQUIRE(set.contains("watermelon") == true);
   REQUIRE(set.contains("pear")       == true);
   REQUIRE(set.contains("grapefruit") == true);
}

TEST_CASE("QSet fromList", "[qset]")
{
   QList<int> list{ 10, 20, 30, 30, 20, 40 };
   QSet<int> set = QSet<int>::fromList(list);

   REQUIRE(list.size() == 6);
   REQUIRE(set.size()  == 4);

   REQUIRE(set.contains(30) == true);
   REQUIRE(set.contains(40) == true);
}

TEST_CASE("QSet iterators", "[qset]")
{
   QSet<QString> set{ "watermelon", "apple", "pear" };

   auto iter = set.begin();

   REQUIRE(iter != set.end());

   ++iter;

   REQUIRE(iter != set.end());
   REQUIRE(set.contains(*iter) == true);
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

      REQUIRE(result.contains("banana") == true);
      REQUIRE(result.contains("cherry") == true);

      REQUIRE(result.contains("apple")  == false);
      REQUIRE(result.contains("date")   == false);

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
      setA = QSet<QString>{ "apple", "banana", "cherry" };

      QSet<QString> result = setA.intersect(QSet<QString>());

      REQUIRE(setA.size() == 0);

      REQUIRE(result.size() == 0);
      REQUIRE(result.isEmpty() == true);
   }

   {
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
      REQUIRE(setE.contains("date")   == true);
      REQUIRE(setE.contains("cherry") == true);
   }

   {
      QSet<QString> result = setA.intersect(setA);

      REQUIRE(result == setA);
   }

   {
      // reset values
      setA = QSet<QString>{ "apple", "banana", "cherry" };

      QSet<QString> result = setA & setB;

      REQUIRE(result.size() == 2);

      REQUIRE(result.contains("banana") == true);
      REQUIRE(result.contains("cherry") == true);

      REQUIRE(result != setA);
   }
}

TEST_CASE("QSet insert", "[qset]")
{
   QSet<QString> set = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(set.size() == 4);
   REQUIRE(set.contains("apple") == true);

   // duplicate
   set.insert("apple");

   REQUIRE(set.size() == 4);
   REQUIRE(set.contains("apple") == true);

   //
   set.insert("mango");

   REQUIRE(set.size() == 5);
   REQUIRE(set.contains("mango") == true);

   // duplicate
   auto iter = set.find("mango");

   REQUIRE(set.insert("mango") == iter);
   REQUIRE(set.size() == 5);

   //
   set << "quince" << "peach";

   REQUIRE(set.size() == 7);
   REQUIRE(set.contains("peach") == true);
}

TEST_CASE("QSet move_assign", "[qset]")
{
   QSet<QString> set_a = { "watermelon", "apple", "pear", "grapefruit", "pear" };
   QSet<QString> set_b(std::move(set_a));

   REQUIRE(set_b.size() == 4);
   REQUIRE(set_b.contains("watermelon") == true);

   //
   QSet<QString> set_c;
   set_c = std::move(set_b);

   REQUIRE(set_c.size() == 4);
   REQUIRE(set_c.contains("watermelon") == true);
}

TEST_CASE("QSet remove", "[qset]")
{
   QSet<QString> set = { "watermelon", "apple", "pear", "grapefruit" };
   bool ok;

   REQUIRE(set.size() == 4);

   //
   ok = set.remove("pear");

   REQUIRE(set.size() == 3);
   REQUIRE(ok == true);

   REQUIRE(set.contains("pear")       == false);
   REQUIRE(set.contains("watermelon") == true);
   REQUIRE(set.contains("apple")      == true);
   REQUIRE(set.contains("grapefruit") == true);

   //
   ok = set.remove("grape");

   REQUIRE(set.size() == 3);
   REQUIRE(ok == false);
}

TEST_CASE("QSet size", "[qset]")
{
   QSet<QString> set = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(set.size()   == 4);
}

TEST_CASE("QSet subtract", "[qset]")
{
   QSet<QString> setA = { "watermelon", "apple", "pear", "grapefruit" };
   QSet<QString> setB = { "orange", "pear"};

   {
      QSet<QString> result = setA.subtract(setB);

      REQUIRE(setA.size()   == 3);
      REQUIRE(setB.size()   == 2);
      REQUIRE(result.size() == 3);

      REQUIRE(result.contains("watermelon") == true);
      REQUIRE(result.contains("apple")      == true);
      REQUIRE(result.contains("pear")       == false);
      REQUIRE(result.contains("grapefruit") == true);

      REQUIRE(result.contains("grape")      == false);
      REQUIRE(result.contains("orange")     == false);
   }

   {
      // reset values
      setA = QSet<QString>{ "watermelon", "apple", "pear", "grapefruit" };

      QSet<QString> result = setA - setB;

      REQUIRE(setA.size()   == 4);
      REQUIRE(setB.size()   == 2);
      REQUIRE(result.size() == 3);
   }
}

TEST_CASE("QSet swap", "[qset]")
{
   QSet<QString> set1 = { "watermelon", "apple", "pear", "grapefruit" };
   QSet<QString> set2 = { "grape", "orange", "peach"};

   set1.swap(set2);

   REQUIRE(set1.contains("orange") == true);
   REQUIRE(set2.contains("orange") == false);
}

TEST_CASE("QSet toList", "[qset]")
{
    QSet<QString> set = { "watermelon", "apple", "pear", "grapefruit" };

    SECTION("Convert to QList") {
        QList<QString> list = set.toList();

        REQUIRE(set.size()  == 4);
        REQUIRE(list.size() == 4);

        REQUIRE(list.contains("watermelon")  == true);
        REQUIRE(list.contains("apple")       == true);
        REQUIRE(list.contains("pear")        == true);
        REQUIRE(list.contains("grapefruit")  == true);
    }

    SECTION("Convert to QList using values") {
        QList<QString> list = set.values();

        REQUIRE(set.size()  == 4);
        REQUIRE(list.size() == 4);

        REQUIRE(list.contains("watermelon")  == true);
        REQUIRE(list.contains("apple")       == true);
        REQUIRE(list.contains("pear")        == true);
        REQUIRE(list.contains("grapefruit")  == true);
    }

    SECTION("Modify QList") {
        QList<QString> list = set.toList();
        list.removeAll("pear");

        REQUIRE(list.size() == 3);
        REQUIRE(list.contains("apple") == true);
        REQUIRE(list.contains("pear")  == false);

        REQUIRE(set.size() == 4);
        REQUIRE(set.contains("pear")   == true);
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
      REQUIRE(result.contains("apple")      == true);
      REQUIRE(result.contains("pear")       == true);
      REQUIRE(result.contains("grapefruit") == true);

      REQUIRE(result.contains("grape")      == true);
      REQUIRE(result.contains("orange")     == true);
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
      REQUIRE(result.contains("peach") == true);
      REQUIRE(result.contains("apple") == true);
   }

   {
      QSet<QString> result = setA.unite(setA);

      REQUIRE(result == setA);
   }

   {
      setA = QSet<QString>{ "watermelon", "apple", "pear", "grapefruit" };
      QSet<QString> result = setA | setB;

      REQUIRE(result.size() == 6);

      REQUIRE(result.contains("watermelon") == true);
      REQUIRE(result.contains("apple")      == true);
      REQUIRE(result.contains("pear")       == true);
      REQUIRE(result.contains("grapefruit") == true);

      REQUIRE(result.contains("grape")      == true);
      REQUIRE(result.contains("orange")     == true);
   }
}
