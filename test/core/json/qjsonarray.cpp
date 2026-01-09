/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#include <catch2/catch.hpp>

TEST_CASE("QJsonArray traits", "[qjsonarray]")
{
   REQUIRE(std::is_copy_constructible_v<QJsonArray> == true);
   REQUIRE(std::is_move_constructible_v<QJsonArray> == true);

   REQUIRE(std::is_copy_assignable_v<QJsonArray> == true);
   REQUIRE(std::is_move_assignable_v<QJsonArray> == true);

   REQUIRE(std::has_virtual_destructor_v<QJsonArray> == false);
}

TEST_CASE("QJsonArray copy_assign", "[qjsonarray]")
{
   QJsonArray data_a;
   data_a.append(QString("d:/cs_docs/file1.dox"));
   data_a.append(QString("d:/cs_docs/file2.dox"));

   //
   QJsonArray data_b(data_a);

   REQUIRE(data_a.count() == 2);
   REQUIRE(data_b.count() == 2);

   REQUIRE(data_a.contains(QString("d:/cs_docs/file1.dox")) == true);
   REQUIRE(data_b.contains(QString("d:/cs_docs/file1.dox")) == true);

   //
   QJsonArray data_c;
   data_c = data_a;

   REQUIRE(data_a.count() == 2);
   REQUIRE(data_c.count() == 2);

   REQUIRE(data_a.contains(QString("d:/cs_docs/file1.dox")) == true);
   REQUIRE(data_c.contains(QString("d:/cs_docs/file1.dox")) == true);
}

TEST_CASE("QJsonArray move_assign", "[qjsonarray]")
{
   QJsonArray data_a;
   data_a.append(QString("d:/cs_docs/file1.dox"));
   data_a.append(QString("d:/cs_docs/file2.dox"));

   //
   QJsonArray data_b(std::move(data_a));

   REQUIRE(data_b.contains(QString("d:/cs_docs/file1.dox")) == true);

   //
   QJsonArray data_c;
   data_c = std::move(data_b);

   REQUIRE(data_c.contains(QString("d:/cs_docs/file1.dox")) == true);
}
