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

#include <qlocalsocket.h>

#include <cs_catch2.h>

TEST_CASE("QLocalSocket traits", "[qlocalsocket]")
{
   REQUIRE(std::is_copy_constructible_v<QLocalSocket> == false);
   REQUIRE(std::is_move_constructible_v<QLocalSocket> == false);

   REQUIRE(std::is_copy_assignable_v<QLocalSocket> == false);
   REQUIRE(std::is_move_assignable_v<QLocalSocket> == false);

   REQUIRE(std::is_nothrow_move_constructible_v<QLocalSocket> == false);
   REQUIRE(std::is_nothrow_move_assignable_v<QLocalSocket> == false);

   REQUIRE(std::has_virtual_destructor_v<QLocalSocket> == true);
}

TEST_CASE("QLocalSocket server", "[qlocalsocket]")
{
   QLocalSocket socket;

   REQUIRE(socket.serverName() == QString());
   REQUIRE(socket.fullServerName() == QString());

   socket.abort();

   REQUIRE(socket.bytesAvailable() == 0);
   REQUIRE(socket.bytesToWrite() == 0);
   REQUIRE(socket.canReadLine() == false);

   socket.close();
   socket.disconnectFromServer();

   REQUIRE(socket.error() == QLocalSocket::UnknownSocketError);
   REQUIRE(socket.errorString() == "Unknown error");

   REQUIRE(socket.flush() == false);
   REQUIRE(socket.isValid() == false);
   REQUIRE(socket.readBufferSize() == 0);

   socket.setReadBufferSize(0);

   REQUIRE(socket.state() == QLocalSocket::UnconnectedState);
   REQUIRE(socket.waitForConnected(0) == false);
}

