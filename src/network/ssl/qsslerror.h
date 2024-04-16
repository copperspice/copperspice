/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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

#ifndef QSSLERROR_H
#define QSSLERROR_H

#ifdef QT_SSL

#include <qvariant.h>
#include <qsslcertificate.h>

class QDebug;
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

   QSslError();
   QSslError(SslError error);
   QSslError(SslError error, const QSslCertificate &certificate);
   QSslError(const QSslError &other);

   ~QSslError();

   SslError error() const;
   QString errorString() const;

   QSslCertificate certificate() const;

   void swap(QSslError &other)  {
      qSwap(d, other.d);
   }

   QSslError &operator=(const QSslError &other);
   bool operator==(const QSslError &other) const;

   bool operator!=(const QSslError &other) const {
      return !(*this == other);
   }

   QSslError &operator=(QSslError &&other)  {
      swap(other);
      return *this;
   }

private:
   QScopedPointer<QSslErrorPrivate> d;
};

Q_NETWORK_EXPORT uint qHash(const QSslError &key, uint seed = 0);

Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, const QSslError &error);
Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, const QSslError::SslError &error);

#else

class QSslError
{
};

#endif   // QT_SSL

#endif
