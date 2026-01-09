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

#include <qvarlengtharray.h>

#include <cs_catch2.h>

TEST_CASE("QVarLengthArray traits", "[qvarlengtharray]")
{
   REQUIRE(std::is_copy_constructible_v<QVarLengthArray<int>> == true);
   REQUIRE(std::is_move_constructible_v<QVarLengthArray<int>> == true);

   REQUIRE(std::is_copy_assignable_v<QVarLengthArray<int>> == true);
   REQUIRE(std::is_move_assignable_v<QVarLengthArray<int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QVarLengthArray<int>> == false);
}

TEST_CASE("QVarLengthArray append", "[qvarlengtharray]")
{
   QVarLengthArray<int> data;

   data.append(10);
   data.append(20);

   REQUIRE(data.size() == 2);
   REQUIRE(data == (QVarLengthArray<int>{10, 20}));

   //
   data.push_back(45);
   data.push_back(32);

   REQUIRE(data.size() == 4);
   REQUIRE(data == (QVarLengthArray<int>{10, 20, 45, 32}));
}

TEST_CASE("QVarLengthArray back_front", "[qvarlengtharray]")
{
   QVarLengthArray<int> data;

   data.append(10);
   data.append(20);
   data.append(30);

   REQUIRE(data.size() == 3);

   REQUIRE(data.front() == 10);
   REQUIRE(data.first() == 10);

   REQUIRE(data.back()  == 30);
   REQUIRE(data.last()  == 30);
}

TEST_CASE("QVarLengthArray clear", "[qvarlengtharray]")
{
   QVarLengthArray<int> data = {40, 25, 73, 25, 10};

   REQUIRE(data.size() == 5);
   REQUIRE(data.isEmpty() == false);

   data.clear();

   REQUIRE(data.size() == 0);
   REQUIRE(data.isEmpty() == true);
}

TEST_CASE("QVarLengthArray comparison", "[qvarlengtharray]")
{
   QVarLengthArray<int> data_a = { 10, 20, 30 };
   QVarLengthArray<int> data_b = { 10, 20, 30 };
   QVarLengthArray<int> data_c = { 10, 50 };

   REQUIRE(data_a == data_b);
   REQUIRE(data_a != data_c);
}

TEST_CASE("QVarLengthArray constructor", "[qvarlengtharray]")
{
   QVarLengthArray<int> data(5);

   REQUIRE(data.size() == 5);

   // unable to not test initial values for a primitive type

   QVarLengthArray<QString> data_b(5);

   REQUIRE(data_b.size() == 5);

   REQUIRE(data_b[0] == QString());
   REQUIRE(data_b[1] == QString());
   REQUIRE(data_b[2] == QString());
   REQUIRE(data_b[3] == QString());
   REQUIRE(data_b[4] == QString());
}

TEST_CASE("QVarLengthArray contains", "[qvarlengtharray]")
{
   QVarLengthArray<int> data = {40, 25, 73, 25, 10};

   REQUIRE(data.contains(73) == true);
   REQUIRE(data.contains(99) == false);
}

TEST_CASE("QVarLengthArray copy_assign", "[qvarlengtharray]")
{
   QVarLengthArray<int> data_a = {40, 25, 73, 25, 10};
   QVarLengthArray<int> data_b(data_a);

   REQUIRE(data_a.size() == 5);
   REQUIRE(data_b.size() == 5);

   REQUIRE(data_a == data_b);

   //
   QVarLengthArray<int> data_c;
   data_c = data_a;

   REQUIRE(data_a.size() == 5);
   REQUIRE(data_c.size() == 5);

   REQUIRE(data_a == data_c);
}

TEST_CASE("QVarLengthArray empty", "[qvarlengtharray]")
{
   QVarLengthArray<int> data_a;

   REQUIRE(data_a.size() == 0);
   REQUIRE(data_a.isEmpty() == true);

   REQUIRE(data_a.capacity() >= 0);

   //
   data_a = QVarLengthArray<int>{ 0 };

   REQUIRE(data_a.size() == 1);
   REQUIRE(data_a.isEmpty() == false);

   //
   data_a = QVarLengthArray<int>( 0 );

   REQUIRE(data_a.size() == 0);
   REQUIRE(data_a.isEmpty() == true);

   //
   QVarLengthArray<QString> data_b{ QString() };

   REQUIRE(data_b.size() == 1);
   REQUIRE(data_b.empty() == false);

   //
   data_b = QVarLengthArray<QString>( 0 );

   REQUIRE(data_b.size() == 0);
   REQUIRE(data_b.empty() == true);
}

TEST_CASE("QVarLengthArray erase", "[qvarlengtharray]")
{
   QVarLengthArray<int> data = { 40, 25, 73, 25, 38, 63, 41, 90 };

   //
   data = QVarLengthArray<int>{ 40, 25, 73, 25, 38, 63, 41, 90 };

   auto iter = data.begin() + 4;
   data.erase(iter);

   REQUIRE(data == (QVarLengthArray<int>{ 40, 25, 73, 25, 63, 41, 90 }));

   //
   iter = data.begin() + 4;
   data.erase(iter, iter + 2);

   REQUIRE(data == (QVarLengthArray<int>{ 40, 25, 73, 25, 90 }));
}

TEST_CASE("QVarLengthArray indexOf", "[qvarlengtharray]")
{
   QVarLengthArray<int> data = {40, 25, 73, 25, 10};

    REQUIRE(data.indexOf(25)     == 1);
    REQUIRE(data.indexOf(999)    == -1);

    REQUIRE(data.lastIndexOf(25) == 3);
}

TEST_CASE("QVarLengthArray insert", "[qvarlengtharray]")
{
   QVarLengthArray<int> data = { 14, 26 };

   //
   data.insert(0, 71);

   REQUIRE(data == (QVarLengthArray<int>{ 71, 14, 26 }));

   //
   data.insert(2, 47);

   REQUIRE(data == (QVarLengthArray<int>{ 71, 14, 47, 26 }));

   //
   data.insert(data.size(), 39);

   REQUIRE(data == (QVarLengthArray<int>{ 71, 14, 47, 26, 39 }));
}

TEST_CASE("QVarLengthArray prepend", "[qvarlengtharray]")
{
   QVarLengthArray<int> data;

   data.prepend(10);
   data.prepend(20);

   REQUIRE(data.size() == 2);
   REQUIRE(data == (QVarLengthArray<int>{20, 10}));
}

TEST_CASE("QVarLengthArray operator", "[qvarlengtharray]")
{
   QVarLengthArray<int> data_a = { 10, 22, 30 };

   REQUIRE(data_a.at(1) == 22);
   REQUIRE(data_a[1] == 22);

   //
   data_a[1] = 55;

   REQUIRE(data_a.at(1) == 55);
   REQUIRE(data_a[1] == 55);

   //
   const QVarLengthArray<int> data_b = { 18, 28, 32 };

   REQUIRE(data_b.at(1) == 28);
   REQUIRE(data_b[1] == 28);

   //
   data_a += 46;

   REQUIRE(data_a.size() == 4);
   REQUIRE(data_a == QVarLengthArray<int>{ 10, 55, 30, 46 });

   //
   data_a << 37;

   REQUIRE(data_a.size() == 5);
   REQUIRE(data_a == QVarLengthArray<int>{ 10, 55, 30, 46, 37 });
}

TEST_CASE("QVarLengthArray remove", "[qvarlengtharray]")
{
   QVarLengthArray<int> data = { 40, 25, 73, 25, 38, 63, 41, 90 };

   //
   data.remove(1);

   REQUIRE(data == (QVarLengthArray<int>{ 40, 73, 25, 38, 63, 41, 90 }));

   //
   data.remove(2, 4);

   REQUIRE(data == (QVarLengthArray<int>{ 40, 73, 90 }));

   //
   data.remove(1, 2);

   REQUIRE(data == (QVarLengthArray<int>{ 40 }));
}

TEST_CASE("QVarLengthArray reserve", "[qvarlengtharray]")
{
   QVarLengthArray<int> data;

   data.reserve(50);

   REQUIRE(data.capacity() >= 50);
   REQUIRE(data.size() == 0);
}

TEST_CASE("QVarLengthArray replace", "[qvarlengtharray]")
{
   QVarLengthArray<QString> data = {"apple", "orange", "grape", "strawberry"};

   REQUIRE(data.size() == 4);

   //
   data.replace(1, "quince");

   REQUIRE(data.size() == 4);
   REQUIRE(data == QVarLengthArray<QString>{"apple", "quince", "grape", "strawberry"});
}

TEST_CASE("QVarLengthArray resize", "[qvarlengtharray]")
{
   QVarLengthArray<QString> data = {"apple", "orange", "grape", "strawberry"};

   REQUIRE(data.size() == 4);

   //
   data.resize(10);

   REQUIRE(data.size() == 10);
   REQUIRE(data[3]     == "strawberry");
   REQUIRE(data[4]     == QString());

   //
   data.resize(2);
   REQUIRE(data[1] == "orange");

   //
   data.resize(6, "pear");

   REQUIRE(data.size() == 6);

   REQUIRE(data[0] == "apple");
   REQUIRE(data[1] == "orange");
   REQUIRE(data[2] == "pear");
   REQUIRE(data[3] == "pear");
   REQUIRE(data[4] == "pear");
   REQUIRE(data[5] == "pear");

   //
   data.resize(4, "X");

   REQUIRE(data.size() == 4);

   REQUIRE(data[0] == "apple");
   REQUIRE(data[1] == "orange");
   REQUIRE(data[2] == "pear");
   REQUIRE(data[3] == "pear");
}

TEST_CASE("QVarLengthArray squeeze", "[qvarlengtharray]")
{
   QVarLengthArray<QString> data = {"apple", "orange", "grape", "strawberry"};

   REQUIRE(data.capacity() >= 4);
   REQUIRE(data.size() == 4);

   //
   data.remove(0);

   data.squeeze();

   REQUIRE(data.capacity() == 3);
   REQUIRE(data.size() == 3);
}
