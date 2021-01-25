/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#include <qchar.h>

#include <catch2/catch.hpp>

TEST_CASE("QChar empty", "[qchar]")
{
   QChar str;

   REQUIRE(str.isNull());
}

TEST_CASE("QChar is_something", "[qchar]")
{
   QChar str = 'B';

   REQUIRE(! str.isDigit());
   REQUIRE(! str.isLower());
   REQUIRE(! str.isNumber());

   REQUIRE(str.isLetter());
   REQUIRE(str.isLetterOrNumber());
   REQUIRE(str.isPrint());
   REQUIRE(str.isUpper());

   str = '7';

   REQUIRE(str.isDigit());
   REQUIRE(! str.isLower());
   REQUIRE(str.isNumber());

   REQUIRE(! str.isLetter());
   REQUIRE(str.isLetterOrNumber());
   REQUIRE(str.isPrint());
   REQUIRE(! str.isUpper());
}

