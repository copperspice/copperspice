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

#ifndef QDBUSPENDINGCALL_P_H
#define QDBUSPENDINGCALL_P_H

#include <qshareddata.h>
#include <qpointer.h>
#include <qlist.h>
#include <qmutex.h>
#include <qwaitcondition.h>

#include "qdbusmessage.h"
#include "qdbus_symbols_p.h"

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

class QDBusPendingCall;
class QDBusPendingCallWatcher;
class QDBusPendingCallWatcherHelper;
class QDBusConnectionPrivate;

class QDBusPendingCallPrivate: public QSharedData
{
public:
    // {
    //     set only during construction:
    const QDBusMessage sentMessage;
    QDBusConnectionPrivate * const connection;

    // for the callback mechanism (see setReplyCallback and QDBusConnectionPrivate::sendWithReplyAsync)
    QPointer<QObject> receiver;
    QList<int> metaTypes;
    int methodIdx;

    // }

    mutable QMutex mutex;
    QWaitCondition waitForFinishedCondition;

    // {
    //    protected by the mutex above:
    QDBusPendingCallWatcherHelper *watcherHelper;
    QDBusMessage replyMessage;
    DBusPendingCall *pending;
    volatile bool waitingForFinished;

    QString expectedReplySignature;
    int expectedReplyCount;
    // }

    QDBusPendingCallPrivate(const QDBusMessage &sent, QDBusConnectionPrivate *connection)
        : sentMessage(sent), connection(connection), watcherHelper(0), pending(0), waitingForFinished(false)
    { }
    ~QDBusPendingCallPrivate();
    bool setReplyCallback(QObject *target, const char *member);
    void waitForFinished();
    void setMetaTypes(int count, const int *types);
    void checkReceivedSignature();

    static QDBusPendingCall fromMessage(const QDBusMessage &msg);
};

class QDBusPendingCallWatcherHelper: public QObject
{
    CS_OBJECT(QDBusPendingCallWatcherHelper)
public:
    void add(QDBusPendingCallWatcher *watcher);

    void emitSignals(const QDBusMessage &replyMessage, const QDBusMessage &sentMessage)
    {
        if (replyMessage.type() == QDBusMessage::ReplyMessage)
            emit reply(replyMessage);
        else
            emit error(replyMessage, sentMessage);
        emit finished();
    }

public:
    CS_SIGNAL_1(Public, void finished())
    CS_SIGNAL_2(finished) 
    CS_SIGNAL_1(Public, void reply(const QDBusMessage & msg))
    CS_SIGNAL_2(reply,msg) 
    CS_SIGNAL_1(Public, void error(const QDBusError & error,const QDBusMessage & msg))
    CS_SIGNAL_2(error,error,msg) 
};

QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif
