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

#include <QtCore/qglobal.h>

#ifdef QT_OPENSSL
#include <qsslsocket_openssl_symbols_p.h>
#endif

#ifdef QT_SECURETRANSPORT
#include "qsslsocket_mac_p.h"
#endif

#include <qssl_p.h>
#include <qsslcertificate.h>
#include <qsslcertificate_p.h>
#include <qsslkey_p.h>

#include <qdebug.h>
#include <qdir.h>
#include <qdiriterator.h>
#include <qfile.h>

QSslCertificate::QSslCertificate(QIODevice *device, QSsl::EncodingFormat format)
   : d(new QSslCertificatePrivate)
{
   QSslSocketPrivate::ensureInitialized();

   if (device) {
      d->init(device->readAll(), format);
   }
}

QSslCertificate::QSslCertificate(const QByteArray &data, QSsl::EncodingFormat format)
   : d(new QSslCertificatePrivate)
{
   QSslSocketPrivate::ensureInitialized();
   d->init(data, format);
}

QSslCertificate::QSslCertificate(const QSslCertificate &other) : d(other.d)
{
}

QSslCertificate::~QSslCertificate()
{
}

QSslCertificate &QSslCertificate::operator=(const QSslCertificate &other)
{
   d = other.d;
   return *this;
}

bool QSslCertificate::isBlacklisted() const
{
   return QSslCertificatePrivate::isBlacklisted(*this);
}
void QSslCertificate::clear()
{
   if (isNull()) {
      return;
   }

   d = new QSslCertificatePrivate;
}

QByteArray QSslCertificate::digest(QCryptographicHash::Algorithm algorithm) const
{
   return QCryptographicHash::hash(toDer(), algorithm);
}

QList<QSslCertificate> QSslCertificate::fromPath(const QString &path,
      QSsl::EncodingFormat format, QRegExp::PatternSyntax syntax)
{
   // $, (,), *, +, ., ?, [, ,], ^, {, | and }.
   // make sure to use the same path separators on Windows and Unix like systems.
   QString sourcePath = QDir::fromNativeSeparators(path);

   // Find the path without the filename
   QString pathPrefix = sourcePath.left(sourcePath.lastIndexOf(QLatin1Char('/')));

   // Check if the path contains any special chars
   int pos = -1;
   if (syntax == QRegExp::Wildcard) {
      pos = pathPrefix.indexOf(QRegExp(QLatin1String("[*?[]")));
   } else if (syntax != QRegExp::FixedString) {
      pos = sourcePath.indexOf(QRegExp(QLatin1String("[\\$\\(\\)\\*\\+\\.\\?\\[\\]\\^\\{\\}\\|]")));
   }

   if (pos != -1) {
      // there was a special char in the path so cut of the part containing that char.
      pathPrefix = pathPrefix.left(pos);
      if (pathPrefix.contains(QLatin1Char('/'))) {
         pathPrefix = pathPrefix.left(pathPrefix.lastIndexOf(QLatin1Char('/')));
      } else {
         pathPrefix.clear();
      }

   } else {
      // Check if the path is a file.
      if (QFileInfo(sourcePath).isFile()) {
         QFile file(sourcePath);
         QIODevice::OpenMode openMode = QIODevice::ReadOnly;
         if (format == QSsl::Pem) {
            openMode |= QIODevice::Text;
         }
         if (file.open(openMode))

         {
            return QSslCertificate::fromData(file.readAll(), format);
         }

         return QList<QSslCertificate>();
      }
   }
   // Special case - if the prefix ends up being nothing, use "." instead.
   int startIndex = 0;
   if (pathPrefix.isEmpty()) {
      pathPrefix = QLatin1String(".");
      startIndex = 2;
   }

   // The path can be a file or directory.
   QList<QSslCertificate> certs;
   QRegExp pattern(sourcePath, Qt::CaseSensitive, syntax);
   QDirIterator it(pathPrefix, QDir::Files, QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);

   while (it.hasNext()) {
      QString filePath = startIndex == 0 ? it.next() : it.next().mid(startIndex);
      if (! pattern.exactMatch(filePath)) {
         continue;
      }

      QFile file(filePath);
      QIODevice::OpenMode openMode = QIODevice::ReadOnly;
      if (format == QSsl::Pem) {
         openMode |= QIODevice::Text;
      }

      if (file.open(openMode)) {
         certs += QSslCertificate::fromData(file.readAll(), format);
      }
   }

   return certs;
}

QList<QSslCertificate> QSslCertificate::fromDevice(QIODevice *device, QSsl::EncodingFormat format)
{
   if (!device) {
      qWarning("QSslCertificate::fromDevice: cannot read from a null device");
      return QList<QSslCertificate>();
   }
   return fromData(device->readAll(), format);
}

QList<QSslCertificate> QSslCertificate::fromData(const QByteArray &data, QSsl::EncodingFormat format)
{
   return (format == QSsl::Pem)
          ? QSslCertificatePrivate::certificatesFromPem(data)
          : QSslCertificatePrivate::certificatesFromDer(data);
}

QList<QSslError> QSslCertificate::verify(const QList<QSslCertificate> &certificateChain, const QString &hostName)
{
   return QSslSocketBackendPrivate::verify(certificateChain, hostName);
}

bool QSslCertificate::importPkcs12(QIODevice *device, QSslKey *key, QSslCertificate *certificate,
                                   QList<QSslCertificate> *caCertificates, const QByteArray &passPhrase)
{
   return QSslSocketBackendPrivate::importPkcs12(device, key, certificate, caCertificates, passPhrase);
}

// These certificates are known to be fraudulent and were created during the comodo
// compromise. See http://www.comodo.com/Comodo-Fraud-Incident-2011-03-23.html
static const char *const certificate_blacklist[] = {
   "04:7e:cb:e9:fc:a5:5f:7b:d0:9e:ae:36:e1:0c:ae:1e", "mail.google.com", // Comodo
   "f5:c8:6a:f3:61:62:f1:3a:64:f5:4f:6d:c9:58:7c:06", "www.google.com", // Comodo
   "d7:55:8f:da:f5:f1:10:5b:b2:13:28:2b:70:77:29:a3", "login.yahoo.com", // Comodo
   "39:2a:43:4f:0e:07:df:1f:8a:a3:05:de:34:e0:c2:29", "login.yahoo.com", // Comodo
   "3e:75:ce:d4:6b:69:30:21:21:88:30:ae:86:a8:2a:71", "login.yahoo.com", // Comodo
   "e9:02:8b:95:78:e4:15:dc:1a:71:0a:2b:88:15:44:47", "login.skype.com", // Comodo
   "92:39:d5:34:8f:40:d1:69:5a:74:54:70:e1:f2:3f:43", "addons.mozilla.org", // Comodo
   "b0:b7:13:3e:d0:96:f9:b5:6f:ae:91:c8:74:bd:3a:c0", "login.live.com", // Comodo
   "d8:f3:5f:4e:b7:87:2b:2d:ab:06:92:e3:15:38:2f:b0", "global trustee", // Comodo

   "05:e2:e6:a4:cd:09:ea:54:d6:65:b0:75:fe:22:a2:56", "*.google.com", // leaf certificate issued by DigiNotar
   "0c:76:da:9c:91:0c:4e:2c:9e:fe:15:d0:58:93:3c:4c", "DigiNotar Root CA", // DigiNotar root
   "f1:4a:13:f4:87:2b:56:dc:39:df:84:ca:7a:a1:06:49", "DigiNotar Services CA", // DigiNotar intermediate signed by DigiNotar Root
   "36:16:71:55:43:42:1b:9d:e6:cb:a3:64:41:df:24:38", "DigiNotar Services 1024 CA", // DigiNotar intermediate signed by DigiNotar Root
   "0a:82:bd:1e:14:4e:88:14:d7:5b:1a:55:27:be:bf:3e", "DigiNotar Root CA G2", // other DigiNotar Root CA
   "a4:b6:ce:e3:2e:d3:35:46:26:3c:b3:55:3a:a8:92:21", "CertiID Enterprise Certificate Authority", // DigiNotar intermediate signed by "DigiNotar Root CA G2"
   "5b:d5:60:9c:64:17:68:cf:21:0e:35:fd:fb:05:ad:41", "DigiNotar Qualified CA", // DigiNotar intermediate signed by DigiNotar Root

   "46:9c:2c:b0",                                     "DigiNotar Services 1024 CA", // DigiNotar intermediate cross-signed by Entrust
   "07:27:10:0d",                                     "DigiNotar Cyber CA", // DigiNotar intermediate cross-signed by CyberTrust
   "07:27:0f:f9",                                     "DigiNotar Cyber CA", // DigiNotar intermediate cross-signed by CyberTrust
   "07:27:10:03",                                     "DigiNotar Cyber CA", // DigiNotar intermediate cross-signed by CyberTrust
   "01:31:69:b0",                                     "DigiNotar PKIoverheid CA Overheid en Bedrijven", // DigiNotar intermediate cross-signed by Dutch gvt
   "01:31:34:bf",                                     "DigiNotar PKIoverheid CA Organisatie - G2", // DigiNotar intermediate cross-signed by Dutch government
   "d6:d0:29:77:f1:49:fd:1a:83:f2:b9:ea:94:8c:5c:b4", "DigiNotar Extended Validation CA", // DigiNotar intermediate signed by DigiNotar EV Root
   "1e:7d:7a:53:3d:45:30:41:96:40:0f:71:48:1f:45:04", "DigiNotar Public CA 2025", // DigiNotar intermediate
   //    "(has not been seen in the wild so far)", "DigiNotar Public CA - G2", // DigiNotar intermediate
   //    "(has not been seen in the wild so far)", "Koninklijke Notariele Beroepsorganisatie CA", // compromised during DigiNotar breach
   //    "(has not been seen in the wild so far)", "Stichting TTP Infos CA," // compromised during DigiNotar breach
   "46:9c:2c:af",                                     "DigiNotar Root CA", // DigiNotar intermediate cross-signed by Entrust
   "46:9c:3c:c9",                                     "DigiNotar Root CA", // DigiNotar intermediate cross-signed by Entrust

   "07:27:14:a9",                                     "Digisign Server ID (Enrich)", // (Malaysian) Digicert Sdn. Bhd. cross-signed by Verizon CyberTrust
   "4c:0e:63:6a",                                     "Digisign Server ID - (Enrich)", // (Malaysian) Digicert Sdn. Bhd. cross-signed by Entrust
   "72:03:21:05:c5:0c:08:57:3d:8e:a5:30:4e:fe:e8:b0", "UTN-USERFirst-Hardware", // comodogate test certificate
   "41",                                              "MD5 Collisions Inc. (http://www.phreedom.org/md5)", // http://www.phreedom.org/research/rogue-ca/

   "08:27",                                           "*.EGO.GOV.TR", // Turktrust mis-issued intermediate certificate
   "08:64",                                           "e-islem.kktcmerkezbankasi.org", // Turktrust mis-issued intermediate certificate

   "03:1d:a7",                                        "AC DG Tr\xC3\xA9sor SSL", // intermediate certificate linking back to ANSSI French NSA
   "27:83",                                           "NIC Certifying Authority", // intermediate certificate from NIC India (2007)
   "27:92",                                           "NIC CA 2011", // intermediate certificate from NIC India (2011)
   "27:b1",                                           "NIC CA 2014", // intermediate certificate from NIC India (2014)
   0
};

bool QSslCertificatePrivate::isBlacklisted(const QSslCertificate &certificate)
{
   for (int a = 0; certificate_blacklist[a] != 0; a++) {
      QString blacklistedCommonName = QString::fromUtf8(certificate_blacklist[(a + 1)]);

      if (certificate.serialNumber() == certificate_blacklist[a++] &&
            (certificate.subjectInfo(QSslCertificate::CommonName).contains(blacklistedCommonName) ||
             certificate.issuerInfo(QSslCertificate::CommonName).contains(blacklistedCommonName)))    {
         return true;
      }
   }

   return false;
}

QByteArray QSslCertificatePrivate::subjectInfoToString(QSslCertificate::SubjectInfo info)
{
   QByteArray str;

   switch (info) {
      case QSslCertificate::Organization:
         str = QByteArray("O");
         break;

      case QSslCertificate::CommonName:
         str = QByteArray("CN");
         break;

      case QSslCertificate::LocalityName:
         str = QByteArray("L");
         break;

      case QSslCertificate::OrganizationalUnitName:
         str = QByteArray("OU");
         break;

      case QSslCertificate::CountryName:
         str = QByteArray("C");
         break;

      case QSslCertificate::StateOrProvinceName:
         str = QByteArray("ST");
         break;

      case QSslCertificate::DistinguishedNameQualifier:
         str = QByteArray("dnQualifier");
         break;

      case QSslCertificate::SerialNumber:
         str = QByteArray("serialNumber");
         break;

      case QSslCertificate::EmailAddress:
         str = QByteArray("emailAddress");
         break;
   }

   return str;
}

QDebug operator<<(QDebug debug, const QSslCertificate &certificate)
{

   // QDebugStateSaver saver(debug);
   // debug.resetFormat().nospace();

   debug << "QSslCertificate("
         << certificate.version()
         << ", " << certificate.serialNumber()
         << ", " << certificate.digest().toBase64()
         << ", " << certificate.issuerInfo(QSslCertificate::Organization)
         << ", " << certificate.subjectInfo(QSslCertificate::Organization)
         << ", " << certificate.subjectAlternativeNames()

#ifndef QT_NO_DATESTRING
         << ", " << certificate.effectiveDate()
         << ", " << certificate.expiryDate()
#endif
         << ')';

   return debug;
}

QDebug operator<<(QDebug debug, QSslCertificate::SubjectInfo info)
{
   switch (info) {
      case QSslCertificate::Organization:
         debug << "Organization";
         break;

      case QSslCertificate::CommonName:
         debug << "CommonName";
         break;

      case QSslCertificate::CountryName:
         debug << "CountryName";
         break;

      case QSslCertificate::LocalityName:
         debug << "LocalityName";
         break;

      case QSslCertificate::OrganizationalUnitName:
         debug << "OrganizationalUnitName";
         break;

      case QSslCertificate::StateOrProvinceName:
         debug << "StateOrProvinceName";
         break;

      case QSslCertificate::DistinguishedNameQualifier:
         debug << "DistinguishedNameQualifier";
         break;

      case QSslCertificate::SerialNumber:
         debug << "SerialNumber";
         break;

      case QSslCertificate::EmailAddress:
         debug << "EmailAddress";
         break;
   }

   return debug;
}

