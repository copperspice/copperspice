/***********************************************************************
*
* Copyright (c) 2012-2022 Barbara Geller
* Copyright (c) 2012-2022 Ansel Sermersheim
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

#include <qhostaddress.h>

#include <cs_catch2.h>

TEST_CASE("QHostAddress traits", "[qhostaddress]")
{
   REQUIRE(std::is_copy_constructible_v<QHostAddress> == true);
   REQUIRE(std::is_move_constructible_v<QHostAddress> == true);

   REQUIRE(std::is_copy_assignable_v<QHostAddress> == true);
   REQUIRE(std::is_move_assignable_v<QHostAddress> == true);

   REQUIRE(std::is_nothrow_move_constructible_v<QHostAddress> == false);
   REQUIRE(std::is_nothrow_move_assignable_v<QHostAddress> == false);

   REQUIRE(std::has_virtual_destructor_v<QHostAddress> == false);
}

TEST_CASE("QHostAddress assignment", "[qhostaddress]")
{
   auto testApp = initCoreApp();

   QHostAddress hostAddr;

   // ipv4
   {
      hostAddr = "0.0.0.0";
      REQUIRE(hostAddr == QHostAddress("0.0.0.0"));
   }

   {
      hostAddr = "127.0.0.1";
      REQUIRE(hostAddr == QHostAddress("127.0.0.1"));
   }

   {
      hostAddr = "123.0.0";
      REQUIRE(hostAddr == QHostAddress("123.0.0.0"));
   }

   {
      hostAddr = "-1.9.9.9";
      REQUIRE(hostAddr == QHostAddress(""));
   }

   {
      hostAddr = "255.4 3.2.1";
      REQUIRE(hostAddr == QHostAddress(""));
   }

   {
      hostAddr = "256.8.8.8";
      REQUIRE(hostAddr == QHostAddress(""));
   }

   // ipv6
   {
      hostAddr = "0:0:0:0:0:0:0:0";
      REQUIRE(hostAddr == QHostAddress("0:0:0:0:0:0:0:0"));
   }

   {
      hostAddr = "1080::8:800:200C:417A";
      REQUIRE(hostAddr == QHostAddress("1080:0:0:0:8:800:200C:417A"));
   }

   {
      hostAddr = "0:0:0:0:0:FFFF:129.144.52.38";
      REQUIRE(hostAddr == QHostAddress("0:0:0:0:0:FFFF:129.144.52.38"));
   }

   {
      hostAddr = "::FFFF:129.144.52.38";
      REQUIRE(hostAddr == QHostAddress("0:0:0:0:0:FFFF:129.144.52.38"));
   }

   {
      hostAddr = "A:B::D:E";
      REQUIRE(hostAddr == QHostAddress("a:b::d:e"));
      REQUIRE(hostAddr == QHostAddress("a:b:0:0:0:0:d:e"));
   }
}


TEST_CASE("QHostAddress protocol", "[qhostaddress]")
{
   auto testApp = initCoreApp();

   QHostAddress hostAddr;

   // ipv4
   {
      hostAddr = "0.0.0.0";
      REQUIRE(hostAddr.protocol() == QAbstractSocket::IPv4Protocol);
      REQUIRE(hostAddr.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol);
      REQUIRE(hostAddr.protocol() != QAbstractSocket::IPv6Protocol);
   }


   {
      hostAddr = "127.0.0.1";
      REQUIRE(hostAddr.protocol() == QAbstractSocket::IPv4Protocol);
      REQUIRE(hostAddr.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol);
      REQUIRE(hostAddr.protocol() != QAbstractSocket::IPv6Protocol);
   }

   {
      hostAddr = "123.0.0";
      REQUIRE(hostAddr.protocol() == QAbstractSocket::IPv4Protocol);
      REQUIRE(hostAddr.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol);
      REQUIRE(hostAddr.protocol() != QAbstractSocket::IPv6Protocol);
   }

   {
      hostAddr = "-1.9.9.9";
      REQUIRE(hostAddr.protocol() != QAbstractSocket::IPv4Protocol);
      REQUIRE(hostAddr.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol);
      REQUIRE(hostAddr.protocol() != QAbstractSocket::IPv6Protocol);
   }

   {
      hostAddr = "255.4 3.2.1";
      REQUIRE(hostAddr.protocol() != QAbstractSocket::IPv4Protocol);
      REQUIRE(hostAddr.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol);
      REQUIRE(hostAddr.protocol() != QAbstractSocket::IPv6Protocol);
   }

   {
      hostAddr = "256.8.8.8";
      REQUIRE(hostAddr.protocol() != QAbstractSocket::IPv4Protocol);
      REQUIRE(hostAddr.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol);
      REQUIRE(hostAddr.protocol() != QAbstractSocket::IPv6Protocol);
   }

   // ipv6
   {
      hostAddr = "0:0:0:0:0:0:0:0";
      REQUIRE(hostAddr.protocol() != QAbstractSocket::IPv4Protocol);
      REQUIRE(hostAddr.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol);
      REQUIRE(hostAddr.protocol() == QAbstractSocket::IPv6Protocol);
   }

   {
      hostAddr = "1080::8:800:200C:417A";
      REQUIRE(hostAddr.protocol() != QAbstractSocket::IPv4Protocol);
      REQUIRE(hostAddr.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol);
      REQUIRE(hostAddr.protocol() == QAbstractSocket::IPv6Protocol);
   }

   {
      hostAddr = "0:0:0:0:0:FFFF:129.144.52.38";
      REQUIRE(hostAddr.protocol() != QAbstractSocket::IPv4Protocol);
      REQUIRE(hostAddr.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol);
      REQUIRE(hostAddr.protocol() == QAbstractSocket::IPv6Protocol);
   }

   {
      hostAddr = "::FFFF:129.144.52.38";
      REQUIRE(hostAddr.protocol() != QAbstractSocket::IPv4Protocol);
      REQUIRE(hostAddr.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol);
      REQUIRE(hostAddr.protocol() == QAbstractSocket::IPv6Protocol);
   }

   {
      hostAddr = "A:B::D:E";
      REQUIRE(hostAddr.protocol() != QAbstractSocket::IPv4Protocol);
      REQUIRE(hostAddr.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol);
      REQUIRE(hostAddr.protocol() == QAbstractSocket::IPv6Protocol);
   }
}

TEST_CASE("QHostAddress setAddress", "[qhostaddress]")
{
   auto testApp = initCoreApp();

   // ipv4
   {
      QString address = "0.0.0.0";

      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.setAddress(address) == true);
   }

   {
      QString address = "127.0.0.1";

      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.setAddress(address) == true);
   }

   {
      QString address = "123.0.0";

      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.setAddress(address) == true);
   }

   {
      QString address = "-1.9.9.9";

      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.setAddress(address) == false);
   }

   {
      QString address = "255.4 3.2.1";

      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.setAddress(address) == false);
   }

   {
      QString address = "256.8.8.8";

      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.setAddress(address) == false);
   }

   // ipv6
   {
      QString address = "0:0:0:0:0:0:0:0";
      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.setAddress(address) == true);
   }

   {
      QString address = "1080::8:800:200C:417A";
      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.setAddress(address) == true);
   }

   {
      QString address = "0:0:0:0:0:FFFF:129.144.52.38";
      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.setAddress(address) == true);
   }

   {
      QString address = "::FFFF:129.144.52.38";
      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.setAddress(address) == true);
   }

   {
      QString address = "A:B::D:E";
      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.setAddress(address) == true);
   }
}

TEST_CASE("QHostAddress toIPv4Address", "[qhostaddress]")
{
   auto testApp = initCoreApp();

   {
      QString address = "0.0.0.0";

      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.toString() == address);
      REQUIRE(hostAddr.toIPv4Address() == 0);
   }

   {
      QString address = "127.0.0.1";

      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.toString() == address);
      REQUIRE(hostAddr.toIPv4Address() == 2130706433);
   }

   {
      QString address = "123.0.0";

      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.toString() == "123.0.0.0");
      REQUIRE(hostAddr.toIPv4Address() == 2063597568);
   }

   {
      QString address = "-1.9.9.9";

      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.toString() == "");
      REQUIRE(hostAddr.toIPv4Address() == 0);
   }

   {
      QString address = "255.4 3.2.1";

      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.toString() == "");
      REQUIRE(hostAddr.toIPv4Address() == 0);
   }

   {
      QString address = "256.8.8.8";

      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.toString() == "");
      REQUIRE(hostAddr.toIPv4Address() == 0);
   }
}

TEST_CASE("QHostAddress toIPv6Address", "[qhostaddress]")
{
   auto testApp = initCoreApp();

   {
      QString address = "0:0:0:0:0:0:0:0";

      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.toString() == "::");

      Q_IPV6ADDR addr = hostAddr.toIPv6Address();

      REQUIRE(addr[0]  == 0x00);
      REQUIRE(addr[1]  == 0x00);
      REQUIRE(addr[2]  == 0x00);
      REQUIRE(addr[3]  == 0x00);

      REQUIRE(addr[4]  == 0x00);
      REQUIRE(addr[5]  == 0x00);
      REQUIRE(addr[6]  == 0x00);
      REQUIRE(addr[7]  == 0x00);

      REQUIRE(addr[8]  == 0x00);
      REQUIRE(addr[9]  == 0x00);
      REQUIRE(addr[10] == 0x00);
      REQUIRE(addr[11] == 0x00);

      REQUIRE(addr[12] == 0x00);
      REQUIRE(addr[13] == 0x00);
      REQUIRE(addr[14] == 0x00);
      REQUIRE(addr[15] == 0x00);
   }

   {
      QString address = "1080::8:800:200C:417A";

      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.toString() == "1080::8:800:200c:417a");

      Q_IPV6ADDR addr = hostAddr.toIPv6Address();

      REQUIRE(addr[0]  == 0x10);
      REQUIRE(addr[1]  == 0x80);
      REQUIRE(addr[2]  == 0x00);
      REQUIRE(addr[3]  == 0x00);

      REQUIRE(addr[4]  == 0x00);
      REQUIRE(addr[5]  == 0x00);
      REQUIRE(addr[6]  == 0x00);
      REQUIRE(addr[7]  == 0x00);

      REQUIRE(addr[8]  == 0x00);
      REQUIRE(addr[9]  == 0x08);
      REQUIRE(addr[10] == 0x08);
      REQUIRE(addr[11] == 0x00);

      REQUIRE(addr[12] == 0x20);
      REQUIRE(addr[13] == 0x0C);
      REQUIRE(addr[14] == 0x41);
      REQUIRE(addr[15] == 0x7A);
   }

   {
      QString address = "0:0:0:0:0:FFFF:129.144.52.38";

      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.toString() == "::ffff:129.144.52.38");

      Q_IPV6ADDR addr = hostAddr.toIPv6Address();

      REQUIRE(addr[0]  == 0x00);
      REQUIRE(addr[1]  == 0x00);
      REQUIRE(addr[2]  == 0x00);
      REQUIRE(addr[3]  == 0x00);

      REQUIRE(addr[4]  == 0x00);
      REQUIRE(addr[5]  == 0x00);
      REQUIRE(addr[6]  == 0x00);
      REQUIRE(addr[7]  == 0x00);

      REQUIRE(addr[8]  == 0x00);
      REQUIRE(addr[9]  == 0x00);
      REQUIRE(addr[10] == 0xFF);
      REQUIRE(addr[11] == 0xFF);

      REQUIRE(addr[12] == 129);
      REQUIRE(addr[13] == 144);
      REQUIRE(addr[14] == 52);
      REQUIRE(addr[15] == 38);
   }

   {
      QString address = "::FFFF:129.144.52.38";

      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.toString() == "::ffff:129.144.52.38");

      Q_IPV6ADDR addr = hostAddr.toIPv6Address();

      REQUIRE(addr[0]  == 0x00);
      REQUIRE(addr[1]  == 0x00);
      REQUIRE(addr[2]  == 0x00);
      REQUIRE(addr[3]  == 0x00);

      REQUIRE(addr[4]  == 0x00);
      REQUIRE(addr[5]  == 0x00);
      REQUIRE(addr[6]  == 0x00);
      REQUIRE(addr[7]  == 0x00);

      REQUIRE(addr[8]  == 0x00);
      REQUIRE(addr[9]  == 0x00);
      REQUIRE(addr[10] == 0xFF);
      REQUIRE(addr[11] == 0xFF);

      REQUIRE(addr[12] == 129);
      REQUIRE(addr[13] == 144);
      REQUIRE(addr[14] == 52);
      REQUIRE(addr[15] == 38);
   }

   {
      QString address = "A:B::D:E";

      QHostAddress hostAddr(address);
      REQUIRE(hostAddr.toString() == "a:b::d:e");

      Q_IPV6ADDR addr = hostAddr.toIPv6Address();

      REQUIRE(addr[0]  == 0x00);
      REQUIRE(addr[1]  == 0x0a);
      REQUIRE(addr[2]  == 0x00);
      REQUIRE(addr[3]  == 0x0b);

      REQUIRE(addr[4]  == 0x00);
      REQUIRE(addr[5]  == 0x00);
      REQUIRE(addr[6]  == 0x00);
      REQUIRE(addr[7]  == 0x00);

      REQUIRE(addr[8]  == 0x00);
      REQUIRE(addr[9]  == 0x00);
      REQUIRE(addr[10] == 0x00);
      REQUIRE(addr[11] == 0x00);

      REQUIRE(addr[12] == 0x00);
      REQUIRE(addr[13] == 0x0d);
      REQUIRE(addr[14] == 0x00);
      REQUIRE(addr[15] == 0x0e);
   }
}
