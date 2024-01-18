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

#include <qudpsocket.h>

#include <cs_catch2.h>

TEST_CASE("QUdpSocket traits", "[qudpsocket]")
{
   REQUIRE(std::is_copy_constructible_v<QUdpSocket> == false);
   REQUIRE(std::is_move_constructible_v<QUdpSocket> == false);

   REQUIRE(std::is_copy_assignable_v<QUdpSocket> == false);
   REQUIRE(std::is_move_assignable_v<QUdpSocket> == false);

   REQUIRE(std::is_nothrow_move_constructible_v<QUdpSocket> == false);
   REQUIRE(std::is_nothrow_move_assignable_v<QUdpSocket> == false);

   REQUIRE(std::has_virtual_destructor_v<QUdpSocket> == true);
}

TEST_CASE("QUdpSocket constructor", "[qudpsocket]")
{
   auto testApp = initCoreApp();

   QUdpSocket socket;

   REQUIRE(socket.isSequential()     == true);
   REQUIRE(socket.isOpen()           == false);
   REQUIRE(socket.socketType()       == QUdpSocket::UdpSocket);
   REQUIRE(socket.bytesAvailable()   == 0);
   REQUIRE(socket.canReadLine()      == false);
   REQUIRE(socket.readLine()         == QByteArray());
   REQUIRE(socket.socketDescriptor() == (qintptr)-1);
   REQUIRE(socket.error()            == QUdpSocket::UnknownSocketError);
   REQUIRE(socket.errorString()      == QString("Unknown error"));
}
