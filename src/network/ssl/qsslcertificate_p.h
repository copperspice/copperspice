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

#ifndef QSSLCERTIFICATE_P_H
#define QSSLCERTIFICATE_P_H

#include <qsslcertificate.h>
#include <qsslsocket_p.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qmap.h>
#include <openssl/x509.h>

QT_BEGIN_NAMESPACE

class QSslCertificatePrivate
{
 public:
   QSslCertificatePrivate()
      : null(true), x509(0) {
      QSslSocketPrivate::ensureInitialized();
   }

   ~QSslCertificatePrivate() {
      if (x509) {
         q_X509_free(x509);
      }
   }

   bool null;
   QByteArray versionString;
   QByteArray serialNumberString;

   QMap<QString, QString> issuerInfo;
   QMap<QString, QString> subjectInfo;
   QDateTime notValidAfter;
   QDateTime notValidBefore;

   X509 *x509;

   void init(const QByteArray &data, QSsl::EncodingFormat format);

   static QByteArray QByteArray_from_X509(X509 *x509, QSsl::EncodingFormat format);
   static QSslCertificate QSslCertificate_from_X509(X509 *x509);
   static QList<QSslCertificate> certificatesFromPem(const QByteArray &pem, int count = -1);
   static QList<QSslCertificate> certificatesFromDer(const QByteArray &der, int count = -1);
   static bool isBlacklisted(const QSslCertificate &certificate);

   friend class QSslSocketBackendPrivate;

   QAtomicInt ref;
};

QT_END_NAMESPACE

#endif
