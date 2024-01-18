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

#ifndef QSSLPRESHAREDKEYAUTHENTICATOR_H
#define QSSLPRESHAREDKEYAUTHENTICATOR_H

#include <qglobal.h>
#include <qstring.h>
#include <qshareddatapointer.h>

class QSslPreSharedKeyAuthenticatorPrivate;

class QSslPreSharedKeyAuthenticator
{
 public:
    Q_NETWORK_EXPORT QSslPreSharedKeyAuthenticator();
    Q_NETWORK_EXPORT QSslPreSharedKeyAuthenticator(const QSslPreSharedKeyAuthenticator &other);

    Q_NETWORK_EXPORT ~QSslPreSharedKeyAuthenticator();

    Q_NETWORK_EXPORT QSslPreSharedKeyAuthenticator &operator=(const QSslPreSharedKeyAuthenticator &other);

    QSslPreSharedKeyAuthenticator &operator=(QSslPreSharedKeyAuthenticator &&other)  {
      swap(other); return *this;
    }

    void swap(QSslPreSharedKeyAuthenticator &other) {
      qSwap(d, other.d);
    }

    Q_NETWORK_EXPORT QByteArray identityHint() const;

    Q_NETWORK_EXPORT void setIdentity(const QByteArray &identity);
    Q_NETWORK_EXPORT QByteArray identity() const;
    Q_NETWORK_EXPORT int maximumIdentityLength() const;

    Q_NETWORK_EXPORT void setPreSharedKey(const QByteArray &preSharedKey);
    Q_NETWORK_EXPORT QByteArray preSharedKey() const;
    Q_NETWORK_EXPORT int maximumPreSharedKeyLength() const;

 private:
    friend Q_NETWORK_EXPORT bool operator==(const QSslPreSharedKeyAuthenticator &lhs, const QSslPreSharedKeyAuthenticator &rhs);
    friend class QSslSocketBackendPrivate;

    QSharedDataPointer<QSslPreSharedKeyAuthenticatorPrivate> d;
};

inline bool operator!=(const QSslPreSharedKeyAuthenticator &lhs, const QSslPreSharedKeyAuthenticator &rhs)
{
    return !operator==(lhs, rhs);
}

#endif
