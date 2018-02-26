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

#ifndef QSSLCERTIFICATE_P_H
#define QSSLCERTIFICATE_P_H

#include <qsslsocket_p.h>
#include <qsslcertificateextension.h>
#include <qdatetime.h>
#include <qmultimap.h>

#ifdef QT_OPENSSL
#include <openssl/x509.h>
#include <qsslsocket_openssl_symbols_p.h>

#else
   struct X509;
   struct X509_EXTENSION;
   struct ASN1_OBJECT;
#endif

class QSslCertificatePrivate
{
 public:
   QSslCertificatePrivate()
      : null(true), x509(0) {
      QSslSocketPrivate::ensureInitialized();
   }

   ~QSslCertificatePrivate() {

#ifdef QT_OPENSSL
      if (x509) {
         q_X509_free(x509);
      }
#endif
   }

   bool null;
   QByteArray versionString;
   QByteArray serialNumberString;

   QMultiMap<QByteArray, QString> issuerInfo;
   QMultiMap<QByteArray, QString> subjectInfo;
   QDateTime notValidAfter;
   QDateTime notValidBefore;

#if ! defined(QT_OPENSSL)
    bool subjectMatchesIssuer;
    QSsl::KeyAlgorithm publicKeyAlgorithm;
    QByteArray publicKeyDerData;
    QMultiMap<QSsl::AlternativeNameEntryType, QString> subjectAlternativeNames;
    QList<QSslCertificateExtension> extensions;

    QByteArray derData;

    bool parse(const QByteArray &data);
    bool parseExtension(const QByteArray &data, QSslCertificateExtension *extension);
#endif

   X509 *x509;

   void init(const QByteArray &data, QSsl::EncodingFormat format);

   static QByteArray asn1ObjectId(ASN1_OBJECT *object);
   static QByteArray asn1ObjectName(ASN1_OBJECT *object);
   static QByteArray QByteArray_from_X509(X509 *x509, QSsl::EncodingFormat format);
   static QString text_from_X509(X509 *x509);
   static QSslCertificate QSslCertificate_from_X509(X509 *x509);
   static QList<QSslCertificate> certificatesFromPem(const QByteArray &pem, int count = -1);
   static QList<QSslCertificate> certificatesFromDer(const QByteArray &der, int count = -1);
   static bool isBlacklisted(const QSslCertificate &certificate);
   static QSslCertificateExtension convertExtension(X509_EXTENSION *ext);
   static QByteArray subjectInfoToString(QSslCertificate::SubjectInfo info);

   friend class QSslSocketBackendPrivate;

   QAtomicInt ref;
};

#endif
