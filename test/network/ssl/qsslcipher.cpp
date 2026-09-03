/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#include <qsslcipher.h>
#include <qsslsocket.h>

#include <cs_catch2.h>

TEST_CASE("QSslCipher traits", "[qsslcipher]")
{
#ifdef QT_SSL
   REQUIRE(std::is_copy_constructible_v<QSslCipher> == true);
   REQUIRE(std::is_move_constructible_v<QSslCipher> == true);

   REQUIRE(std::is_copy_assignable_v<QSslCipher> == true);
   REQUIRE(std::is_move_assignable_v<QSslCipher> == true);

   REQUIRE(std::is_nothrow_move_constructible_v<QSslCipher> == false);
   REQUIRE(std::is_nothrow_move_assignable_v<QSslCipher> == false);

   REQUIRE(std::has_virtual_destructor_v<QSslCipher> == false);
#endif
}

TEST_CASE("QSslCipher assign", "[qsslcipher]")
{
#ifdef QT_SSL
   if (! QSslSocket::supportsSsl()) {
     return;
   }

   QSslCipher cipher_A;
   QSslCipher cipher_B;

   cipher_B = cipher_A;

   REQUIRE(cipher_A.isNull() == true);
   REQUIRE(cipher_B.isNull() == true);

   REQUIRE(cipher_A == cipher_B);
#endif
}

TEST_CASE("QSslCipher constructor", "[qsslcipher]")
{
#ifdef QT_SSL
   if (! QSslSocket::supportsSsl()) {
     return;
   }

   QSslCipher cipher;

   REQUIRE(cipher.isNull() == true);
#endif
}

TEST_CASE("QSslCipher copy", "[qsslcipher]")
{
#ifdef QT_SSL
   if (! QSslSocket::supportsSsl()) {
     return;
   }

   QSslCipher cipher_A;
   QSslCipher cipher_B(cipher_A);

   REQUIRE(cipher_A.isNull() == true);
   REQUIRE(cipher_B.isNull() == true);

   REQUIRE(cipher_A == cipher_B);
#endif
}

TEST_CASE("QSslCipher name", "[qsslcipher]")
{
#ifdef QT_SSL
   if (! QSslSocket::supportsSsl()) {
     return;
   }

   //
   QSslCipher cipher;

   REQUIRE(cipher.name().isEmpty() == true);

   //
   cipher = QSslCipher("INVALID-CIPHER-NAME");

   REQUIRE(cipher.isNull() == true);
#endif
}
