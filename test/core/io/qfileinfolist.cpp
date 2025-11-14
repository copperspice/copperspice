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

#include <qdir.h>
#include <qfileinfo.h>
#include <qfileinfolist.h>
#include <qtemporaryfile.h>

#include <cs_catch2.h>

TEST_CASE("QFileInfoList traits", "[qfileinfolist]")
{
   REQUIRE(std::is_copy_constructible_v<QFileInfoList> == true);
   REQUIRE(std::is_move_constructible_v<QFileInfoList> == true);

   REQUIRE(std::is_copy_assignable_v<QFileInfoList> == true);
   REQUIRE(std::is_move_assignable_v<QFileInfoList> == true);

   REQUIRE(std::has_virtual_destructor_v<QFileInfoList> == false);
}

TEST_CASE("QFileInfoList contains", "[qfileinfolist]")
{
   QFileInfo fileA("testA.txt");
   QFileInfo fileB("testB.txt");

   QFileInfoList list;

   list.append(fileA);
   list.append(fileB);

   REQUIRE(list.contains(fileB) == true);
   REQUIRE(list.contains(QFileInfo("notfound.txt")) == false);
}

TEST_CASE("QFileInfoList clear", "[qfileinfolist]")
{
   QFileInfoList list;
   list << QFileInfo("a.txt") << QFileInfo("b.txt");

   REQUIRE(list.isEmpty() == false);
   REQUIRE(list.count() == 2);

   list.clear();

   REQUIRE(list.isEmpty() == true);
   REQUIRE(list.count() == 0);
}

TEST_CASE("QFileInfoList comparison", "[qfileinfolist]")
{
   QFileInfoList listA;

   listA.prepend(QFileInfo("fileB.txt"));
   listA.insert(1, QFileInfo("fileA.txt"));

   QFileInfoList listB = listA;

   REQUIRE(listA == listB);

   //
   listB.append(QFileInfo("file3.txt"));

   REQUIRE(listA != listB);
}

TEST_CASE("QFileInfoList constructor", "[qfileinfolist]")
{
   QFileInfoList list;

   REQUIRE(list.count() == 0);
   REQUIRE(list.isEmpty() == true);
}

TEST_CASE("QFileInfoList copy_assign", "[qfileinfolist]")
{
   QFileInfoList listA;
   listA.append(QFileInfo("test_copy.txt"));

   //
   QFileInfoList listB(listA);

   REQUIRE(listA.count() == 1);
   REQUIRE(listB.count() == 1);

   REQUIRE(listA.at(0).fileName() == "test_copy.txt");
   REQUIRE(listB.at(0).fileName() == "test_copy.txt");

   //
   QFileInfoList listC;
   listC = listA;

   REQUIRE(listA.count() == 1);
   REQUIRE(listB.count() == 1);
   REQUIRE(listC.count() == 1);

   REQUIRE(listA.at(0).fileName() == "test_copy.txt");
   REQUIRE(listB.at(0).fileName() == "test_copy.txt");
   REQUIRE(listC.at(0).fileName() == "test_copy.txt");
}

TEST_CASE("QFileInfoList index_a", "[qfileinfolist]")
{
   QTemporaryFile tmpFileA;
   tmpFileA.open();
   QFileInfo fileA(tmpFileA.fileName());

   QTemporaryFile tmpFileB;
   tmpFileB.open();
   QFileInfo fileB(tmpFileB.fileName());

   QTemporaryFile tmpFileC;
   tmpFileC.open();
   QFileInfo fileC(tmpFileC.fileName());

   //
   QFileInfoList list;

   list.append(fileA);
   list.append(fileB);
   list.append(fileC);

   REQUIRE(list.indexOf(fileA) == 0);
   REQUIRE(list.indexOf(fileB) == 1);
   REQUIRE(list.indexOf(fileC) == 2);
}

TEST_CASE("QFileInfoList index_b", "[qfileinfolist]")
{
   QFileInfo fileA("testA.txt");
   QFileInfo fileB("testB.txt");
   QFileInfo fileC("testC.txt");

   //
   QFileInfoList list;

   list.append(fileA);
   list.append(fileB);
   list.append(fileC);

   REQUIRE(list.indexOf(fileA) == 0);
   REQUIRE(list.indexOf(fileB) == 1);
   REQUIRE(list.indexOf(fileC) == 2);
}

TEST_CASE("QFileInfoList move_assign", "[qfileinfolist]")
{
   QFileInfoList listA;
   listA.append(QFileInfo("test_copy.txt"));

   //
   QFileInfoList listB(std::move(listA));

   REQUIRE(listB.count() == 1);
   REQUIRE(listB.at(0).fileName() == "test_copy.txt");

   //
   QFileInfoList listC;
   listC = std::move(listB);

   REQUIRE(listC.count() == 1);
   REQUIRE(listC.at(0).fileName() == "test_copy.txt");
}

TEST_CASE("QFileInfoList position", "[qfileinfolist]")
{
   QFileInfoList list;

   QFileInfo fileA("testA.txt");
   QFileInfo fileB("testB.txt");
   QFileInfo fileC("testC.txt");

   list.append(fileA);
   list.append(fileB);
   list.append(fileC);

   REQUIRE(list.at(0).fileName() == "testA.txt");
   REQUIRE(list.at(2).fileName() == "testC.txt");

   REQUIRE(list.first().fileName() == "testA.txt");
   REQUIRE(list.last().fileName()  == "testC.txt");
}

TEST_CASE("QFileInfoList remove", "[qfileinfolist]")
{
   QFileInfoList list;
   list << QFileInfo("testA.txt") << QFileInfo("testB.txt");

   REQUIRE(list.isEmpty() == false);
   REQUIRE(list.count() == 2);

   list.removeAt(0);

   REQUIRE(list.count() == 1);
   REQUIRE(list[0].fileName() == "testB.txt");
}

TEST_CASE("QFileInfoList sort", "[qfileinfolist]")
{
   QFileInfoList list;

   list.prepend(QFileInfo("orange.txt"));
   list.prepend(QFileInfo("apple.txt"));
   list.prepend(QFileInfo("cherry.txt"));
   list.prepend(QFileInfo("grape.txt"));

   std::sort(list.begin(), list.end(), [](const QFileInfo &a, const QFileInfo &b) {
      return a.fileName().toCaseFolded() < b.fileName().toCaseFolded();
   });

   REQUIRE(list[0].fileName() == "apple.txt");
   REQUIRE(list[1].fileName() == "cherry.txt");
   REQUIRE(list[2].fileName() == "grape.txt");
   REQUIRE(list[3].fileName() == "orange.txt");
}
