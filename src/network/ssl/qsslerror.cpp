/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qsslerror.h>
#include <qsslsocket.h>
#include <qdebug.h>

class QSslErrorPrivate
{
 public:
   QSslError::SslError error;
   QSslCertificate certificate;
};

// RVCT compiler in debug build does not like about default values in const-
// So as an workaround we define all constructor overloads here explicitly
QSslError::QSslError()
   : d(new QSslErrorPrivate)
{
   d->error = QSslError::NoError;
   d->certificate = QSslCertificate();
}

QSslError::QSslError(SslError error)
   : d(new QSslErrorPrivate)
{
   d->error = error;
   d->certificate = QSslCertificate();
}

QSslError::QSslError(SslError error, const QSslCertificate &certificate)
   : d(new QSslErrorPrivate)
{
   d->error = error;
   d->certificate = certificate;
}

QSslError::QSslError(const QSslError &other)
   : d(new QSslErrorPrivate)
{
   *d.data() = *other.d.data();
}

QSslError::~QSslError()
{
}

QSslError &QSslError::operator=(const QSslError &other)
{
   *d.data() = *other.d.data();
   return *this;
}

bool QSslError::operator==(const QSslError &other) const
{
   return d->error == other.d->error && d->certificate == other.d->certificate;
}

QSslError::SslError QSslError::error() const
{
   return d->error;
}

QString QSslError::errorString() const
{
   QString errStr;
   switch (d->error) {
      case NoError:
         errStr = QSslSocket::tr("No error");
         break;
      case UnableToGetIssuerCertificate:
         errStr = QSslSocket::tr("The issuer certificate could not be found");
         break;
      case UnableToDecryptCertificateSignature:
         errStr = QSslSocket::tr("The certificate signature could not be decrypted");
         break;
      case UnableToDecodeIssuerPublicKey:
         errStr = QSslSocket::tr("The public key in the certificate could not be read");
         break;
      case CertificateSignatureFailed:
         errStr = QSslSocket::tr("The signature of the certificate is invalid");
         break;
      case CertificateNotYetValid:
         errStr = QSslSocket::tr("The certificate is not yet valid");
         break;
      case CertificateExpired:
         errStr = QSslSocket::tr("The certificate has expired");
         break;
      case InvalidNotBeforeField:
         errStr = QSslSocket::tr("The certificate's notBefore field contains an invalid time");
         break;
      case InvalidNotAfterField:
         errStr = QSslSocket::tr("The certificate's notAfter field contains an invalid time");
         break;
      case SelfSignedCertificate:
         errStr = QSslSocket::tr("The certificate is self-signed, and untrusted");
         break;
      case SelfSignedCertificateInChain:
         errStr = QSslSocket::tr("The root certificate of the certificate chain is self-signed, and untrusted");
         break;
      case UnableToGetLocalIssuerCertificate:
         errStr = QSslSocket::tr("The issuer certificate of a locally looked up certificate could not be found");
         break;
      case UnableToVerifyFirstCertificate:
         errStr = QSslSocket::tr("No certificates could be verified");
         break;
      case InvalidCaCertificate:
         errStr = QSslSocket::tr("One of the CA certificates is invalid");
         break;
      case PathLengthExceeded:
         errStr = QSslSocket::tr("The basicConstraints path length parameter has been exceeded");
         break;
      case InvalidPurpose:
         errStr = QSslSocket::tr("The supplied certificate is unsuitable for this purpose");
         break;
      case CertificateUntrusted:
         errStr = QSslSocket::tr("The root CA certificate is not trusted for this purpose");
         break;
      case CertificateRejected:
         errStr = QSslSocket::tr("The root CA certificate is marked to reject the specified purpose");
         break;
      case SubjectIssuerMismatch:    // hostname mismatch
         errStr = QSslSocket::tr("The current candidate issuer certificate was rejected because its"
                                 " subject name did not match the issuer name of the current certificate");
         break;
      case AuthorityIssuerSerialNumberMismatch:
         errStr = QSslSocket::tr("The current candidate issuer certificate was rejected because"
                                 " its issuer name and serial number was present and did not match the"
                                 " authority key identifier of the current certificate");
         break;
      case NoPeerCertificate:
         errStr = QSslSocket::tr("The peer did not present any certificate");
         break;
      case HostNameMismatch:
         errStr = QSslSocket::tr("The host name did not match any of the valid hosts"
                                 " for this certificate");
         break;
      case NoSslSupport:
         break;
      case CertificateBlacklisted:
         errStr = QSslSocket::tr("The peer certificate is blacklisted");
         break;
      default:
         errStr = QSslSocket::tr("Unknown error");
         break;
   }

   return errStr;
}

QSslCertificate QSslError::certificate() const
{
   return d->certificate;
}

uint qHash(const QSslError &key, uint seed)
{
   // 2x boost::hash_combine inlined:
   seed ^= qHash(key.error())       + 0x9e3779b9 + (seed << 6) + (seed >> 2);
   seed ^= qHash(key.certificate()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

   return seed;
}

QDebug operator<<(QDebug debug, const QSslError &error)
{
   debug << error.errorString();
   return debug;
}

QDebug operator<<(QDebug debug, const QSslError::SslError &error)
{
   debug << QSslError(error).errorString();
   return debug;
}
