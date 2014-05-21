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

#ifndef QABSTRACTSOCKET_H
#define QABSTRACTSOCKET_H

#include <QtCore/qiodevice.h>
#include <QtCore/qobject.h>

#ifndef QT_NO_DEBUG_STREAM
#include <QtCore/qdebug.h>
#endif

QT_BEGIN_NAMESPACE

class QAbstractSocketPrivate;
class QAuthenticator;
class QHostAddress;
class QHostInfo;

#ifndef QT_NO_NETWORKPROXY
class QNetworkProxy;
#endif

class Q_NETWORK_EXPORT QAbstractSocket : public QIODevice
{
    CS_OBJECT(QAbstractSocket)

    NET_CS_ENUM(SocketType)
    NET_CS_ENUM(NetworkLayerProtocol)
    NET_CS_ENUM(SocketError)
    NET_CS_ENUM(SocketState)
    NET_CS_ENUM(SocketOption)

public:
    enum SocketType {
        TcpSocket,
        UdpSocket,
        UnknownSocketType = -1
    };
    enum NetworkLayerProtocol {
        IPv4Protocol,
        IPv6Protocol,
        AnyIPProtocol,
        UnknownNetworkLayerProtocol = -1
    };
    enum SocketError {
        ConnectionRefusedError,
        RemoteHostClosedError,
        HostNotFoundError,
        SocketAccessError,
        SocketResourceError,
        SocketTimeoutError,                     /* 5 */
        DatagramTooLargeError,
        NetworkError,
        AddressInUseError,
        SocketAddressNotAvailableError,
        UnsupportedSocketOperationError,        /* 10 */
        UnfinishedSocketOperationError,
        ProxyAuthenticationRequiredError,
        SslHandshakeFailedError,
        ProxyConnectionRefusedError,
        ProxyConnectionClosedError,             /* 15 */
        ProxyConnectionTimeoutError,
        ProxyNotFoundError,
        ProxyProtocolError,

        UnknownSocketError = -1
    };

    enum SocketState {
        UnconnectedState,
        HostLookupState,
        ConnectingState,
        ConnectedState,
        BoundState,
        ListeningState,
        ClosingState
    };

    enum SocketOption {
        LowDelayOption, // TCP_NODELAY
        KeepAliveOption, // SO_KEEPALIVE
        MulticastTtlOption, // IP_MULTICAST_TTL
        MulticastLoopbackOption // IP_MULTICAST_LOOPBACK
    };

    QAbstractSocket(SocketType socketType, QObject *parent);
    virtual ~QAbstractSocket();

    // ### Qt5/Make connectToHost() and disconnectFromHost() virtual.
    void connectToHost(const QString &hostName, quint16 port, OpenMode mode = ReadWrite);
    void connectToHost(const QHostAddress &address, quint16 port, OpenMode mode = ReadWrite);
    void disconnectFromHost();

    bool isValid() const;

    qint64 bytesAvailable() const;
    qint64 bytesToWrite() const;

    bool canReadLine() const;

    quint16 localPort() const;
    QHostAddress localAddress() const;
    quint16 peerPort() const;
    QHostAddress peerAddress() const;
    QString peerName() const;

    // ### Qt5/Make setReadBufferSize() virtual
    qint64 readBufferSize() const;
    void setReadBufferSize(qint64 size);

    void abort();

    // ### Qt5/Make socketDescriptor() and setSocketDescriptor() virtual.
    int socketDescriptor() const;
    bool setSocketDescriptor(int socketDescriptor, SocketState state = ConnectedState,
                             OpenMode openMode = ReadWrite);

    // ### Qt5/Make virtual?
    void setSocketOption(QAbstractSocket::SocketOption option, const QVariant &value);
    QVariant socketOption(QAbstractSocket::SocketOption option);

    SocketType socketType() const;
    SocketState state() const;
    SocketError error() const;

    // from QIODevice
    void close();
    bool isSequential() const;
    bool atEnd() const;
    bool flush();

    // for synchronous access
    // ### Qt5/Make waitForConnected() and waitForDisconnected() virtual.
    bool waitForConnected(int msecs = 30000);
    bool waitForReadyRead(int msecs = 30000);
    bool waitForBytesWritten(int msecs = 30000);
    bool waitForDisconnected(int msecs = 30000);

#ifndef QT_NO_NETWORKPROXY
    void setProxy(const QNetworkProxy &networkProxy);
    QNetworkProxy proxy() const;
#endif

    NET_CS_SIGNAL_1(Public, void hostFound())
    NET_CS_SIGNAL_2(hostFound) 

    NET_CS_SIGNAL_1(Public, void connected())
    NET_CS_SIGNAL_2(connected) 

    NET_CS_SIGNAL_1(Public, void disconnected())
    NET_CS_SIGNAL_2(disconnected) 

    NET_CS_SIGNAL_1(Public, void stateChanged(QAbstractSocket::SocketState un_named_arg1))
    NET_CS_SIGNAL_2(stateChanged,un_named_arg1) 

    NET_CS_SIGNAL_1(Public, void error(QAbstractSocket::SocketError un_named_arg1))
    NET_CS_SIGNAL_OVERLOAD(error,(QAbstractSocket::SocketError),un_named_arg1) 

#ifndef QT_NO_NETWORKPROXY
    NET_CS_SIGNAL_1(Public, void proxyAuthenticationRequired(const QNetworkProxy & proxy,QAuthenticator * authenticator))
    NET_CS_SIGNAL_2(proxyAuthenticationRequired,proxy,authenticator) 
#endif

protected :
    virtual void connectToHostImplementation(const QString & hostName,unsigned short port,QIODevice::OpenMode mode = ReadWrite);

    NET_CS_SLOT_1(Protected, void disconnectFromHostImplementation())
    NET_CS_SLOT_2(disconnectFromHostImplementation) 

    qint64 readData(char *data, qint64 maxlen);
    qint64 readLineData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

    void setSocketState(SocketState state);
    void setSocketError(SocketError socketError);
    void setLocalPort(quint16 port);
    void setLocalAddress(const QHostAddress &address);
    void setPeerPort(quint16 port);
    void setPeerAddress(const QHostAddress &address);
    void setPeerName(const QString &name);

    QAbstractSocket(SocketType socketType, QAbstractSocketPrivate &dd, QObject *parent = 0);

private:
    Q_DECLARE_PRIVATE(QAbstractSocket)
    Q_DISABLE_COPY(QAbstractSocket)

    NET_CS_SLOT_1(Private, void _q_connectToNextAddress())
    NET_CS_SLOT_2(_q_connectToNextAddress)

    NET_CS_SLOT_1(Private, void _q_startConnecting(const QHostInfo & un_named_arg1))
    NET_CS_SLOT_2(_q_startConnecting)

    NET_CS_SLOT_1(Private, void _q_abortConnectionAttempt())
    NET_CS_SLOT_2(_q_abortConnectionAttempt)

    NET_CS_SLOT_1(Private, void _q_testConnection())
    NET_CS_SLOT_2(_q_testConnection)

    NET_CS_SLOT_1(Private, void _q_forceDisconnect())
    NET_CS_SLOT_2(_q_forceDisconnect)
};

#ifndef QT_NO_DEBUG_STREAM
Q_NETWORK_EXPORT QDebug operator<<(QDebug, QAbstractSocket::SocketError);
Q_NETWORK_EXPORT QDebug operator<<(QDebug, QAbstractSocket::SocketState);
#endif

QT_END_NAMESPACE

// moved to bottom of file to avoid recursive include issues
#include "qhostinfo.h"

#ifndef QT_NO_NETWORKPROXY
#include "qnetworkproxy.h"
#endif

#endif // QABSTRACTSOCKET_H
