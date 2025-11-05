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

#include <qstring8.h>
#include <qjsonvalue.h>

#include <catch2/catch.hpp>

TEST_CASE("QJsonValue traits", "[qjsonvalue")
{
   REQUIRE(std::is_copy_constructible_v<QJsonValue> == true);
   REQUIRE(std::is_move_constructible_v<QJsonValue> == true);

   REQUIRE(std::is_copy_assignable_v<QJsonValue> == true);
   REQUIRE(std::is_move_assignable_v<QJsonValue> == true);

   REQUIRE(std::has_virtual_destructor_v<QJsonValue> == false);
}

TEST_CASE("QJsonValue constructor", "[qjsonvalue]")
{
   QJsonValue data(true);

   REQUIRE(data.isBool()   == true);
   REQUIRE(data.isDouble() == false);
   REQUIRE(data.isString() == false);
   REQUIRE(data.isArray()  == false);
   REQUIRE(data.isObject() == false);

   REQUIRE(data.type() == QJsonValue::Bool);

   //
   data = QJsonValue(5.87);

   REQUIRE(data.isBool()   == false);
   REQUIRE(data.isDouble() == true);
   REQUIRE(data.isString() == false);
   REQUIRE(data.isArray()  == false);
   REQUIRE(data.isObject() == false);

   REQUIRE(data.type() == QJsonValue::Double);
}

TEST_CASE("QJsonValue copy_assign", "[qjsonvalue]")
{
   QJsonValue data_a(QString("MM/dd/yyyy"));
   QJsonValue data_b(data_a);

   REQUIRE(data_a.isString() == true);
   REQUIRE(data_b.isString() == true);

   REQUIRE(data_a.toString() == QString("MM/dd/yyyy"));
   REQUIRE(data_b.toString() == QString("MM/dd/yyyy"));

   //
   QJsonValue data_c;
   data_c = data_a;

   REQUIRE(data_a.isString() == true);
   REQUIRE(data_c.isString() == true);

   REQUIRE(data_a.toString() == QString("MM/dd/yyyy"));
   REQUIRE(data_c.toString() == QString("MM/dd/yyyy"));
}

TEST_CASE("QJsonValue move_assign", "[qjsonvalue]")
{
   QJsonValue data_a(QString("MM/dd/yyyy"));
   QJsonValue data_b(std::move(data_a));

   //
   REQUIRE(data_b.isString() == true);
   REQUIRE(data_b.toString() == QString("MM/dd/yyyy"));

   //
   QJsonValue data_c;
   data_c = std::move(data_b);

   REQUIRE(data_c.isString() == true);
   REQUIRE(data_c.toString() == QString("MM/dd/yyyy"));
}
