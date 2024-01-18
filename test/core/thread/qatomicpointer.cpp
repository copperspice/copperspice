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

#include <qatomicpointer.h>

#include <cs_catch2.h>

TEST_CASE("QAtomicPointer traits", "[qatomicpointer]")
{
   REQUIRE(std::is_copy_constructible_v<QAtomicPointer<int>> == true);
   REQUIRE(std::is_move_constructible_v<QAtomicPointer<int>> == true);

   REQUIRE(std::is_copy_assignable_v<QAtomicPointer<int>> == true);
   REQUIRE(std::is_move_assignable_v<QAtomicPointer<int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QAtomicPointer<int>> == false);
}

TEST_CASE("QAtomicPointer assignment", "[qatomic_pointer]")
{
   QAtomicPointer<int> atomic1(new int(17));
   QAtomicPointer<int> atomic2 = new int(17);
   QAtomicPointer<int> atomic3;

   atomic3 = atomic2;

   REQUIRE(*atomic1.load() == 17);
   REQUIRE(*atomic2.load() == 17);
   REQUIRE(*atomic3.load() == 17);

   delete atomic1.load();
   delete atomic2.load();
}

TEST_CASE("QAtomicPointer load", "[qatomic_pointer]")
{
   QAtomicPointer<int> atomic1(new int(17));
   QAtomicPointer<int> atomic2 = new int(17);
   QAtomicPointer<int> atomic3 = atomic2;

   REQUIRE(*atomic1.load() == 17);
   REQUIRE(*atomic2.load() == 17);
   REQUIRE(*atomic3.load() == 17);

   delete atomic1.load();
   delete atomic2.load();
}

TEST_CASE("QAtomicPointer operators", "[qatomic_pointer]")
{
   int array[3] = {5, 10, 15};

   QAtomicPointer<int> atomic = array;

   REQUIRE(*atomic.load() == 5);

   SECTION ("add") {
      int *tmp1 = ++atomic;
      REQUIRE(*atomic.load() == 10);
      REQUIRE(*tmp1 == 10);

      int *tmp2 = atomic++;
      REQUIRE(*atomic.load() == 15);
      REQUIRE(*tmp2 == 10);
   }

   SECTION ("subtract") {
      ++atomic;
      ++atomic;

      int *tmp1 = --atomic;
      REQUIRE(*atomic.load() == 10);
      REQUIRE(*tmp1 == 10);

      int *tmp2 = atomic--;
      REQUIRE(*atomic.load() == 5);
      REQUIRE(*tmp2 == 10);
   }

}


