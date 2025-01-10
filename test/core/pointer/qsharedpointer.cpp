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

#include <qsharedpointer.h>
#include <qfile.h>

#include <cs_catch2.h>

TEST_CASE("QSharedPointer traits", "[qsharedpointer]")
{
   REQUIRE(std::is_copy_constructible_v<QSharedPointer<int>> == true);
   REQUIRE(std::is_move_constructible_v<QSharedPointer<int>> == true);

   REQUIRE(std::is_copy_assignable_v<QSharedPointer<int>> == true);
   REQUIRE(std::is_move_assignable_v<QSharedPointer<int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QSharedPointer<int>> == false);
}

TEST_CASE("QSharedPointer cast", "[qsharedpointer]")
{
   class Fruit
   {
   };

   class Apple : public Fruit
   {
   };

   CsPointer::CsSharedPointer<Fruit> ptr1;
   CsPointer::CsSharedPointer<Apple> ptr2 = CsPointer::make_shared<Apple>();

   {
      ptr1 = CsPointer::static_pointer_cast<Fruit>(ptr2);

      REQUIRE(ptr1 == ptr2);
   }

   {
      ptr1 = CsPointer::dynamic_pointer_cast<Fruit>(ptr2);

      REQUIRE(ptr1 == ptr2);
   }

   {
      CsPointer::CsSharedPointer<const Apple> ptr3 = ptr2;
      CsPointer::CsSharedPointer<Apple> ptr4;

      ptr4 = CsPointer::const_pointer_cast<Apple>(ptr3);

      REQUIRE(ptr3 == ptr4);
   }
}

TEST_CASE("QSharedPointer convert_a", "[qsharedpointer]")
{
   QSharedPointer<int> ptr1 = QMakeShared<int>(42);

   std::shared_ptr<int> ptr2 = std::move(ptr1);

   REQUIRE(ptr1 == nullptr);
   REQUIRE(ptr2 != nullptr);

   REQUIRE(*ptr2 == 42);
}

TEST_CASE("QSharedPointer convert_b", "[qsharedpointer]")
{
   QSharedPointer<QFileDevice> ptr1 = QSharedPointer<QFile>(new QFile());

   std::shared_ptr<QIODevice> ptr2 = ptr1;

   REQUIRE(ptr1 != nullptr);
   REQUIRE(ptr2 != nullptr);

   REQUIRE(ptr1.get() == ptr2.get());
}

TEST_CASE("QSharedPointer copy", "[qsharedpointer]")
{
   QSharedPointer<int> ptr1;
   int *rawPointer = nullptr;

   {
      QSharedPointer<int> ptr2 = QMakeShared<int>();
      rawPointer = ptr2.data();
      ptr1 = ptr2;
   }

   REQUIRE(rawPointer == ptr1.data());
}

TEST_CASE("QSharedPointer custom_deleter", "[qsharedpointer]")
{
   bool deleterExecuted = false;

   {
      QSharedPointer<int> ptr(new int, [&deleterExecuted] (int *obj)
         {
            deleterExecuted = true;
            delete obj;
         }
      );

      REQUIRE(deleterExecuted == false);
   }

   REQUIRE(deleterExecuted == true);
}

TEST_CASE("QSharedPointer empty", "[qsharedpointer]")
{
   QSharedPointer<int> ptr;

   REQUIRE(ptr == nullptr);
   REQUIRE(ptr.isNull() == true);
   REQUIRE(ptr == ptr);

   REQUIRE(! (ptr != nullptr));
   REQUIRE(! (nullptr != ptr));
   REQUIRE(! (ptr != ptr)) ;
}

TEST_CASE("QSharedPointer move_assign", "[qsharedpointer]")
{
   QSharedPointer<int> ptr1;
   int *rawPointer = nullptr;

   {
      QSharedPointer<int> ptr2(new int);
      rawPointer = ptr2.data();
      ptr1 = std::move(ptr2);

      REQUIRE(ptr2.isNull());
   }

   REQUIRE(rawPointer == ptr1.data());
}

TEST_CASE("QSharedPointer move_construct", "[qsharedpointer]")
{
   QSharedPointer<int> ptr1 = QMakeShared<int>();
   QSharedPointer<int> ptr2(std::move(ptr1));

   REQUIRE(ptr1.isNull() == true);
   REQUIRE(ptr2.isNull() == false);
}

TEST_CASE("QSharedPointer nullptr", "[qsharedpointer]")
{
   QSharedPointer<int> ptr = nullptr;

   REQUIRE(ptr == nullptr);
   REQUIRE(nullptr == ptr);

   REQUIRE(ptr.isNull() == true);

   ptr = nullptr;

   REQUIRE(ptr == nullptr);
   REQUIRE(nullptr == ptr);

   REQUIRE(ptr.isNull() == true);
}

TEST_CASE("QSharedPointer operators", "[qsharedpointer]")
{
   QSharedPointer<int> ptr1;
   QSharedPointer<int> ptr2 = QMakeShared<int>();
   QSharedPointer<int> ptr3 = ptr2;

   REQUIRE( (ptr1 < ptr2) == true);
   REQUIRE( (ptr2 < ptr1) == false);
   REQUIRE( (ptr2 < ptr2) == false);
   REQUIRE( (ptr1 < ptr1) == false);

   REQUIRE( (ptr1 > ptr2) == false);
   REQUIRE( (ptr2 > ptr1) == true);
   REQUIRE( (ptr2 > ptr2) == false);
   REQUIRE( (ptr1 > ptr1) == false);

   REQUIRE( (ptr1 <= ptr2) == true);
   REQUIRE( (ptr2 <= ptr1) == false);
   REQUIRE( (ptr2 <= ptr2) == true);
   REQUIRE( (ptr1 <= ptr1) == true);

   REQUIRE( (ptr1 >= ptr2) == false);
   REQUIRE( (ptr2 >= ptr1) == true);
   REQUIRE( (ptr2 >= ptr2) == true);
   REQUIRE( (ptr1 >= ptr1) == true);

   REQUIRE( (ptr2 <  ptr3) == false);
   REQUIRE( (ptr2 >  ptr3) == false);
   REQUIRE( (ptr2 <= ptr3) == true);
   REQUIRE( (ptr2 >= ptr3) == true);
}

TEST_CASE("QSharedPointer reset", "[qsharedpointer]")
{
   QSharedPointer<int> ptr = QMakeShared<int>();
   ptr.reset();

   REQUIRE(ptr == nullptr);
   REQUIRE(ptr.isNull() == true);
}

TEST_CASE("QSharedPointer swap", "[qsharedpointer]")
{
   QSharedPointer<int> ptr1 = QMakeShared<int>(8);
   QSharedPointer<int> ptr2 = QMakeShared<int>(17);

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
