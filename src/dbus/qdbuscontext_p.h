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

#ifndef QDBUSCONTEXT_P_H
#define QDBUSCONTEXT_P_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

class QDBusMessage;
class QDBusConnection;

class QDBusContext;
class QDBusContextPrivate
{
public:
    inline QDBusContextPrivate(const QDBusConnection &conn, const QDBusMessage &msg)
        : connection(conn), message(msg) {}

    QDBusConnection connection;
    const QDBusMessage &message;

    static QDBusContextPrivate *set(QObject *obj, QDBusContextPrivate *newContext);
};

QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif

