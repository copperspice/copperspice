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

#ifndef QSSLSOCKET_MAC_P_H
#define QSSLSOCKET_MAC_P_H

#include <qabstractsocket.h>
#include <qsslsocket_p.h>

#include <QtCore/qglobal.h>
#include <qstring.h>
#include <qlist.h>

#include <Security/Security.h>
#include <Security/SecureTransport.h>



class QSecureTransportContext
{
 public:
    explicit QSecureTransportContext(SSLContextRef context);
    ~QSecureTransportContext();

    operator SSLContextRef () const;
    void reset(SSLContextRef newContext);

 private:
    SSLContextRef context;

    Q_DISABLE_COPY(QSecureTransportContext)
};

class QSslSocketBackendPrivate : public QSslSocketPrivate
{
    Q_DECLARE_PUBLIC(QSslSocket)

 public:
    QSslSocketBackendPrivate();
    virtual ~QSslSocketBackendPrivate();

    // Final-overriders (QSslSocketPrivate):
    void continueHandshake() override;
    void disconnected() override;
    void disconnectFromHost() override;
    QSslCipher sessionCipher() const override;
    QSsl::SslProtocol sessionProtocol() const override;
    void startClientEncryption() override;
    void startServerEncryption() override;
    void transmit() override;

    static QList<QSslError> verify(QList<QSslCertificate> certificateChain, const QString &hostName);

    static bool importPkcs12(QIODevice *device, QSslKey *key, QSslCertificate *cert,
                  QList<QSslCertificate> *caCertificates, const QByteArray &passPhrase);

    static QSslCipher QSslCipher_from_SSLCipherSuite(SSLCipherSuite cipher);

 private:
    // SSL context management/properties:
    bool initSslContext();
    void destroySslContext();
    bool setSessionCertificate(QString &errorDescription, QAbstractSocket::SocketError &errorCode);
    bool setSessionProtocol();
    // Aux. functions to do a verification during handshake phase:
    bool canIgnoreTrustVerificationFailure() const;
    bool verifySessionProtocol() const;
    bool verifyPeerTrust();

    bool checkSslErrors();
    bool startHandshake();

    QSecureTransportContext context;

    Q_DISABLE_COPY(QSslSocketBackendPrivate)
};



#endif
