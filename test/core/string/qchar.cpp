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

TEST_CASE("QChar u32_constructor", "[qchar]")
{
   QChar ch = U'b';

   REQUIRE(ch.isNull() == false);

   REQUIRE(ch.unicode() == char32_t(98));
   REQUIRE(ch == U'b');
   REQUIRE(ch != U'c');

   REQUIRE(U'b' == ch);
   REQUIRE(U'c' != ch);
}

TEST_CASE("QChar copy_assign", "[qchar]")
{
   QChar data_a('W');
   QChar data_b(data_a);

   REQUIRE(data_a == data_b);
   REQUIRE(data_b == QChar('W'));

   //
   QChar data_c;
   data_c = data_a;

   REQUIRE(data_a == data_c);
   REQUIRE(data_c == QChar('W'));
}

TEST_CASE("QChar empty", "[qchar]")
{
   QChar ch;

   REQUIRE(ch.isNull() == true);
}

TEST_CASE("QChar is_methods", "[qchar]")
{
   QChar ch = 'B';

   REQUIRE(ch.isDigit() == false);
   REQUIRE(ch.isLower() == false);
   REQUIRE(ch.isNumber() == false);

   REQUIRE(ch.isHex() == true );
   REQUIRE(ch.isLetter() == true);
   REQUIRE(ch.isLetterOrNumber() == true);
   REQUIRE(ch.isPrint() == true);
   REQUIRE(ch.isUpper() == true);

   ch = '7';

   REQUIRE(ch.isDigit() == true);
   REQUIRE(ch.isLower() == false);
   REQUIRE(ch.isNumber() == true);

   REQUIRE(ch.isHex() == true);
   REQUIRE(ch.isLetter() == false);
   REQUIRE(ch.isLetterOrNumber() == true);
   REQUIRE(ch.isPrint() == true);
   REQUIRE(ch.isUpper() == false);

   ch = '!';

   REQUIRE(ch.isDigit() == false);
   REQUIRE(ch.isLower() == false);
   REQUIRE(ch.isNumber() == false);

   REQUIRE(ch.isHex() == false);
   REQUIRE(ch.isLetter() == false);
   REQUIRE(ch.isLetterOrNumber() == false);
   REQUIRE(ch.isPrint() == true);
   REQUIRE(ch.isUpper() == false);
}

TEST_CASE("QChar move_assign", "[qchar]")
{
   QChar data_a('W');
   QChar data_b(std::move(data_a));

   REQUIRE(data_b == QChar('W'));

   //
   QChar data_c;
   data_c = std::move(data_b);

   REQUIRE(data_c == QChar('W'));
}
