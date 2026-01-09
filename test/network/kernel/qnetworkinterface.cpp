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

#include <qnetworkinterface.h>

#include <qregularexpression.h>

#include <cs_catch2.h>

TEST_CASE("QNetworkInterface traits", "[qnetworkinterface]")
{
   REQUIRE(std::is_copy_constructible_v<QNetworkInterface> == true);
   REQUIRE(std::is_move_constructible_v<QNetworkInterface> == true);

   REQUIRE(std::is_copy_assignable_v<QNetworkInterface> == true);
   REQUIRE(std::is_move_assignable_v<QNetworkInterface> == true);

   REQUIRE(std::is_nothrow_move_constructible_v<QNetworkInterface> == false);
   REQUIRE(std::is_nothrow_move_assignable_v<QNetworkInterface> == false);

   REQUIRE(std::has_virtual_destructor_v<QNetworkInterface> == false);
}

TEST_CASE("QNetworkInterface hardwareAddress", "[qnetworkinterface]")
{
   auto list = QNetworkInterface::allInterfaces();
   REQUIRE(list.size() > 0);

   //
   QString address;

   for (auto item : list) {
      address = item.hardwareAddress();

      if ((item.flags() & QNetworkInterface::IsUp) && (item.flags() & QNetworkInterface::IsRunning) &&
            ! (item.flags() & QNetworkInterface::IsLoopBack)) {
         break;
      }
   }

   if (! address.isEmpty()) {
      QRegularExpression regExp("([[:xdigit:]]{2}:){5}[[:xdigit:]]{2}");
      QRegularExpressionMatch match = regExp.match(address);

      REQUIRE(regExp.isValid() == true);
      REQUIRE(match.hasMatch() == true);
   }
}
