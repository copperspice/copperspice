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

#ifndef QSSLCIPHER_H
#define QSSLCIPHER_H

#include <qstring.h>
#include <qscopedpointer.h>
#include <qssl.h>

#ifdef QT_SSL

class QSslCipherPrivate;

class Q_NETWORK_EXPORT QSslCipher
{

public:
   QSslCipher();
   explicit QSslCipher(const QString &name);
   QSslCipher(const QString &name, QSsl::SslProtocol protocol);
   QSslCipher(const QSslCipher &other);

   ~QSslCipher();

   bool isNull() const;
   QString name() const;
   int supportedBits() const;
   int usedBits() const;

   QString keyExchangeMethod() const;
   QString authenticationMethod() const;
   QString encryptionMethod() const;
   QString protocolString() const;
   QSsl::SslProtocol protocol() const;

   void swap(QSslCipher &other) {
      qSwap(d, other.d);
   }


   QSslCipher &operator=(QSslCipher &&other) {
      swap(other);
      return *this;
   }

   QSslCipher &operator=(const QSslCipher &other);
   bool operator==(const QSslCipher &other) const;

   inline bool operator!=(const QSslCipher &other) const {
      return !operator==(other);
   }

private:
   QScopedPointer<QSslCipherPrivate> d;
   friend class QSslSocketBackendPrivate;
};

class QDebug;
Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, const QSslCipher &cipher);

#endif

#endif

