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

#ifndef QDBUSINTEGRATOR_P_H
#define QDBUSINTEGRATOR_P_H

#include "qdbus_symbols_p.h"

#include "qcoreevent.h"
#include "qeventloop.h"
#include "qhash.h"
#include "qobject.h"
#include "qobject_p.h"
#include "qlist.h"
#include "qpointer.h"
#include "qsemaphore.h"

#include "qdbusconnection.h"
#include "qdbusmessage.h"
#include "qdbusconnection_p.h"

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

class QDBusConnectionPrivate;

// Really private structs used by qdbusintegrator.cpp
// Things that aren't used by any other file

struct QDBusSlotCache
{
    struct Data
    {
        int flags;
        int slotIdx;
        QList<int> metaTypes;
    };
    typedef QMultiHash<QString, Data> Hash;
    Hash hash;
};

class QDBusCallDeliveryEvent: public QMetaCallEvent
{
public:
    QDBusCallDeliveryEvent(const QDBusConnection &c, int id, QObject *sender,
                           const QDBusMessage &msg, const QList<int> &types, int f = 0)
        : QMetaCallEvent(0, id, 0, sender, -1), connection(c), message(msg), metaTypes(types), flags(f)
        { }

    void placeMetaCall(QObject *object)
    {
        QDBusConnectionPrivate::d(connection)->deliverCall(object, flags, message, metaTypes, id());
    }

private:
    QDBusConnection connection; // just for refcounting
    QDBusMessage message;
    QList<int> metaTypes;
    int flags;
};

class QDBusActivateObjectEvent: public QMetaCallEvent
{
public:
    QDBusActivateObjectEvent(const QDBusConnection &c, QObject *sender,
                             const QDBusConnectionPrivate::ObjectTreeNode &n,
                             int p, const QDBusMessage &m, QSemaphore *s = 0)
        : QMetaCallEvent(0, -1, 0, sender, -1, 0, 0, 0, s), connection(c), node(n),
          pathStartPos(p), message(m), handled(false)
        { }
    ~QDBusActivateObjectEvent();

    void placeMetaCall(QObject *);

private:
    QDBusConnection connection; // just for refcounting
    QDBusConnectionPrivate::ObjectTreeNode node;
    int pathStartPos;
    QDBusMessage message;
    bool handled;
};

class QDBusConnectionCallbackEvent : public QEvent
{
public:
    QDBusConnectionCallbackEvent()
        : QEvent(User), subtype(Subtype(0))
    { }

    DBusWatch *watch;
    union {
        int timerId;
        int fd;
    };
    int extra;

    enum Subtype {
        AddTimeout = 0,
        KillTimer,
        AddWatch,
        //RemoveWatch,
        ToggleWatch
    } subtype;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QDBusSlotCache)

#endif // QT_NO_DBUS
#endif
