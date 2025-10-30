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

TEST_CASE("QEasingCurve comparison", "[qeasingcurve]")
{
   QEasingCurve data_a(QEasingCurve::InOutSine);
   QEasingCurve data_b(QEasingCurve::InOutSine);
   QEasingCurve data_c(QEasingCurve::OutElastic);

   REQUIRE(data_a == data_b);
   REQUIRE(data_a != data_c);
}

TEST_CASE("QEasingCurve constructor", "[qeasingcurve]")
{
   QEasingCurve data;

   REQUIRE(data.amplitude() == 1.0);
   REQUIRE_THAT(data.overshoot(), Catch::Matchers::WithinAbs(1.70158, 0.0001));
   REQUIRE(data.period()    == 0.3);
   REQUIRE(data.type()      == QEasingCurve::Linear);

   //
   data = QEasingCurve(QEasingCurve::InOutBack);

   REQUIRE(data.amplitude() == 1.0);
   REQUIRE_THAT(data.overshoot(), Catch::Matchers::WithinAbs(1.70158, 0.0001));
   REQUIRE(data.period()    == 0.3);
   REQUIRE(data.type()      == QEasingCurve::InOutBack);
}

TEST_CASE("QEasingCurve copy_assign", "[qeasingcurve]")
{
   QEasingCurve data_a(QEasingCurve::InOutExpo);
   QEasingCurve data_b(data_a);

   REQUIRE(data_a.type() == data_b.type());
   REQUIRE(data_a == data_b);

   //
   QEasingCurve data_c;
   data_c = data_a;

   REQUIRE(data_a.type() == data_c.type());
   REQUIRE(data_a == data_c);
}

TEST_CASE("QEasingCurve move_assign", "[qeasingcurve]")
{
   QEasingCurve data_a(QEasingCurve::InOutExpo);
   QEasingCurve data_b(std::move(data_a));

   REQUIRE(data_b.type() == QEasingCurve::InOutExpo);

   //
   QEasingCurve data_c;
   data_c = std::move(data_b);

   REQUIRE(data_c.type() == QEasingCurve::InOutExpo);
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

TEST_CASE("QEasingCurve swap", "[qeasingcurve]")
{
   QEasingCurve data_a(QEasingCurve::InCubic);
   QEasingCurve data_b(QEasingCurve::OutBounce);

   REQUIRE(data_a.type() == QEasingCurve::InCubic);
   REQUIRE(data_b.type() == QEasingCurve::OutBounce);

   data_a.swap(data_b);

   REQUIRE(data_a.type() == QEasingCurve::OutBounce);
   REQUIRE(data_b.type() == QEasingCurve::InCubic);
}

TEST_CASE("QEasingCurve valueForProgress", "[qeasingcurve]")
{
   QEasingCurve data;

   REQUIRE(data.type() == QEasingCurve::Linear);

   REQUIRE_THAT(data.valueForProgress(0.0), Catch::Matchers::WithinAbs(0.0, 0.0001));
   REQUIRE_THAT(data.valueForProgress(0.5), Catch::Matchers::WithinAbs(0.5, 0.0001));
   REQUIRE_THAT(data.valueForProgress(1.0), Catch::Matchers::WithinAbs(1.0, 0.0001));

   {
      data.setType(QEasingCurve::OutBounce);

      REQUIRE(data.type() == QEasingCurve::OutBounce);

      REQUIRE_THAT(data.valueForProgress(0.0), Catch::Matchers::WithinAbs(0.0,    0.0001));
      REQUIRE_THAT(data.valueForProgress(0.5), Catch::Matchers::WithinAbs(0.7656, 0.0001));
      REQUIRE_THAT(data.valueForProgress(1.0), Catch::Matchers::WithinAbs(1.0,    0.0001));
   }

   {
      data.setType(QEasingCurve::InQuad);

      REQUIRE_THAT(data.valueForProgress(0.0), Catch::Matchers::WithinAbs(0.0, 0.0001));
      REQUIRE_THAT(data.valueForProgress(0.5), Catch::Matchers::WithinAbs(0.5, 0.0001));
      REQUIRE_THAT(data.valueForProgress(1.0), Catch::Matchers::WithinAbs(1.0, 0.0001));

   }
}
