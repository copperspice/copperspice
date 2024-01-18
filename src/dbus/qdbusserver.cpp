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

#include "qdbusserver.h"
#include "qdbusconnection_p.h"
#include "qdbusconnectionmanager_p.h"

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

/*!
    \class QDBusServer
    \inmodule QtDBus
    \internal

    \brief The QDBusServer class provides peer-to-peer communication
    between processes on the same computer.
*/

/*!
    Constructs a QDBusServer with the given \a address, and the given
    \a parent.
*/
QDBusServer::QDBusServer(const QString &address, QObject *parent)
    : QObject(parent)
{
    if (address.isEmpty())
        return;

    if (!qdbus_loadLibDBus()) {
        d = 0;
        return;
    }
    d = new QDBusConnectionPrivate(this);

    QObject::connect(d, SIGNAL(newServerConnection(QDBusConnection)),
                     this, SIGNAL(newConnection(QDBusConnection)));

    QDBusErrorInternal error;
    d->setServer(q_dbus_server_listen(address.toUtf8().constData(), error), error);
}

/*!
    Destructs a QDBusServer
*/
QDBusServer::~QDBusServer()
{
    if (QDBusConnectionManager::instance()) {
        QMutexLocker locker(&QDBusConnectionManager::instance()->mutex);
        for (const QString &name : d->serverConnectionNames) {
            QDBusConnectionManager::instance()->removeConnection(name);
        }
        d->serverConnectionNames.clear();
    }
}

/*!
    Returns true if this QDBusServer object is connected.

    If it isn't connected, you need to call the constructor again.
*/
bool QDBusServer::isConnected() const
{
    return d && d->server && q_dbus_server_get_is_connected(d->server);
}

/*!
    Returns the last error that happened in this server.

    This function is provided for low-level code.
*/
QDBusError QDBusServer::lastError() const
{
    return d->lastError;
}

/*!
    Returns the address this server is associated with.
*/
QString QDBusServer::address() const
{
    QString addr;
    if (d && d->server) {
        char *c = q_dbus_server_get_address(d->server);
        addr = QString::fromUtf8(c);
        q_dbus_free(c);
    }

    return addr;
}

/*!
  \fn void QDBusServer::newConnection(const QDBusConnection &connection)

  This signal is emitted when a new client connection \a connection is
  established to the server.
 */

QT_END_NAMESPACE

#endif // QT_NO_DBUS
