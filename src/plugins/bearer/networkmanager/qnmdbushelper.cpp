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

// this class is for helping qdbus get stuff

#include "qnmdbushelper.h"

#include "qnetworkmanagerservice.h"

#include <QDBusError>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>

#include <QDebug>

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

QNmDBusHelper::QNmDBusHelper(QObject * parent)
        : QObject(parent)
{
}

QNmDBusHelper::~QNmDBusHelper()
{
}

void QNmDBusHelper::deviceStateChanged(quint32 state)
 {
    QDBusMessage msg = this->message();
    if(state == NM_DEVICE_STATE_ACTIVATED
       || state == NM_DEVICE_STATE_DISCONNECTED
       || state == NM_DEVICE_STATE_UNAVAILABLE
       || state == NM_DEVICE_STATE_FAILED) {
        emit pathForStateChanged(msg.path(), state);
    }
 }

void QNmDBusHelper::slotAccessPointAdded(QDBusObjectPath path)
{
    if(path.path().length() > 2) {
        QDBusMessage msg = this->message();
        emit pathForAccessPointAdded(msg.path(), path);
    }
}

void QNmDBusHelper::slotAccessPointRemoved(QDBusObjectPath path)
{
    if(path.path().length() > 2) {
        QDBusMessage msg = this->message();
        emit pathForAccessPointRemoved(msg.path(), path);
    }
}

void QNmDBusHelper::slotPropertiesChanged(QMap<QString,QVariant> map)
{
    QDBusMessage msg = this->message();
    QMapIterator<QString, QVariant> i(map);
    while (i.hasNext()) {
        i.next();
        if( i.key() == "State") { //state only applies to device interfaces
            quint32 state = i.value().toUInt();
            if( state == NM_DEVICE_STATE_ACTIVATED
                || state == NM_DEVICE_STATE_DISCONNECTED
                || state == NM_DEVICE_STATE_UNAVAILABLE
                || state == NM_DEVICE_STATE_FAILED) {
                emit  pathForPropertiesChanged( msg.path(), map);
            }
        } else if( i.key() == "ActiveAccessPoint") {
            emit pathForPropertiesChanged(msg.path(), map);
            //            qWarning()  << __PRETTY_FUNCTION__ << i.key() << ": " << i.value().value<QDBusObjectPath>().path();
            //      } else if( i.key() == "Strength")
            //            qWarning()  << __PRETTY_FUNCTION__ << i.key() << ": " << i.value().toUInt();
            //   else
            //            qWarning()  << __PRETTY_FUNCTION__ << i.key() << ": " << i.value();
        } else if (i.key() == "ActiveConnections") {
            emit pathForPropertiesChanged(msg.path(), map);
        }
    }
}

void QNmDBusHelper::slotSettingsRemoved()
{
    QDBusMessage msg = this->message();
    emit pathForSettingsRemoved(msg.path());
}

QT_END_NAMESPACE

#endif // QT_NO_DBUS
