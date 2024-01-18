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

#include <qbytearray.h>

#include <cs_catch2.h>

TEST_CASE("QByteArray traits", "[qbytearray]")
{
   REQUIRE(std::is_copy_constructible_v<QByteArray> == true);
   REQUIRE(std::is_move_constructible_v<QByteArray> == true);

   REQUIRE(std::is_copy_assignable_v<QByteArray> == true);
   REQUIRE(std::is_move_assignable_v<QByteArray> == true);

   REQUIRE(std::has_virtual_destructor_v<QByteArray> == false);
}

TEST_CASE("QByteArray append", "[qbytearray]")
{
   QByteArray str = "A wacky fox and sizeable pig";

   str.append(" went to lunch");

   REQUIRE(str == "A wacky fox and sizeable pig went to lunch");
}

TEST_CASE("QByteArray clear", "[qbytearray]")
{
   QByteArray str = "A wacky fox and sizeable pig jumped halfway over a blue moon.";

   str.clear();

   REQUIRE(str.length() == 0);
}

TEST_CASE("QByteArray compress", "[qbytearray]")
{
   QByteArray str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   QByteArray data   = qCompress(str);
   QByteArray output = qUncompress(data);

   REQUIRE(str == output);
}

TEST_CASE("QByteArray contains", "[qbytearray]")
{
   QByteArray str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   REQUIRE(str.contains("jumped"));
   REQUIRE(! str.contains("lunch"));
}

TEST_CASE("QByteArray empty", "[qbytearray]")
{
   QByteArray str;

   REQUIRE(str.isEmpty());
}

TEST_CASE("QByteArray length", "[qbytearray]")
{
   QByteArray str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   REQUIRE(str.length() == 60);
}

TEST_CASE("QByteArray replace", "[qbytearray]")
{
   QByteArray str = "A wacky fox went to lunch";

   str.replace(12, 13, "took a nap");

   REQUIRE(str == "A wacky fox took a nap");
}

TEST_CASE("QByteArray truncate", "[qbytearray]")
{
   QByteArray str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   str.truncate(20);

   REQUIRE(str == "A wacky fox and size");
}

TEST_CASE("QByteArray to_something", "[qbytearray]")
{
   QByteArray str = "Mango";

   str = str.toUpper();
   REQUIRE(str == "MANGO");

   str = str.toLower();
   REQUIRE(str == "mango");
}

TEST_CASE("QByteArray trim", "[qbytearray]")
{
   QByteArray str = "    A wacky fox and sizeable pig \n\t ";

   str = str.trimmed();

   REQUIRE(str == "A wacky fox and sizeable pig");
}

