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

#include <qatomicint.h>

#include <cs_catch2.h>

TEST_CASE("QAtomicInt traits", "[qatomicint]")
{
   REQUIRE(std::is_copy_constructible_v<QAtomicInt> == true);
   REQUIRE(std::is_move_constructible_v<QAtomicInt> == true);

   REQUIRE(std::is_copy_assignable_v<QAtomicInt> == true);
   REQUIRE(std::is_move_assignable_v<QAtomicInt> == true);

   REQUIRE(std::has_virtual_destructor_v<QAtomicInt> == false);
}

TEST_CASE("QAtomicInt assignment", "[qatomic_int]")
{
   QAtomicInt atomic1(17);
   atomic1 = 42;

   QAtomicInt atomic2(17);
   atomic2 = atomic1;

   REQUIRE(atomic1.load() == 42);    // emerald, should be load("relaxed")
   REQUIRE(atomic2.load() == 42);
}

TEST_CASE("QAtomicInt constructor", "[qatomic_int]")
{
   QAtomicInt atomic1(17);
   QAtomicInt atomic2 = 17;
   QAtomicInt atomic3 = atomic2;

   REQUIRE(atomic1.load() == 17);
   REQUIRE(atomic2.load() == 17);
   REQUIRE(atomic3.load() == 17);
}

TEST_CASE("QAtomicInt ref", "[qatomic_int]")
{
   QAtomicInt atomic = -2;

   REQUIRE(atomic.ref() == true);
   REQUIRE(atomic.load() == -1);

   REQUIRE(atomic.ref() == false);
   REQUIRE(atomic.load() == 0);

   REQUIRE(atomic.ref() == true);
   REQUIRE(atomic.load() == 1);
}

TEST_CASE("QAtomicInt deref", "[qatomic_int]")
{
   QAtomicInt atomic = 2;

   REQUIRE(atomic.deref() == true);
   REQUIRE(atomic.load() == 1);

   REQUIRE(atomic.deref() == false);
   REQUIRE(atomic.load() == 0);

   REQUIRE(atomic.deref() == true);
   REQUIRE(atomic.load() == -1);
}
