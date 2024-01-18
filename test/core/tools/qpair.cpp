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

#include <qbuffer.h>
#include <qpair.h>

#include <cs_catch2.h>

TEST_CASE("QPair traits", "[qpair]")
{
   REQUIRE(std::is_copy_constructible_v<QPair<int, int>> == true);
   REQUIRE(std::is_move_constructible_v<QPair<int, int>> == true);

   REQUIRE(std::is_copy_assignable_v<QPair<int, int>> == true);
   REQUIRE(std::is_move_assignable_v<QPair<int, int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QPair<int, int>> == false);
}

TEST_CASE("QPair construction", "[qpair]")
{
   QPair<QString, QString> pair;

   REQUIRE(pair.first == "");
   REQUIRE(pair.second == "");
}

TEST_CASE("QPair stream", "[qpair]")
{
   QPair<int, int> pair1 = {10, 20};
   QPair<int, int> pair2;

   QByteArray buffer;

   QDataStream out(&buffer, QIODevice::WriteOnly);
   out << pair1;

   QDataStream in(&buffer, QIODevice::ReadOnly);
   in >> pair2;

   REQUIRE(pair2.first == 10);
   REQUIRE(pair2.second == 20);
}

TEST_CASE("QPair copy", "[qpair]")
{
   QPair<QString, int> pair1 = {"answer", 42};
   QPair<QString, int> pair2(pair1);

   REQUIRE(pair2.first == "answer");
   REQUIRE(pair2.second == 42);
}

TEST_CASE("QPair move", "[qpair]")
{
   QPair<QString, int> pair1 = {"answer", 42};
   QPair<QString, int> pair2(std::move(pair1));

   REQUIRE(pair2.first == "answer");
   REQUIRE(pair2.second == 42);
}

TEST_CASE("QPair assign", "[qpair]")
{
   QPair<QString, int> pair1 = {"answer", 42};
   QPair<QString, int> pair2;

   pair2 = pair1;

   REQUIRE(pair2.first == "answer");
   REQUIRE(pair2.second == 42);
}

TEST_CASE("QPair move assign", "[qpair]")
{
   QPair<QString, int> pair1 = {"answer", 42};

   QPair<QString, int> pair2;
   pair2 = std::move(pair1);

   REQUIRE(pair2.first == "answer");
   REQUIRE(pair2.second == 42);
}

TEST_CASE("QPair qMakePair", "[qpair]")
{
   QPair<int, int> pair1 = qMakePair(1, 42);
   QPair<int, int> pair2 = pair1;

   REQUIRE(pair2.first == 1);
   REQUIRE(pair2.second == 42);
}

TEST_CASE("QPair qMakePair odd", "[qpair]")
{
   QPair<int, int> pair1 = qMakePair(65, 42);
   QPair<QString, int> pair2(pair1);

   REQUIRE(pair2.first.at(0) == 'A');
   REQUIRE(pair2.second == 42);
}
