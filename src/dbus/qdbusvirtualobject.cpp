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

#include "qdbusvirtualobject.h"

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

QDBusVirtualObject::QDBusVirtualObject(QObject *parent) :
    QObject(parent)
{
}

QDBusVirtualObject::~QDBusVirtualObject()
{
}

QT_END_NAMESPACE


/*!
    \internal
    \class QDBusVirtualObject
    \inmodule QtDBus
    \since 4.8

    \brief The QDBusVirtualObject class is used to handle several DBus paths with one class.
*/

/*!
    \internal
    \fn bool QDBusVirtualObject::handleMessage(const QDBusMessage &message, const QDBusConnection &connection) = 0

    This function needs to handle all messages to the path of the
    virtual object, when the SubPath option is specified.
    The service, path, interface and methos are all part of the message.
    Must return true when the message is handled, otherwise false (will generate dbus error message).
*/


/*!
    \internal
    \fn QString QDBusVirtualObject::introspect(const QString &path) const

    This function needs to handle the introspection of the
    virtual object. It must return xml of the form:

    \code
<interface name="org.qtproject.QtDBus.MyObject" >
    <property access="readwrite" type="i" name="prop1" />
</interface>
    \endcode

    If you pass the SubPath option, this introspection has to include all child nodes.
    Otherwise QDBus handles the introspection of the child nodes.
*/

#endif // QT_NO_DBUS
