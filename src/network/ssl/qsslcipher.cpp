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

#include <qsslcipher.h>
#include <qsslcipher_p.h>
#include <qsslsocket.h>
#include <qsslconfiguration.h>
#include <qdebug.h>

QSslCipher::QSslCipher()
   : d(new QSslCipherPrivate)
{
}

QSslCipher::QSslCipher(const QString &name)
   : d(new QSslCipherPrivate)
{
   for (const QSslCipher &cipher : QSslConfiguration::supportedCiphers()) {
      if (cipher.name() == name) {
         *this = cipher;
         return;
      }
   }
}

QSslCipher::QSslCipher(const QString &name, QSsl::SslProtocol protocol)
   : d(new QSslCipherPrivate)
{
   for (const QSslCipher &cipher : QSslConfiguration::supportedCiphers()) {
      if (cipher.name() == name && cipher.protocol() == protocol) {
         *this = cipher;
         return;
      }
   }
}

QSslCipher::QSslCipher(const QSslCipher &other)
   : d(new QSslCipherPrivate)
{
   *d.data() = *other.d.data();
}

QSslCipher::~QSslCipher()
{
}

QSslCipher &QSslCipher::operator=(const QSslCipher &other)
{
   *d.data() = *other.d.data();
   return *this;
}

bool QSslCipher::operator==(const QSslCipher &other) const
{
   return d->name == other.d->name && d->protocol == other.d->protocol;
}

bool QSslCipher::isNull() const
{
   return d->isNull;
}

QString QSslCipher::name() const
{
   return d->name;
}

int QSslCipher::supportedBits() const
{
   return d->supportedBits;
}

int QSslCipher::usedBits() const
{
   return d->bits;
}

QString QSslCipher::keyExchangeMethod() const
{
   return d->keyExchangeMethod;
}

QString QSslCipher::authenticationMethod() const
{
   return d->authenticationMethod;
}

QString QSslCipher::encryptionMethod() const
{
   return d->encryptionMethod;
}

QString QSslCipher::protocolString() const
{
   return d->protocolString;
}

QSsl::SslProtocol QSslCipher::protocol() const
{
   return d->protocol;
}

QDebug operator<<(QDebug debug, const QSslCipher &cipher)
{
   // QDebugStateSaver saver(debug);
   // debug.resetFormat().nospace().noquote();

   debug << "QSslCipher(name=" << cipher.name()
         << ", bits="  << cipher.usedBits()
         << ", proto=" << cipher.protocolString()
         << ')';
   return debug;
}

