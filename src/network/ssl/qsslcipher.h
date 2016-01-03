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

#ifndef QSSLCIPHER_H
#define QSSLCIPHER_H

#include <QtCore/qstring.h>
#include <QtCore/qscopedpointer.h>
#include <QtNetwork/qssl.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_OPENSSL

class QSslCipherPrivate;

class Q_NETWORK_EXPORT QSslCipher
{

 public:
   QSslCipher();
   QSslCipher(const QString &name, QSsl::SslProtocol protocol);
   QSslCipher(const QSslCipher &other);
   ~QSslCipher();
   QSslCipher &operator=(const QSslCipher &other);
   bool operator==(const QSslCipher &other) const;
   inline bool operator!=(const QSslCipher &other) const {
      return !operator==(other);
   }

   bool isNull() const;
   QString name() const;
   int supportedBits() const;
   int usedBits() const;

   QString keyExchangeMethod() const;
   QString authenticationMethod() const;
   QString encryptionMethod() const;
   QString protocolString() const;
   QSsl::SslProtocol protocol() const;

 private:
   QScopedPointer<QSslCipherPrivate> d;
   friend class QSslSocketBackendPrivate;
};

class QDebug;
Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, const QSslCipher &cipher);

#endif // QT_NO_OPENSSL

QT_END_NAMESPACE

#endif

