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

#include <qmetamethod.h>

#include <cs_catch2.h>

TEST_CASE("QMetaMethod traits", "[qmetamethod]")
{
   REQUIRE(std::is_copy_constructible_v<QMetaMethod> == true);
   REQUIRE(std::is_move_constructible_v<QMetaMethod> == true);

   REQUIRE(std::is_copy_assignable_v<QMetaMethod> == true);
   REQUIRE(std::is_move_assignable_v<QMetaMethod> == true);

   REQUIRE(std::has_virtual_destructor_v<QMetaMethod> == false);
}

class Ginger_MM : public QObject
{
   CS_OBJECT(Ginger_MM)

   public:
      CS_INVOKABLE_METHOD_1(Public, QStringView someMethodName(QString, bool))
      CS_INVOKABLE_METHOD_2(someMethodName)

      CS_SIGNAL_1(Public, void titleChanged(QString str))
      CS_SIGNAL_2(titleChanged, str)

   private:
      CS_SLOT_1(Private, void actionB(const QString &value))
      CS_SLOT_2(actionB)
};

QStringView Ginger_MM::someMethodName(QString, bool)
{
   return QStringView();
}

void Ginger_MM::actionB(const QString &)
{
}


TEST_CASE("QMetaMethod access_methods", "[qmetamethod]")
{
   Ginger_MM obj;

   const QMetaObject &metaObj = Ginger_MM::staticMetaObject();

   {
      QMetaMethod method;
      REQUIRE(method.isValid() == false);
      REQUIRE(method.name().isEmpty());
   }

   {
      QMetaMethod method = metaObj.method(0);

      REQUIRE(method.isValid() == true);
      REQUIRE(Ginger_MM::staticMetaObject().methodCount() == 8);

      REQUIRE(Ginger_MM::staticMetaObject().indexOfSignal("titleChanged(QString)") == 0);
      REQUIRE(! method.name().isEmpty());
      REQUIRE(method.access() == QMetaMethod::Public);
      REQUIRE(method.methodType() == QMetaMethod::Signal);
   }

   {
      QMetaMethod method = metaObj.method(metaObj.indexOfMethod("actionB(const QString &)"));

      REQUIRE(method.isValid() == true);
      REQUIRE(method.name() == "actionB");
      REQUIRE(method.parameterCount() == 1);
      REQUIRE(method.access() == QMetaMethod::Private);
      REQUIRE(method.methodType() == QMetaMethod::Slot);
      REQUIRE(method.typeName() == "void");
   }
}

TEST_CASE("QMetaMethod method", "[qmetamethod]")
{
   QString signature = "someMethodName(QString,bool)";

   Ginger_MM obj;

   const QMetaObject &metaObj = Ginger_MM::staticMetaObject();

   int index = metaObj.indexOfMethod(signature);
   QMetaMethod method = metaObj.method(index);

   REQUIRE(index != -1);

   REQUIRE(method.isValid() == true);
   REQUIRE(method.name() == "someMethodName");
   REQUIRE(method.parameterCount() == 2);
   REQUIRE(method.access() == QMetaMethod::Public);
   REQUIRE(method.methodType() == QMetaMethod::Method);
   REQUIRE(method.typeName() == "QStringView");
}

TEST_CASE("QMetaMethod invalid_signature", "[qmetamethod]")
{
   Ginger_MM obj;

   const QMetaObject &metaObj = Ginger_MM::staticMetaObject();

   // signature is invalid on purpose (missing &)
   QMetaMethod method = metaObj.method(metaObj.indexOfMethod("actionB(const QString)"));

   REQUIRE(method.isValid() == false);
}
