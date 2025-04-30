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

TEST_CASE("QSharedPointer comparison", "[qsharedpointer]")
{
   QSharedPointer<int> ptr1 = nullptr;

   QSharedPointer<int> ptr2 = QMakeShared<int>();
   QSharedPointer<int> ptr3 = ptr2;

   int *ptr4 = ptr2.get();

   REQUIRE( (ptr1 == ptr2) == false);
   REQUIRE( (ptr1 != ptr2) == true);
   REQUIRE( (ptr1 <  ptr2) == true);
   REQUIRE( (ptr1 >  ptr2) == false);
   REQUIRE( (ptr1 <= ptr2) == true);
   REQUIRE( (ptr1 >= ptr2) == false);

   REQUIRE( (ptr2 == ptr1) == false);
   REQUIRE( (ptr2 != ptr1) == true);
   REQUIRE( (ptr2 <  ptr1) == false);
   REQUIRE( (ptr2 >  ptr1) == true);
   REQUIRE( (ptr2 <= ptr1) == false);
   REQUIRE( (ptr2 >= ptr1) == true);

   REQUIRE( (ptr2 == ptr3) == true);
   REQUIRE( (ptr2 != ptr3) == false);
   REQUIRE( (ptr2 <  ptr3) == false);
   REQUIRE( (ptr2 >  ptr3) == false);
   REQUIRE( (ptr2 <= ptr3) == true);
   REQUIRE( (ptr2 >= ptr3) == true);

   REQUIRE( (ptr1 == ptr4) == false);
   REQUIRE( (ptr1 != ptr4) == true);
   REQUIRE( (ptr1 <  ptr4) == true);
   REQUIRE( (ptr1 >  ptr4) == false);
   REQUIRE( (ptr1 <= ptr4) == true);
   REQUIRE( (ptr1 >= ptr4) == false);

   REQUIRE( (ptr2 == ptr4) == true);
   REQUIRE( (ptr2 != ptr4) == false);
   REQUIRE( (ptr2 <  ptr4) == false);
   REQUIRE( (ptr2 >  ptr4) == false);
   REQUIRE( (ptr2 <= ptr4) == true);
   REQUIRE( (ptr2 >= ptr4) == true);

   REQUIRE( (ptr1 == nullptr) == true);
   REQUIRE( (ptr1 != nullptr) == false);
   REQUIRE( (ptr1 <  nullptr) == false);
   REQUIRE( (ptr1 >  nullptr) == false);
   REQUIRE( (ptr1 <= nullptr) == true);
   REQUIRE( (ptr1 >= nullptr) == true);

   REQUIRE( (ptr2 == nullptr) == false);
   REQUIRE( (ptr2 != nullptr) == true);
   REQUIRE( (ptr2 <  nullptr) == false);
   REQUIRE( (ptr2 >  nullptr) == true);
   REQUIRE( (ptr2 <= nullptr) == false);
   REQUIRE( (ptr2 >= nullptr) == true);

   REQUIRE( (ptr1 == ptr1) == true);
   REQUIRE( (ptr1 != ptr1) == false);
   REQUIRE( (ptr1 <  ptr1) == false);
   REQUIRE( (ptr1 >  ptr1) == false);
   REQUIRE( (ptr1 <= ptr1) == true);
   REQUIRE( (ptr1 >= ptr1) == true);

   REQUIRE( (ptr2 == ptr2) == true);
   REQUIRE( (ptr2 != ptr2) == false);
   REQUIRE( (ptr2 <  ptr2) == false);
   REQUIRE( (ptr2 >  ptr2) == false);
   REQUIRE( (ptr2 <= ptr2) == true);
   REQUIRE( (ptr2 >= ptr2) == true);
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

TEST_CASE("QSharedPointer copy_assign", "[qsharedpointer]")
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
   QSharedPointer<int> ptr1;

   REQUIRE(ptr1 == nullptr);
   REQUIRE(ptr1.isNull() == true);
   REQUIRE(ptr1 == ptr1);

   REQUIRE(! (ptr1 != nullptr));
   REQUIRE(! (nullptr != ptr1));
   REQUIRE(! (ptr1 != ptr1)) ;
   REQUIRE(ptr1.isNull() == true);
   REQUIRE(ptr1.unique() == false);
   REQUIRE(ptr1.use_count() == 0);

   //
   QSharedPointer<int> ptr2(nullptr, [](auto p) { (void) p; });
   QSharedPointer<int> ptr3(nullptr, [](auto p) { (void) p; }, std::allocator<int>());
   QSharedPointer<int> ptr4(static_cast<int*>(nullptr), [](auto p) { (void) p; }, std::allocator<int>());

   REQUIRE(ptr2 == nullptr);
   REQUIRE(ptr3 == nullptr);
   REQUIRE(ptr4 == nullptr);
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

TEST_CASE("QSharedPointer move_constructor", "[qsharedpointer]")
{
   QSharedPointer<int> ptr1 = QMakeShared<int>();
   QSharedPointer<int> ptr2(std::move(ptr1));

   REQUIRE(ptr1.isNull() == true);
   REQUIRE(ptr2.isNull() == false);
}

TEST_CASE("QSharedPointer is_nullptr", "[qsharedpointer]")
{
   QSharedPointer<int> ptr = nullptr;

   REQUIRE(ptr == nullptr);
   REQUIRE(nullptr == ptr);

   REQUIRE(! ptr == true);
   REQUIRE(ptr.isNull() == true);

   ptr = nullptr;

   REQUIRE(ptr == nullptr);
   REQUIRE(nullptr == ptr);

   REQUIRE(ptr.isNull() == true);
}

TEST_CASE("QSharedPointer reset", "[qsharedpointer]")
{
   QSharedPointer<int> ptr = QMakeShared<int>();
   ptr.reset();

   REQUIRE(ptr == nullptr);
   REQUIRE(ptr.isNull() == true);
   ptr.reset(new int(42));

   REQUIRE(ptr != nullptr);
   REQUIRE(*ptr == 42);

   ptr.reset(ptr.get());

   REQUIRE(ptr != nullptr);
   REQUIRE(*ptr == 42);

   ptr.reset(new int(43), std::default_delete<int>());

   REQUIRE(ptr != nullptr);
   REQUIRE(*ptr == 43);

   ptr.reset(ptr.get(), std::default_delete<int>());
   REQUIRE(ptr != nullptr);
   REQUIRE(*ptr == 43);

   ptr.reset(new int(44), std::default_delete<int>(), std::allocator<int>());
   REQUIRE(ptr != nullptr);
   REQUIRE(*ptr == 44);

   ptr.reset(ptr.get(), std::default_delete<int>(), std::allocator<int>());
   REQUIRE(ptr != nullptr);
   REQUIRE(*ptr == 44);
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
TEST_CASE("QSharedPointer use_count", "[qsharedpointer]")
{
   QSharedPointer<int> ptr1 = QMakeShared<int>(8);
   QSharedPointer<int> ptr2 = QMakeShared<int>(17);

   REQUIRE(ptr1.unique() == true);
   REQUIRE(ptr1.use_count() == 1);
   REQUIRE(ptr2.unique() == true);
   REQUIRE(ptr2.use_count() == 1);

   ptr1 = ptr2;

   REQUIRE(ptr1.unique() == false);
   REQUIRE(ptr1.use_count() == 2);
   REQUIRE(ptr2.unique() == false);
   REQUIRE(ptr2.use_count() == 2);

   ptr2.clear();

   REQUIRE(ptr1.unique() == true);
   REQUIRE(ptr1.use_count() == 1);
   REQUIRE(ptr2.unique() == false);
   REQUIRE(ptr2.use_count() == 0);
}
