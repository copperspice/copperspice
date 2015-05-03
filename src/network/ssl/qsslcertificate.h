/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#ifndef QSSLCERTIFICATE_H
#define QSSLCERTIFICATE_H

#include <QtCore/qnamespace.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qcryptographichash.h>
#include <QtCore/qregexp.h>
#include <QtCore/qsharedpointer.h>
#include <QtNetwork/qssl.h>
#include <QtCore/qcontainerfwd.h>

typedef struct x509_st X509; // ### check if this works

QT_BEGIN_NAMESPACE

#ifndef QT_NO_OPENSSL

class QDateTime;
class QIODevice;
class QSslKey;
class QStringList;
class QSslCertificatePrivate;

class Q_NETWORK_EXPORT QSslCertificate
{

 public:
   enum SubjectInfo {
      Organization,
      CommonName,
      LocalityName,
      OrganizationalUnitName,
      CountryName,
      StateOrProvinceName
   };

   QSslCertificate(QIODevice *device, QSsl::EncodingFormat format = QSsl::Pem);
   QSslCertificate( // ### s/encoded/data (to be consistent with signature in .cpp file) ?
      const QByteArray &encoded = QByteArray(), QSsl::EncodingFormat format = QSsl::Pem);

   QSslCertificate(const QSslCertificate &other);
   ~QSslCertificate();

   QSslCertificate &operator=(const QSslCertificate &other);
   bool operator==(const QSslCertificate &other) const;

   inline bool operator!=(const QSslCertificate &other) const {
      return !operator==(other);
   }

   bool isNull() const;
   bool isValid() const;
   void clear();

   // Certificate info
   QByteArray version() const;
   QByteArray serialNumber() const;
   QByteArray digest(QCryptographicHash::Algorithm algorithm = QCryptographicHash::Md5) const;
   QString issuerInfo(SubjectInfo info) const;
   QString issuerInfo(const QByteArray &tag) const;
   QString subjectInfo(SubjectInfo info) const;
   QString subjectInfo(const QByteArray &tag) const;
   QMultiMap<QSsl::AlternateNameEntryType, QString> alternateSubjectNames() const;
   QDateTime effectiveDate() const;
   QDateTime expiryDate() const;
   QSslKey publicKey() const;

   QByteArray toPem() const;
   QByteArray toDer() const;

   static QList<QSslCertificate> fromPath(const QString &path, QSsl::EncodingFormat format = QSsl::Pem,
         QRegExp::PatternSyntax syntax = QRegExp::FixedString);

   static QList<QSslCertificate> fromDevice(QIODevice *device, QSsl::EncodingFormat format = QSsl::Pem);
   static QList<QSslCertificate> fromData(const QByteArray &data, QSsl::EncodingFormat format = QSsl::Pem);

   Qt::HANDLE handle() const;

 private:
   QExplicitlySharedDataPointer<QSslCertificatePrivate> d;
   friend class QSslCertificatePrivate;
   friend class QSslSocketBackendPrivate;
};

class QDebug;
Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, const QSslCertificate &certificate);
Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, QSslCertificate::SubjectInfo info);

#endif // QT_NO_OPENSSL

QT_END_NAMESPACE

#endif
