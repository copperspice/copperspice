/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "qsslkey.h"
#include "qsslkey_p.h"
#include "qasn1element_p.h"

#include <QtCore/qdatastream.h>
#include <QtCore/qcryptographichash.h>

QT_USE_NAMESPACE

static const quint8 bits_table[256] = {
   0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
};

// OIDs of named curves allowed in TLS as per RFCs 4492 and 7027,
// see also https://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-8

typedef QMap<QByteArray, int> OidLengthMap;
static OidLengthMap createOidMap()
{
   OidLengthMap oids;
   oids.insert(oids.cend(), QByteArrayLiteral("1.2.840.10045.3.1.1"), 192); // secp192r1 a.k.a prime192v1
   oids.insert(oids.cend(), QByteArrayLiteral("1.2.840.10045.3.1.7"), 256); // secp256r1 a.k.a prime256v1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.1"), 193); // sect193r2
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.10"), 256); // secp256k1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.16"), 283); // sect283k1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.17"), 283); // sect283r1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.26"), 233); // sect233k1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.27"), 233); // sect233r1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.3"), 239); // sect239k1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.30"), 160); // secp160r2
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.31"), 192); // secp192k1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.32"), 224); // secp224k1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.33"), 224); // secp224r1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.34"), 384); // secp384r1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.35"), 521); // secp521r1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.36"), 409); // sect409k1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.37"), 409); // sect409r1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.38"), 571); // sect571k1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.39"), 571); // sect571r1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.8"), 160); // secp160r1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.132.0.9"), 160); // secp160k1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.36.3.3.2.8.1.1.11"), 384); // brainpoolP384r1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.36.3.3.2.8.1.1.13"), 521); // brainpoolP512r1
   oids.insert(oids.cend(), QByteArrayLiteral("1.3.36.3.3.2.8.1.1.7"), 256); // brainpoolP256r1
   return oids;
}
Q_GLOBAL_STATIC_WITH_ARGS(OidLengthMap, oidLengthMap, (createOidMap()))

static int curveBits(const QByteArray &oid)
{
   const int length = oidLengthMap()->value(oid);
   return length ? length : -1;
}

static int numberOfBits(const QByteArray &modulus)
{
   int bits = modulus.size() * 8;

   for (int i = 0; i < modulus.size(); ++i) {
      quint8 b = modulus[i];
      bits -= 8;

      if (b != 0) {
         bits += bits_table[b];
         break;
      }
   }
   return bits;
}

static QByteArray deriveKey(QSslKeyPrivate::Cipher cipher, const QByteArray &passPhrase, const QByteArray &iv)
{
   QByteArray key;
   QCryptographicHash hash(QCryptographicHash::Md5);

   hash.addData(passPhrase);
   hash.addData(iv);

   switch (cipher) {
      case QSslKeyPrivate::DesCbc:
         key = hash.result().left(8);
         break;
      case QSslKeyPrivate::DesEde3Cbc:
         key = hash.result();
         hash.reset();
         hash.addData(key);
         hash.addData(passPhrase);
         hash.addData(iv);
         key += hash.result().left(8);
         break;
      case QSslKeyPrivate::Rc2Cbc:
         key = hash.result();
         break;
   }
   return key;
}

void QSslKeyPrivate::clear(bool deep)
{
   Q_UNUSED(deep);
   isNull = true;
   derData.clear();
   keyLength = -1;
}

void QSslKeyPrivate::decodeDer(const QByteArray &der, bool deepClear)
{
   clear(deepClear);

   if (der.isEmpty()) {
      return;
   }

   QAsn1Element elem;
   if (!elem.read(der) || elem.type() != QAsn1Element::SequenceType) {
      return;
   }

   if (type == QSsl::PublicKey) {
      // key info
      QDataStream keyStream(elem.value());
      if (!elem.read(keyStream) || elem.type() != QAsn1Element::SequenceType) {
         return;
      }
      QVector<QAsn1Element> infoItems = elem.toVector();
      if (infoItems.size() < 2 || infoItems[0].type() != QAsn1Element::ObjectIdentifierType) {
         return;
      }
      if (algorithm == QSsl::Rsa) {
         if (infoItems[0].toObjectId() != RSA_ENCRYPTION_OID) {
            return;
         }
         // key data
         if (!elem.read(keyStream) || elem.type() != QAsn1Element::BitStringType || elem.value().isEmpty()) {
            return;
         }
         if (!elem.read(elem.value().mid(1)) || elem.type() != QAsn1Element::SequenceType) {
            return;
         }
         if (!elem.read(elem.value()) || elem.type() != QAsn1Element::IntegerType) {
            return;
         }
         keyLength = numberOfBits(elem.value());
      } else if (algorithm == QSsl::Dsa) {
         if (infoItems[0].toObjectId() != DSA_ENCRYPTION_OID) {
            return;
         }
         if (infoItems[1].type() != QAsn1Element::SequenceType) {
            return;
         }
         // key params
         QVector<QAsn1Element> params = infoItems[1].toVector();
         if (params.isEmpty() || params[0].type() != QAsn1Element::IntegerType) {
            return;
         }
         keyLength = numberOfBits(params[0].value());
      } else if (algorithm == QSsl::Ec) {
         if (infoItems[0].toObjectId() != EC_ENCRYPTION_OID) {
            return;
         }
         if (infoItems[1].type() != QAsn1Element::ObjectIdentifierType) {
            return;
         }
         keyLength = curveBits(infoItems[1].toObjectId());
      }

   } else {
      QVector<QAsn1Element> items = elem.toVector();
      if (items.isEmpty()) {
         return;
      }

      // version
      if (items[0].type() != QAsn1Element::IntegerType) {
         return;
      }
      const QByteArray versionHex = items[0].value().toHex();

      if (algorithm == QSsl::Rsa) {
         if (versionHex != "00") {
            return;
         }
         if (items.size() != 9 || items[1].type() != QAsn1Element::IntegerType) {
            return;
         }
         keyLength = numberOfBits(items[1].value());
      } else if (algorithm == QSsl::Dsa) {
         if (versionHex != "00") {
            return;
         }
         if (items.size() != 6 || items[1].type() != QAsn1Element::IntegerType) {
            return;
         }
         keyLength = numberOfBits(items[1].value());
      } else if (algorithm == QSsl::Ec) {
         if (versionHex != "01") {
            return;
         }
         if (items.size() != 4
               || items[1].type() != QAsn1Element::OctetStringType
               || items[2].type() != QAsn1Element::Context0Type
               || items[3].type() != QAsn1Element::Context1Type) {
            return;
         }
         QAsn1Element oidElem;
         if (!oidElem.read(items[2].value())
               || oidElem.type() != QAsn1Element::ObjectIdentifierType) {
            return;
         }
         keyLength = curveBits(oidElem.toObjectId());
      }
   }

   derData = der;
   isNull = false;
}

void QSslKeyPrivate::decodePem(const QByteArray &pem, const QByteArray &passPhrase,
                               bool deepClear)
{
   QMap<QByteArray, QByteArray> headers;
   QByteArray data = derFromPem(pem, &headers);
   if (headers.value("Proc-Type") == "4,ENCRYPTED") {
      QList<QByteArray> dekInfo = headers.value("DEK-Info").split(',');
      if (dekInfo.size() != 2) {
         clear(deepClear);
         return;
      }

      Cipher cipher;
      if (dekInfo.first() == "DES-CBC") {
         cipher = DesCbc;
      } else if (dekInfo.first() == "DES-EDE3-CBC") {
         cipher = DesEde3Cbc;
      } else if (dekInfo.first() == "RC2-CBC") {
         cipher = Rc2Cbc;
      } else {
         clear(deepClear);
         return;
      }

      const QByteArray iv = QByteArray::fromHex(dekInfo.last());
      const QByteArray key = deriveKey(cipher, passPhrase, iv);
      data = decrypt(cipher, data, key, iv);
   }
   decodeDer(data, deepClear);
}

int QSslKeyPrivate::length() const
{
   return keyLength;
}

QByteArray QSslKeyPrivate::toPem(const QByteArray &passPhrase) const
{
   QByteArray data;
   QMap<QByteArray, QByteArray> headers;

   if (type == QSsl::PrivateKey && !passPhrase.isEmpty()) {
      // ### use a cryptographically secure random number generator
      QByteArray iv;
      iv.resize(8);
      for (int i = 0; i < iv.size(); ++i) {
         iv[i] = (qrand() & 0xff);
      }

      Cipher cipher = DesEde3Cbc;
      const QByteArray key = deriveKey(cipher, passPhrase, iv);
      data = encrypt(cipher, derData, key, iv);

      headers.insert("Proc-Type", "4,ENCRYPTED");
      headers.insert("DEK-Info", "DES-EDE3-CBC," + iv.toHex());
   } else {
      data = derData;
   }

   return pemFromDer(data, headers);
}

Qt::HANDLE QSslKeyPrivate::handle() const
{
   return opaque;
}
