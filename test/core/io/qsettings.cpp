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

#include <qdir.h>
#include <qfileinfo.h>
#include <qsize.h>
#include <qsettings.h>
#include <qstandardpaths.h>
#include <qstringlist.h>

#include <cs_catch2.h>

TEST_CASE("QSettings traits", "[qsettings]")
{
   REQUIRE(std::is_copy_constructible_v<QSettings> == false);
   REQUIRE(std::is_move_constructible_v<QSettings> == false);

   REQUIRE(std::is_copy_assignable_v<QSettings> == false);
   REQUIRE(std::is_move_assignable_v<QSettings> == false);

   REQUIRE(std::has_virtual_destructor_v<QSettings> == true);
}

TEST_CASE("QSettings name", "[qsettings]")
{
   QSettings data("CopperSpice Test", "CsCoreTest");

   REQUIRE(data.organizationName() == "CopperSpice Test");
   REQUIRE(data.applicationName()  == "CsCoreTest");
}

TEST_CASE("QSettings allkeys", "[qsettings]")
{
   QMap<QString, QVariant> map;
   map.insert("50", QString("duck"));
   map.insert("10", QString("chicken"));
   map.insert("20", QString("fish"));

   QSettings settings1("CopperSpice Test", "CsCoreTest");

   // user
   settings1.setValue("room_size",  QSize(10, 16));
   settings1.setValue("sofa",       false);
   settings1.setValue("tv",         true);
   settings1.setValue("table",      2);
   settings1.setValue("fruit",      QStringList() << "grape" << "apple" << "pear");
   settings1.setValue("food",       map);
   settings1.sync();

   // global
   QSettings settings2("CopperSpice Test");

   settings2.setValue("catchTest",  true);
   settings2.sync();

   {
      QStringList keys = settings1.allKeys();

      REQUIRE(keys.contains("room_size") == true);
      REQUIRE(keys.contains("sofa") == true);
      REQUIRE(keys.contains("table") == true);

      REQUIRE(keys.contains("catchTest") == true);

#if ! defined(Q_OS_DARWIN)
      // size can vary
      REQUIRE(keys.size() == 7);
#endif

      REQUIRE(settings1.value("room_size") == QSize(10, 16));
      REQUIRE(settings1.value("sofa") == false);
      REQUIRE(settings1.value("table") == 2);
   }

   {
      QStringList list = settings1.value("fruit").toStringList();

      REQUIRE(list.contains("pear") == true);
      REQUIRE(list.contains("grape") == true);
      REQUIRE(list.contains("peach") == false);
   }

   {
#if defined(Q_OS_DARWIN)
      // unusual
      QMultiMap<QString, QVariant> result = settings1.value("food").toMultiMap();

#else
      REQUIRE(settings1.value("food").toMap() == map);
      QMap<QString, QVariant> result = settings1.value("food").toMap();

#endif

      REQUIRE(result.size() == 3);

      REQUIRE(result.value("50").toString() == "duck");
      REQUIRE(result.value("10").toString() == "chicken");
   }

   {
      settings1.setFallbacksEnabled(false);

      QStringList keys = settings1.allKeys();

      REQUIRE(keys.contains("room_size"));
      REQUIRE(keys.contains("sofa"));

      REQUIRE(keys.contains("catchTest") == false);

      REQUIRE(keys.size() == 6);
   }

   settings1.remove("room_size");
   settings1.remove("sofa");
   settings1.remove("tv");
   settings1.remove("table");
   settings1.remove("fruit");
   settings1.remove("food");
}

TEST_CASE("QSettings filename", "[qsettings]")
{
   QCoreApplication::setOrganizationName("CopperSpice");
   QCoreApplication::setApplicationName("CsCoreTest");

   REQUIRE(QCoreApplication::applicationName() == "CsCoreTest");

   QString targetLocation = QDir::tempPath();

   // A
   QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, targetLocation);
   QSettings data1(targetLocation + "/TestSettings.ini", QSettings::IniFormat);

   QFileInfo fileInfo1 = data1.fileName();

   REQUIRE(fileInfo1.fileName() == "TestSettings.ini");
   REQUIRE(fileInfo1.path() == targetLocation);

   // B
   QSettings data2(QSettings::IniFormat, QSettings::UserScope, "CopperSpice");

   QFileInfo fileInfo2 = data2.fileName();

   REQUIRE(fileInfo2.fileName() == "CopperSpice.ini");
   REQUIRE(fileInfo2.path() == targetLocation);

   QCoreApplication::setOrganizationName("");
   QCoreApplication::setApplicationName("");
}
