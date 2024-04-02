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

#include <qcommandlineparser.h>
#include <qstringlist.h>

#include <type_traits>

#include <cs_catch2.h>

TEST_CASE("QCommandLineParser traits", "[qcommandlineparser]")
{
   REQUIRE(std::is_copy_constructible_v<QCommandLineParser> == false);
   REQUIRE(std::is_move_constructible_v<QCommandLineParser> == false);

   REQUIRE(std::is_copy_assignable_v<QCommandLineParser> == false);
   REQUIRE(std::is_move_assignable_v<QCommandLineParser> == false);

   REQUIRE(std::has_virtual_destructor_v<QCommandLineParser> == false);
}

QUniquePointer<QCommandLineParser> parseCommandLine(const QStringList &arguments,
      QList<QCommandLineOption> options)
{
   QUniquePointer<QCommandLineParser> parser = QMakeUnique<QCommandLineParser>();

   parser->setApplicationDescription("CsCore Test");
   parser->addHelpOption();

   parser->addVersionOption();

   for (const QCommandLineOption &item : options) {
      parser->addOption(item);
   }

   //
   parser->addPositionalArgument("source",      "Source path and file name");
   parser->addPositionalArgument("destination", "Destination path and file name");

   parser->process(arguments);

   return parser;
}

TEST_CASE("QCommandLineParser non_copyable", "[qcommandlineparser]")
{
   REQUIRE(((! std::is_copy_constructible_v<QCommandLineParser> && ! std::is_copy_assignable_v<QCommandLineParser>) ));
}

TEST_CASE("QCommandLineParser constructor", "[qcommandlineparser]")
{
   QUniquePointer<QCommandLineParser> parser = parseCommandLine(
         {"", "c:\\system\\file.txt", "c:\\tmp\\output.txt"}, { });

   const QStringList args = parser->positionalArguments();

   REQUIRE(args.size() == 2);
   REQUIRE(args.at(0) == "c:\\system\\file.txt");
   REQUIRE(args.at(1) == "c:\\tmp\\output.txt");

   REQUIRE(parser->optionNames().size() == 0);
}

TEST_CASE("QCommandLineParser options", "[qcommandlineparser]")
{
   // boolean option with a single name
   QCommandLineOption showProgressOption("p", "Show progress during copy");

   // boolean option with multiple names
   QCommandLineOption forceOption(QStringList() << "f" << "force", "Overwrite existing files");

   // option with a value
   QCommandLineOption targetDirectoryOption(QStringList() << "t" << "target-directory",
         "Copy all source files into <directory>.", "directory");

   {
      QUniquePointer<QCommandLineParser> parser = parseCommandLine(
            {"copy", "source-location", "destination-location"},
            {showProgressOption, forceOption, targetDirectoryOption});

      QStringList args = parser->positionalArguments();

      REQUIRE(args.at(0) == "source-location");
      REQUIRE(args.at(1) == "destination-location");

      REQUIRE(parser->isSet(showProgressOption) == false);
      REQUIRE(parser->isSet(forceOption) == false);

      QString targetDir = parser->value(targetDirectoryOption);
      REQUIRE(targetDir == "");
   }

   {
      QUniquePointer<QCommandLineParser> parser = parseCommandLine(
            {"copy", "-f", "-t", "destination-location"},
            {showProgressOption, forceOption, targetDirectoryOption});

      REQUIRE(parser->isSet(showProgressOption) == false);
      REQUIRE(parser->isSet(forceOption) == true);

      QString targetDir = parser->value(targetDirectoryOption);

      REQUIRE(targetDir == "destination-location");
   }
}
