/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QSSL_H
#define QSSL_H

#include <QtCore/qglobal.h>
#include <QtCore/QFlags>

QT_BEGIN_NAMESPACE

namespace QSsl {
enum KeyType {
   PrivateKey,
   PublicKey
};

enum EncodingFormat {
   Pem,
   Der
};

enum KeyAlgorithm {
   Rsa,
   Dsa
};

enum AlternateNameEntryType {
   EmailEntry,
   DnsEntry
};

enum SslProtocol {
   SslV3,
   SslV2,
   TlsV1, // ### Qt5/rename to TlsV1_0 or so
   AnyProtocol,
   TlsV1SslV3,
   SecureProtocols,
   UnknownProtocol = -1
};

enum SslOption {
   SslOptionDisableEmptyFragments = 0x01,
   SslOptionDisableSessionTickets = 0x02,
   SslOptionDisableCompression = 0x04,
   SslOptionDisableServerNameIndication = 0x08,
   SslOptionDisableLegacyRenegotiation = 0x10
};
using SslOptions = QFlags<SslOption>;
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QSsl::SslOptions)

QT_END_NAMESPACE

#endif // QSSL_H
