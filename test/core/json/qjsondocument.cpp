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

#include <qstring8.h>
#include <qjsondocument.h>

#include <catch2/catch.hpp>

TEST_CASE("QJsonDocument traits", "[qjsondocument]")
{
   REQUIRE(std::is_copy_constructible_v<QJsonDocument> == true);
   REQUIRE(std::is_move_constructible_v<QJsonDocument> == true);

   REQUIRE(std::is_copy_assignable_v<QJsonDocument> == true);
   REQUIRE(std::is_move_assignable_v<QJsonDocument> == true);

   REQUIRE(std::has_virtual_destructor_v<QJsonDocument> == false);
}

TEST_CASE("QJsonDocument values", "[qjson]")
{
   QJsonObject object;
   QJsonValue  value;
   QJsonArray  list;

   object.insert("pathPrior",    QString("d:/w_docs/cs_api"));
   object.insert("tabSpacing",   3);
   object.insert("useSpaces",    true);
   object.insert("lineNumbers",  false);
   object.insert("pos-x",        400);
   object.insert("pos-y",        200);

   list.append(QString("d:/cs_docs/api/doxy_api.json"));
   list.append(QString("d:/cs_docs/overview/doxy_overview.json"));
   object.insert("opened-files", list);

   value = QJsonValue(QString("MM/dd/yyyy"));
   object.insert("formatDate",   value);

   value = QJsonValue(QString("h:mm ap"));
   object.insert("formatTime",   value);

   // saving data in Json format
   QJsonDocument doc(object);
   QByteArray input = doc.toJson();

   // retrieve data as a document
   QJsonDocument doc2  = QJsonDocument::fromJson(input);
   QJsonObject object2 = doc2.object();

   value = object2.value("pos-x");
   REQUIRE(value.isDouble() == true);
   REQUIRE(value.toDouble() == 400);

   value = object2.value("useSpaces");
   REQUIRE(value.isBool() == true);
   REQUIRE(value.toBool() == true);

   value = object2.value("pathPrior");
   REQUIRE(value.isString() == true);
   REQUIRE(value.toString() == "d:/w_docs/cs_api");

   list = object2.value("opened-files").toArray();
   REQUIRE(list.count() == 2);
   REQUIRE(list[1].toString() == "d:/cs_docs/overview/doxy_overview.json");

   {
      REQUIRE(object2.contains("formatDate") == true);
      REQUIRE(object2.contains("myDataDate") == false);

      REQUIRE(object2.empty() == false);
      REQUIRE(object2.count() == 9);
   }

   SECTION ("erase") {
      QJsonObject::const_iterator iter = object2.find("tabSpacing");
      object2.erase(iter);

      REQUIRE(object2.count() == 8);
   }

   SECTION ("copy_assign") {
      QJsonObject object3;
      object3 = object2;
      REQUIRE(object2 == object3);
   }

   SECTION ("copy_constructor") {
      QJsonObject object3(object2);
      REQUIRE(object2 == object3);
   }

   SECTION ("move_assign") {
      QJsonObject object3;
      object3 = std::move(object2);
      REQUIRE(object3.count() == 9);
   }

   SECTION ("move_constructor") {
      QJsonObject object3(std::move(object2));
      REQUIRE(object3.count() == 9);
   }
}

TEST_CASE("QJsonDocument large_numbers", "[qjson]")
{
   qint64 numbers[] = {
      -1, 0, 1,
      0x8000000000000ll,               // 13 zeros
      0x40000000000000ll,
      0x100000000000000ll,             // 14 zeros
      -0x8000000000000ll,
      -0x40000000000000ll,
      -0x100000000000000ll,
      0x8000000000000ll   - 1,
      0x40000000000000ll  - 1,
      0x100000000000000ll - 1,
      -(0x8000000000000ll   - 1),
      -(0x40000000000000ll  - 1),
      -(0x100000000000000ll - 1),
   };

   QJsonArray arrayA;

   for (int value : numbers) {
      arrayA.append((double)value);
   }

   QByteArray input   = QJsonDocument(arrayA).toJson();
   QJsonDocument json = QJsonDocument::fromJson(input);

   QJsonArray arrayB = json.array();
   REQUIRE(arrayA.size() == arrayB.size());
}

