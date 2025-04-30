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

   v.append("quince");

   REQUIRE(v.contains("quince"));
   REQUIRE(v.at(4) == "quince");
   REQUIRE(v[4] == "quince");
   REQUIRE(v.length() == 5);

   v.push_back("orange");

   REQUIRE(v.contains("orange"));
   REQUIRE(v.at(5) == "orange");
   REQUIRE(v[5] == "orange");
   REQUIRE(v.length() == 6);
}

TEST_CASE("QVector clear", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   v.clear();

   REQUIRE(v.size() == 0);
}

TEST_CASE("QVector constructor", "[qvector]")
{
   {
      QVector<int> v(5);

      REQUIRE(v.size() == 5);

      for (int value : v) {
         REQUIRE(value == 0);
      }
   }

   {
      QVector<int> v(4, 7);

      REQUIRE(v.size() == 4);

      for (int val : v) {
         REQUIRE(val == 7);
      }
   }

   {
      QVector<QString> v { "apple", "banana", "cherry" };

      REQUIRE(v.size() == 3);

      REQUIRE(v.at(0) == "apple");
      REQUIRE(v.at(1) == "banana");
      REQUIRE(v.at(2) == "cherry");
   }
}

TEST_CASE("QVector contains_a", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(v.contains("pear"));
   REQUIRE(! v.contains("orange"));
}

TEST_CASE("QVector contains_b", "[qvector]")
{
   QVector<unsigned int> v = { 5, 11, 23 };

   unsigned int data1 = 5;
   int data2 = 5;

   REQUIRE(v.contains(data1) == true);
   REQUIRE(v.contains(data2) == true);
}

TEST_CASE("QVector copy_move_constructor", "[qvector]")
{
   QVector<QString> v1 = { "watermelon", "apple", "pear", "grapefruit" };

   {
      QVector<QString> v2 = v1;

      REQUIRE(v1 == v2);
      REQUIRE(v1.size() == 4);
      REQUIRE(v2.size() == 4);
   }

   {
      QVector<QString> v2(std::move(v1));

      REQUIRE(v1.size() == 0);
      REQUIRE(v2.size() == 4);
   }
}

TEST_CASE("QVector copy_move_assign", "[qvector]")
{
   QVector<QString> v1 = { "watermelon", "apple", "pear", "grapefruit" };

   {
      QVector<QString> v2;
      v2 = v1;

      REQUIRE(v1 == v2);
      REQUIRE(v1.size() == 4);
      REQUIRE(v2.size() == 4);
   }

   {
      QVector<QString> v2;
      v2 = std::move(v1);

      REQUIRE(v1.size() == 0);
      REQUIRE(v2.size() == 4);
   }
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

TEST_CASE("QVector empty", "[qvector]")
{
   QVector<QString> v;

   REQUIRE(v.isEmpty());
   REQUIRE(v.size() == 0);
}

TEST_CASE("QVector comparison", "[qvector]")
{
   QVector<QString> v1 = { "watermelon", "apple", "pear", "grapefruit" };
   QVector<QString> v2 = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(v1 == v2);
   REQUIRE(! (v1 != v2));
}

TEST_CASE("QVector erase", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   v.erase(v.begin() + 1);

   REQUIRE(! v.contains("apple"));

   REQUIRE(v.contains("watermelon"));
   REQUIRE(v.contains("pear"));
   REQUIRE(v.contains("grapefruit"));

   REQUIRE(v.length() == 3);
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

   REQUIRE(v.contains("mango"));
   REQUIRE(v[1] == "mango");
   REQUIRE(v.length() == 5);
}

TEST_CASE("QVector length", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   REQUIRE(v.length() == 4);
   REQUIRE(v.size() == 4);
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

   REQUIRE(v.contains("quince"));
   REQUIRE(v.at(0) == "quince");
   REQUIRE(v[0] == "quince");
   REQUIRE(v.front() == "quince");

   REQUIRE(v.length() == 5);
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

TEST_CASE("QVector remove_at", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   v.removeAt(1);

   REQUIRE(v.contains("apple") == false);
   REQUIRE(v.length() == 3);
}

TEST_CASE("QVector remove_one", "[qvector]")
{
   QVector<QString> v = { "watermelon", "apple", "pear", "grapefruit" };

   v.removeOne("pear");

   REQUIRE(v.contains("pear") == false);
   REQUIRE(v.length() == 3);
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

TEST_CASE("QVector startsWith_endsWith", "[qvector]")
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

   QString value = v.takeAt(2);

   REQUIRE(value == "pear");
   REQUIRE(v.length() == 3);
}
