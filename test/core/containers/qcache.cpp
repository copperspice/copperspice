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

#include <qcache.h>

#include <cs_catch2.h>

TEST_CASE("QCache traits", "[qcache]")
{
   REQUIRE(std::is_copy_constructible_v<QCache<int, int>> == false);
   REQUIRE(std::is_move_constructible_v<QCache<int, int>> == false);

   REQUIRE(std::is_copy_assignable_v<QCache<int, int>> == false);
   REQUIRE(std::is_move_assignable_v<QCache<int, int>> == false);

   REQUIRE(std::has_virtual_destructor_v<QCache<int, int>> == false);
}

TEST_CASE("QCache clear", "[qcache]")
{
   QCache<QString, int> cache(2);

   cache.insert("key1", new int(100));
   cache.insert("key2", new int(200));

   REQUIRE(cache.contains("key1") == true);

   REQUIRE(cache.isEmpty() == false);
   REQUIRE(cache.count() == 2);
   REQUIRE(cache.size()  == 2);

   cache.clear();
   REQUIRE(cache.isEmpty() == true);
   REQUIRE(cache.count() == 0);
   REQUIRE(cache.size()  == 0);
}

TEST_CASE("QCache empty", "[qcache]")
{
   QCache<QString, int> cache;

   REQUIRE(cache.isEmpty() == true);
   REQUIRE(cache.count() == 0);
   REQUIRE(cache.maxCost() == 100);
   REQUIRE(cache.totalCost() == 0);
}

TEST_CASE("QCache remove", "[qcache]")
{
   QCache<QString, int> cache(50);

   REQUIRE(cache.isEmpty());
   REQUIRE(cache.count() == 0);
   REQUIRE(cache.totalCost() == 0);
   REQUIRE(cache.maxCost() == 50);

   //
   cache.insert("key1", new int(17), 10);
   cache.insert("key2", new int(35), 70);

   REQUIRE(cache.contains("key1") == true);
   REQUIRE(cache.contains("key2") == false);
   REQUIRE(cache.count() == 1);

   //
   int *retrieved = cache["key1"];

   CHECK(retrieved != nullptr);
   CHECK(*retrieved == 17);

   cache.remove("key1");
   REQUIRE(cache.contains("key1") == false);
}

TEST_CASE("QCache take", "[qcache]")
{
   QCache<QString, QString> cache(10);

   cache.insert("key1", new QString("orange"));
   cache.insert("key2", new QString("berry"));

   REQUIRE(cache.isEmpty() == false);
   REQUIRE(cache.count() == 2);

   //
   QString *retrieved = cache.take("key1");

   REQUIRE(retrieved != nullptr);
   REQUIRE(*retrieved == "orange");

   delete retrieved;
}

TEST_CASE("QCache various_cost", "[qcache]")
{
   QCache<QString, int> cache1(100);
   QCache<QString, int> cache2(200);
   QCache<QString, int> cache3(-25);

   {
      cache1.insert("one", new int(1), 1);
      cache1.insert("two", new int(2), 1);

      REQUIRE(cache1.maxCost() == 100);
      REQUIRE(cache1.size() == 2);
      REQUIRE(cache1.totalCost() == 2);

      cache1.insert("three", new int(3), 85);

      REQUIRE(cache1.maxCost() == 100);
      REQUIRE(cache1.size() == 3);
      REQUIRE(cache1.totalCost() == 87);
   }

   {
      cache2.setMaxCost(150);

      REQUIRE(cache2.maxCost() == 150);
      REQUIRE(cache2.size() == 0);
      REQUIRE(cache2.totalCost() == 0);
   }

   {
      cache3.insert("one", new int(10), 1);

      REQUIRE(cache3.maxCost() == -25);
      REQUIRE(cache3.size() == 0);
      REQUIRE(cache3.totalCost() == 0);
   }
}
