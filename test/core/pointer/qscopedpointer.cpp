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

#include <qobject.h>
#include <qscopedpointer.h>

#include <cs_catch2.h>

TEST_CASE("QScopedPointer traits", "[qscopedpointer]")
{
   REQUIRE(std::is_copy_constructible_v<QScopedPointer<int>> == false);
   REQUIRE(std::is_move_constructible_v<QScopedPointer<int>> == false);

   REQUIRE(std::is_copy_assignable_v<QScopedPointer<int>> == false);
   REQUIRE(std::is_move_assignable_v<QScopedPointer<int>> == false);

   REQUIRE(std::has_virtual_destructor_v<QScopedPointer<int>> == false);
}

TEST_CASE("QScopedPointer custom_deleter", "[qscopedpointer]")
{
   bool deleterExecuted = false;

   {
      auto customDeleter = [&deleterExecuted] (int *obj) {
         deleterExecuted = true;
         delete obj;
      };

      QScopedPointer<int, decltype(customDeleter)> ptr(new int, customDeleter);
      REQUIRE(deleterExecuted == false);
   }

   REQUIRE(deleterExecuted == true);
}

TEST_CASE("QScopedPointer empty", "[qscopedpointer]")
{
   QScopedPointer<int> ptr;

   REQUIRE(ptr == nullptr);
   REQUIRE(nullptr == ptr);
   REQUIRE(ptr == ptr);

   REQUIRE(! (ptr != nullptr));
   REQUIRE(! (nullptr != ptr));
   REQUIRE(! (ptr != ptr)) ;

   REQUIRE(ptr.isNull() == true);
}

TEST_CASE("QScopedPointer equality", "[qscopedpointer]")
{
   QScopedPointer<int> ptr1 = QMakeScoped<int>();
   QScopedPointer<int> ptr2(nullptr);

   REQUIRE((ptr1 == ptr2) == false);
   REQUIRE((ptr1 != ptr2) == true);

   REQUIRE((ptr1 == ptr2.get()) == false);
   REQUIRE((ptr1.get() == ptr2) == false);

   REQUIRE((ptr1 != ptr2.get()) == true);
   REQUIRE((ptr1.get() != ptr2) == true);
}

TEST_CASE("QScopedPointer release", "[qscopedpointer]")
{
   QScopedPointer<int> ptr = QMakeScoped<int>();
   int *p1 = ptr.get();
   int *p2 = ptr.release();

   REQUIRE(p1 == p2);
   REQUIRE(ptr == nullptr);

   delete p2;

   REQUIRE(ptr.release() == nullptr);
}

TEST_CASE("QScopedPointer reset", "[qscopedpointer]")
{
   QScopedPointer<int> ptr = QMakeScoped<int>();
   int *rawPtr = ptr.get();

   ptr.reset(rawPtr);
   REQUIRE(ptr == rawPtr);
   ptr.reset();

   REQUIRE(ptr == nullptr);
   REQUIRE(ptr.isNull() == true);
}

TEST_CASE("QScopedPointer swap", "[qscopedpointer]")
{
   QScopedPointer<int> ptr1 = QMakeUnique<int>(8);
   QScopedPointer<int> ptr2 = QMakeUnique<int>(17);

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

TEST_CASE("QScopedPointer take", "[qscopedpointer]")
{
   QScopedPointer<int> ptr1 = QMakeScoped<int>(42);
   QScopedPointer<int> ptr2(ptr1.take());

   REQUIRE(ptr1 == nullptr);
   REQUIRE(*ptr2 == 42);
}
