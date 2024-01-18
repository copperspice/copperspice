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

#include <qmutex.h>

#include <thread>

#include <cs_catch2.h>

TEST_CASE("QMutex traits", "[qmutex]")
{
   REQUIRE(std::is_copy_constructible_v<QMutex> == false);
   REQUIRE(std::is_move_constructible_v<QMutex> == false);

   REQUIRE(std::is_copy_assignable_v<QMutex> == false);
   REQUIRE(std::is_move_assignable_v<QMutex> == false);

   REQUIRE(std::has_virtual_destructor_v<QMutex> == false);
}

TEST_CASE("QRecursiveMutex traits", "[qrecursivemutex]")
{
   REQUIRE(std::is_copy_constructible_v<QRecursiveMutex> == false);
   REQUIRE(std::is_move_constructible_v<QRecursiveMutex> == false);

   REQUIRE(std::is_copy_assignable_v<QRecursiveMutex> == false);
   REQUIRE(std::is_move_assignable_v<QRecursiveMutex> == false);

   REQUIRE(std::has_virtual_destructor_v<QRecursiveMutex> == false);
}

TEST_CASE("QMutex basic", "[qmutex]")
{
   QMutex m;

   REQUIRE(m.tryLock(10) == true);

   m.unlock();
}

TEST_CASE("QRecursiveMutex recursive", "[qrecursivemutex]")
{
   // QMutex data(QMutex::Recursive);     retain to test compile assert
   QRecursiveMutex data;

   REQUIRE(data.tryLock(10) == true);
   REQUIRE(data.tryLock(10) == true);

   data.unlock();
   data.unlock();

   data.lock();

   REQUIRE(data.tryLock(10) == true);
   REQUIRE(data.tryLock(10) == true);

   data.unlock();
   data.unlock();
   data.unlock();
}

TEST_CASE("QMutex thread_trylock", "[qmutex]")
{
   QMutex data;
   QAtomicInt count = 0;

   auto lamb = [&data, &count] ()
      {
         if (data.tryLock(10) == true) {
            ++count;
         }

         data.unlock();
      };

   std::thread workerA(lamb);
   workerA.join();

   REQUIRE(count.load() == 1);
}

TEST_CASE("QMutex thread_stress", "[qmutex]")
{
   QMutex data;
   int count = 0;

   auto lamb = [&data, &count] ()
      {
         for (int i = 0; i < 500; ++i) {

            data.lock();
            ++count;

            data.unlock();
         }
      };

   std::thread workerA(lamb);
   std::thread workerB(lamb);

   workerA.join();
   workerB.join();

   REQUIRE(count == 1000);
}
