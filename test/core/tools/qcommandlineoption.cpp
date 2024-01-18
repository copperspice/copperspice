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

#include <qcommandlineoption.h>
#include <qstringlist.h>

#include <cs_catch2.h>

TEST_CASE("QCommandLineOption traits", "[qcommandlineoption]")
{
   REQUIRE(std::is_copy_constructible_v<QCommandLineOption> == true);
   REQUIRE(std::is_move_constructible_v<QCommandLineOption> == true);

   REQUIRE(std::is_copy_assignable_v<QCommandLineOption> == true);
   REQUIRE(std::is_move_assignable_v<QCommandLineOption> == true);

   REQUIRE(std::has_virtual_destructor_v<QCommandLineOption> == false);
}

TEST_CASE("QCommondLineOption constructor", "[qcommandlineoption]")
{
   QString name("optionname");
   QString value("valuename");

   QCommandLineOption data(name, "", value);

   REQUIRE(data.names().size() == 1);
   REQUIRE(data.names().first() == name);
   REQUIRE(data.description() == "");
   REQUIRE(data.valueName() == value);

   REQUIRE(data.defaultValues().size() == 0);
}

TEST_CASE("QCommondLineOption copy", "[qcommandlineoption]")
{
   QString name = "name";
   QString description = "description";

   QCommandLineOption data1(name, description);
   QCommandLineOption data2(data1);

   REQUIRE(data2.names().size() == 1);
   REQUIRE(data2.names().first() == name);
   REQUIRE(data2.description() == description);
   REQUIRE(data2.valueName() == "");

   REQUIRE(data2.defaultValues().size() == 0);
}

TEST_CASE("QCommondLineOption example", "[qcommandlineoption]")
{
   QCommandLineOption data1("verbose", "Verbose mode. This will print more information.");
   QCommandLineOption data2(QStringList() << "o" << "output", "Write generated data into <file>.", "file");

   REQUIRE(data2.names().size() == 2);
}
