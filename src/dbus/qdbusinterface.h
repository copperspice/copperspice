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

#ifndef QDBUSINTERFACE_H
#define QDBUSINTERFACE_H

#include <QtDBus/qdbusabstractinterface.h>
#include <QtDBus/qdbusconnection.h>

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

class QDBusInterfacePrivate;

class Q_DBUS_EXPORT QDBusInterface: public QDBusAbstractInterface
{
    friend class QDBusConnection;
private:
    QDBusInterface(QDBusInterfacePrivate *p);

public:
    QDBusInterface(const QString &service, const QString &path, const QString &interface = QString(),
                   const QDBusConnection &connection = QDBusConnection::sessionBus(),
                   QObject *parent = nullptr);
    ~QDBusInterface();

    virtual const QMetaObject *metaObject() const;
    virtual void *qt_metacast(const char *);
    virtual int qt_metacall(QMetaObject::Call, int, void **);

private:
    Q_DECLARE_PRIVATE(QDBusInterface)
};

QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif
