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
#include <qpointer.h>

#include <cs_catch2.h>

TEST_CASE("QPointer traits", "[qpointer]")
{
   REQUIRE(std::is_copy_constructible_v<QPointer<int>> == true);
   REQUIRE(std::is_move_constructible_v<QPointer<int>> == true);

   REQUIRE(std::is_copy_assignable_v<QPointer<int>> == true);
   REQUIRE(std::is_move_assignable_v<QPointer<int>> == true);

   REQUIRE(std::has_virtual_destructor_v<QPointer<int>> == false);
}

TEST_CASE("QPointer constructor", "[qpointer]")
{
   QObject obj;

   QPointer<QObject> p1;
   QPointer<QObject> p2(&obj);
   QPointer<QObject> p3(p2);

   REQUIRE(p1 == nullptr);
   REQUIRE(p2 == p3);
   REQUIRE(p3 == &obj);
}

TEST_CASE("QPointer destructor", "[qpointer]")
{
   QObject *obj = new QObject;

   QPointer<QObject> p1 = obj;
   QPointer<QObject> p2 = obj;

   REQUIRE(p1 == QPointer<QObject>(obj));
   REQUIRE(p2 == QPointer<QObject>(obj));
   REQUIRE(p1 == p2);

   delete obj;

   REQUIRE(p1 == nullptr);
   REQUIRE(p2 == nullptr);
   REQUIRE(p1 == p2);
}

TEST_CASE("QPointer assignment_operators", "[qpointer]")
{
   QObject *obj = new QObject;

   QPointer<QObject> p1;
   QPointer<QObject> p2;

   {
      p1 = obj;
      p2 = p1;

      REQUIRE(p1 == QPointer<QObject>(obj));
      REQUIRE(p2 == QPointer<QObject>(obj));
      REQUIRE(p1 == p2);
   }

   {
      p1 = nullptr;
      p2 = p1;

      REQUIRE(p1 == nullptr);
      REQUIRE(p2 == nullptr);
      REQUIRE(p1 == p2);
   }

   delete obj;
}
