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

#ifndef QSSLKEY_H
#define QSSLKEY_H

#include <QtCore/qnamespace.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qsharedpointer.h>
#include <QtNetwork/qssl.h>
#include <qcontainerfwd.h>

QT_BEGIN_NAMESPACE

#ifdef QT_SSL

class QDebug;
class QIODevice;
class QSslKeyPrivate;

class Q_NETWORK_EXPORT QSslKey
{

 public:
   QSslKey();
   QSslKey(const QByteArray &encoded, QSsl::KeyAlgorithm algorithm, QSsl::EncodingFormat format = QSsl::Pem,
           QSsl::KeyType type = QSsl::PrivateKey, const QByteArray &passPhrase = QByteArray());

   QSslKey(QIODevice *device, QSsl::KeyAlgorithm algorithm, QSsl::EncodingFormat format = QSsl::Pem,
           QSsl::KeyType type = QSsl::PrivateKey, const QByteArray &passPhrase = QByteArray());

   explicit QSslKey(Qt::HANDLE handle, QSsl::KeyType type = QSsl::PrivateKey);
   QSslKey(const QSslKey &other);

   ~QSslKey();

   QSslKey &operator=(QSslKey &&other) {
      swap(other);
      return *this;
   }

   QSslKey &operator=(const QSslKey &other);

   bool isNull() const;
   void clear();

   int length() const;
   QSsl::KeyType type() const;
   QSsl::KeyAlgorithm algorithm() const;

   QByteArray toPem(const QByteArray &passPhrase = QByteArray()) const;
   QByteArray toDer(const QByteArray &passPhrase = QByteArray()) const;

   Qt::HANDLE handle() const;

   void swap(QSslKey &other) {
      qSwap(d, other.d);
   }

   bool operator==(const QSslKey &key) const;
   inline bool operator!=(const QSslKey &key) const {
      return !operator==(key);
   }

 private:
   QExplicitlySharedDataPointer<QSslKeyPrivate> d;
   friend class QSslCertificate;
   friend class QSslSocketBackendPrivate;
};

Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, const QSslKey &key);

#endif

QT_END_NAMESPACE

#endif
