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

#ifndef QSSLCERTIFICATE_H
#define QSSLCERTIFICATE_H

#include <qnamespace.h>
#include <qbytearray.h>
#include <qcryptographichash.h>
#include <qdatetime.h>
#include <qregexp.h>
#include <qsharedpointer.h>
#include <qssl.h>
#include <qcontainerfwd.h>
#include <qmetatype.h>

#ifdef QT_SSL

class QDateTime;
class QIODevice;
class QSslError;
class QSslKey;
class QSslCertificateExtension;
class QStringList;
class QSslCertificatePrivate;

class QSslCertificate;
Q_NETWORK_EXPORT uint qHash(const QSslCertificate &key, uint seed = 0);

class Q_NETWORK_EXPORT QSslCertificate
{

public:
   enum SubjectInfo {
      Organization,
      CommonName,
      LocalityName,
      OrganizationalUnitName,
      CountryName,
      StateOrProvinceName,
      DistinguishedNameQualifier,
      SerialNumber,
      EmailAddress
   };

   explicit QSslCertificate(QIODevice *device, QSsl::EncodingFormat format = QSsl::Pem);
   explicit QSslCertificate(const QByteArray &data = QByteArray(), QSsl::EncodingFormat format = QSsl::Pem);
   QSslCertificate(const QSslCertificate &other);

   ~QSslCertificate();

   QSslCertificate &operator=(QSslCertificate &&other) {
      swap(other);
      return *this;
   }

   QSslCertificate &operator=(const QSslCertificate &other);

   void swap(QSslCertificate &other)  {
      qSwap(d, other.d);
   }

   bool operator==(const QSslCertificate &other) const;
   inline bool operator!=(const QSslCertificate &other) const {
      return !operator==(other);
   }

   bool isNull() const;

   bool isBlacklisted() const;
   bool isSelfSigned() const;
   void clear();

   // Certificate info
   QByteArray version() const;
   QByteArray serialNumber() const;
   QByteArray digest(QCryptographicHash::Algorithm algorithm = QCryptographicHash::Md5) const;
   QStringList issuerInfo(SubjectInfo info) const;
   QStringList issuerInfo(const QByteArray &attribute) const;
   QStringList subjectInfo(SubjectInfo info) const;
   QStringList subjectInfo(const QByteArray &attribute) const;
   QList<QByteArray> subjectInfoAttributes() const;
   QList<QByteArray> issuerInfoAttributes() const;

   QMultiMap<QSsl::AlternativeNameEntryType, QString> subjectAlternativeNames() const;

   QDateTime effectiveDate() const;
   QDateTime expiryDate() const;
   QSslKey publicKey() const;
   QList<QSslCertificateExtension> extensions() const;

   QByteArray toPem() const;
   QByteArray toDer() const;
   QString toText() const;

   static QList<QSslCertificate> fromPath(const QString &path, QSsl::EncodingFormat format = QSsl::Pem,
                  QRegExp::PatternSyntax syntax = QRegExp::FixedString);

   static QList<QSslCertificate> fromDevice(QIODevice *device, QSsl::EncodingFormat format = QSsl::Pem);
   static QList<QSslCertificate> fromData(const QByteArray &data, QSsl::EncodingFormat format = QSsl::Pem);

   static QList<QSslError> verify(const QList<QSslCertificate> &certificateChain, const QString &hostName = QString());
   static bool importPkcs12(QIODevice *device, QSslKey *key, QSslCertificate *cert,
                  QList<QSslCertificate> *caCertificates = nullptr, const QByteArray &passPhrase = QByteArray());

   Qt::HANDLE handle() const;

private:
   QExplicitlySharedDataPointer<QSslCertificatePrivate> d;
   friend class QSslCertificatePrivate;
   friend class QSslSocketBackendPrivate;

   friend Q_NETWORK_EXPORT uint qHash(const QSslCertificate &key, uint seed);
};

class QDebug;
Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, const QSslCertificate &certificate);
Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, QSslCertificate::SubjectInfo info);

Q_DECLARE_METATYPE(QSslCertificate)

#endif

#endif
