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

#include <qsslcertificate.h>
#include <qsslkey.h>
#include <qsslsocket.h>

#include <cs_catch2.h>

TEST_CASE("QSslCertificate traits", "[qsslcertificate]")
{
#ifdef QT_SSL
   REQUIRE(std::is_copy_constructible_v<QSslCertificate> == true);
   REQUIRE(std::is_move_constructible_v<QSslCertificate> == true);

   REQUIRE(std::is_copy_assignable_v<QSslCertificate> == true);
   REQUIRE(std::is_move_assignable_v<QSslCertificate> == true);

   REQUIRE(std::is_nothrow_move_constructible_v<QSslCertificate> == false);
   REQUIRE(std::is_nothrow_move_assignable_v<QSslCertificate> == false);

   REQUIRE(std::has_virtual_destructor_v<QSslCertificate> == false);
#endif
}

TEST_CASE("QSslCertificate certificate-data", "[qsslcertificate]")
{
#ifdef QT_SSL
   if (! QSslSocket::supportsSsl()) {
     return;
   }

   auto data = QByteArray();
   const QSslCertificate certificate(data);

   REQUIRE(certificate.isNull() == true);
#endif
}

TEST_CASE("QSslCertificate constructor", "[qsslcertificate]")
{
#ifdef QT_SSL
   if (! QSslSocket::supportsSsl()) {
     return;
   }

   QSslCertificate certificate;

   REQUIRE(certificate.isNull()  == true);

   REQUIRE(certificate.publicKey().isNull() == true);
   REQUIRE(certificate.toDer().isEmpty()    == true);
   REQUIRE(certificate.toPem().isEmpty()    == true);

   //
   certificate.clear();

   REQUIRE(certificate.isNull() == true);
#endif
}

TEST_CASE("QSslCertificate pem-data", "[qsslcertificate]")
{
#ifdef QT_SSL
   if (! QSslSocket::supportsSsl()) {
     return;
   }

   const QByteArray data("This is not a valid PEM certificate");
   QSslCertificate certificate(data, QSsl::Pem);

   REQUIRE(certificate.isNull() == true);
#endif
}

TEST_CASE("QSslCertificate validity-dates", "[qsslcertificate]")
{
#ifdef QT_SSL
   if (! QSslSocket::supportsSsl()) {
     return;
   }

   QSslCertificate certificate;

   REQUIRE(certificate.effectiveDate().isValid() == false);
   REQUIRE(certificate.expiryDate().isValid() == false);
#endif
}
