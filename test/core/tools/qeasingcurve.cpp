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

#include <qeasingcurve.h>

#include <cs_catch2.h>

TEST_CASE("QEasingCurve traits", "[qeasingcurve]")
{
   REQUIRE(std::is_copy_constructible_v<QEasingCurve> == true);
   REQUIRE(std::is_move_constructible_v<QEasingCurve> == true);

   REQUIRE(std::is_copy_assignable_v<QEasingCurve> == true);
   REQUIRE(std::is_move_assignable_v<QEasingCurve> == true);

   REQUIRE(std::has_virtual_destructor_v<QEasingCurve> == false);
}

TEST_CASE("QEasingCurve assignment_copy", "[qeasingcurve]")
{
   QEasingCurve dataA(QEasingCurve::InOutExpo);
   QEasingCurve dataB = dataA;

   QEasingCurve dataC;
   dataC = dataA;

   REQUIRE(dataA.type() == dataB.type());
   REQUIRE(dataA.type() == dataC.type());
}

TEST_CASE("QEasingCurve equals", "[qeasingcurve]")
{
   QEasingCurve dataA(QEasingCurve::InOutSine);
   QEasingCurve dataB(QEasingCurve::InOutSine);
   QEasingCurve dataC(QEasingCurve::OutElastic);

   REQUIRE(dataA == dataB);
   REQUIRE(dataA != dataC);
}

TEST_CASE("QEasingCurve parameters", "[qeasingcurve]")
{
   QEasingCurve data(QEasingCurve::OutElastic);

   data.setAmplitude(2.0);
   data.setOvershoot(1.5);
   data.setPeriod(0.3);

   REQUIRE_THAT(data.amplitude(), Catch::Matchers::WithinAbs(2.0, 0.0001));
   REQUIRE_THAT(data.overshoot(), Catch::Matchers::WithinAbs(1.5, 0.0001));
   REQUIRE_THAT(data.period(),    Catch::Matchers::WithinAbs(0.3, 0.0001));
}

TEST_CASE("QEasingCurve type", "[qeasingcurve]")
{
   QEasingCurve data;

   REQUIRE(data.type() == QEasingCurve::Linear);

   REQUIRE_THAT(data.valueForProgress(0.0), Catch::Matchers::WithinAbs(0.0, 0.0001));
   REQUIRE_THAT(data.valueForProgress(0.5), Catch::Matchers::WithinAbs(0.5, 0.0001));
   REQUIRE_THAT(data.valueForProgress(1.0), Catch::Matchers::WithinAbs(1.0, 0.0001));

   {
      data.setType(QEasingCurve::OutBounce);
      REQUIRE(data.type() == QEasingCurve::OutBounce);
   }

   {
      data.setType(QEasingCurve::InQuad);

      REQUIRE_THAT(data.valueForProgress(0.0), Catch::Matchers::WithinAbs(0.0, 0.0001));
      REQUIRE_THAT(data.valueForProgress(1.0), Catch::Matchers::WithinAbs(1.0, 0.0001));
      REQUIRE_THAT(data.valueForProgress(0.5), Catch::Matchers::WithinAbs(0.5, 0.0001));
   }
}
