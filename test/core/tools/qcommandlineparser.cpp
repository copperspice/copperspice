/***********************************************************************
*
* Copyright (c) 2012-2022 Barbara Geller
* Copyright (c) 2012-2022 Ansel Sermersheim
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

   REQUIRE(std::is_nothrow_move_constructible_v<QCommandLineParser> == false);
   REQUIRE(std::is_nothrow_move_assignable_v<QCommandLineParser> == false);

   REQUIRE(std::has_virtual_destructor_v<QCommandLineParser> == false);
}

std::unique_ptr<QCommandLineParser> GetQCommandLineParser(const QStringList &arguments,
      QList<QCommandLineOption> options)
{
   std::unique_ptr<QCommandLineParser> parser = std::make_unique<QCommandLineParser>();

   parser->setApplicationDescription("Test helper");
   parser->addHelpOption();
   parser->addVersionOption();
   parser->addPositionalArgument("source", QCoreApplication::translate("main", "Source file to copy."));
   parser->addPositionalArgument("destination", QCoreApplication::translate("main", "Destination directory."));

   for (const QCommandLineOption &commandLineOption : options) {
      parser->addOption(commandLineOption);
   }

   parser->process(arguments);

   return parser;
}

TEST_CASE("QCommandListParser non_copyable", "[qcommandlineparser]")
{
   REQUIRE(((! std::is_copy_constructible_v<QCommandLineParser> && ! std::is_copy_assignable_v<QCommandLineParser>) ));
}

TEST_CASE("QCommandListParser constructor", "[qcommandlineparser]")
{
   std::unique_ptr<QCommandLineParser> commandlistparser = GetQCommandLineParser({"", "source-location", "destination-location"}, {});
   const QStringList args = commandlistparser->positionalArguments();

   REQUIRE(args.size() == 2);
   REQUIRE(args.at(0) == "source-location");
   REQUIRE(args.at(1) == "destination-location");
}

TEST_CASE("QCommandListParser options", "[qcommandlineparser]")
{
   // boolean option with a single name
   QCommandLineOption showProgressOption("p", QCoreApplication::translate("main", "Show progress during copy"));

   // boolean option with multiple names
   QCommandLineOption forceOption(QStringList() << "f" << "force",
               QCoreApplication::translate("main", "Overwrite existing files."));

   // option with a value
   QCommandLineOption targetDirectoryOption(QStringList() << "t" << "target-directory",
               QCoreApplication::translate("main", "Copy all source files into <directory>."),
               QCoreApplication::translate("main", "directory"));

   {
      std::unique_ptr<QCommandLineParser> commandlistparser = GetQCommandLineParser(
               {"copy", "source-location", "destination-location"},
               {showProgressOption, forceOption, targetDirectoryOption});

      QStringList args = commandlistparser->positionalArguments();

      REQUIRE(args.at(0) == "source-location");
      REQUIRE(args.at(1) == "destination-location");

      REQUIRE(commandlistparser->isSet(showProgressOption) == false);
      REQUIRE(commandlistparser->isSet(forceOption) == false);

      QString targetDir = commandlistparser->value(targetDirectoryOption);
      REQUIRE(targetDir == "");
   }

   {
      std::unique_ptr<QCommandLineParser> commandlistparser = GetQCommandLineParser(
               {"copy", "-f", "-t", "destination-location"},
               {showProgressOption, forceOption, targetDirectoryOption});

      REQUIRE(commandlistparser->isSet(showProgressOption) == false);
      REQUIRE(commandlistparser->isSet(forceOption) == true);

      QString targetDir = commandlistparser->value(targetDirectoryOption);

      REQUIRE(targetDir == "destination-location");
   }
}
