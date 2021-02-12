/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#include <qtemporaryfile.h>

#include <cs_catch2.h>

TEST_CASE("QTemporaryFile fname_a", "[qtemporaryfile]")
{
   QTemporaryFile tmpFile;
   tmpFile.open();

   REQUIRE(tmpFile.fileName().contains("cs_temp."));
   REQUIRE(! tmpFile.fileName().endsWith(".XXXXXX"));
}

TEST_CASE("QTemporaryFile fname_b", "[qtemporaryfile]")
{
   QCoreApplication::setApplicationName("CsCoreTest");

   QTemporaryFile tmpFile;
   tmpFile.open();

   REQUIRE(tmpFile.fileName().contains("CsCoreTest"));
   REQUIRE(! tmpFile.fileName().endsWith("XXXXXX"));

   QCoreApplication::setApplicationName("");
}

TEST_CASE("QTemporaryFile fname_c", "[qtemporaryfile]")
{
   QCoreApplication::setApplicationName("CsCoreTestXXXXXX.exe");

   QTemporaryFile tmpFile;
   tmpFile.open();

   REQUIRE(tmpFile.fileName().contains("CsCoreTestXXXXXX"));
   REQUIRE(! tmpFile.fileName().endsWith("XXXXXX"));

   QCoreApplication::setApplicationName("");
}

TEST_CASE("QTemporaryFile fname_d", "[qtemporaryfile]")
{
   QCoreApplication::setApplicationName("CsCoreTest.exeXX");

   QTemporaryFile tmpFile;
   tmpFile.open();

   REQUIRE(tmpFile.fileName().contains("CsCoreTest"));
   REQUIRE(! tmpFile.fileName().endsWith("XXXXXX"));

   QCoreApplication::setApplicationName("");
}

TEST_CASE("QTemporaryFile fname_e", "[qtemporaryfile]")
{
   QTemporaryFile tmpFile("MyCsCoreTestXXXXXX.name");
   tmpFile.open();

   REQUIRE(tmpFile.fileName().contains("MyCsCoreTest"));
   REQUIRE(! tmpFile.fileName().contains("XXXXXX"));
   REQUIRE(tmpFile.fileName().endsWith(".name"));
}

TEST_CASE("QTemporaryFile file_template", "[qtemporaryfile]")
{
   QTemporaryFile tmpFile;
   bool ok = tmpFile.open();

   REQUIRE(ok == true);

   QString result = tmpFile.fileTemplate();

   REQUIRE(result.endsWith("cs_temp.XXXXXX"));
   REQUIRE(tmpFile.autoRemove() == true);
}

