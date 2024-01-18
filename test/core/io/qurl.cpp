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

#include <qurl.h>

#include <cs_catch2.h>

TEST_CASE("QUrl traits", "[qurl]")
{
   REQUIRE(std::is_copy_constructible_v<QUrl> == true);
   REQUIRE(std::is_move_constructible_v<QUrl> == true);

   REQUIRE(std::is_copy_assignable_v<QUrl> == true);
   REQUIRE(std::is_move_assignable_v<QUrl> == true);

   REQUIRE(std::has_virtual_destructor_v<QUrl> == false);
}

TEST_CASE("QUrl to_encoded", "[qurl]")
{
   QString str("http://www.example.com/List of Holidays.xml");

   QUrl uri = QUrl(str, QUrl::StrictMode);
   REQUIRE(uri.toEncoded() == "");

   uri = QUrl(str, QUrl::TolerantMode);
   REQUIRE(uri.toEncoded() == "http://www.example.com/List%20of%20Holidays.xml");
}

TEST_CASE("QUrl is_valid", "[qurl]")
{
   QUrl uri("https://www.copperspice.com/style_guide/source_code_style.html");
   REQUIRE(uri.isValid());

   uri = QUrl("https://www.copperspice.com/some_random_name");
   REQUIRE(uri.isValid());
}

TEST_CASE("QUrl scheme", "[qurl]")
{
   QUrl uri("https://www.copperspice.com/about.html");
   REQUIRE(uri.scheme() == "https");
}

TEST_CASE("QUrl user_info", "[qurl]")
{
   QUrl uri1("https://www.copperspice.com/about.html");
   REQUIRE(uri1.userInfo() == "");

   QUrl uri2("https://user:password@www.copperspice.com/about.html");
   REQUIRE(uri2.userInfo() == "user:password");
}

TEST_CASE("QUrl host", "[qurl]")
{
   QUrl uri("https://www.copperspice.com/about.html");
   REQUIRE(uri.host() == "www.copperspice.com");
}

TEST_CASE("QUrl path", "[qurl]")
{
   QUrl uri1("https://www.copperspice.com/about.html");
   REQUIRE(uri1.path() == "/about.html");

   QUrl uri2("mailto:postmaster@example.com");
   REQUIRE(uri2.path() == "postmaster@example.com");
}

TEST_CASE("QUrl to_string", "[qurl]")
{
   QUrl uri("https://www.copperspice.com/about.html");
   REQUIRE(uri.toString() == "https://www.copperspice.com/about.html");
}
