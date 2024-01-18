/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

#include <qbitarray.h>

#include <cs_catch2.h>

TEST_CASE("QBitArray traits", "[QBitArray]")
{
   REQUIRE(std::is_copy_constructible_v<QBitArray> == true);
   REQUIRE(std::is_move_constructible_v<QBitArray> == true);

   REQUIRE(std::is_copy_assignable_v<QBitArray> == true);
   REQUIRE(std::is_move_assignable_v<QBitArray> == true);

   REQUIRE(std::has_virtual_destructor_v<QBitArray> == false);
}

TEST_CASE("QBitArray at", "[qbitarray]")
{
   QBitArray data1(3, true);
   REQUIRE(data1.at(0));
   REQUIRE(data1.at(1));
   REQUIRE(data1.at(2));

   QBitArray data2(3, false);
   REQUIRE(! data2.at(0));
   REQUIRE(! data2.at(1));
   REQUIRE(! data2.at(2));
}

TEST_CASE("QBitArray clear", "[qbitarray]")
{
   int size = 20;

   QBitArray data(size, true);
   data.clear();

   REQUIRE(data.isEmpty());
   REQUIRE(data.isNull());
}

TEST_CASE("QBitArray empty", "[qbitarray]")
{
   QBitArray data;

   REQUIRE(data.isEmpty());
   REQUIRE(data.isNull());
}

TEST_CASE("QBitArray fill", "[qbitarray]")
{
   int size = 17;

   QBitArray data(size, true);
   data.fill(false, 31);

   REQUIRE(! data.isEmpty());
   REQUIRE(data.isNull() == false);
}

TEST_CASE("QBitArray operator_a", "[qbitarray]")
{
   int size = 16;

   QBitArray data1(8, true);
   QBitArray data2(size, false);

   data2.setBit(1);
   data1 &= data2;

   REQUIRE(data1.testBit(1));
   REQUIRE(! data1.testBit(size - 1));
}

TEST_CASE("QBitArray operator_b", "[qbitarray]")
{
   QBitArray data1(7, true);
   QBitArray data2(7, true);

   REQUIRE(data1 == data2);
}

TEST_CASE("QBitArray operator_c", "[qbitarray]")
{
   QBitArray data1(13, true);
   QBitArray data2(13, false);

   data2 ^= data1;

   REQUIRE(data1 == data2);
}

TEST_CASE("QBitArray operator", "[qbitarray]")
{
   QBitArray data1(3, true);
   REQUIRE(data1[0]);
   REQUIRE(data1[1]);
   REQUIRE(data1[2]);

   QBitArray data2(3, false);
   REQUIRE(! data2[0]);
   REQUIRE(! data2[1]);
   REQUIRE(! data2[2]);
}

TEST_CASE("QBitArray resize", "[qbitarray]")
{
   int size = 17;

   QBitArray data(size, true);
   data.resize(0);

   REQUIRE(data.isEmpty());
   REQUIRE(data.isNull() == false);
}

TEST_CASE("QBitArray size", "[qbitarray]")
{
   int size = 20;

   QBitArray data(size, true);

   REQUIRE(data.size() == size);
   REQUIRE(! data.isEmpty());
   REQUIRE(data.isNull() == false);
}

TEST_CASE("QBitArray setbit_lowest", "[qbitarray]")
{
   int size = 31;

   QBitArray data(size, false);
   data.setBit(0);

   REQUIRE(data.testBit(0));
   REQUIRE(! data.isEmpty());
   REQUIRE(data.isNull() == false);
}

TEST_CASE("QBitArray setbit_highest", "[qbitarray]")
{
   int size = 17;

   QBitArray data(size, false);
   data.setBit(size - 1);

   REQUIRE(data.testBit(size - 1));
   REQUIRE(! data.isEmpty());
   REQUIRE(data.isNull() == false);
}

TEST_CASE("QBitArray swap", "[qbitarray]")
{
   QBitArray data1(11, true);
   QBitArray data2(11, false);
   QBitArray data3(11, true);

   REQUIRE(data1 != data2);
   REQUIRE(data1 == data3);

   data2.swap(data3);

   REQUIRE(data1 == data2);
   REQUIRE(data1 != data3);
}
