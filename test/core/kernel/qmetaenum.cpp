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

#include <csmeta.h>
#include <qnamespace.h>

#include <cs_catch2.h>

TEST_CASE("QMetaEnum enum_count_a", "[qmetaenum]")
{
   const QMetaObject &metaObject = Qt::staticMetaObject();

   int index = metaObject.indexOfEnumerator("Orientation");
   QMetaEnum enumObj = metaObject.enumerator(index);

   REQUIRE(enumObj.isValid() == true);

   REQUIRE(enumObj.name()   == "Orientation");
   REQUIRE(enumObj.scope()  == "Qt");
   REQUIRE(enumObj.isFlag() == false);

   REQUIRE(enumObj.keyCount() == 2);

   // keys are in a map, in alphabetical order
   REQUIRE(enumObj.key(0)   == "Horizontal");
   REQUIRE(enumObj.value(0) == 1);

   REQUIRE(enumObj.key(1)   == "Vertical");
   REQUIRE(enumObj.value(1) == 2);
}

TEST_CASE("QMetaEnum enum_count_b", "[qmetaenum]")
{
   const QMetaObject &metaObject = Qt::staticMetaObject();

   int index = metaObject.indexOfEnumerator("AlignmentFlag");
   QMetaEnum enumObj = metaObject.enumerator(index);

   REQUIRE(enumObj.isValid() == true);

   REQUIRE(enumObj.name()   == "AlignmentFlag");
   REQUIRE(enumObj.scope()  == "Qt");
   REQUIRE(enumObj.isFlag() == false);

   REQUIRE(enumObj.keyCount() == 14);

   REQUIRE(enumObj.keyToValue("AlignLeft")    == 1);
   REQUIRE(enumObj.keyToValue("AlignRight")   == 2);
   REQUIRE(enumObj.keyToValue("AlignVCenter") == 128);
   REQUIRE(enumObj.keyToValue("AlignHCenter") == 4);
   REQUIRE(enumObj.keyToValue("AlignTop")     == 32);
   REQUIRE(enumObj.keyToValue("AlignBottom")  == 64);

   {
      // part 2
      int index = metaObject.indexOfEnumerator("Alignment");
      QMetaEnum enumObj = metaObject.enumerator(index);

      REQUIRE(enumObj.isValid() == true);

      REQUIRE(enumObj.name()   == "Alignment");
      REQUIRE(enumObj.scope()  == "Qt");
      REQUIRE(enumObj.isFlag() == true);

      REQUIRE(enumObj.keyToValue("AlignTop")    == 32);
      REQUIRE(enumObj.keyToValue("AlignBottom") == 64);
   }
}

TEST_CASE("QMetaEnum enum_count_c", "[qmetaenum]")
{
   // disable QWarning() for this test
   csInstallMsgHandler([](QtMsgType, QStringView){ });

   const QMetaObject &metaObject = Qt::staticMetaObject();

   int index = metaObject.indexOfEnumerator("SortOrder");
   QMetaEnum enumObj = metaObject.enumerator(index);

   REQUIRE(enumObj.isValid() == true);

   REQUIRE(enumObj.name()   == "SortOrder");
   REQUIRE(enumObj.scope()  == "Qt");
   REQUIRE(enumObj.isFlag() == false);

   // enum values are *not* registered
   REQUIRE(enumObj.keyCount() == 0);

   REQUIRE(enumObj.key(0)   == "");
   REQUIRE(enumObj.value(0) == -1);

   csInstallMsgHandler(nullptr);
}

TEST_CASE("QMetaEnum enum_count_d", "[qmetaenum]")
{
   const QMetaObject &metaObject = Qt::staticMetaObject();

   int index = metaObject.indexOfEnumerator("FocusPolicy");
   QMetaEnum enumObj = metaObject.enumerator(index);

   REQUIRE(enumObj.isValid() == true);

   REQUIRE(enumObj.name()   == "FocusPolicy");
   REQUIRE(enumObj.scope()  == "Qt");
   REQUIRE(enumObj.isFlag() == false);

   REQUIRE(enumObj.keyCount() == 5);

   REQUIRE(enumObj.keyToValue("NoFocus")    == 0);
   REQUIRE(enumObj.keyToValue("TabFocus")   == 1);
   REQUIRE(enumObj.keyToValue("ClickFocus") == 2);
}

TEST_CASE("QMetaEnum flag_value", "[qmetaenum]")
{
   const QMetaObject &metaObject = Qt::staticMetaObject();

   int index = metaObject.indexOfEnumerator("InputMethodHint");
   QMetaEnum enumObj = metaObject.enumerator(index);

   REQUIRE(enumObj.keyToValue("ImhNone") == 0);
   REQUIRE(enumObj.keyToValue("ImhDate") == 0x80);
   REQUIRE(enumObj.keyToValue("ImhTime") == 0x100);

   REQUIRE(enumObj.keyToValue("ImhLatinOnly")          == 0x800000);
   REQUIRE(enumObj.keyToValue("ImhExclusiveInputMask") == static_cast<int>(0xffff0000));
}
