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

#include <qversionnumber.h>

#include <cs_catch2.h>

TEST_CASE("QVersionNumber traits", "[qversionnumber]")
{
   REQUIRE(std::is_copy_constructible_v<QVersionNumber> == true);
   REQUIRE(std::is_move_constructible_v<QVersionNumber> == true);

   REQUIRE(std::is_copy_assignable_v<QVersionNumber> == true);
   REQUIRE(std::is_move_assignable_v<QVersionNumber> == true);

   REQUIRE(std::has_virtual_destructor_v<QVersionNumber> == false);
}

TEST_CASE("QVersionNumber constructor", "[qversionnumber]")
{
   {
      QVersionNumber data1(1);
      QVersionNumber data2(QVector<int>() << 1);

      REQUIRE(data1.segments() == data2.segments());
   }

   {
      QVersionNumber data1(1, 2);
      QVersionNumber data2(QVector<int>() << 1 << 2);

      REQUIRE(data1.segments() == data2.segments());
   }

   {
      QVersionNumber data1(1, 2, 3);
      QVersionNumber data2(QVector<int>() << 1 << 2 << 3);

      REQUIRE(data1.segments() == data2.segments());
   }

   {
      QVersionNumber data1(4, 5, 6);
      QVersionNumber data2 = {4, 5, 6};

      REQUIRE(data1.segments() == data2.segments());
   }
}

TEST_CASE("QVersionNumber copy_constructor", "[qversionnumber]")
{
   QVector<int> segments;
   QVersionNumber result;

   {
      segments = QVector{1, 2};
      result   = QVersionNumber(1, 2);

      QVersionNumber data1(segments);
      QVersionNumber data2(data1);

      REQUIRE(data2.majorVersion() == result.majorVersion());
      REQUIRE(data2.minorVersion() == result.minorVersion());
      REQUIRE(data2.microVersion() == result.microVersion());
      REQUIRE(data2.segments()     == result.segments());
   }

   {
      segments = QVector{4, 5, 6};
      result   = QVersionNumber(4, 5, 6);

      QVersionNumber data1(segments);
      QVersionNumber data2(data1);

      REQUIRE(data2.majorVersion() == result.majorVersion());
      REQUIRE(data2.minorVersion() == result.minorVersion());
      REQUIRE(data2.microVersion() == result.microVersion());
      REQUIRE(data2.segments()     == result.segments());
   }
}

TEST_CASE("QVersionNumber empty", "[qversionnumber]")
{
   QVersionNumber version;

   REQUIRE(version.majorVersion() == 0);
   REQUIRE(version.minorVersion() == 0);
   REQUIRE(version.microVersion() == 0);

   REQUIRE(version.segments() == QVector<int>());
}

TEST_CASE("QVersionNumber comparison", "[qversionnumber]")
{
   QVersionNumber data1;
   QVersionNumber data2(4, 5, 6);
   QVersionNumber data3(6, 4);

   REQUIRE( (data1 == data2) == false);
   REQUIRE( (data1 != data2) == true);
   REQUIRE( (data1 <  data2) == true);
   REQUIRE( (data1 >  data2) == false);
   REQUIRE( (data1 <= data2) == true);
   REQUIRE( (data1 >= data2) == false);

   REQUIRE( (data2 == data3) == false);
   REQUIRE( (data2 != data3) == true);
   REQUIRE( (data2 <  data3) == true);
   REQUIRE( (data2 >  data3) == false);
   REQUIRE( (data2 <= data3) == true);
   REQUIRE( (data2 >= data3) == false);
}

TEST_CASE("QVersionNumber commonPrefix", "[qversionnumber]")
{
   QVersionNumber data1;
   QVersionNumber data2;
   QVersionNumber common;

   {
      data1  = QVersionNumber();
      data2  = QVersionNumber(5, 1);
      common = QVersionNumber();

      QVersionNumber calculatedPrefix = QVersionNumber::commonPrefix(data1, data2);

      REQUIRE(calculatedPrefix == common);
      REQUIRE(calculatedPrefix.segments() == common.segments());
   }

   {
      data1  = QVersionNumber(5, 1, 2);
      data2  = QVersionNumber(5, 1);
      common = QVersionNumber(5, 1);

      QVersionNumber calculatedPrefix = QVersionNumber::commonPrefix(data1, data2);

      REQUIRE(calculatedPrefix == common);
      REQUIRE(calculatedPrefix.segments() == common.segments());
   }

   {
      data1  = QVersionNumber(5, 2, 8);
      data2  = QVersionNumber(5, 9);
      common = QVersionNumber(5);

      QVersionNumber calculatedPrefix = QVersionNumber::commonPrefix(data1, data2);

      REQUIRE(calculatedPrefix == common);
      REQUIRE(calculatedPrefix.segments() == common.segments());
   }
}

TEST_CASE("QVersionNumber prefixOf", "[qversionnumber]")
{
   QVersionNumber data1;
   QVersionNumber data2;

   {
      data1 = QVersionNumber();
      data2 = QVersionNumber(5, 1);

      REQUIRE(data1.isPrefixOf(data2) == true);
   }

   {
      data1 = QVersionNumber(5, 1, 2);
      data2 = QVersionNumber(5, 1);

      REQUIRE(data1.isPrefixOf(data2) == false);
   }

   {
      data1 = QVersionNumber(5, 1);
      data2 = QVersionNumber(5, 1, 2);

      REQUIRE(data1.isPrefixOf(data2) == true);
   }
}
