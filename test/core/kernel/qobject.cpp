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

#include <qstring8.h>
#include <qobject.h>
#include <qvariant.h>

#include <cs_catch2.h>

class Ginger : public QObject
{
   CS_OBJECT(Ginger)

   public:
      void actionA(QString);

      CS_SIGNAL_1(Public, void titleChanged(QString str))
      CS_SIGNAL_2(titleChanged, str)

      QString m_titleA = "Title A (Original)";
      QString m_titleB = "Title B";

   private:
      CS_SLOT_1(Private, void actionB(QString value))
      CS_SLOT_2(actionB)
};

void Ginger::actionA(QString value)
{
   m_titleA += " " + value;
}

void Ginger::actionB(QString value)
{
   m_titleB = value;
}

TEST_CASE("QObject children", "[qobject]")
{
   Ginger obj;

   REQUIRE(obj.children().isEmpty() == true);
   REQUIRE(obj.parent() == nullptr);
}

TEST_CASE("QObject connect", "[qobject]")
{
   Ginger obj;

   QObject::connect(&obj, &Ginger::titleChanged,         &obj, &Ginger::actionA);
   QObject::connect(&obj, SIGNAL(titleChanged(QString)), &obj, SLOT(actionB(QString)));

   REQUIRE(obj.m_titleA == "Title A (Original)");
   REQUIRE(obj.m_titleB == "Title B");

   obj.titleChanged("New Title");

   REQUIRE(obj.m_titleA == "Title A (Original) New Title");
   REQUIRE(obj.m_titleB == "New Title");
}

TEST_CASE("QObject inherits", "[qobject]")
{
   Ginger obj;

   REQUIRE(obj.inherits("QObject") == true);
   REQUIRE(obj.inherits("Ginger") == true);
   REQUIRE(obj.inherits("QFile") == false);

   REQUIRE(obj.isWidgetType() == false);
   REQUIRE(obj.isWindowType() == false);
}

TEST_CASE("QObject object_name", "[qobject]")
{
   Ginger obj;

   REQUIRE(obj.objectName().isEmpty() == true);

   obj.setObjectName("SomeObject");
   REQUIRE(obj.objectName() == "SomeObject");

   REQUIRE(obj.property<QString>("objectName") == "SomeObject");
}

TEST_CASE("QObject propertt", "[qobject]")
{
   Ginger obj;

   REQUIRE(obj.dynamicPropertyNames().isEmpty() == true);

   obj.setProperty("newProperty", QVariant(17));

   REQUIRE(obj.property<int>("newProperty") == 17);

   {
      QList<QString> list = obj.dynamicPropertyNames();
      REQUIRE(list.size() == 1);
      REQUIRE(list.contains("newProperty") == true);
   }
}

