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

#include <qxmlstreamreader.h>

#include <catch2/catch.hpp>

TEST_CASE("QXmlStreamReader traits", "[qxmlstreamreader]")
{
   REQUIRE(std::is_copy_constructible_v<QXmlStreamReader> == false);
   REQUIRE(std::is_move_constructible_v<QXmlStreamReader> == false);

   REQUIRE(std::is_copy_assignable_v<QXmlStreamReader> == false);
   REQUIRE(std::is_move_assignable_v<QXmlStreamReader> == false);

   REQUIRE(std::has_virtual_destructor_v<QXmlStreamReader> == false);
}


TEST_CASE("QXmlStreamReader parse_malformed_xml", "[qxmlstreamreader]")
{
   QString xml = "<root><child></root>";
   QXmlStreamReader reader(xml);

   while (! reader.atEnd()) {
      reader.readNext();
   }

   REQUIRE(reader.hasError() == true);
   REQUIRE(reader.error() == QXmlStreamReader::NotWellFormedError);
   REQUIRE(reader.error() != QXmlStreamReader::PrematureEndOfDocumentError);
   REQUIRE(reader.error() != QXmlStreamReader::UnexpectedElementError);

}

TEST_CASE("QXmlStreamReader parse_valid_xml", "[qxmlstreamreader]")
{
   QString xml = "<root><child>text</child></root>";
   QXmlStreamReader reader(xml);

   // <root>
   REQUIRE(reader.readNextStartElement());
   REQUIRE(reader.name() == "root");

   // <child>
   REQUIRE(reader.readNextStartElement());
   REQUIRE(reader.name() == "child");
   REQUIRE(reader.readElementText() == "text");

   // end of root
   reader.readNext();
   REQUIRE(reader.tokenType() == QXmlStreamReader::EndElement);
   REQUIRE(reader.name() == "root");

   REQUIRE(!reader.hasError());
}
