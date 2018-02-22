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

#include <qsslpresharedkeyauthenticator.h>
#include <qsslpresharedkeyauthenticator_p.h>

#include <QSharedData>

// internal

QSslPreSharedKeyAuthenticatorPrivate::QSslPreSharedKeyAuthenticatorPrivate()
   : maximumIdentityLength(0), maximumPreSharedKeyLength(0)
{
}

QSslPreSharedKeyAuthenticator::QSslPreSharedKeyAuthenticator()
   : d(new QSslPreSharedKeyAuthenticatorPrivate)
{
}

QSslPreSharedKeyAuthenticator::~QSslPreSharedKeyAuthenticator()
{
}

QSslPreSharedKeyAuthenticator::QSslPreSharedKeyAuthenticator(const QSslPreSharedKeyAuthenticator &authenticator)
   : d(authenticator.d)
{
}

QSslPreSharedKeyAuthenticator &QSslPreSharedKeyAuthenticator::operator=(const QSslPreSharedKeyAuthenticator &authenticator)
{
   d = authenticator.d;
   return *this;
}

QByteArray QSslPreSharedKeyAuthenticator::identityHint() const
{
   return d->identityHint;
}

void QSslPreSharedKeyAuthenticator::setIdentity(const QByteArray &identity)
{
   d->identity = identity;
}

QByteArray QSslPreSharedKeyAuthenticator::identity() const
{
   return d->identity;
}


int QSslPreSharedKeyAuthenticator::maximumIdentityLength() const
{
   return d->maximumIdentityLength;
}

void QSslPreSharedKeyAuthenticator::setPreSharedKey(const QByteArray &preSharedKey)
{
   d->preSharedKey = preSharedKey;
}

QByteArray QSslPreSharedKeyAuthenticator::preSharedKey() const
{
   return d->preSharedKey;
}

int QSslPreSharedKeyAuthenticator::maximumPreSharedKeyLength() const
{
   return d->maximumPreSharedKeyLength;
}

bool operator==(const QSslPreSharedKeyAuthenticator &lhs, const QSslPreSharedKeyAuthenticator &rhs)
{
   return ((lhs.d == rhs.d) ||
           (lhs.d->identityHint == rhs.d->identityHint &&
            lhs.d->identity == rhs.d->identity &&
            lhs.d->maximumIdentityLength == rhs.d->maximumIdentityLength &&
            lhs.d->preSharedKey == rhs.d->preSharedKey &&
            lhs.d->maximumPreSharedKeyLength == rhs.d->maximumPreSharedKeyLength));
}

