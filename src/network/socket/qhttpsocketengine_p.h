/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#ifndef QHTTPSOCKETENGINE_P_H
#define QHTTPSOCKETENGINE_P_H

#include "qabstractsocketengine_p.h"
#include "qabstractsocket.h"
#include "qnetworkproxy.h"
#include "qauthenticator_p.h"

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_NETWORKPROXY) && !defined(QT_NO_HTTP)

class QTcpSocket;
class QHttpSocketEnginePrivate;

class QHttpSocketEngine : public QAbstractSocketEngine
{
    CS_OBJECT(QHttpSocketEngine)
public:
    enum HttpState {
        None,
        ConnectSent,
        Connected,
        SendAuthentication,
        ReadResponseContent
    };
    QHttpSocketEngine(QObject *parent = 0);
    ~QHttpSocketEngine();

    bool initialize(QAbstractSocket::SocketType type, QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::IPv4Protocol);
    bool initialize(int socketDescriptor, QAbstractSocket::SocketState socketState = QAbstractSocket::ConnectedState);

    void setProxy(const QNetworkProxy &networkProxy);

    int socketDescriptor() const;

    bool isValid() const;

    bool connectInternal();
    bool connectToHost(const QHostAddress &address, quint16 port);
    bool connectToHostByName(const QString &name, quint16 port);
    bool bind(const QHostAddress &address, quint16 port);
    bool listen();
    int accept();
    void close();

    qint64 bytesAvailable() const;

    qint64 read(char *data, qint64 maxlen);
    qint64 write(const char *data, qint64 len);

#ifndef QT_NO_UDPSOCKET
#ifndef QT_NO_NETWORKINTERFACE
    bool joinMulticastGroup(const QHostAddress &groupAddress,
                            const QNetworkInterface &interface);
    bool leaveMulticastGroup(const QHostAddress &groupAddress,
                             const QNetworkInterface &interface);
    QNetworkInterface multicastInterface() const;
    bool setMulticastInterface(const QNetworkInterface &iface);
#endif // QT_NO_NETWORKINTERFACE

    qint64 readDatagram(char *data, qint64 maxlen, QHostAddress *addr = 0,
        quint16 *port = 0);
    qint64 writeDatagram(const char *data, qint64 len, const QHostAddress &addr,
        quint16 port);
    bool hasPendingDatagrams() const;
    qint64 pendingDatagramSize() const;
#endif // QT_NO_UDPSOCKET

    qint64 bytesToWrite() const;

    int option(SocketOption option) const;
    bool setOption(SocketOption option, int value);

    bool waitForRead(int msecs = 30000, bool *timedOut = 0);
    bool waitForWrite(int msecs = 30000, bool *timedOut = 0);
    bool waitForReadOrWrite(bool *readyToRead, bool *readyToWrite,
                            bool checkRead, bool checkWrite,
                            int msecs = 30000, bool *timedOut = 0);

    bool isReadNotificationEnabled() const;
    void setReadNotificationEnabled(bool enable);
    bool isWriteNotificationEnabled() const;
    void setWriteNotificationEnabled(bool enable);
    bool isExceptionNotificationEnabled() const;
    void setExceptionNotificationEnabled(bool enable);

public :
    NET_CS_SLOT_1(Public, void slotSocketConnected())
    NET_CS_SLOT_2(slotSocketConnected) 
    NET_CS_SLOT_1(Public, void slotSocketDisconnected())
    NET_CS_SLOT_2(slotSocketDisconnected) 
    NET_CS_SLOT_1(Public, void slotSocketReadNotification())
    NET_CS_SLOT_2(slotSocketReadNotification) 
    NET_CS_SLOT_1(Public, void slotSocketBytesWritten())
    NET_CS_SLOT_2(slotSocketBytesWritten) 
    NET_CS_SLOT_1(Public, void slotSocketError(QAbstractSocket::SocketError error))
    NET_CS_SLOT_2(slotSocketError) 
    NET_CS_SLOT_1(Public, void slotSocketStateChanged(QAbstractSocket::SocketState state))
    NET_CS_SLOT_2(slotSocketStateChanged) 

private :
    NET_CS_SLOT_1(Private, void emitPendingReadNotification())
    NET_CS_SLOT_2(emitPendingReadNotification) 
    NET_CS_SLOT_1(Private, void emitPendingWriteNotification())
    NET_CS_SLOT_2(emitPendingWriteNotification) 
    NET_CS_SLOT_1(Private, void emitPendingConnectionNotification())
    NET_CS_SLOT_2(emitPendingConnectionNotification) 

private:
    void emitReadNotification();
    void emitWriteNotification();
    void emitConnectionNotification();

    Q_DECLARE_PRIVATE(QHttpSocketEngine)
    Q_DISABLE_COPY(QHttpSocketEngine)

};


class QHttpSocketEnginePrivate : public QAbstractSocketEnginePrivate
{
    Q_DECLARE_PUBLIC(QHttpSocketEngine)
public:
    QHttpSocketEnginePrivate();
    ~QHttpSocketEnginePrivate();

    QNetworkProxy proxy;
    QString peerName;
    QTcpSocket *socket;
    QByteArray readBuffer; // only used for parsing the proxy response
    QHttpSocketEngine::HttpState state;
    QAuthenticator authenticator;
    bool readNotificationEnabled;
    bool writeNotificationEnabled;
    bool exceptNotificationEnabled;
    bool readNotificationActivated;
    bool writeNotificationActivated;
    bool readNotificationPending;
    bool writeNotificationPending;
    bool connectionNotificationPending;
    bool credentialsSent;
    uint pendingResponseData;
};

class QHttpSocketEngineHandler : public QSocketEngineHandler
{
public:
    virtual QAbstractSocketEngine *createSocketEngine(QAbstractSocket::SocketType socketType,
                                                      const QNetworkProxy &, QObject *parent);
    virtual QAbstractSocketEngine *createSocketEngine(int socketDescripter, QObject *parent);
};
#endif

QT_END_NAMESPACE

#endif // QHTTPSOCKETENGINE_H
