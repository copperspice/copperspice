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

#include <qtranslator.h>

#include <qfile.h>
#include <qbytearray.h>

#include <cs_catch2.h>

static const QString qmPath = ":/test_translator";

TEST_CASE("QTranslator traits", "[qtranslator]")
{
   REQUIRE(std::is_copy_constructible_v<QTranslator> == false);
   REQUIRE(std::is_move_constructible_v<QTranslator> == false);

   REQUIRE(std::is_copy_assignable_v<QTranslator> == false);
   REQUIRE(std::is_move_assignable_v<QTranslator> == false);

   REQUIRE(std::has_virtual_destructor_v<QTranslator> == true);
}

TEST_CASE("QTranslator load_a", "[qtranslator]")
{
   QTranslator obj;
   obj.load(":/test_translator/test_de");

   REQUIRE(obj.isEmpty() == false);
   REQUIRE(obj.translate("QPushButton", "Open") == "Öffnen");
   REQUIRE(obj.translate("QPushButton", "Close") == "Schließen");
}

TEST_CASE("QTranslator load_b", "[qtranslator]")
{
   QTranslator obj;

   QFile file(":/test_translator/test_de.qm");
   file.open(QFile::ReadOnly);

   QByteArray data = file.readAll();
   obj.load((const uchar *)data.constData(), data.length());

   REQUIRE(obj.isEmpty() == false);
   REQUIRE(obj.translate("QPushButton", "Open") == "Öffnen");
   REQUIRE(obj.translate("QPushButton", "Close") == "Schließen");
}

TEST_CASE("QTranslator plural", "[qtranslator]")
{
   QTranslator obj;
   obj.load(":/test_translator/test_de");

   REQUIRE(obj.isEmpty() == false);

   REQUIRE(obj.translate("QThing", "Attending %n conference(s)", nullptr, 0) == "Teilnahme an 0 Konferenzen");
   REQUIRE(obj.translate("QThing", "Attending %n conference(s)", nullptr, 1) == "Teilnahme an 1 Konferenz");
   REQUIRE(obj.translate("QThing", "Attending %n conference(s)", nullptr, 2) == "Teilnahme an 2 Konferenzen");
}

TEST_CASE("QTranslator macros", "[qtranslator]")
{
   const char *source = cs_mark_tr("QIODevice", "Too many open files");

   REQUIRE(QString::fromLatin1(source) == "Too many open files");

   //
   QTranslator obj;
   obj.load(":/test_translator/test_de");

   REQUIRE(obj.translate("QIODevice", source) == "Zu viele Dateien geöffnet");
}
