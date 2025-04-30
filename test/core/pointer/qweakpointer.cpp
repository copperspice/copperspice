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

TEST_CASE("QWeakPointer traits", "[qweakpointer]")
{
   REQUIRE(std::is_copy_constructible_v<QWeakPointer<int>> == true);
   REQUIRE(std::is_move_constructible_v<QWeakPointer<int>> == true);

   REQUIRE(std::is_copy_assignable_v<QWeakPointer<int>> == true);
   REQUIRE(std::is_move_assignable_v<QWeakPointer<int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QWeakPointer<int>> == false);
}

TEST_CASE("QWeakPointer clear", "[qweakpointer]")
{
   QSharedPointer<int> ptr = QMakeShared<int>();
   QWeakPointer<int> weakPointer = ptr.toWeakRef();

   weakPointer.clear();

   REQUIRE(weakPointer == nullptr);
   REQUIRE(weakPointer.isNull() == true);
}

TEST_CASE("QWeakPointer data", "[qweakpointer]")
{
   QObject *obj = new QObject;

   QWeakPointer<QObject> weakPointer = obj;

   REQUIRE(weakPointer.data() == obj);

   delete obj;

   REQUIRE(weakPointer.data() == nullptr);
}

TEST_CASE("QWeakPointer is_nullptr", "[qweakpointer]")
{
   QSharedPointer<int> sharedPtr = QMakeShared<int>();
   QWeakPointer<int> weakPtr = sharedPtr.toWeakRef();

#if ! defined(Q_CC_MSVC)
   REQUIRE(sharedPtr == weakPtr);
#endif

   REQUIRE(weakPtr == sharedPtr);

   REQUIRE(sharedPtr == weakPtr.lock());
   REQUIRE(sharedPtr == weakPtr.toStrongRef());

   REQUIRE(sharedPtr != nullptr);
   REQUIRE(sharedPtr.isNull() == false);

   REQUIRE(static_cast<bool>(sharedPtr) == true);
   REQUIRE(static_cast<bool>(weakPtr) == true);

   {
      QSharedPointer<int> tmp(weakPtr);

      REQUIRE(sharedPtr == tmp);

      tmp = weakPtr;

      REQUIRE(sharedPtr == tmp);
   }
   sharedPtr.reset();

   REQUIRE(static_cast<bool>(sharedPtr) == false);
   REQUIRE(static_cast<bool>(weakPtr) == false);

   REQUIRE(sharedPtr == nullptr);
   REQUIRE(sharedPtr.isNull() == true);

   REQUIRE(weakPtr == nullptr);
   REQUIRE(weakPtr.isNull() == true);

   REQUIRE(sharedPtr != weakPtr);   // unusual but accurate
   REQUIRE(weakPtr != sharedPtr);

   REQUIRE(sharedPtr == weakPtr.lock());
   REQUIRE(weakPtr.lock() == sharedPtr);

   REQUIRE(sharedPtr == weakPtr.toStrongRef());
   REQUIRE(weakPtr.toStrongRef() == sharedPtr);
}
