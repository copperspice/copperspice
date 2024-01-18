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

#include <qtcpserver.h>
#include <qtcpsocket.h>

#include <cs_catch2.h>

TEST_CASE("QTcpServer traits", "[qtcpserver]")
{
   REQUIRE(std::is_copy_constructible_v<QTcpServer> == false);
   REQUIRE(std::is_move_constructible_v<QTcpServer> == false);

   REQUIRE(std::is_copy_assignable_v<QTcpServer> == false);
   REQUIRE(std::is_move_assignable_v<QTcpServer> == false);

   REQUIRE(std::is_nothrow_move_constructible_v<QTcpServer> == false);
   REQUIRE(std::is_nothrow_move_assignable_v<QTcpServer> == false);

   REQUIRE(std::has_virtual_destructor_v<QTcpServer> == true);
}

TEST_CASE("QTcpServer constructor", "[qtcpserver]")
{
   auto testApp = initCoreApp();

   QTcpServer server;

   REQUIRE(server.isListening() == false);
   REQUIRE((int)server.serverPort() == 0);
   REQUIRE(server.serverAddress() == QHostAddress());
   REQUIRE(server.maxPendingConnections() == 30);
   REQUIRE(server.hasPendingConnections() == false);
   REQUIRE(server.socketDescriptor() == -1);
   REQUIRE(server.serverError() == QAbstractSocket::UnknownSocketError);
}

TEST_CASE("QTcpServer clientServerLoop", "[qtcpserver]")
{
   auto testApp = initCoreApp();

   QTcpServer server;

   REQUIRE(server.isListening() == false);
   REQUIRE(server.hasPendingConnections() == false);
   REQUIRE(server.listen(QHostAddress::Any, 11423) == true);
   REQUIRE(server.isListening() == true);

   QTcpSocket client;
   QHostAddress serverAddress = QHostAddress::LocalHost;

   if (! (server.serverAddress() == QHostAddress::Any) && ! (server.serverAddress() == QHostAddress::AnyIPv6) &&
         ! (server.serverAddress() == QHostAddress::AnyIPv4)) {
       serverAddress = server.serverAddress();
   }

   client.connectToHost(serverAddress, server.serverPort());
   REQUIRE(client.waitForConnected(5000));

   REQUIRE(server.waitForNewConnection(5000));
   REQUIRE(server.hasPendingConnections());

   QTcpSocket *socket = server.nextPendingConnection();
   REQUIRE(socket != nullptr);

   REQUIRE(socket->write("Hello client\n", 13) == 13);
   socket->flush();

   REQUIRE(client.waitForReadyRead(5000));

   QByteArray data = client.readAll();
   REQUIRE(data == "Hello client\n");

   REQUIRE(client.write("Nice to see you Server\n", 23) == 23);
   client.flush();

   REQUIRE(socket->waitForReadyRead(5000));

   data = socket->readAll();
   REQUIRE(data == "Nice to see you Server\n");
}

TEST_CASE("QTcpServer dualStack", "[qtcpserver]")
{
   auto testApp = initCoreApp();

   QHostAddress bindAddress;

   printf("Network Dual Stack (test 1 of 3)\n");

   {
      QTcpServer server;
      QTcpSocket v4client;
      QTcpSocket v6client;

      bindAddress = QHostAddress::Any;

      REQUIRE(server.listen(bindAddress));

      v4client.connectToHost(QHostAddress::LocalHost,     server.serverPort());
      v6client.connectToHost(QHostAddress::LocalHostIPv6, server.serverPort());

      REQUIRE(v4client.waitForConnected(3000) == true);
      REQUIRE(v6client.waitForConnected(3000) == true);
   }

   printf("Network Dual Stack (test 2 of 3)\n");

   {
      QTcpServer server;
      QTcpSocket v4client;
      QTcpSocket v6client;

      bindAddress = QHostAddress::AnyIPv4;

      REQUIRE(server.listen(bindAddress));

      v4client.connectToHost(QHostAddress::LocalHost,     server.serverPort());
      v6client.connectToHost(QHostAddress::LocalHostIPv6, server.serverPort());

      REQUIRE(v4client.waitForConnected(3000) == true);
      REQUIRE(v6client.waitForConnected(3000) == false);
   }

   printf("Network Dual Stack (test 3 of 3)\n");

   {
      QTcpServer server;
      QTcpSocket v4client;
      QTcpSocket v6client;

      bindAddress = QHostAddress::AnyIPv6;

      REQUIRE(server.listen(bindAddress));

      v4client.connectToHost(QHostAddress::LocalHost,     server.serverPort());
      v6client.connectToHost(QHostAddress::LocalHostIPv6, server.serverPort());

      REQUIRE(v4client.waitForConnected(3000) == false);
      REQUIRE(v6client.waitForConnected(3000) == true);
   }
}

TEST_CASE("QTcpServer ipv6Server", "[qtcpserver]")
{
   auto testApp = initCoreApp();

   QTcpServer server;

   if (! server.listen(QHostAddress::LocalHostIPv6, 8944)) {
      REQUIRE(server.serverError() == QAbstractSocket::UnsupportedSocketOperationError);

   } else {
      REQUIRE(server.serverPort() == quint16(8944));
      REQUIRE(server.serverAddress() == QHostAddress::LocalHostIPv6);

      QTcpSocket client;
      client.connectToHost("::1", 8944);

      REQUIRE(client.waitForConnected(5000)  == true);
      REQUIRE(server.waitForNewConnection()  == true);
      REQUIRE(server.hasPendingConnections() == true);

      QTcpSocket *serverSocket = nullptr;

      REQUIRE((serverSocket = server.nextPendingConnection()));

      serverSocket->close();
      delete serverSocket;
   }
}

TEST_CASE("QTcpServer ipv6ServerMapped", "[qtcpserver]")
{
   auto testApp = initCoreApp();

   QTcpServer server;

   REQUIRE(server.listen(QHostAddress::LocalHost) == true);

   printf("Network Server v4 Mapped (test 1 of 4)\n");

   {
      QTcpSocket client;
      client.connectToHost("127.0.0.1", server.serverPort());

      REQUIRE(server.waitForNewConnection(3000) == true);
      delete server.nextPendingConnection();
   }

   printf("Network Server v6 Mapped (test 2 of 4)\n");

   {
      QTcpSocket client;
      client.connectToHost("::ffff:127.0.0.1", server.serverPort());

      REQUIRE(server.waitForNewConnection(3000) == true);
      delete server.nextPendingConnection();
   }

   printf("Network Server v6 Mapped (test 3 of 4)\n");

   {
      QTcpSocket client;
      client.connectToHost("::ffff:7F00:0001", server.serverPort());

      REQUIRE(server.waitForNewConnection(5000) == true);
      delete server.nextPendingConnection();
   }

   printf("Network Server v6 Mapped (test 4 of 4)\n");

   {
      QTcpSocket client;
      client.connectToHost("::1", server.serverPort());

      REQUIRE(server.waitForNewConnection(5000) == false);
   }
}

void emptyHandler(QtMsgType, QStringView)
{
}

TEST_CASE("QTcpServer listen", "[qtcpserver]")
{
   QTcpServer server;

   auto oldHandler = csInstallMsgHandler(emptyHandler);

   REQUIRE(server.listen() == true);
   REQUIRE(server.listen() == false);

   csInstallMsgHandler(oldHandler);
}

TEST_CASE("QTcpServer maxPendingConnections", "[qtcpserver]")
{
   auto testApp = initCoreApp();

   QTcpServer server;

   QTcpSocket socket1;
   QTcpSocket socket2;
   QTcpSocket socket3;

   server.setMaxPendingConnections(2);
   REQUIRE(server.listen() == true);

   socket1.connectToHost(QHostAddress::LocalHost, server.serverPort());
   socket2.connectToHost(QHostAddress::LocalHost, server.serverPort());
   socket3.connectToHost(QHostAddress::LocalHost, server.serverPort());

   printf("Network Max Pending Connections\n");

   REQUIRE(server.waitForNewConnection(3000));

   REQUIRE(server.hasPendingConnections()  == true);
   REQUIRE(server.nextPendingConnection()  != nullptr);

   REQUIRE(server.hasPendingConnections()  == true);
   REQUIRE(server.nextPendingConnection()  != nullptr);

   REQUIRE(server.hasPendingConnections()  == false);
   REQUIRE(server.nextPendingConnection()  == nullptr);

   REQUIRE(server.waitForNewConnection(3000) == true);

   REQUIRE(server.hasPendingConnections() == true);
   REQUIRE(server.nextPendingConnection() != nullptr);
}

TEST_CASE("QTcpServer setMax", "[qtcpserver]")
{
   auto testApp = initCoreApp();

   QTcpServer server;

   server.setMaxPendingConnections(0);
   REQUIRE(server.maxPendingConnections() == 0);

   server.setMaxPendingConnections(INT_MIN);
   REQUIRE(server.maxPendingConnections() == INT_MIN);

   server.setMaxPendingConnections(INT_MAX);
   REQUIRE(server.maxPendingConnections() == INT_MAX);
}
