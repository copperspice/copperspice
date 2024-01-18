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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#ifndef QDBUSCONNECTIONMANAGER_P_H
#define QDBUSCONNECTIONMANAGER_P_H

#include "qdbusconnection_p.h"

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

class QDBusConnectionManager
{
public:
    QDBusConnectionManager() {}
    ~QDBusConnectionManager();
    static QDBusConnectionManager* instance();

    QDBusConnectionPrivate *connection(const QString &name) const;
    void removeConnection(const QString &name);
    void setConnection(const QString &name, QDBusConnectionPrivate *c);

    QDBusConnectionPrivate *sender() const;
    void setSender(const QDBusConnectionPrivate *s);

    mutable QMutex mutex;
private:
    QHash<QString, QDBusConnectionPrivate *> connectionHash;

    mutable QMutex senderMutex;
    QString senderName; // internal; will probably change
};

QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif
