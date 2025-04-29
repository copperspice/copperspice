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

#include <qtimeline.h>

#include <cs_catch2.h>

TEST_CASE("QTimeLine traits", "[qtimeline]")
{
   REQUIRE(std::is_copy_constructible_v<QTimeLine> == false);
   REQUIRE(std::is_move_constructible_v<QTimeLine> == false);

   REQUIRE(std::is_copy_assignable_v<QTimeLine> == false);
   REQUIRE(std::is_move_assignable_v<QTimeLine> == false);

   REQUIRE(std::has_virtual_destructor_v<QTimeLine> == true);
}

TEST_CASE("QTimeLine Basic empty", "[qtimeline]")
{
   QTimeLine timeline;

   REQUIRE(timeline.duration() == 1000);
   REQUIRE(timeline.loopCount() == 1);
   REQUIRE(timeline.direction() == QTimeLine::Forward);
   REQUIRE(timeline.currentTime() == 0);
   REQUIRE(timeline.state() == QTimeLine::NotRunning);

   {
      timeline.setDuration(500);
      REQUIRE(timeline.duration() == 500);
   }

   {
      timeline.setLoopCount(3);
      REQUIRE(timeline.loopCount() == 3);
   }

   {
      timeline.setDirection(QTimeLine::Backward);
      REQUIRE(timeline.direction() == QTimeLine::Backward);
   }

   {
      timeline.setCurveShape(QTimeLine::EaseInOutCurve);
      REQUIRE(timeline.curveShape() == QTimeLine::EaseInOutCurve);
   }
}
