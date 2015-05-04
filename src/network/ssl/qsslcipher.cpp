/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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


/*!
    \class QSslCipher
    \brief The QSslCipher class represents an SSL cryptographic cipher.
    \since 4.3

    \reentrant
    \ingroup network
    \ingroup ssl
    \inmodule QtNetwork

    QSslCipher stores information about one cryptographic cipher. It
    is most commonly used with QSslSocket, either for configuring
    which ciphers the socket can use, or for displaying the socket's
    ciphers to the user.

    \sa QSslSocket, QSslKey
*/

#include <qsslcipher.h>
#include <qsslcipher_p.h>
#include <qsslsocket.h>

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

/*!
    Constructs an empty QSslCipher object.
*/
QSslCipher::QSslCipher()
   : d(new QSslCipherPrivate)
{
}

/*!
    Constructs a QSslCipher object for the cipher determined by \a
    name and \a protocol. The constructor accepts only supported
    ciphers (i.e., the \a name and \a protocol must identify a cipher
    in the list of ciphers returned by
    QSslSocket::supportedCiphers()).

    You can call isNull() after construction to check if \a name and
    \a protocol correctly identified a supported cipher.
*/
QSslCipher::QSslCipher(const QString &name, QSsl::SslProtocol protocol)
   : d(new QSslCipherPrivate)
{
   foreach (const QSslCipher & cipher, QSslSocket::supportedCiphers()) {
      if (cipher.name() == name && cipher.protocol() == protocol) {
         *this = cipher;
         return;
      }
   }
}

/*!
    Constructs an identical copy of the \a other cipher.
*/
QSslCipher::QSslCipher(const QSslCipher &other)
   : d(new QSslCipherPrivate)
{
   *d.data() = *other.d.data();
}

/*!
    Destroys the QSslCipher object.
*/
QSslCipher::~QSslCipher()
{
}

/*!
    Copies the contents of \a other into this cipher, making the two
    ciphers identical.
*/
QSslCipher &QSslCipher::operator=(const QSslCipher &other)
{
   *d.data() = *other.d.data();
   return *this;
}

/*!
    Returns true if this cipher is the same as \a other; otherwise,
    false is returned.
*/
bool QSslCipher::operator==(const QSslCipher &other) const
{
   return d->name == other.d->name && d->protocol == other.d->protocol;
}

/*!
    \fn bool QSslCipher::operator!=(const QSslCipher &other) const

    Returns true if this cipher is not the same as \a other;
    otherwise, false is returned.
*/

/*!
    Returns true if this is a null cipher; otherwise returns false.
*/
bool QSslCipher::isNull() const
{
   return d->isNull;
}

/*!
    Returns the name of the cipher, or an empty QString if this is a null
    cipher.

    \sa isNull()
*/
QString QSslCipher::name() const
{
   return d->name;
}

/*!
    Returns the number of bits supported by the cipher.

    \sa usedBits()
*/
int QSslCipher::supportedBits()const
{
   return d->supportedBits;
}

/*!
    Returns the number of bits used by the cipher.

    \sa supportedBits()
*/
int QSslCipher::usedBits() const
{
   return d->bits;
}

/*!
    Returns the cipher's key exchange method as a QString.
*/
QString QSslCipher::keyExchangeMethod() const
{
   return d->keyExchangeMethod;
}

/*!
    Returns the cipher's authentication method as a QString.
*/
QString QSslCipher::authenticationMethod() const
{
   return d->authenticationMethod;
}

/*!
    Returns the cipher's encryption method as a QString.
*/
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
   debug << "QSslCipher(name=" << qPrintable(cipher.name())
         << ", bits=" << cipher.usedBits()
         << ", proto=" << qPrintable(cipher.protocolString())
         << ')';
   return debug;
}

QT_END_NAMESPACE
