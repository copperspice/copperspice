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

#include <qmutex.h>

#include <thread>

#include <cs_catch2.h>

TEST_CASE("QMutex basic", "[qmutex]")
{
   QMutex m;

   REQUIRE(m.tryLock(10) == true);
   REQUIRE(m.tryLock(10) == false);

   m.unlock();
   m.lock();
   REQUIRE(m.tryLock(10) == false);
   REQUIRE(m.tryLock(10) == false);
   m.unlock();

   REQUIRE(m.isRecursive() == false);
}

TEST_CASE("QMutex recursive", "[qmutex]")
{
   QMutex data(QMutex::Recursive);

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

   REQUIRE(data.isRecursive() == true);
}

TEST_CASE("QMutex thread_trylock", "[qmutex]")
{
   QMutex data;
   QAtomicInt count = 0;

   std::thread workerA([&data, &count] ()
      {
         if (data.tryLock(10) == true) {
            ++count;
         }

         if (data.tryLock(10) == false) {
            ++count;
         }
      } );

   workerA.join();

   REQUIRE(count.load() == 2);
   REQUIRE(data.tryLock(10) == false);

   data.unlock();
}





