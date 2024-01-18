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

#include <qmetaobject.h>

#include <cs_catch2.h>

TEST_CASE("QMetaObject traits", "[qmetaobject]")
{
   REQUIRE(std::is_copy_constructible_v<QMetaObject> == false);
   REQUIRE(std::is_move_constructible_v<QMetaObject> == false);

   REQUIRE(std::is_copy_assignable_v<QMetaObject> == true);
   REQUIRE(std::is_move_assignable_v<QMetaObject> == true);

   REQUIRE(std::has_virtual_destructor_v<QMetaObject> == true);
}

namespace metaObjTest {

class Ginger : public QObject
{
   CS_OBJECT(Ginger)

   CS_ENUM(Spices)

   // title is type QString
   CS_PROPERTY_READ(title,        getTitle)
   CS_PROPERTY_WRITE(title,       setTitle)
   CS_PROPERTY_RESET(title,       resetTitle)
   CS_PROPERTY_NOTIFY(title,      titleChanged)
   CS_PROPERTY_REVISION(title,    31415)
   CS_PROPERTY_DESIGNABLE(title,  true)
   CS_PROPERTY_SCRIPTABLE(title,  isScriptTitle())
   CS_PROPERTY_STORED(title,      false)
   CS_PROPERTY_USER(title,        100 > 10)
   CS_PROPERTY_CONSTANT(title)
   CS_PROPERTY_FINAL(title)

   // favorite is type enum Spices
   CS_PROPERTY_READ(favorite,     getFavorite)
   CS_PROPERTY_WRITE(favorite,    setFavorite)

   public:
      enum Spices { mint, basil, Salt, Pepper =100, cloves };

      // (1) methods for title property
      QString getTitle() const {
         return m_title;
      }

      void setTitle(QString data) {
         m_title = data;
         emit titleChanged(m_title);
      }

      void resetTitle() {
         m_title.clear();
      }

      CS_SIGNAL_1(Public, void titleChanged(QString title))
      CS_SIGNAL_2(titleChanged, title)

      CS_SLOT_1(Public, void newTitle(QString))
      CS_SLOT_2(newTitle)

      // methods for favorite property
      Spices getFavorite() const {
         return m_favorite;
      }

      void setFavorite(Spices data) {
         m_favorite = data;
      }

   private:
      QString m_title;        // property
      Spices m_favorite;      // property

      static bool isScriptTitle() {
         return true;
      }
};

void Ginger::newTitle(QString value)
{
   m_title = value;
}

}

TEST_CASE("QMetaObject enums", "[qmetaobject]")
{
   metaObjTest::Ginger obj;

   const QMetaObject &metaObj = metaObjTest::Ginger::staticMetaObject();

   REQUIRE(metaObj.enumeratorCount() == 1);

   {
      QMetaEnum metaEnum = metaObj.enumerator(metaObj.indexOfEnumerator("SomeEnum"));

      REQUIRE(metaObj.indexOfEnumerator("SomeEnum") == -1);
      REQUIRE(metaEnum.isValid() == false);
   }

   {
      QMetaEnum metaEnum = metaObj.enumerator(metaObj.indexOfEnumerator("Spices"));

      REQUIRE(metaObj.indexOfEnumerator("Spices") >= 0);
      REQUIRE(metaEnum.isValid() == true);
   }
}

TEST_CASE("QMetaObject is_as", "[qmetaobject]")
{
   metaObjTest::Ginger obj;

   const QMetaObject &metaObj = metaObjTest::Ginger::staticMetaObject();

   {
      QMetaProperty metaProp = metaObj.property(metaObj.indexOfProperty("title"));

      REQUIRE(metaProp.isReadable() == true);
      REQUIRE(metaProp.isWritable() == true);
      REQUIRE(metaProp.isResettable() == true);
      REQUIRE(metaProp.hasNotifySignal() == true);
      REQUIRE(metaProp.revision() == 31415);
      REQUIRE(metaProp.isDesignable() == true);
      REQUIRE(metaProp.isScriptable() == true);
      REQUIRE(metaProp.isStored() == false);
      REQUIRE(metaProp.isUser() == true);
      REQUIRE(metaProp.isConstant() == true);
      REQUIRE(metaProp.isFinal() == true);
   }

   {
      QMetaProperty metaProp = metaObj.property(metaObj.indexOfProperty("favorite"));

      REQUIRE(metaProp.isReadable() == true);
      REQUIRE(metaProp.isWritable() == true);
      REQUIRE(metaProp.isResettable() == false);
      REQUIRE(metaProp.hasNotifySignal() == false);
      REQUIRE(metaProp.revision() == 0);
      REQUIRE(metaProp.isDesignable() == true);
      REQUIRE(metaProp.isScriptable() == true);
      REQUIRE(metaProp.isStored() == true);
      REQUIRE(metaProp.isUser() == false);
      REQUIRE(metaProp.isConstant() == false);
      REQUIRE(metaProp.isFinal() == false);
   }
}

TEST_CASE("QMetaObject isValid", "[qmetaobject]")
{
   metaObjTest::Ginger obj;

   const QMetaObject &metaObj = metaObjTest::Ginger::staticMetaObject();

   REQUIRE(metaObj.className() == "Ginger");
   REQUIRE(metaObj.superClass()->className() == "QObject");
}

TEST_CASE("QMetaObject property", "[qmetaobject]")
{
   metaObjTest::Ginger obj;

   const QMetaObject &metaObj = metaObjTest::Ginger::staticMetaObject();

   // name is from QObject, two from Ginger
   REQUIRE(metaObj.propertyCount() == 3);

   {
      QMetaProperty metaProp = metaObj.property(metaObj.indexOfProperty("SomeProperty"));

      REQUIRE(metaObj.indexOfProperty("SomeProperty") == -1);
      REQUIRE(metaProp.isValid() == false);

      REQUIRE(metaProp.type() == QVariant::Invalid);
      REQUIRE(metaProp.userType() == QVariant::Invalid);

      REQUIRE(metaProp.typeName() == "");
   }

   {
      QMetaProperty metaProp = metaObj.property(metaObj.indexOfProperty("title"));

      REQUIRE(metaObj.indexOfProperty("title") >= 0);
      REQUIRE(metaProp.isValid() == true);

      REQUIRE(metaProp.type() == QVariant::String);
      REQUIRE(metaProp.userType() == QVariant::String);
      REQUIRE(metaProp.typeName() == "QString8");
   }
}

TEST_CASE("QMetaObject signature", "[qmetaobject]")
{
   {
      QString data = "mySlot(int const &) const";
      REQUIRE(QMetaObject::normalizedSignature(data) == "mySlot(int&)");
   }

  {
      QString data = "clearSelection(bool display_A = true)";
      REQUIRE(QMetaObject::normalizedSignature(data) == "clearSelection(bool)");
   }

   {
      QString data = "clearSelection(bool display_B = true)=0";
      REQUIRE(QMetaObject::normalizedSignature(data) == "clearSelection(bool)");
   }
}
