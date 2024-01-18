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

#ifndef QSSLKEY_P_H
#define QSSLKEY_P_H

#include <qsslkey.h>
#include <qsslsocket_p.h>       // includes wincrypt.h

#ifdef QT_OPENSSL
#include <openssl/rsa.h>
#include <openssl/dsa.h>
#endif

class QSslKeyPrivate
{
 public:
   enum Cipher {
        DesCbc,
        DesEde3Cbc,
        Rc2Cbc
    };

   QSslKeyPrivate()
      : algorithm(QSsl::Opaque), opaque(nullptr)
   {
      clear(false);
   }

   QSslKeyPrivate(const QSslKeyPrivate &) = delete;
   QSslKeyPrivate &operator=(const QSslKeyPrivate &) = delete;

   ~QSslKeyPrivate() {
      clear();
   }

#ifdef QT_OPENSSL
    bool fromEVP_PKEY(EVP_PKEY *pkey);
#endif

   void clear(bool deep = true);

   void decodeDer(const QByteArray &der, bool deepClear = true);
   void decodePem(const QByteArray &pem, const QByteArray &passPhrase, bool deepClear = true);

   QByteArray pemHeader() const;
   QByteArray pemFooter() const;
   QByteArray pemFromDer(const QByteArray &der, const QMap<QByteArray, QByteArray> &headers) const;
   QByteArray derFromPem(const QByteArray &pem, QMap<QByteArray, QByteArray> *headers) const;

   int length() const;
   QByteArray toPem(const QByteArray &passPhrase) const;
   Qt::HANDLE handle() const;

   bool isNull;
   QSsl::KeyType type;
   QSsl::KeyAlgorithm algorithm;

   static QByteArray decrypt(Cipher cipher, const QByteArray &data, const QByteArray &key, const QByteArray &iv);
   static QByteArray encrypt(Cipher cipher, const QByteArray &data, const QByteArray &key, const QByteArray &iv);

#ifdef QT_OPENSSL
    union {
        EVP_PKEY *opaque;
        RSA *rsa;
        DSA *dsa;
#ifndef OPENSSL_NO_EC
        EC_KEY *ec;
#endif
    };

#else
    Qt::HANDLE opaque;
    QByteArray derData;
    int keyLength;
#endif

    QAtomicInt ref;
};

#endif
