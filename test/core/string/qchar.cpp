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

#include <qchar.h>

#include <catch2/catch.hpp>

TEST_CASE("QChar traits", "[qchar]")
{
   REQUIRE(std::is_copy_constructible_v<QChar> == true);
   REQUIRE(std::is_move_constructible_v<QChar> == true);

   REQUIRE(std::is_copy_assignable_v<QChar> == true);
   REQUIRE(std::is_move_assignable_v<QChar> == true);

   REQUIRE(std::has_virtual_destructor_v<QChar> == false);
}

TEST_CASE("QChar u8_constructor", "[qchar]")
{
   QChar ch = u8'b';

   REQUIRE(ch.isNull() == false);

   REQUIRE(ch.unicode() == char32_t(98));
   REQUIRE(ch == u8'b');
   REQUIRE(ch != u8'c');

   REQUIRE(u8'b' == ch);
   REQUIRE(u8'c' != ch);
}

TEST_CASE("QChar u_constructor", "[qchar]")
{
   QChar ch = u'b';

   REQUIRE(ch.isNull() == false);

   REQUIRE(ch.unicode() == char32_t(98));
   REQUIRE(ch == u'b');
   REQUIRE(ch != u'c');

   REQUIRE(u'b' == ch);
   REQUIRE(u'c' != ch);
}

TEST_CASE("QChar U_constructor", "[qchar]")
{
   QChar ch = U'b';

   REQUIRE(ch.isNull() == false);

   REQUIRE(ch.unicode() == char32_t(98));
   REQUIRE(ch == U'b');
   REQUIRE(ch != U'c');

   REQUIRE(U'b' == ch);
   REQUIRE(U'c' != ch);
}

TEST_CASE("QChar empty", "[qchar]")
{
   QChar ch;

   REQUIRE(ch.isNull() == true);
}

TEST_CASE("QChar is_methods", "[qchar]")
{
   QChar ch = 'B';

   REQUIRE(! ch.isDigit());
   REQUIRE(! ch.isLower());
   REQUIRE(! ch.isNumber());

   REQUIRE(ch.isLetter());
   REQUIRE(ch.isLetterOrNumber());
   REQUIRE(ch.isPrint());
   REQUIRE(ch.isUpper());

   ch = '7';

   REQUIRE(ch.isDigit());
   REQUIRE(! ch.isLower());
   REQUIRE(ch.isNumber());

   REQUIRE(! ch.isLetter());
   REQUIRE(ch.isLetterOrNumber());
   REQUIRE(ch.isPrint());
   REQUIRE(! ch.isUpper());
}

