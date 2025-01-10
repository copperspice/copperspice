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

TEST_CASE("QWeakPointer nullptr", "[qweakpointer]")
{
   QSharedPointer<int> sharedPtr = QMakeShared<int>();
   QWeakPointer<int> weakPointer = sharedPtr.toWeakRef();

   REQUIRE(sharedPtr == weakPointer);
   REQUIRE(weakPointer == sharedPtr);

   REQUIRE(sharedPtr == weakPointer.lock());
   REQUIRE(sharedPtr == weakPointer.toStrongRef());

   REQUIRE(sharedPtr != nullptr);
   REQUIRE(sharedPtr.isNull() == false);

   REQUIRE(static_cast<bool>(sharedPtr) == true);
   REQUIRE(static_cast<bool>(weakPointer) == true);

   sharedPtr.reset();

   REQUIRE(static_cast<bool>(sharedPtr) == false);
   REQUIRE(static_cast<bool>(weakPointer) == false);

   REQUIRE(sharedPtr == nullptr);
   REQUIRE(sharedPtr.isNull() == true);

   REQUIRE(weakPointer == nullptr);
   REQUIRE(weakPointer.isNull() == true);

   REQUIRE(sharedPtr != weakPointer);   // unusual but accurate
   REQUIRE(weakPointer != sharedPtr);

   REQUIRE(sharedPtr == weakPointer.lock());
   REQUIRE(weakPointer.lock() == sharedPtr);

   REQUIRE(sharedPtr == weakPointer.toStrongRef());
   REQUIRE(weakPointer.toStrongRef() == sharedPtr);
}

