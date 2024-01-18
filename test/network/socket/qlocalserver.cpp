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

#include <qlocalserver.h>

#include <cs_catch2.h>

TEST_CASE("QLocalServer traits", "[qlocalserver]")
{
   REQUIRE(std::is_copy_constructible_v<QLocalServer> == false);
   REQUIRE(std::is_move_constructible_v<QLocalServer> == false);

   REQUIRE(std::is_copy_assignable_v<QLocalServer> == false);
   REQUIRE(std::is_move_assignable_v<QLocalServer> == false);

   REQUIRE(std::is_nothrow_move_constructible_v<QLocalServer> == false);
   REQUIRE(std::is_nothrow_move_assignable_v<QLocalServer> == false);

   REQUIRE(std::has_virtual_destructor_v<QLocalServer> == true);
}

TEST_CASE("QLocalServer connections", "[qlocalserver]")
{
   QLocalServer server;
   server.setMaxPendingConnections(10);

   REQUIRE(server.maxPendingConnections() == 10);
}

TEST_CASE("QLocalServer server", "[qlocalserver]")
{
   QLocalServer server;
   server.close();

   REQUIRE(server.serverName() == QString());
   REQUIRE(server.fullServerName() == QString());
   REQUIRE(server.serverError() == QAbstractSocket::UnknownSocketError);
   REQUIRE(server.errorString() == QString());

   REQUIRE(server.hasPendingConnections() == false);
   REQUIRE(server.isListening() == false);
   REQUIRE(server.maxPendingConnections() == 30);
   REQUIRE(server.nextPendingConnection() == nullptr);

   //
   server.setMaxPendingConnections(20);
   bool timedOut = true;

   REQUIRE(server.waitForNewConnection(3000, &timedOut) == false);
   REQUIRE(timedOut == false);

   REQUIRE(server.listen("") == false);
}
