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

#include <qbrush.h>
#include <qcolor.h>

#include <cs_catch2.h>

void registerGuiVariant();

TEST_CASE("QVariant convert_color", "[qvariant_gui]")
{
   registerGuiVariant();

   QVariant data = QString("blue");

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::String);

   SECTION ("bool") {
      REQUIRE(data.convert<bool>() == true);
      REQUIRE(data.type() == QVariant::Bool);
      REQUIRE(data.toBool() == true);
   }

   SECTION ("brush") {
      REQUIRE(data.convert<QBrush>() == false);
      REQUIRE(data.type() == QVariant::Brush);
      REQUIRE(data.value<QBrush>() == QBrush());
   }

   SECTION ("color") {
      REQUIRE(data.convert<QColor>() == true);
      REQUIRE(data.type() == QVariant::Color);
      REQUIRE(data.value<QColor>() == QColor("blue"));
   }

}
