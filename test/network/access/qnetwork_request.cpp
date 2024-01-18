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

#include <qnetwork_request.h>

#include <cs_catch2.h>

TEST_CASE("QNetworkRequest traits", "[qnetwork_request]")
{
   REQUIRE(std::is_copy_constructible_v<QNetworkRequest> == true);
   REQUIRE(std::is_move_constructible_v<QNetworkRequest> == true);

   REQUIRE(std::is_copy_assignable_v<QNetworkRequest> == true);
   REQUIRE(std::is_move_assignable_v<QNetworkRequest> == true);

   REQUIRE(std::is_nothrow_move_constructible_v<QNetworkRequest> == false);
   REQUIRE(std::is_nothrow_move_assignable_v<QNetworkRequest> == false);

   REQUIRE(std::has_virtual_destructor_v<QNetworkRequest> == false);
}

TEST_CASE("QNetworkRequest constructor", "[qnetwork_request]")
{
   QUrl url;

   {
      QNetworkRequest request;
      REQUIRE(url == request.url());
   }

   {
      QNetworkRequest request(url);
      REQUIRE(url == request.url());
   }

   url = QUrl("https://copperspice.com");

   {
      QNetworkRequest request(url);
      REQUIRE(url == request.url());
   }
}

TEST_CASE("QNetworkRequest setUrl", "[qnetwork_request]")
{
   QUrl url;
   QNetworkRequest request;

   {
      request.setUrl(url);

      REQUIRE(url == request.url());
   }

   url = QUrl("https://copperspice.com");

   {
      request.setUrl(url);

      REQUIRE(url == request.url());
   }
}
