/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qobject.h>
#include <qsharedpointer.h>

#include <cs_catch2.h>

TEST_CASE("QSharedArrayPointer traits", "[qsharedarraypointer]")
{
   // without brackets
   REQUIRE(std::is_copy_constructible_v<QSharedArrayPointer<int>> == true);
   REQUIRE(std::is_move_constructible_v<QSharedArrayPointer<int>> == true);

   REQUIRE(std::is_copy_assignable_v<QSharedArrayPointer<int>> == true);
   REQUIRE(std::is_move_assignable_v<QSharedArrayPointer<int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QSharedArrayPointer<int>> == false);

   // with brackets
   REQUIRE(std::is_copy_constructible_v<QSharedArrayPointer<int[]>> == true);
   REQUIRE(std::is_move_constructible_v<QSharedArrayPointer<int[]>> == true);

   REQUIRE(std::is_copy_assignable_v<QSharedArrayPointer<int[]>> == true);
   REQUIRE(std::is_move_assignable_v<QSharedArrayPointer<int[]>> == true);

   REQUIRE(std::has_virtual_destructor_v<QSharedArrayPointer<int[]>> == false);
}

TEST_CASE("QSharedArrayPointer copy_assign", "[qsharedarraypointer]")
{
   QSharedArrayPointer<int[]> ptr1 = QMakeShared<int[]>(1);
   QSharedArrayPointer<int>   ptr2(ptr1);

   ptr1[0] = 8;
   ptr2[0] = 17;

   REQUIRE(ptr1.is_null() == false);
   REQUIRE(ptr2.is_null() == false);

   REQUIRE(*ptr1 == 17);
   REQUIRE(*ptr2 == 17);

   //
   QSharedArrayPointer<int[]> ptr3;
   ptr3 = ptr2;

   REQUIRE(ptr2.is_null() == false);
   REQUIRE(ptr3.is_null() == false);

   REQUIRE(*ptr2 == 17);
   REQUIRE(*ptr3 == 17);
}

TEST_CASE("QSharedArrayPointer empty", "[qsharedarraypointer]")
{
   QSharedArrayPointer<int[]> ptr;

   REQUIRE(ptr == nullptr);
   REQUIRE(nullptr == ptr);
   REQUIRE(ptr == ptr);

   REQUIRE(! (ptr != nullptr));
   REQUIRE(! (nullptr != ptr));
   REQUIRE(! (ptr != ptr)) ;

   REQUIRE(ptr.isNull() == true);
}

TEST_CASE("QSharedArrayPointer move_assign", "[qsharedarraypointer]")
{
   QSharedArrayPointer<int[]> ptr1 = QMakeShared<int[]>(1);
   QSharedArrayPointer<int>   ptr2(std::move(ptr1));

   ptr2[0] = 17;

   REQUIRE(ptr1.is_null() == true);
   REQUIRE(ptr2.is_null() == false);

   REQUIRE(*ptr2 == 17);

   //
   QSharedArrayPointer<int[]> ptr3;
   ptr3 = std::move(ptr2);

   REQUIRE(ptr2.is_null() == true);
   REQUIRE(ptr3.is_null() == false);

   REQUIRE(*ptr3 == 17);
}

TEST_CASE("QSharedArrayPointer reset", "[qsharedarraypointer]")
{
   QSharedArrayPointer<int[]> ptr = QMakeShared<int[]>(1);
   ptr.reset();

   REQUIRE(ptr == nullptr);
   REQUIRE(ptr.isNull() == true);
}

TEST_CASE("QSharedArrayPointer swap", "[qsharedarraypointer]")
{
   QSharedArrayPointer<int[]> ptr1 = QMakeShared<int[]>(1);
   QSharedArrayPointer<int[]> ptr2 = QMakeShared<int[]>(1);

   ptr1[0] = 8;
   ptr2[0] = 17;

   REQUIRE(*ptr1 == 8);
   REQUIRE(*ptr2 == 17);

   ptr1.swap(ptr2);

   REQUIRE(*ptr1 == 17);
   REQUIRE(*ptr2 == 8);

   ptr1.reset();
   ptr1.swap(ptr2);

   REQUIRE(*ptr1 == 8);
   REQUIRE(ptr2 == nullptr);

   ptr1.swap(ptr1);

   REQUIRE(*ptr1 == 8);
   REQUIRE(ptr2 == nullptr);

   ptr2.swap(ptr2);

   REQUIRE(*ptr1 == 8);
   REQUIRE(ptr2 == nullptr);
}
