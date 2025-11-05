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

#include <qjsonarray.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>

#include <catch2/catch.hpp>

TEST_CASE("QJsonObject traits", "[qjsonobject]")
{
   REQUIRE(std::is_copy_constructible_v<QJsonObject> == true);
   REQUIRE(std::is_move_constructible_v<QJsonObject> == true);

   REQUIRE(std::is_copy_assignable_v<QJsonObject> == true);
   REQUIRE(std::is_move_assignable_v<QJsonObject> == true);

   REQUIRE(std::has_virtual_destructor_v<QJsonObject> == false);
}

TEST_CASE("QJsonObject copy_assign", "[qjsonobject]")
{
   QJsonObject data_a;

   QJsonValue value = QJsonValue(QString("MM/dd/yyyy"));
   data_a.insert("formatDate", value);

   QJsonArray list;
   list.append(QString("d:/cs_docs/file1.dox"));
   data_a.insert("file-names", list);

   //
   QJsonObject data_b(data_a);

   REQUIRE(data_a.count() == 2);
   REQUIRE(data_b.count() == 2);

   REQUIRE(data_a.value("formatDate") == QString("MM/dd/yyyy"));
   REQUIRE(data_b.value("formatDate") == QString("MM/dd/yyyy"));

   //
   QJsonValue data_c;
   data_c = data_a;

   REQUIRE(data_a.count() == 2);
   REQUIRE(data_b.count() == 2);

   REQUIRE(data_a.value("file-names") == list);
   REQUIRE(data_b.value("file-names") == list);
}

TEST_CASE("QJsonObject move_assign", "[qjsonobject]")
{
   QJsonObject data_a;

   QJsonValue value = QJsonValue(QString("MM/dd/yyyy"));
   data_a.insert("formatDate", value);

   QJsonArray list;
   list.append(QString("d:/cs_docs/file1.dox"));
   data_a.insert("file-names", list);

   QJsonObject data_b(std::move(data_a));

   //
   REQUIRE(data_b.count() == 2);

   REQUIRE(data_b.value("formatDate") == QString("MM/dd/yyyy"));
   REQUIRE(data_b.value("file-names") == list);

   //
   QJsonObject data_c;
   data_c = std::move(data_b);

   REQUIRE(data_c.count() == 2);

   REQUIRE(data_c.value("formatDate") == QString("MM/dd/yyyy"));
   REQUIRE(data_c.value("file-names") == list);
}
