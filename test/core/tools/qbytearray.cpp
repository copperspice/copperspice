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

#include <qbytearray.h>

#include <cs_catch2.h>

TEST_CASE("QByteArray traits", "[qbytearray]")
{
   REQUIRE(std::is_copy_constructible_v<QByteArray> == true);
   REQUIRE(std::is_move_constructible_v<QByteArray> == true);

   REQUIRE(std::is_copy_assignable_v<QByteArray> == true);
   REQUIRE(std::is_move_assignable_v<QByteArray> == true);

   REQUIRE(std::has_virtual_destructor_v<QByteArray> == false);
}

TEST_CASE("QByteArray append", "[qbytearray]")
{
   QByteArray str = "A wacky fox and sizeable pig";

   //
   str.append(" went to lunch");

   REQUIRE(str == "A wacky fox and sizeable pig went to lunch");

   //
   str.append('.');

   REQUIRE(str == "A wacky fox and sizeable pig went to lunch.");

   //
   QByteArray str_a = "A wacky fox";
   QByteArray str_b = " took a nap";

   str_a.append(str_b);

   REQUIRE(str_a == "A wacky fox took a nap");
}

TEST_CASE("QByteArray chop", "[qbytearray]")
{
   QByteArray str("CopperSpice");

   SECTION("a")
   {
      str.chop(5);

      REQUIRE(str == "Copper");
   }

   SECTION("b")
   {
      str.chop(100);

      REQUIRE(str.isEmpty() == true);
   }
}

TEST_CASE("QByteArray clear", "[qbytearray]")
{
   QByteArray str = "A wacky fox and sizeable pig jumped halfway over a blue moon.";

   REQUIRE(str.length()  == 61);
   REQUIRE(str.size()    == 61);

   REQUIRE(str.isEmpty() == false);
   REQUIRE(str.isNull()  == false);

   //
   str.clear();

   REQUIRE(str.length()  == 0);
   REQUIRE(str.size()    == 0);
   REQUIRE(str.isEmpty() == true);
   REQUIRE(str.isNull()  == true);
}

TEST_CASE("QByteArray comparisons", "[qbytearray]")
{
   QByteArray str1("apple");
   QByteArray str2("apple");
   QByteArray str3("banana");

   REQUIRE(str1 == str2);
   REQUIRE(str1 != str3);

   REQUIRE(str1 < str3);
   REQUIRE(str3 > str1);

   REQUIRE((str1 < str2) == false);
   REQUIRE((str1 > str2) == false);

   REQUIRE(QByteArray("abc") < QByteArray("abd"));
   REQUIRE(QByteArray("z")   > QByteArray("a"));
}

TEST_CASE("QByteArray compress", "[qbytearray]")
{
   QByteArray str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   QByteArray data   = qCompress(str);
   QByteArray output = qUncompress(data);

   REQUIRE(str == output);
}

TEST_CASE("QByteArray constructor", "[qbytearray]")
{
   SECTION("const char *")
   {
      QByteArray str("grape");

      REQUIRE(str.isEmpty()  == false);
      REQUIRE(str.isNull()   == false);

      REQUIRE(str.length()   == 5);
      REQUIRE(str.size()     == 5);
      REQUIRE(str.capacity() >= 5);

      REQUIRE(str == "grape");
   }

   SECTION("fill character")
   {
      QByteArray str(5, 'x');

      REQUIRE(str.isEmpty()  == false);
      REQUIRE(str.isNull()   == false);

      REQUIRE(str.length()   == 5);
      REQUIRE(str.size()     == 5);
      REQUIRE(str.capacity() >= 5);

      REQUIRE(str == "xxxxx");
   }

   SECTION("Construct from char*, length") {
      const char *data = "abcdef";
      QByteArray str(data, 3);

      REQUIRE(str.isEmpty()  == false);
      REQUIRE(str.isNull()   == false);
      REQUIRE(str.size()     == 3);

      REQUIRE(str == "abc");
   }

   SECTION("Empty array") {
      QByteArray str("");

      REQUIRE(str.isEmpty()  == true);
      REQUIRE(str.isNull()   == false);

      REQUIRE(str.length()   == 0);
      REQUIRE(str.size()     == 0);
      REQUIRE(str.capacity() >= 0);
   }
}

TEST_CASE("QByteArray copy_assign", "[qbytearray]")
{
   QByteArray str1("lemon");
   QByteArray str2(str1);

   QByteArray str3;
   str3 = str1;

   REQUIRE(str1.isEmpty()  == false);
   REQUIRE(str1.isNull()   == false);

   REQUIRE(str2.isEmpty()  == false);
   REQUIRE(str2.isNull()   == false);

   REQUIRE(str3.isEmpty()  == false);
   REQUIRE(str3.isNull()   == false);

   REQUIRE(str1 == str2);
   REQUIRE(str3 == str1);

   REQUIRE(! (str1 != str2));
}

TEST_CASE("QByteArray contains", "[qbytearray]")
{
   QByteArray str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   REQUIRE(str.contains("jumped") == true);
   REQUIRE(str.contains("lunch")  == false);
}

TEST_CASE("QByteArray conversions", "[qbytearray]")
{
   bool ok;

   SECTION("a")
   {
      int value = QByteArray("x99").toInt(&ok);
      REQUIRE(value == 0);
      REQUIRE(ok == false);

      REQUIRE(QByteArray("42").toInt(&ok) == 42);
      REQUIRE(ok == true);

      REQUIRE(QByteArray("-123").toLongLong(&ok) == -123);
      REQUIRE(ok == true);

      REQUIRE_THAT(QByteArray("3.14").toDouble(&ok), Catch::Matchers::WithinAbs(3.14, 0.0001));
      REQUIRE(ok == true);
   }

   SECTION("b")
   {
      REQUIRE(QByteArray::number(255, 10) == "255");
      REQUIRE(QByteArray::number(255, 16).toLower() == "ff");
   }
}

TEST_CASE("QByteArray count", "[qbytearray]")
{
   QByteArray str("banana");

   REQUIRE(str.count("a")  == 3);
   REQUIRE(str.count("na") == 2);
   REQUIRE(str.count("x")  == 0);

   REQUIRE(str.count() == str.size());
}

TEST_CASE("QByteArray data", "[qbytearray]")
{
   QByteArray str("CopperSpice");

   SECTION("a")
   {
     char *ptr = str.data();

     REQUIRE(ptr != nullptr);
     REQUIRE(std::string(ptr) == "CopperSpice");
   }

   SECTION("b")
   {
     char *ptr = str.data();
     ptr[0] = 'c';

     REQUIRE(str == "copperSpice");
   }
}

TEST_CASE("QByteArray encode_base64", "[qbytearray]")
{
   QByteArray original("hello world");

   QByteArray encoded = original.toBase64();
   QByteArray decoded = QByteArray::fromBase64(encoded);

   REQUIRE(encoded == "aGVsbG8gd29ybGQ=");
   REQUIRE(decoded == original);
}

TEST_CASE("QByteArray encode_hex", "[qbytearray]")
{
   QByteArray original("ABC");
   QByteArray encoded = original.toHex();

   REQUIRE(encoded == "414243");

   //
   QByteArray decoded = QByteArray::fromHex("414243");

   REQUIRE(decoded == "ABC");
}

TEST_CASE("QByteArray elements", "[qbytearray]")
{
   QByteArray str("abcdef");

   REQUIRE(str[0] == 'a');
   REQUIRE(str[5] == 'f');

   REQUIRE(str.at(1) == 'b');
   REQUIRE(str.at(4) == 'e');
}

TEST_CASE("QByteArray empty", "[qbytearray]")
{
   QByteArray str;

   REQUIRE(str.isEmpty()  == true);
   REQUIRE(str.isNull()   == true);

   REQUIRE(str.size()     == 0);
   REQUIRE(str.capacity() >= 0);

   //
   str.resize(0);

   REQUIRE(str.isEmpty() == true);
   REQUIRE(str.isNull()  == false);

   REQUIRE(str.size()    == 0);
}

TEST_CASE("QByteArray index", "[qbytearray]")
{
   QByteArray str("A wacky fox and sizeable pig jumped halfway over a blue moon");

   REQUIRE(str.indexOf("fox") == 8);
   REQUIRE(str.indexOf("cow") == -1);

   REQUIRE(str.lastIndexOf("f") == 39);
}

TEST_CASE("QByteArray insert", "[qbytearray]")
{
   QByteArray str("pear melon apple");

   //
   str.insert(5, "water");

   REQUIRE(str == "pear watermelon apple");

   //
   str.insert(0, ">>> ");

   REQUIRE(str == ">>> pear watermelon apple");

   //
   str.insert(25, '!');
   REQUIRE(str == ">>> pear watermelon apple!");

   str.insert(28, '?');
   REQUIRE(str == ">>> pear watermelon apple!  ?");
}

TEST_CASE("QByteArray left_right", "[qbytearray]")
{
   QByteArray str("watermelon");

   REQUIRE(str.left(3) == "wat");
   REQUIRE(str.left(0).isEmpty() == true);
   REQUIRE(str.left(15) == "watermelon");

   REQUIRE(str.right(2) == "on");
   REQUIRE(str.right(0).isEmpty() == true);
   REQUIRE(str.right(15) == "watermelon");
}

TEST_CASE("QByteArray length", "[qbytearray]")
{
   QByteArray str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   REQUIRE(str.length() == 60);
}

TEST_CASE("QByteArray mid", "[qbytearray]")
{
   QByteArray str("watermelon");

   REQUIRE(str.mid(0,  5) == "water");
   REQUIRE(str.mid(2,  2) == "te");
   REQUIRE(str.mid(5, 15) == "melon");

   REQUIRE(str.mid(5) == "melon");
   REQUIRE(str.mid(0) == "watermelon");

   REQUIRE(str.mid(15).isEmpty() == true);
   REQUIRE(str.mid(10, 5).isEmpty() == true);
   REQUIRE(str.mid(2, -1).isEmpty() == false);
}

TEST_CASE("QByteArray move_assign", "[qbytearray]")
{
   QByteArray str1("apple");
   QByteArray str2(std::move(str1));

   REQUIRE(str1.isEmpty() == true);

   REQUIRE(str2.isEmpty() == false);
   REQUIRE(str2 == "apple");
}

TEST_CASE("QByteArray prepend", "[qbytearray]")
{
   QByteArray str = "A wacky fox took a nap";

   //
   str.prepend("On Sunday, ");

   REQUIRE(str == "On Sunday, A wacky fox took a nap");

   //
   str.prepend('*');

   REQUIRE(str == "*On Sunday, A wacky fox took a nap");
}

TEST_CASE("QByteArray remove", "[qbytearray]")
{
   QByteArray str = "pear watermelon apple";

   //
   str.remove(5, 5);

   REQUIRE(str == "pear melon apple");

   //
   str.remove('p');

   REQUIRE(str == "ear melon ale");
}

TEST_CASE("QByteArray replace", "[qbytearray]")
{
   QByteArray str = "A wacky fox went to lunch";

   //
   str.replace(12, 13, "took a nap");

   REQUIRE(str == "A wacky fox took a nap");

   //
   str.replace("fox", "pig");

   REQUIRE(str == "A wacky pig took a nap");
}

TEST_CASE("QByteArray reserve", "[qbytearray]")
{
   QByteArray str("A wacky fox went to lunch");

   REQUIRE(str.size() == 25);

   //
   int oldCapacity = str.capacity();
   str.reserve(oldCapacity + 30);

   REQUIRE(str.size() == 25);
   REQUIRE(str.capacity() >= oldCapacity + 30);

   //
   str.squeeze();

   REQUIRE(str.capacity() == str.size());

   //
   str = QByteArray("A wacky fox went to lunch");
   str.reserve(100);

   REQUIRE(str.capacity() >= 100);

   //
   str.squeeze();

   REQUIRE(str.capacity() == str.size());
}

TEST_CASE("QByteArray resize", "[qbytearray]")
{
   QByteArray str("CopperSpice");

   //
   str.resize(15);

   REQUIRE(str.size() == 15);
   REQUIRE(str.length() == 15);

   REQUIRE(str[0]  == 'C');
   REQUIRE(str[10] == 'e');

   //
   str.resize(5);

   REQUIRE(str.size() == 5);
   REQUIRE(str.length() == 5);

   REQUIRE(str[0] == 'C');
   REQUIRE(str[4] == 'e');
}

TEST_CASE("QByteArray simplified", "[qbytearray]")
{
   QByteArray str("   a   b   c     ");

   REQUIRE(str.simplified() == "a b c");
}

TEST_CASE("QByteArray split", "[qbytearray]")
{
   QByteArray str("apple, pear, banana, orange");

   QList<QByteArray> parts = str.split(',');

   REQUIRE(parts.size() == 4);
   REQUIRE(parts[0] == "apple");
   REQUIRE(parts[3] == " orange");
}

TEST_CASE("QByteArray startsWith_endsWith", "[qbytearray]")
{
   QByteArray str("A wacky fox and sizeable pig jumped halfway over a blue moon");

   REQUIRE(str.startsWith('A') == true);
   REQUIRE(str.startsWith("A wacky")  == true);
   REQUIRE(str.startsWith(QByteArray("A wacky")) == true);

   REQUIRE(str.endsWith("moon") == true);
   REQUIRE(str.endsWith(QByteArray("blue moon")) == true);
}

TEST_CASE("QByteArray toLower", "[qbytearray]")
{
   QByteArray str = "Mango";

   str = str.toLower();

   REQUIRE(str == "mango");
}

TEST_CASE("QByteArray toUpper", "[qbytearray]")
{
   QByteArray str = "Mango";

   str = str.toUpper();

   REQUIRE(str == "MANGO");
}

TEST_CASE("QByteArray trimmed", "[qbytearray]")
{
   QByteArray str = "    A wacky fox and sizeable pig \n\t ";

   str = str.trimmed();

   REQUIRE(str == "A wacky fox and sizeable pig");
}

TEST_CASE("QByteArray truncate", "[qbytearray]")
{
   QByteArray str = "A wacky fox and sizeable pig jumped halfway over a blue moon";

   str.truncate(20);

   REQUIRE(str == "A wacky fox and size");
}
