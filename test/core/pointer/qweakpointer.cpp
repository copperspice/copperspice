/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#include <cs_catch2.h>

TEST_CASE("QWeakPointer clear", "[qweakpointer]")
{
   QSharedPointer<int> ptr = QMakeShared<int>();
   QWeakPointer<int> weakPointer = ptr.toWeakRef();

   weakPointer.clear();

   REQUIRE(weakPointer == nullptr);
   REQUIRE(weakPointer.isNull() == true);
}

TEST_CASE("QWeakPointer nullptr", "[qweakpointer]")
{
   QSharedPointer<int> ptr = QMakeShared<int>();

   QWeakPointer<int> weakPointer = ptr.toWeakRef();
   ptr.reset();

   REQUIRE(static_cast<bool>(weakPointer) == false);
   REQUIRE(static_cast<bool>(ptr) == false);

   REQUIRE(ptr == nullptr);
   REQUIRE(ptr.isNull() == true);

   REQUIRE(weakPointer == nullptr);
   REQUIRE(weakPointer.isNull()  == true);

   REQUIRE(ptr != weakPointer);
   REQUIRE(weakPointer != ptr);
}

