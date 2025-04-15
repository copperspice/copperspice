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
#include <qxmlstreamwriter.h>

#include <qbuffer.h>

#include <catch2/catch.hpp>

TEST_CASE("QXmlStreamWriter traits", "[qxmlstreamwriter]")
{
   REQUIRE(std::is_copy_constructible_v<QXmlStreamWriter> == false);
   REQUIRE(std::is_move_constructible_v<QXmlStreamWriter> == false);

   REQUIRE(std::is_copy_assignable_v<QXmlStreamWriter> == false);
   REQUIRE(std::is_move_assignable_v<QXmlStreamWriter> == false);

   REQUIRE(std::has_virtual_destructor_v<QXmlStreamWriter> == false);
}

TEST_CASE("QXmlStreamWriter parse_valid_xml", "[qxmlstreamwriter]")
{
    QByteArray output;

    QBuffer buffer(&output);
    buffer.open(QIODevice::WriteOnly);

    QXmlStreamWriter writer(&buffer);
    writer.setAutoFormatting(true);

    writer.writeStartDocument();
    writer.writeStartElement("greeting");
    writer.writeTextElement("text", "Hello, CopperSpice!");
    writer.writeEndElement();
    writer.writeEndDocument();

    buffer.close();

    QString writtenXml = QString::fromUtf8(output);

    REQUIRE(writtenXml.contains("<greeting>"));
    REQUIRE(writtenXml.contains("<text>Hello, CopperSpice!</text>"));
    REQUIRE(writtenXml.contains("</greeting>"));
}

TEST_CASE("QXmlStreamWriter read_write", "[qxmlstreamwriter]")
{
    QByteArray output;

    QBuffer buffer(&output);
    buffer.open(QIODevice::WriteOnly);

    QXmlStreamWriter writer(&buffer);

    writer.writeStartDocument();
    writer.writeStartElement("data");
    writer.writeTextElement("entry", "value");
    writer.writeEndElement();
    writer.writeEndDocument();

    buffer.close();

    QString xml = QString::fromUtf8(output);
    QXmlStreamReader reader(xml);

    REQUIRE(reader.hasError() == false);

    REQUIRE(reader.readNextStartElement());
    REQUIRE(reader.name() == "data");

    REQUIRE(reader.readNextStartElement());
    REQUIRE(reader.name() == "entry");

    REQUIRE(reader.readElementText() == "value");
}
