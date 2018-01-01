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

#include <qmlostplugin.h>
#include <qostdevice.h>
#include <qdeclarativedebugserver_p.h>
#include <qpacketprotocol_p.h>

QT_BEGIN_NAMESPACE

static const TInt KQmlOstProtocolId = 0x94;

class QmlOstPluginPrivate {
public:
    QmlOstPluginPrivate();

    QOstDevice *ost;
    QPacketProtocol *protocol;
    QDeclarativeDebugServer *debugServer;
};

QmlOstPluginPrivate::QmlOstPluginPrivate() :
    ost(0),
    protocol(0),
    debugServer(0)
{
}

QmlOstPlugin::QmlOstPlugin() :
    d_ptr(new QmlOstPluginPrivate)
{
}

QmlOstPlugin::~QmlOstPlugin()
{
    delete d_ptr;
}

void QmlOstPlugin::setServer(QDeclarativeDebugServer *server)
{
    Q_D(QmlOstPlugin);
    d->debugServer = server;
}

bool QmlOstPlugin::isConnected() const
{
    Q_D(const QmlOstPlugin);
    return d->ost && d->ost->isOpen();
}

void QmlOstPlugin::send(const QByteArray &message)
{
    Q_D(QmlOstPlugin);

    if (!isConnected())
        return;

    QPacket pack;
    pack.writeRawData(message.data(), message.length());

    d->protocol->send(pack);
    //d->socket->flush();
}

void QmlOstPlugin::disconnect()
{
    Q_D(QmlOstPlugin);

    delete d->protocol;
    d->protocol = 0;
}

bool QmlOstPlugin::waitForMessage()
{
    Q_D(QmlOstPlugin);
    return d->protocol->waitForReadyRead(-1);
}

void QmlOstPlugin::setPort(int port, bool block)
{
    Q_UNUSED(port);
    Q_UNUSED(block);

    Q_D(QmlOstPlugin);

    d->ost = new QOstDevice(this);
    bool ok = d->ost->open(KQmlOstProtocolId);
    if (!ok) {
        if (d->ost->errorString().length())
            qDebug("Error from QOstDevice: %s", qPrintable(d->ost->errorString()));
        qWarning("QDeclarativeDebugServer: Unable to listen on OST"); // This message is part of the signalling - do not change the format!
        return;
    }
    d->protocol = new QPacketProtocol(d->ost, this);
    QObject::connect(d->protocol, SIGNAL(readyRead()), this, SLOT(readyRead()));
    qWarning("QDeclarativeDebugServer: Waiting for connection via OST"); // This message is part of the signalling - do not change the format!
}

void QmlOstPlugin::readyRead()
{
    Q_D(QmlOstPlugin);
    QPacket packet = d->protocol->read();

    QByteArray content = packet.data();
    d->debugServer->receiveMessage(content);
}

Q_EXPORT_PLUGIN2(qmlostplugin, QmlOstPlugin)

QT_END_NAMESPACE
