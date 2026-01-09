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

#include <qvector.h>

#include <cs_catch2.h>

TEST_CASE("QVector traits", "[qvector]")
{
   REQUIRE(std::is_copy_constructible_v<QVector<int>> == true);
   REQUIRE(std::is_move_constructible_v<QVector<int>> == true);

   REQUIRE(std::is_copy_assignable_v<QVector<int>> == true);
   REQUIRE(std::is_move_assignable_v<QVector<int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QVector<int>> == false);
}

TEST_CASE("QVector append", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   //
   v.append("quince");

   REQUIRE(v.contains("quince") == true);
   REQUIRE(v.at(4) == "quince");
   REQUIRE(v[4] == "quince");

   REQUIRE(v.length() == 5);

   //
   v.push_back("orange");

   REQUIRE(v.contains("orange") == true);
   REQUIRE(v.at(5) == "orange");
   REQUIRE(v[5] == "orange");

   REQUIRE(v.length() == 6);

   //
   v.append(QVector<QString>{ "grape", "cherry" });

   REQUIRE(v == QVector<QString>{ "watermelon", "apple", "pear", "grapefruit", "quince", "orange", "grape", "cherry" });
   REQUIRE(v.at(7) == "cherry");

   REQUIRE(v.length() == 8);
}

TEST_CASE("QVector clear", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   v.clear();

   REQUIRE(v.isEmpty() == true);
   REQUIRE(v.size() == 0);
}

TEST_CASE("QVector constructor", "[qvector]")
{
   {
      QVector<int> v;

      REQUIRE(v.isEmpty() == true);
      REQUIRE(v.size() == 0);
   }

   {
      QVector<int> v(5);               //  uses size_type constructor

      REQUIRE(v.isEmpty() == false);
      REQUIRE(v.size() == 5);

      for (int value : v) {
         REQUIRE(value == 0);
      }
   }

   {
      QVector<int> v{5};               //  uses an initializer list

      REQUIRE(v.isEmpty() == false);
      REQUIRE(v.size() == 1);

      for (int value : v) {
         REQUIRE(value == 5);
      }
   }

   {
      QVector<int> v(4, 7);

      REQUIRE(v.isEmpty() == false);
      REQUIRE(v.size() == 4);

      for (int value : v) {
         REQUIRE(value == 7);
      }
   }

   {
      QVector<QString> v { "apple", "banana", "cherry" };

      REQUIRE(v.isEmpty() == false);
      REQUIRE(v.size() == 3);

      REQUIRE(v.at(0) == "apple");
      REQUIRE(v.at(1) == "banana");
      REQUIRE(v.at(2) == "cherry");
   }
}

TEST_CASE("QVector contains_a", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(v.contains("pear")   == true);
   REQUIRE(v.contains("orange") == false);
}

TEST_CASE("QVector contains_b", "[qvector]")
{
   QVector<unsigned int> v = { 5, 11, 23 };

   unsigned int data1 = 5;
   int data2 = 5;

   REQUIRE(v.contains(data1) == true);
   REQUIRE(v.contains(data2) == true);
}

TEST_CASE("QVector copy_assign", "[qvector]")
{
   QVector<QString> v1 = { "watermelon", "apple", "pear", "grapefruit" };
   QVector<QString> v2(v1);

   REQUIRE(v1.size() == 4);
   REQUIRE(v2.size() == 4);

   REQUIRE(v1 == v2);

   //
   QVector<QString> v3;
   v3 = v1;

   REQUIRE(v1.size() == 4);
   REQUIRE(v3.size() == 4);

   REQUIRE(v1 == v3);
}

TEST_CASE("QVector count", "[qvector]")
{
   QVector<int> v;

   REQUIRE(v.count(1) == 0);

   //
   v = { 5, 8, 17, 8, 20 };

   REQUIRE(v.count(17)  == 1);
   REQUIRE(v.count(8)   == 2);
   REQUIRE(v.count(5)   == 1);
   REQUIRE(v.count(100) == 0);
}

TEST_CASE("QVector comparison", "[qvector]")
{
   QVector<QString> v1 = { "watermelon", "apple", "pear", "grapefruit" };
   QVector<QString> v2 = { "watermelon", "apple", "pear", "grapefruit" };
   QVector<QString> v3 = { "orange", "pineapple", "quince", "blackberry" };

   REQUIRE(v1 == v2);
   REQUIRE(! (v1 != v2));

   REQUIRE(v1 != v3);
}

TEST_CASE("QVector empty", "[qvector]")
{
   QVector<QString> v;

   REQUIRE(v.empty() == true);
   REQUIRE(v.size() == 0);

   v.append("apple");

   REQUIRE(v.empty() == false);
   REQUIRE(v.size() == 1);
}

TEST_CASE("QVector erase", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   v.erase(v.begin() + 1);

   REQUIRE(v.contains("apple") == false);

   REQUIRE(v.contains("watermelon") == true);
   REQUIRE(v.contains("pear") == true);
   REQUIRE(v.contains("grapefruit") == true);

   REQUIRE(v.length() == 3);
}

TEST_CASE("QVector fill", "[qvector]")
{
   QVector<int> v;

   //
   v.fill(7, 5);

   REQUIRE(v.size() == 5);

   for (int value : v) {
      REQUIRE(value == 7);
   }

   //
   v = QVector<int>{1, 2, 3};
   v.fill(9);

   REQUIRE(v.size() == 3);

   for (int value : v) {
      REQUIRE(value == 9);
   }

   //
   v = QVector<int>();
   v.fill(42, 0);

   REQUIRE(v.isEmpty() == true);
   REQUIRE(v.size() == 0);
}

TEST_CASE("QVector indexOf_a", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit", "apple" };

   REQUIRE(v.indexOf("apple") == 1);
   REQUIRE(v.indexOf("berry") == -1);

   REQUIRE(v.lastIndexOf("apple") == 4);
}

TEST_CASE("QVector indexOf_b", "[qvector]")
{
   QVector<unsigned int> v = { 5, 11, 23, 11 };

   unsigned int data1 = 5;
   int data2 = 5;

   REQUIRE(v.indexOf(data1) == 0);
   REQUIRE(v.indexOf(data2) == 0);

   REQUIRE(v.lastIndexOf(11) == 3);
}

TEST_CASE("QVector insert", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

#if ! defined(Q_CC_MSVC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif

   v.insert(1, "mango");

#if ! defined(Q_CC_MSVC)
#pragma GCC diagnostic pop
#endif

   REQUIRE(v.contains("mango") == true);
   REQUIRE(v[1] == "mango");
   REQUIRE(v.length() == 5);
}

TEST_CASE("QVector iterators", "[qvector]")
{
   QVector<int> v{ 10, 20, 30, 40 };

   //
   auto iter = v.cbegin();
   REQUIRE(*iter == 10);

   ++iter;
   REQUIRE(*iter == 20);

   //
   auto r_iter = v.crbegin();
   REQUIRE(*r_iter == 40);

   ++r_iter;
   REQUIRE(*r_iter == 30);
}

TEST_CASE("QVector length", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(v.length() == 4);
   REQUIRE(v.size() == 4);
}

TEST_CASE("QVector mid", "[qvector]")
{
   QVector<int> v1{ 10, 20, 30, 40, 50 };

   {
      QVector<int> v2 = v1.mid(2);
      REQUIRE(v2 == QVector<int>{30, 40, 50});
   }

   {
      QVector<int> v2 = v1.mid(1, 3);
      REQUIRE(v2 == QVector<int>{20, 30, 40});
   }

   {
      QVector<int> v2 = v1.mid(3, 10);
      REQUIRE(v2 == QVector<int>{40, 50});
   }

   {
      QVector<int> v2 = v1.mid(10);
      REQUIRE(v2.isEmpty() == true);
   }
}

TEST_CASE("QVector move", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   //
   v.move(0, 2);

   REQUIRE(v == QVector<QString>{ "apple", "pear", "watermelon", "grapefruit" });

   //
   v.move(3, 1);

   REQUIRE(v == QVector<QString>{ "apple", "grapefruit", "pear", "watermelon" });

   //
   v.move(2, 2);

   REQUIRE(v == QVector<QString>{ "apple", "grapefruit", "pear", "watermelon" });
}

TEST_CASE("QVector move_assign", "[qvector]")
{
   QVector<QString> v1 = { "watermelon", "apple", "pear", "grapefruit" };
   QVector<QString> v2(std::move(v1));

   REQUIRE(v2.size() == 4);
   REQUIRE(v2 == QVector<QString>{ "watermelon", "apple", "pear", "grapefruit" });

   //
   QVector<QString> v3;
   v3 = std::move(v2);

   REQUIRE(v3.size() == 4);
   REQUIRE(v3 == QVector<QString>{ "watermelon", "apple", "pear", "grapefruit" });
}

TEST_CASE("QVector operator", "[qvector]")
{
   QVector<int> v1 { 10, 20, 30 };
   QVector<int> v2 { 40, 50 };

   //
   v1 += v2;

   REQUIRE(v1 == QVector<int>{ 10, 20, 30, 40, 50 });

   //
   v1 += 200;

   REQUIRE(v1 == QVector<int>{ 10, 20, 30, 40, 50, 200 });

   //
   QVector<int> v3;
   v1 += v3;

   REQUIRE(v1 == QVector<int>{ 10, 20, 30, 40, 50, 200 });
}

TEST_CASE("QVector position", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(v.first() == "watermelon");
   REQUIRE(v.last()  == "grapefruit");

   REQUIRE(v.front() == "watermelon");
   REQUIRE(v.back()  == "grapefruit");
}

TEST_CASE("QVector prepend", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   v.prepend("quince");

   REQUIRE(v.contains("quince") == true);
   REQUIRE(v.at(0) == "quince");
   REQUIRE(v[0] == "quince");
   REQUIRE(v.front() == "quince");

   REQUIRE(v.length() == 5);
}

TEST_CASE("QVector pop push", "[qvector]")
{
   //
   QVector<int> v { 2 };
   v.pop_front();

   REQUIRE(v.isEmpty() == true);

   //
   v = { 2, 3, 4 };
   v.push_front(1);

   REQUIRE(v.front() == 1);
   REQUIRE(v == QVector<int>{ 1, 2, 3, 4 });

   //
   v.pop_front();

   REQUIRE(v == QVector<int>{ 2, 3, 4 });

   //
   v.pop_back();

   REQUIRE(v == QVector<int>{ 2, 3 });

   //
   v.push_back(5);

   REQUIRE(v == QVector<int>{ 2, 3, 5 });

}

TEST_CASE("QVector remove", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   v.removeOne("apple");
   v.remove(0);

   REQUIRE(v.contains("apple") == false);
   REQUIRE(v.contains("watermelon") == false);

   REQUIRE(v.contains("pear") == true);
   REQUIRE(v.contains("grapefruit") == true);

   REQUIRE(v.length() == 2);
}

TEST_CASE("QVector remove_all", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit", "apple" };

   //
   int howMany = v.removeAll("apple");

   REQUIRE(howMany == 2);
   REQUIRE(v.contains("apple") == false);
   REQUIRE(v.length() == 3);

   //
   howMany = v.removeAll("cherry");

   REQUIRE(howMany == 0);
   REQUIRE(v.length() == 3);
}

TEST_CASE("QVector remove_at", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   v.removeAt(1);

   REQUIRE(v.contains("apple") == false);
   REQUIRE(v.length() == 3);
}


TEST_CASE("QVector removeFirst removeLast", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   //
   v.removeFirst();
   v.removeLast();

   REQUIRE(v == QVector<QString>{"apple", "pear"});

   //
   v = { "grapefruit" };
   v.removeFirst();

   REQUIRE(v.isEmpty() == true);

   v = { "apple" };
   v.removeLast();

   REQUIRE(v.isEmpty() == true);
}

TEST_CASE("QVector removeOne", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   v.removeOne("pear");

   REQUIRE(v.contains("pear") == false);
   REQUIRE(v.length() == 3);

   //
   bool retval = v.removeOne("quince");

   REQUIRE(retval == false);
   REQUIRE(v == QVector<QString>{ "watermelon", "apple", "grapefruit" });
}

TEST_CASE("QVector replace", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   {
      v.replace(1, "cherry");

      REQUIRE(v[1] == "cherry");

      REQUIRE(v.contains("apple") == false);
      REQUIRE(v.contains("watermelon") == true);
      REQUIRE(v.contains("grapefruit") == true);

      REQUIRE(v == QVector<QString>{ "watermelon", "cherry", "pear", "grapefruit" });
      REQUIRE(v.length() == 4);
   }

   {
      v.replace(0, "orange");
      v.replace(2, "berry");

      REQUIRE(v[0] == "orange");
      REQUIRE(v[1] == "cherry");
      REQUIRE(v[2] == "berry");
      REQUIRE(v[3] == "grapefruit");
   }
}

TEST_CASE("QVector reserve", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(v.size() == 4);

   v.reserve(10);

   REQUIRE(v.capacity() == 10);
   REQUIRE(v.size() == 4);
}

TEST_CASE("QVector resize", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   {
      v.resize(5);

      REQUIRE(v[4].isEmpty() == true);
      REQUIRE(v.size() == 5);
   }

   {
      v.resize(3);
      v.squeeze();

      REQUIRE(v.capacity() == v.size());
   }
}

TEST_CASE("QVector sort", "[qvector]")
{
   QVector<int> v      = { 17, 21, 30, 42, 80, 30 };
   QVector<int> result = { 17, 21, 30, 30, 42, 80 };

   REQUIRE(v[4] == 80);
   REQUIRE(v.contains(30) == true);
   REQUIRE(v.contains(100) == false);

   //
   std::sort(v.begin(), v.end());

   REQUIRE(v == result);
   REQUIRE(v[4] == 42);
}

TEST_CASE("QVector startsWith endsWith", "[qvector]")
{
   QVector<QString> v;

   REQUIRE(v.startsWith("orange") == false);
   REQUIRE(v.endsWith("lemon") == false);

   //
   v = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(v.startsWith("watermelon") == true);
   REQUIRE(v.startsWith("lemon")  == false);

   REQUIRE(v.endsWith("grapefruit") == true);
   REQUIRE(v.endsWith("lemon")  == false);
}

TEST_CASE("QVector takeAt", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   {
      QString value = v.takeAt(2);

      REQUIRE(value == "pear");
      REQUIRE(v.length() == 3);
   }

   {
      QString value = v.takeAt(0);

      REQUIRE(value == "watermelon");
      REQUIRE(v == QVector<QString>{"apple", "grapefruit"});
   }
}

TEST_CASE("QVector takeFirst takeLast", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   {
      QString value = v.takeFirst();

      REQUIRE(value == "watermelon");
      REQUIRE(v == QVector<QString>{ "apple", "pear", "grapefruit" });
   }

   {
      QString value = v.takeLast();

      REQUIRE(value == "grapefruit");
      REQUIRE(v == QVector<QString>{ "apple", "pear"});
   }

   {
      v = { "apple" };

      REQUIRE(v.takeFirst() == "apple");
      REQUIRE(v.isEmpty() == true);
   }

   {
       v = { "pear" };

      REQUIRE(v.takeLast() == "pear");
      REQUIRE(v.isEmpty() == true);
   }
}


TEST_CASE("QVector value", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(v.value(0) == "watermelon");
   REQUIRE(v.value(2) == "pear");

   REQUIRE(v.value(10) == QString());

   REQUIRE(v.value(10, "default_fruit") == "default_fruit");
}
