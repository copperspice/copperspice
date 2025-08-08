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

#include <qdiriterator.h>

#include <qfile.h>
#include <qtemporarydir.h>
#include <qtemporaryfile.h>

#include <cs_catch2.h>

TEST_CASE("QDirIterator traits", "[qdiriterator]")
{
   REQUIRE(std::is_copy_constructible_v<QDirIterator> == false);
   REQUIRE(std::is_move_constructible_v<QDirIterator> == false);

   REQUIRE(std::is_copy_assignable_v<QDirIterator> == false);
   REQUIRE(std::is_move_assignable_v<QDirIterator> == false);

   REQUIRE(std::has_virtual_destructor_v<QDirIterator> == false);
}

TEST_CASE("QDirIterator hasNext", "[qdiriterator]")
{
   QTemporaryDir dir;
   QString path = dir.path();

   QString fName1 = path + "/file1.txt";

   QTemporaryFile tmpFile1(fName1);
   tmpFile1.open();

   tmpFile1.write("CopperSpice - test file one");

   //
   QString fName2 = path + "/file2.txt";

   QTemporaryFile tmpFile2(fName2);
   tmpFile2.open();

   tmpFile2.write("CopperSpice - test file two");

   {
      QDirIterator iter(path, QDirIterator::Subdirectories);

      QStringList found;

      while (iter.hasNext()) {
         iter.next();

         QString tmpFName = iter.fileName();
         found << tmpFName;
      }

      REQUIRE(found.count() == 4);

      REQUIRE(found.contains("file1.txt") == false);
      REQUIRE(found.contains("file2.txt") == false);

      REQUIRE(found.contains(".") == true);
      REQUIRE(found.contains("..") == true);
   }

   {
      QDirIterator iter(path, QDir::Files, QDirIterator::Subdirectories);

      QStringList found;

      while (iter.hasNext()) {
         iter.next();

         QString tmpFName = iter.fileName();
         tmpFName.chop(7);

         if (tmpFName.endsWith(".txt")) {
            found << tmpFName;
         }
      }

      REQUIRE(found.count() == 2);

      REQUIRE(found.contains("file1.txt") == true);
      REQUIRE(found.contains("file2.txt") == true);
   }

   {
      QDirIterator iter(path, QDir::NoDotAndDotDot | QDir::Files, QDirIterator::Subdirectories);

      QStringList found;

      while (iter.hasNext()) {
         iter.next();

         QString tmpFName = iter.fileName();
         tmpFName.chop(7);

         if (tmpFName.endsWith(".txt")) {
            found << tmpFName;
         }
      }

      REQUIRE(found.count() == 2);

      REQUIRE(found.contains("file1.txt") == true);
      REQUIRE(found.contains("file2.txt") == true);
   }
}

TEST_CASE("QDirIterator name_filter", "[qdiriterator]")
{
   QTemporaryDir dir;
   QString path = dir.path();

   QString fName1 = path + "/file1.txt";

   QTemporaryFile tmpFile1(fName1);
   tmpFile1.open();

   tmpFile1.write("CopperSpice - test file one");

   //
   QString fName2 = path + "/file2.txt";

   QTemporaryFile tmpFile2(fName2);
   tmpFile2.open();

   tmpFile2.write("CopperSpice - test file two");

   //
   QStringList nameFilters;
   nameFilters << "*.txt*";

   QDirIterator iter(path, nameFilters, QDir::Files, QDirIterator::Subdirectories);

   QStringList found;

   while (iter.hasNext()) {
      iter.next();

      QString tmpFName = iter.fileName();
      tmpFName.chop(7);

      if (tmpFName.endsWith(".txt")) {
         found << tmpFName;
      }
   }

   REQUIRE(found.count() == 2);

   REQUIRE(found.contains("file1.txt") == true);
   REQUIRE(found.contains("file2.txt") == true);
}
