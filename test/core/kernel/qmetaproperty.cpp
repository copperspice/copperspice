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

// must be first
#include <qobject.h>

#include <qmetaproperty.h>
#include <qnamespace.h>

#include <cs_catch2.h>

class Ginger_MP : public QObject
{
   CS_OBJECT(Ginger_MP)

   CS_PROPERTY_READ(flavor,  getFlavor)
   CS_PROPERTY_WRITE(flavor, setFlavor)

   CS_PROPERTY_READ(shape,   getCursorShape)
   CS_PROPERTY_DESIGNABLE(shape, false)
   CS_PROPERTY_SCRIPTABLE(shape, true)
   CS_PROPERTY_STORED(shape, false)

   CS_ENUM(Spices)

   public:
      enum Spices {
         basil,
         mint,
         pepper,
         thyme,
      };

      Spices getFlavor() const {
         return m_flavor;
      }

      void setFlavor(Spices data) {
         m_flavor = data;
      }

      Qt::CursorShape getCursorShape() const {
         return Qt::BusyCursor;
      }

   private:
     Spices m_flavor;
};

TEST_CASE("QMetaProperty enumerator", "[qmetaproperty]")
{
   Ginger_MP obj;

   const QMetaObject &metaObj = Ginger_MP::staticMetaObject();

   int index = metaObj.indexOfProperty("flavor");
   QMetaProperty prop = metaObj.property(index);

   REQUIRE(prop.isValid() == true);

   QMetaEnum tmpEnum = prop.enumerator();

   REQUIRE(tmpEnum.isValid() == true);
   REQUIRE(tmpEnum.name() == "Spices");

   REQUIRE(prop.isDesignable() == true);
}

TEST_CASE("QMetaProperty external_enum", "[qmetaproperty]")
{
   Ginger_MP obj;

   const QMetaObject &metaObj = Ginger_MP::staticMetaObject();

   int index = metaObj.indexOfProperty("shape");
   REQUIRE(index >= 0);

   QMetaProperty prop = metaObj.property(index);
   REQUIRE(prop.isValid() == true);
   REQUIRE(prop.name() == "shape");

   QMetaEnum tmpEnum = prop.enumerator();

   REQUIRE(tmpEnum.isValid() == true);
   REQUIRE(tmpEnum.name() == "CursorShape");

   REQUIRE(prop.isDesignable() == false);
   REQUIRE(prop.isScriptable() == true);
   REQUIRE(prop.isStored() == false);
}

TEST_CASE("QMetaProperty type_name", "[qmetaproperty]")
{
   Ginger_MP obj;

   const QMetaObject &metaObj = Ginger_MP::staticMetaObject();

   int index = metaObj.indexOfProperty("flavor");
   QMetaProperty prop = metaObj.property(index);

   QString name = prop.name();

   REQUIRE(prop.isValid() == true);
   REQUIRE(prop.isReadable() == true);
   REQUIRE(prop.isWritable() == true);

   REQUIRE(prop.typeName() == "Ginger_MP::Spices");
   REQUIRE(name == "flavor");
}

