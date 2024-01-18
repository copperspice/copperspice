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

#define CS_STRING_ALLOW_UNSAFE

#include <cs_string.h>

#include <cs_catch2.h>

// ** utf-8
TEST_CASE("CsString_utf8 u8_append", "[cs_string]")
{
   CsString::CsString_utf8 str = u8"A wacky fox and sizeable pig";

   str.append(u8" went to lunch");

   REQUIRE(str == u8"A wacky fox and sizeable pig went to lunch");
}

TEST_CASE("CsString_utf8 u8_length", "[cs_string]")
{
   CsString::CsString_utf8 str = u8"!ä";

   REQUIRE(str.length() ==  2);

   REQUIRE(str[0].unicode() == char32_t(33));
   REQUIRE(str[1].unicode() == char32_t(228));
}

TEST_CASE("CsString_utf8 u8_storage_iterators", "[cs_string]")
{
   CsString::CsString_utf8 str;

   REQUIRE(str.storage_begin()  == str.storage_end());
   REQUIRE(str.storage_rbegin() == str.storage_rend());

   str = u8"grape";
   REQUIRE(*str.storage_begin()  == u8'g');
   REQUIRE(*str.storage_rbegin() == u8'e');
   REQUIRE(str.storage_end() - str.storage_begin() == 5);

   // unicode 21B4, three storage units
   CsString::CsString_utf8 arrow(1, U'↴');

   str = u8"grape" + arrow;

   REQUIRE(*(str.storage_end() - 4) == u8'e');
   REQUIRE(*(str.storage_rbegin() + 3) == u8'e');
}

TEST_CASE("CsString_utf8 u8_constructor", "[cs_string]")
{
   CsString::CsString_utf8 str = u8"On a clear day you can see forever";
   REQUIRE(str == u8"On a clear day you can see forever");
}


// ** utf-16
TEST_CASE("CsString_utf16 u8_append", "[cs_string]")
{
   CsString::CsString_utf16 str = u8"A wacky fox and sizeable pig";

   str.append(u8" went to lunch");

   REQUIRE(str == u8"A wacky fox and sizeable pig went to lunch");
}

TEST_CASE("CsString_utf16 u8_length", "[cs_string]")
{
   CsString::CsString_utf16 str = u8"!ä";

   REQUIRE(str.length() ==  2);

   REQUIRE(str[0].unicode() == char32_t(33));
   REQUIRE(str[1].unicode() == char32_t(228));
}

TEST_CASE("CsString_utf16 u8_storage_iterators", "[cs_string]")
{
   CsString::CsString_utf16 str;

   REQUIRE(str.storage_begin()  == str.storage_end());
   REQUIRE(str.storage_rbegin() == str.storage_rend());

   str = u8"grape";
   REQUIRE(*str.storage_begin()  == u'g');
   REQUIRE(*str.storage_rbegin() == u'e');
   REQUIRE(str.storage_end() - str.storage_begin() == 5);

   // unicode 21B4, one storage units
   CsString::CsString_utf16 arrow(1, U'↴');

   str = u8"grape" + arrow;

   REQUIRE(*(str.storage_end() - 2) == u'e');
   REQUIRE(*(str.storage_rbegin() + 1) == u'e');
}

TEST_CASE("CsString_utf16 u8_constructor", "[cs_string]")
{
   CsString::CsString_utf16 str = u8"On a clear day you can see forever";
   REQUIRE(str == u8"On a clear day you can see forever");
}


// ** utf-16, u literal prefix
TEST_CASE("CsString_utf16 u_append", "[cs_string]")
{
   CsString::CsString_utf16 str = u"A wacky fox and sizeable pig";

   str.append(u" went to lunch");

   REQUIRE(str == u"A wacky fox and sizeable pig went to lunch");
}

TEST_CASE("CsString_utf16 u_length", "[cs_string]")
{
   CsString::CsString_utf16 str = u"!ä";

   REQUIRE(str.length() ==  2);

   REQUIRE(str[0].unicode() == char32_t(33));
   REQUIRE(str[1].unicode() == char32_t(228));
}

TEST_CASE("CsString_utf16 u_storage_iterators", "[cs_string]")
{
   CsString::CsString_utf16 str;

   REQUIRE(str.storage_begin()  == str.storage_end());
   REQUIRE(str.storage_rbegin() == str.storage_rend());

   str = u"grape";
   REQUIRE(*str.storage_begin()  == u'g');
   REQUIRE(*str.storage_rbegin() == u'e');
   REQUIRE(str.storage_end() - str.storage_begin() == 5);

   // unicode 21B4, one storage units
   CsString::CsString_utf16 arrow(1, U'↴');

   str = u"grape" + arrow;
   REQUIRE(*(str.storage_end() - 2) == u'e');
   REQUIRE(*(str.storage_rbegin() + 1) == u'e');
}

TEST_CASE("CsString_utf16 u_constructor", "[cs_string]")
{
   CsString::CsString_utf16 str = u"On a clear day you can see forever";

   REQUIRE(str == u"On a clear day you can see forever");
}

TEST_CASE("CsString_utf16 from_utf_a", "[cs_string]")
{
   CsString::CsString_utf16 str1 = u"HeizölrückstoßabdämpFung \u20A8 \u2122";

   CsString::CsString_utf8  str2 = CsString::CsString_utf8::fromUtf16(u"HeizölrückstoßabdämpFung \u20A8 \u2122");
   CsString::CsString_utf16 str3 = CsString::CsString_utf16::fromUtf16(u"HeizölrückstoßabdämpFung \u20A8 \u2122");

   CsString::CsString_utf16 str4 = CsString::CsString_utf16::fromUtf8(u8"HeizölrückstoßabdämpFung \u20A8 \u2122");

   REQUIRE(str1 == str2);
   REQUIRE(str1 == str3);
   REQUIRE(str1 == str4);
}

TEST_CASE("CsString_utf16 from_utf_b", "[cs_string]")
{
   {
      CsString::CsString_utf16 str = u"\U0001F3B5";

      REQUIRE(str.size() == 1);
      REQUIRE(str.size_storage() == 2);

      REQUIRE(str[0].unicode() == U'\U0001F3B5');
   }

   {
      CsString::CsString_utf16 str1 = u"HeizölrückstoßabdämpFung ";
      CsString::CsString_utf16 str2 = u"HeizölrückstoßabdämpFung \U0001F3B5";

      REQUIRE(str1.size() == 25);
      REQUIRE(str2.size() == 26);

      REQUIRE(str1 != str2);
   }

   {
      CsString::CsString_utf16 str1 = u"HeizölrückstoßabdämpFung \U0001F3B5";

      CsString::CsString_utf8  str2 = CsString::CsString_utf8::fromUtf16(u"HeizölrückstoßabdämpFung \U0001F3B5");
      CsString::CsString_utf16 str3 = CsString::CsString_utf16::fromUtf16(u"HeizölrückstoßabdämpFung \U0001F3B5");

      CsString::CsString_utf16 str4 = CsString::CsString_utf16::fromUtf8(u8"HeizölrückstoßabdämpFung \U0001F3B5");

      REQUIRE(str1.size() == 26);
      REQUIRE(str2.size() == 26);
      REQUIRE(str3.size() == 26);
      REQUIRE(str4.size() == 26);

      REQUIRE(str1 == str2);
      REQUIRE(str1 == str3);
      REQUIRE(str1 == str4);

      REQUIRE(str1[25].unicode() == U'\U0001F3B5');
      REQUIRE(str2[25].unicode() == U'\U0001F3B5');
      REQUIRE(str3[25].unicode() == U'\U0001F3B5');
      REQUIRE(str4[25].unicode() == U'\U0001F3B5');
   }
}

TEST_CASE("CsString_utf8 prefix", "[cs_string]")
{

#if defined(__cpp_char8_t)
   // c++20

   REQUIRE(std::is_same_v<char8_t,  decltype(u8'a')> == true);
   REQUIRE(std::is_same_v<char16_t, decltype(u8'a')> == false);

   REQUIRE(std::is_same_v<const char8_t(&)[15], decltype(u8"On a clear day")> == true);

#else
   REQUIRE(std::is_same_v<char,     decltype(u8'a')> == true);
   REQUIRE(std::is_same_v<char16_t, decltype(u8'a')> == false);

   REQUIRE(std::is_same_v<const char(&)[15], decltype(u8"On a clear day")> == true);
#endif

}

// utf-32
TEST_CASE("CsString_utf8 u32_append", "[cs_string]")
{
   CsString::CsString_utf8 str = U"A wacky fox and sizeable pig";

   str.append(U" went to lunch");

   REQUIRE(str == U"A wacky fox and sizeable pig went to lunch");
}

TEST_CASE("CsString_utf8 u32_length", "[cs_string]")
{
   CsString::CsString_utf8 str = U"!ä";

   REQUIRE(str.length() ==  2);

   REQUIRE(str[0].unicode() == char32_t(33));
   REQUIRE(str[1].unicode() == char32_t(228));
}

TEST_CASE("CsString_utf8 u32_constructor", "[cs_string]")
{
   {
      CsString::CsString_utf8 str = U"On a clear day you can see forever";
      REQUIRE(str == U"On a clear day you can see forever");
   }

   {
      CsString::CsString_utf8 str(U"On a clear day you can see forever", 10);

      REQUIRE(str.size() == 10);
      REQUIRE(str == U"On a clear");
   }
}


// C++20 only
TEST_CASE("CsString_utf8 char8_t_constructor", "[cs_string]")
{
#if defined(__cpp_char8_t)
   printf("\nC++20 mode enabled, char8_t checks enabled\n");

   const char8_t *data = u8"A wacky fox and sizeable pig";

   CsString::CsString_utf8 str = data;

   REQUIRE(str == "A wacky fox and sizeable pig");

#else
   printf("\nC++20 mode not enabled, char8_t checks omitted\n");
#endif
}

