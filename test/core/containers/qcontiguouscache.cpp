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

#include <qcontiguouscache.h>

#include <cs_catch2.h>

TEST_CASE("QContiguousCache traits", "[qcontiguouscache]")
{
   REQUIRE(std::is_copy_constructible_v<QContiguousCache<int>> == true);
   REQUIRE(std::is_move_constructible_v<QContiguousCache<int>> == true);

   REQUIRE(std::is_copy_assignable_v<QContiguousCache<int>> == true);
   REQUIRE(std::is_move_assignable_v<QContiguousCache<int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QContiguousCache<int>> == false);
}

TEST_CASE("QContiguousCache append", "[qcontiguouscache]")
{
   QContiguousCache<int> data(3);

   data.append(1);
   data.append(2);
   data.append(3);

   REQUIRE(data.size() == 3);
   REQUIRE(data.firstIndex() == 0);
   REQUIRE(data.lastIndex()  == 2);

   REQUIRE(data[0] == 1);
   REQUIRE(data[1] == 2);
   REQUIRE(data[2] == 3);
}

TEST_CASE("QContiguousCache clear", "[qcontiguouscache]")
{
   QContiguousCache<int> data(3);

   data.append(1);
   data.append(2);

   REQUIRE(data.size() == 2);
   REQUIRE(data.isEmpty() == false);

   data.clear();

   REQUIRE(data.size() == 0);
   REQUIRE(data.isEmpty() == true);
}

TEST_CASE("QContiguousCache constructor", "[qcontiguouscache]")
{
   QContiguousCache<int> data;

   REQUIRE(data.capacity() == 0);
   REQUIRE(data.size() == 0);
   REQUIRE(data.isEmpty() == true);

   //
   data = QContiguousCache<int>(5);

   REQUIRE(data.capacity() == 5);
   REQUIRE(data.size() == 0);
   REQUIRE(data.isEmpty() == true);
}

TEST_CASE("QContiguousCache contains", "[qcontiguouscache]")
{
   QContiguousCache<QString> data(2);

   data.insert(10, "a");
   data.insert(11, "b");

   REQUIRE(data.containsIndex(12) == false);
   REQUIRE(data.containsIndex(11) == true);
   REQUIRE(data.containsIndex(10) == true);
   REQUIRE(data.containsIndex(9)  == false);

   REQUIRE(data.at(10) == "a");
   REQUIRE(data[11]    == "b");
}

TEST_CASE("QContiguousCache copy_assign", "[qcontiguouscache]")
{
   QContiguousCache<QString> data_1(2);
   data_1.append("watermelon");
   data_1.append("apple");

   QContiguousCache<QString> data_2(data_1);

   REQUIRE(data_1.size() == 2);
   REQUIRE(data_2.size() == 2);

   REQUIRE(data_1 == data_2);

   //
   QContiguousCache<QString> data_3;
   data_3 = data_1;

   REQUIRE(data_1.size() == 2);
   REQUIRE(data_3.size() == 2);

   REQUIRE(data_1 == data_3);
}

TEST_CASE("QContiguousCache evict", "[qcontiguouscache]")
{
   QContiguousCache<int> data(3);

   data.append(1);
   data.append(2);
   data.append(3);
   data.append(4);    // evicts 1

   REQUIRE(data.size() == 3);

   REQUIRE(data.containsIndex(0) == false);
   REQUIRE(data.containsIndex(1) == true);
   REQUIRE(data.containsIndex(2) == true);
   REQUIRE(data.containsIndex(3) == true);

   REQUIRE(data[1] == 2);
   REQUIRE(data[2] == 3);
   REQUIRE(data[3] == 4);
}

TEST_CASE("QContiguousCache firstIndex/lastIndex", "[qcontiguouscache]")
{
   QContiguousCache<int> data(4);

   data.insert(100, 1);
   data.insert(101, 2);
   data.insert(102, 3);

   REQUIRE(data.firstIndex() == 100);
   REQUIRE(data.lastIndex()  == 102);
}

TEST_CASE("QContiguousCache gap", "[qcontiguouscache]")
{
   QContiguousCache<int> data(3);

   data.insert(10, 1);

   REQUIRE(data.size() == 1);

   REQUIRE(data.containsIndex(10) == true);
   REQUIRE(data.containsIndex(11) == false);
   REQUIRE(data.containsIndex(12) == false);

   //
   data.insert(12, 3);    // gap at 11 causes the container to be cleared

   REQUIRE(data.size() == 1);

   REQUIRE(data.containsIndex(10) == false);
   REQUIRE(data.containsIndex(11) == false);
   REQUIRE(data.containsIndex(12) == true);
}

TEST_CASE("QContiguousCache insert", "[qcontiguouscache]")
{
   QContiguousCache<int> data(3);

   data.insert(0, 10);
   data.insert(1, 20);
   data.insert(2, 30);

   REQUIRE(data.size() == 3);
   REQUIRE(data.firstIndex() == 0);
   REQUIRE(data.lastIndex()  == 2);

   REQUIRE(data[0] == 10);
   REQUIRE(data[1] == 20);
   REQUIRE(data[2] == 30);
}

TEST_CASE("QContiguousCache move_assign", "[qcontiguouscache]")
{
   QContiguousCache<QString> data_1(2);
   data_1.append("watermelon");
   data_1.append("apple");

   QContiguousCache<QString> data_2(std::move(data_1));

   REQUIRE(data_2.size() == 2);

   REQUIRE(data_2[0] == "watermelon");
   REQUIRE(data_2[1] == "apple");

   //
   QContiguousCache<QString> data_3;
   data_3 = std::move(data_2);

   REQUIRE(data_3.size() == 2);

   REQUIRE(data_3[0] == "watermelon");
   REQUIRE(data_3[1] == "apple");
}

TEST_CASE("QContiguousCache overwrite", "[qcontiguouscache]")
{
   QContiguousCache<int> data(2);

   data.insert(5, 10);
   data.insert(5, 20);          // overwrite

   REQUIRE(data.size() == 1);
   REQUIRE(data[5] == 20);
}

TEST_CASE("QContiguousCache prepend", "[qcontiguouscache]")
{
   QContiguousCache<int> data(3);

   data.prepend(3);
   data.prepend(2);
   data.prepend(1);

   data.normalizeIndexes();

   REQUIRE(data.size() == 3);
   REQUIRE(data.firstIndex() == 0);
   REQUIRE(data.lastIndex()  == 2);

   REQUIRE(data[0]  == 1);
   REQUIRE(data[1]  == 2);
   REQUIRE(data[2]  == 3);
}

TEST_CASE("QContiguousCache setCapacity", "[qcontiguouscache]")
{
   QContiguousCache<int> data(5);

   data.append(10);
   data.append(20);
   data.append(30);
   data.append(40);

   REQUIRE(data.size() == 4);
   REQUIRE(data.capacity()  == 5);
   REQUIRE(data.lastIndex() == 3);

   REQUIRE(data[0] == 10);
   REQUIRE(data[1] == 20);
   REQUIRE(data[2] == 30);
   REQUIRE(data[3] == 40);

   //
   data.setCapacity(2);

   REQUIRE(data.size() == 2);
   REQUIRE(data.capacity() == 2);
   REQUIRE(data.lastIndex() == 3);

   REQUIRE(data[2] == 30);
   REQUIRE(data[3] == 40);

   // accessing an out of range element clears the container
   // additionally, element 0 is default constructed
   REQUIRE(data[0] == 0);

   REQUIRE(data.size() == 1);
   REQUIRE(data.capacity() == 2);
}
