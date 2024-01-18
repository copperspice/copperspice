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

#include <qelapsedtimer.h>

#include <cs_catch2.h>

TEST_CASE("QElapsedTimer traits", "[qelapsedtimer]")
{
   REQUIRE(std::is_copy_constructible_v<QElapsedTimer> == true);
   REQUIRE(std::is_move_constructible_v<QElapsedTimer> == true);

   REQUIRE(std::is_copy_assignable_v<QElapsedTimer> == true);
   REQUIRE(std::is_move_assignable_v<QElapsedTimer> == true);

   REQUIRE(std::has_virtual_destructor_v<QElapsedTimer> == false);
}

TEST_CASE("QElapsedTimer operators", "[qelapsedtimer]")
{
   QElapsedTimer timer;
   timer.start();

   REQUIRE(timer.msecsSinceReference() != 0);
   REQUIRE(timer.msecsTo(timer) == qint64(0));
   REQUIRE(timer.secsTo(timer) == qint64(0));

   REQUIRE(timer == timer);
   REQUIRE(! (timer != timer));
   REQUIRE(! (timer < timer));

   QThread::msleep(10);

   REQUIRE(timer.nsecsElapsed() > 0);
   REQUIRE(timer.elapsed() > 0);
}

TEST_CASE("QElapsedTimer elapsed", "[qelapsedtimer]")
{
   QElapsedTimer timer;
   timer.start();

   qint64 t1 = timer.elapsed();

   QThread::msleep(10);
   qint64 t2 = timer.elapsed();

   REQUIRE(t2 > t1);
}

TEST_CASE("QElapsedTimer valid", "[qelapsedtimer]")
{
   QElapsedTimer timer;

   REQUIRE(timer.isValid() == false);

   timer.start();
   REQUIRE(timer.isValid() == true);

   timer.invalidate();
   REQUIRE(timer.isValid() == false);
}


