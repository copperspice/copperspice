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

#include <qstring8.h>
#include <qobject.h>
#include <qvariant.h>

#include <cs_catch2.h>

TEST_CASE("QObject traits", "[qobject]")
{
   REQUIRE(std::is_copy_constructible_v<QObject> == false);
   REQUIRE(std::is_move_constructible_v<QObject> == false);

   REQUIRE(std::is_copy_assignable_v<QObject> == false);
   REQUIRE(std::is_move_assignable_v<QObject> == false);

   REQUIRE(std::has_virtual_destructor_v<QObject> == true);
}

class Ginger : public QObject
{
   CS_OBJECT(Ginger)

   public:
      void actionA(QString);

      void bags(int value);
      void bags(bool value, int offset);
      void bags(int value) const;
      void bags();

      // set up for overloaded signal
      CS_SIGNAL_1(Public, void cargo(bool var1, int var2))
      CS_SIGNAL_OVERLOAD(cargo, (bool, int), var1, var2)

      CS_SIGNAL_1(Public, void cargo(int var))
      CS_SIGNAL_OVERLOAD(cargo, (int), var)

      CS_SIGNAL_1(Public, void cargo())
      CS_SIGNAL_OVERLOAD(cargo, ())

      CS_SIGNAL_1(Public, void titleChanged(QString str))
      CS_SIGNAL_2(titleChanged, str)

      QString m_titleA = "Title A (Original)";
      QString m_titleB = "Title B";

      mutable int m_bags = 0;

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

void Ginger::bags(int value)
{
   m_bags = value + 5;
}

void Ginger::bags()
{
   m_bags = -1;
}

void Ginger::bags(bool value, int offset)
{
   if (value) {
      m_bags = 1 + offset;
   } else {
      m_bags = 0 + offset;
   }
}

void Ginger::bags(int value) const
{
   m_bags = value + 10;
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

TEST_CASE("QObject property", "[qobject]")
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

TEST_CASE("QObject method_overload", "[qobject]")
{
   Ginger obj;

   // overload 1
   QObject::connect(&obj, cs_mp_cast<int>(&Ginger::cargo), &obj, cs_mp_cast<int>(&Ginger::bags));

   obj.cargo(8);
   REQUIRE(obj.m_bags == 13);

   obj.cargo(17);
   REQUIRE(obj.m_bags == 22);


   // overload 2
   QObject::connect(&obj, cs_mp_cast<bool, int>(&Ginger::cargo), &obj, cs_mp_cast<bool, int>(&Ginger::bags));

   obj.cargo(true, 15);
   REQUIRE(obj.m_bags == 16);

   obj.cargo(false, 3);
   REQUIRE(obj.m_bags == 3);


   // overload 3a
   QObject::connect(&obj, cs_mp_cast<int>(&Ginger::cargo), &obj, cs_mp_cast<int>(&Ginger::bags));

   obj.cargo(9);
   REQUIRE(obj.m_bags == 14);

   obj.cargo(13);
   REQUIRE(obj.m_bags == 18);


   // overload 3b, slot is const
   QObject::connect(&obj, cs_mp_cast<int>(&Ginger::cargo), &obj, cs_cmp_cast<int>(&Ginger::bags));

   obj.cargo(2);
   REQUIRE(obj.m_bags == 12);

   obj.cargo(5);
   REQUIRE(obj.m_bags == 15);


   // overload 4
   QObject::connect(&obj, cs_mp_cast<>(&Ginger::cargo), &obj, cs_mp_cast<>(&Ginger::bags));

   obj.cargo();
   REQUIRE(obj.m_bags == -1);
}
