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

#include <qsslkey.h>
#include <qsslkey_p.h>
#include <qsslsocket.h>
#include <qsslsocket_p.h>

#include <qatomic.h>
#include <qbytearray.h>
#include <qiodevice.h>

#include <qdebug.h>

QSslKey::QSslKey()
   : d(new QSslKeyPrivate)
{
}

/*!
    \internal
*/
QByteArray QSslKeyPrivate::pemHeader() const
{
   if (type == QSsl::PublicKey) {
      return QByteArrayLiteral("-----BEGIN PUBLIC KEY-----");

   } else if (algorithm == QSsl::Rsa) {
      return QByteArrayLiteral("-----BEGIN RSA PRIVATE KEY-----");

   } else if (algorithm == QSsl::Dsa) {
      return QByteArrayLiteral("-----BEGIN DSA PRIVATE KEY-----");

   } else if (algorithm == QSsl::Ec) {
      return QByteArrayLiteral("-----BEGIN EC PRIVATE KEY-----");

   }

   return QByteArray();
}

/*!
    \internal
*/
QByteArray QSslKeyPrivate::pemFooter() const
{
   if (type == QSsl::PublicKey) {
      return QByteArrayLiteral("-----END PUBLIC KEY-----");
   } else if (algorithm == QSsl::Rsa) {
      return QByteArrayLiteral("-----END RSA PRIVATE KEY-----");
   } else if (algorithm == QSsl::Dsa) {
      return QByteArrayLiteral("-----END DSA PRIVATE KEY-----");
   } else if (algorithm == QSsl::Ec) {
      return QByteArrayLiteral("-----END EC PRIVATE KEY-----");
   }

   return QByteArray();
}

/*!
    \internal

    Returns a DER key formatted as PEM.
*/
QByteArray QSslKeyPrivate::pemFromDer(const QByteArray &der, const QMap<QByteArray, QByteArray> &headers) const
{
   QByteArray pem(der.toBase64());

   const int lineWidth = 64; // RFC 1421
   const int newLines = pem.size() / lineWidth;
   const bool rem = pem.size() % lineWidth;

   // ### optimize
   for (int i = 0; i < newLines; ++i) {
      pem.insert((i + 1) * lineWidth + i, '\n');
   }
   if (rem) {
      pem.append('\n');   // ###
   }

   QByteArray extra;
   if (!headers.isEmpty()) {
      QMap<QByteArray, QByteArray>::const_iterator it = headers.constEnd();
      do {
         --it;
         extra += it.key() + ": " + it.value() + '\n';
      } while (it != headers.constBegin());
      extra += '\n';
   }
   pem.prepend(pemHeader() + '\n' + extra);
   pem.append(pemFooter() + '\n');

   return pem;
}

/*!
    \internal

    Returns a PEM key formatted as DER.
*/
QByteArray QSslKeyPrivate::derFromPem(const QByteArray &pem, QMap<QByteArray, QByteArray> *headers) const
{
   const QByteArray header = pemHeader();
   const QByteArray footer = pemFooter();

   QByteArray der(pem);

   const int headerIndex = der.indexOf(header);
   const int footerIndex = der.indexOf(footer);

   if (headerIndex == -1 || footerIndex == -1) {
      return QByteArray();
   }

   der = der.mid(headerIndex + header.size(), footerIndex - (headerIndex + header.size()));

   if (der.contains("Proc-Type:")) {
      int i = 0;

      while (i < der.count()) {
         // field-name
         int j = der.indexOf(":", i);

         if (j == -1) {
            break;
         }

         const QByteArray field = der.mid(i, j - i).trimmed();
         j++;

         // any number of LWS is allowed before and after the value
         QByteArray value;

         do {
            i = der.indexOf("\n", j);

            if (i == -1) {
               break;
            }

            if (! value.isEmpty()) {
               value += ' ';
            }

            // check if we have CRLF or only LF
            bool hasCR = (i && der[i - 1] == '\r');
            int length = i - (hasCR ? 1 : 0) - j;
            value += der.mid(j, length).trimmed();
            j = ++i;

         } while (i < der.count() && (der.at(i) == ' ' || der.at(i) == '\t'));

         if (i == -1) {
            // something is wrong
            break;
         }

         headers->insert(field, value);
      }
      der = der.mid(i);
   }

   return QByteArray::fromBase64(der); // ignores newlines
}

QSslKey::QSslKey(const QByteArray &encoded, QSsl::KeyAlgorithm algorithm,
                 QSsl::EncodingFormat encoding, QSsl::KeyType type, const QByteArray &passPhrase)
   : d(new QSslKeyPrivate)
{
   d->type = type;
   d->algorithm = algorithm;

   if (encoding == QSsl::Der) {
      d->decodeDer(encoded);
   } else {
      d->decodePem(encoded, passPhrase);
   }
}

QSslKey::QSslKey(QIODevice *device, QSsl::KeyAlgorithm algorithm, QSsl::EncodingFormat encoding,
                 QSsl::KeyType type, const QByteArray &passPhrase)
   : d(new QSslKeyPrivate)
{
   QByteArray encoded;
   if (device) {
      encoded = device->readAll();
   }
   d->type = type;
   d->algorithm = algorithm;

   if (encoding == QSsl::Der) {
      d->decodeDer(encoded);
   } else {
      d->decodePem(encoded, passPhrase);
   }
}

QSslKey::QSslKey(Qt::HANDLE handle, QSsl::KeyType type)
   : d(new QSslKeyPrivate)
{

#ifdef QT_OPENSSL
   d->opaque = reinterpret_cast<EVP_PKEY *>(handle);
#else
   d->opaque = handle;
#endif
   d->algorithm = QSsl::Opaque;
   d->type = type;
   d->isNull = !d->opaque;
}

QSslKey::QSslKey(const QSslKey &other) : d(other.d)
{
}

QSslKey::~QSslKey()
{
}

QSslKey &QSslKey::operator=(const QSslKey &other)
{
   d = other.d;
   return *this;
}

bool QSslKey::isNull() const
{
   return d->isNull;
}

void QSslKey::clear()
{
   d = new QSslKeyPrivate;
}

int QSslKey::length() const
{
   return d->length();
}

QSsl::KeyType QSslKey::type() const
{
   return d->type;
}

QSsl::KeyAlgorithm QSslKey::algorithm() const
{
   return d->algorithm;
}

QByteArray QSslKey::toDer(const QByteArray &passPhrase) const
{
   if (d->isNull || d->algorithm == QSsl::Opaque) {
      return QByteArray();
   }

   // Encrypted DER is nonsense, see QTBUG-41038.
   if (d->type == QSsl::PrivateKey && !passPhrase.isEmpty()) {
      return QByteArray();
   }

#ifdef QT_OPENSSL
   QMap<QByteArray, QByteArray> headers;
   return d->derFromPem(toPem(passPhrase), &headers);
#else
   return d->derData;
#endif
}

QByteArray QSslKey::toPem(const QByteArray &passPhrase) const
{
   return d->toPem(passPhrase);
}

Qt::HANDLE QSslKey::handle() const
{
   return d->handle();
}

bool QSslKey::operator==(const QSslKey &other) const
{
   if (isNull()) {
      return other.isNull();
   }
   if (other.isNull()) {
      return isNull();
   }
   if (algorithm() != other.algorithm()) {
      return false;
   }
   if (type() != other.type()) {
      return false;
   }
   if (length() != other.length()) {
      return false;
   }
   if (algorithm() == QSsl::Opaque) {
      return handle() == other.handle();
   }
   return toDer() == other.toDer();
}


#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, const QSslKey &key)
{
   // QDebugStateSaver saver(debug);
   // debug.resetFormat().nospace();

   debug << "QSslKey("
         << (key.type() == QSsl::PublicKey ? "PublicKey" : "PrivateKey")
         << ", " << (key.algorithm() == QSsl::Opaque ? "OPAQUE" :
                     (key.algorithm() == QSsl::Rsa ? "RSA" : ((key.algorithm() == QSsl::Dsa) ? "DSA" : "EC")))
         << ", " << key.length()
         << ')';
   return debug;
}
#endif

QT_END_NAMESPACE
