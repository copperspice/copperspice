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

#include <qsslsocket.h>
#include <qtcpserver.h>
#include <qtcpsocket.h>

#include <cs_catch2.h>

TEST_CASE("QTcpSocket traits", "[qtcpsocket]")
{
   REQUIRE(std::is_copy_constructible_v<QTcpSocket> == false);
   REQUIRE(std::is_move_constructible_v<QTcpSocket> == false);

   REQUIRE(std::is_copy_assignable_v<QTcpSocket> == false);
   REQUIRE(std::is_move_assignable_v<QTcpSocket> == false);

   REQUIRE(std::is_nothrow_move_constructible_v<QTcpSocket> == false);
   REQUIRE(std::is_nothrow_move_assignable_v<QTcpSocket> == false);

   REQUIRE(std::has_virtual_destructor_v<QTcpSocket> == true);
}

static QTcpSocket * newSocket()
{
   QTcpSocket *socket;

#ifdef QT_SSL
   // path to openSSL library required
   socket = new QSslSocket;
#else
   socket = new QTcpSocket;
#endif

   return socket;
}

TEST_CASE("QTcpSocket constructor", "[qtcpsocket]")
{
   auto testApp = initCoreApp();

   QTcpSocket *socket = newSocket();

   REQUIRE(socket->state()        == QTcpSocket::UnconnectedState);
   REQUIRE(socket->isSequential() == true);
   REQUIRE(socket->isOpen()       == false);
   REQUIRE(socket->isValid()      == false);
   REQUIRE(socket->socketType()   == QTcpSocket::TcpSocket);

   char c;

   REQUIRE(socket->getChar(&c)        == false);
   REQUIRE(socket->bytesAvailable()   == 0);
   REQUIRE(socket->canReadLine()      == false);
   REQUIRE(socket->readLine()         == QByteArray());
   REQUIRE(socket->socketDescriptor() == (qintptr)-1);
   REQUIRE(socket->localPort()        == 0);
   REQUIRE(socket->localAddress()     == QHostAddress());
   REQUIRE(socket->peerPort()         == 0);
   REQUIRE(socket->peerAddress()      == QHostAddress());
   REQUIRE(socket->error()            == QTcpSocket::UnknownSocketError);
   REQUIRE(socket->errorString()      == QString("Unknown error"));

   delete socket;
}

TEST_CASE("QTcpSocket bindThenResolveHost", "[qtcpsocket]")
{
   auto testApp = initCoreApp();

   QTcpServer server;

   QHostAddress serverAddress = QHostAddress::LocalHost;
   const quint16 port = 11423;

   REQUIRE(server.listen(QHostAddress::Any, port) == true);
   REQUIRE(server.isListening() == true);

   // part one
   QTcpSocket *socket = newSocket();

   REQUIRE(socket->bind(QHostAddress(QHostAddress::AnyIPv4), 0) == true);
   REQUIRE(socket->state() == QAbstractSocket::BoundState);

   quint16 boundPort = socket->localPort();
   qintptr fd = socket->socketDescriptor();

   REQUIRE(fd != -1);

   // part two
   socket->connectToHost(serverAddress, port);

   REQUIRE((socket->state() == QAbstractSocket::HostLookupState
         || socket->state() == QAbstractSocket::ConnectingState));

   REQUIRE(socket->waitForConnected(3000) == true);

   REQUIRE(socket->state() == QAbstractSocket::ConnectedState);

   REQUIRE(socket->localPort() == boundPort);
   REQUIRE(socket->socketDescriptor() == fd);

   delete socket;
}

TEST_CASE("QTcpSocket descripter", "[qtcpsocket]")
{
   auto testApp = initCoreApp();

   QTcpSocket *socket = newSocket();

   REQUIRE(socket->socketDescriptor() == (qintptr)-1);

   REQUIRE(socket->setSocketDescriptor(-5, QTcpSocket::UnconnectedState) == false);
   REQUIRE(socket->socketDescriptor() == (qintptr)-1);

   REQUIRE(socket->error() == QTcpSocket::UnsupportedSocketOperationError);

   delete socket;
}

TEST_CASE("QTcpSocket connectToHost", "[qtcpsocket]")
{
   auto testApp = initCoreApp();

   QTcpSocket *socket = newSocket();
   socket->connectToHost("nosuchserver.copperspice.com", 80);

   REQUIRE(socket->waitForConnected() == false);
   REQUIRE(socket->state() == QTcpSocket::UnconnectedState);
   REQUIRE(socket->error() == QTcpSocket::HostNotFoundError);

   delete socket;
}
