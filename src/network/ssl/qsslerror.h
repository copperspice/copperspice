/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/


#ifndef QSSLERROR_H
#define QSSLERROR_H

#include <QtCore/qvariant.h>
#include <QtNetwork/qsslcertificate.h>

QT_BEGIN_NAMESPACE

#ifdef QT_NO_OPENSSL

class Q_NETWORK_EXPORT QSslError
{
};


#else

class QSslErrorPrivate;

class Q_NETWORK_EXPORT QSslError
{
 public:
   enum SslError {
      NoError,
      UnableToGetIssuerCertificate,
      UnableToDecryptCertificateSignature,
      UnableToDecodeIssuerPublicKey,
      CertificateSignatureFailed,
      CertificateNotYetValid,
      CertificateExpired,
      InvalidNotBeforeField,
      InvalidNotAfterField,
      SelfSignedCertificate,
      SelfSignedCertificateInChain,
      UnableToGetLocalIssuerCertificate,
      UnableToVerifyFirstCertificate,
      CertificateRevoked,
      InvalidCaCertificate,
      PathLengthExceeded,
      InvalidPurpose,
      CertificateUntrusted,
      CertificateRejected,
      SubjectIssuerMismatch, // hostname mismatch?
      AuthorityIssuerSerialNumberMismatch,
      NoPeerCertificate,
      HostNameMismatch,
      NoSslSupport,
      CertificateBlacklisted,
      UnspecifiedError = -1
   };

   // RVCT compiler in debug build does not like about default values in const-
   // So as an workaround we define all constructor overloads here explicitly
   QSslError();
   QSslError(SslError error);
   QSslError(SslError error, const QSslCertificate &certificate);

   QSslError(const QSslError &other);

   ~QSslError();
   QSslError &operator=(const QSslError &other);
   bool operator==(const QSslError &other) const;
   inline bool operator!=(const QSslError &other) const {
      return !(*this == other);
   }

   SslError error() const;
   QString errorString() const;
   QSslCertificate certificate() const;

 private:
   QScopedPointer<QSslErrorPrivate> d;
};

class QDebug;
Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, const QSslError &error);
Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, const QSslError::SslError &error);

#endif // QT_NO_OPENSSL

QT_END_NAMESPACE

#endif
