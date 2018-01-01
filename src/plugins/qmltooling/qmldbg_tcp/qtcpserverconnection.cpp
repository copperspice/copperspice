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

#include <qtcpserverconnection.h>
#include <QtCore/qplugin.h>
#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>
#include <qdeclarativedebugserver_p.h>
#include <qpacketprotocol_p.h>

QT_BEGIN_NAMESPACE

class QTcpServerConnectionPrivate {
public:
    QTcpServerConnectionPrivate();

    int port;
    bool block;
    QTcpSocket *socket;
    QPacketProtocol *protocol;
    QTcpServer *tcpServer;

    QDeclarativeDebugServer *debugServer;
};

QTcpServerConnectionPrivate::QTcpServerConnectionPrivate() :
    port(0),
    block(false),
    socket(0),
    protocol(0),
    tcpServer(0),
    debugServer(0)
{
}

QTcpServerConnection::QTcpServerConnection() :
    d_ptr(new QTcpServerConnectionPrivate)
{

}

QTcpServerConnection::~QTcpServerConnection()
{
    delete d_ptr;
}

void QTcpServerConnection::setServer(QDeclarativeDebugServer *server)
{
    Q_D(QTcpServerConnection);
    d->debugServer = server;
}

bool QTcpServerConnection::isConnected() const
{
    Q_D(const QTcpServerConnection);
    return d->socket && d->socket->state() == QTcpSocket::ConnectedState;
}

void QTcpServerConnection::send(const QByteArray &message)
{
    Q_D(QTcpServerConnection);

    if (!isConnected()
            || !d->protocol || !d->socket)
        return;

    QPacket pack;
    pack.writeRawData(message.data(), message.length());

    d->protocol->send(pack);
    d->socket->flush();
}

void QTcpServerConnection::disconnect()
{
    Q_D(QTcpServerConnection);

    // protocol might still be processing packages at this point
    d->protocol->deleteLater();
    d->protocol = 0;
    d->socket->deleteLater();
    d->socket = 0;
}

bool QTcpServerConnection::waitForMessage()
{
    Q_D(QTcpServerConnection);
    if (d->protocol->packetsAvailable() > 0) {
        QPacket packet = d->protocol->read();
        d->debugServer->receiveMessage(packet.data());
        return true;
    } else {
        return d->protocol->waitForReadyRead(-1);
    }
}

void QTcpServerConnection::setPort(int port, bool block)
{
    Q_D(QTcpServerConnection);
    d->port = port;
    d->block = block;

    listen();
    if (block)
        d->tcpServer->waitForNewConnection(-1);
}

void QTcpServerConnection::listen()
{
    Q_D(QTcpServerConnection);

    d->tcpServer = new QTcpServer(this);
    QObject::connect(d->tcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
    if (d->tcpServer->listen(QHostAddress::Any, d->port)) {
        qDebug("QDeclarativeDebugServer: Waiting for connection on port %d...", d->port);
    } else {
        qWarning("QDeclarativeDebugServer: Unable to listen on port %d", d->port);
    }
}


void QTcpServerConnection::readyRead()
{
    Q_D(QTcpServerConnection);
    if (!d->protocol)
        return;

    while (d->protocol->packetsAvailable() > 0) {
        QPacket packet = d->protocol->read();
        d->debugServer->receiveMessage(packet.data());
    }
}

void QTcpServerConnection::newConnection()
{
    Q_D(QTcpServerConnection);

    if (d->socket) {
        qWarning("QDeclarativeDebugServer: Another client is already connected");
        QTcpSocket *faultyConnection = d->tcpServer->nextPendingConnection();
        delete faultyConnection;
        return;
    }

    d->socket = d->tcpServer->nextPendingConnection();
    d->socket->setParent(this);
    d->protocol = new QPacketProtocol(d->socket, this);
    QObject::connect(d->protocol, SIGNAL(readyRead()), this, SLOT(readyRead()));

    if (d->block) {
        d->protocol->waitForReadyRead(-1);
    }
}

Q_EXPORT_PLUGIN2(tcpserver, QTcpServerConnection)

QT_END_NAMESPACE

