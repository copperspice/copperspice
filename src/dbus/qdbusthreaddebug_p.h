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

#ifndef QDBUSTHREADDEBUG_P_H
#define QDBUSTHREADDEBUG_P_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_DBUS

#if QDBUS_THREAD_DEBUG

QT_BEGIN_NAMESPACE
typedef void (*qdbusThreadDebugFunc)(int, int, QDBusConnectionPrivate *);
Q_DBUS_EXPORT void qdbusDefaultThreadDebug(int, int, QDBusConnectionPrivate *);
extern Q_DBUS_EXPORT qdbusThreadDebugFunc qdbusThreadDebug;
QT_END_NAMESPACE

#endif

enum ThreadAction {
    ConnectAction = 0,
    DisconnectAction = 1,
    RegisterObjectAction = 2,
    UnregisterObjectAction = 3,
    ObjectRegisteredAtAction = 4,

    CloseConnectionAction = 10,
    ObjectDestroyedAction = 11,
    RelaySignalAction = 12,
    HandleObjectCallAction = 13,
    HandleSignalAction = 14,
    ConnectRelayAction = 15,
    DisconnectRelayAction = 16,
    FindMetaObject1Action = 17,
    FindMetaObject2Action = 18,
    RegisterServiceAction = 19,
    UnregisterServiceAction = 20,
    UpdateSignalHookOwnerAction = 21,
    HandleObjectCallPostEventAction = 22,
    HandleObjectCallSemaphoreAction = 23,
    DoDispatchAction = 24,
    SendWithReplyAsyncAction = 25,
    MessageResultReceivedAction = 26,
    ActivateSignalAction = 27,
    PendingCallBlockAction = 28,

    AddTimeoutAction = 50,
    RealAddTimeoutAction = 51,
    RemoveTimeoutAction = 52,
    KillTimerAction = 58,
    TimerEventAction = 59,
    AddWatchAction = 60,
    RemoveWatchAction = 61,
    ToggleWatchAction = 62,
    SocketReadAction = 63,
    SocketWriteAction = 64
};

struct QDBusLockerBase
{
    enum Condition
    {
        BeforeLock,
        AfterLock,
        BeforeUnlock,
        AfterUnlock,

        BeforePost,
        AfterPost,
        BeforeDeliver,
        AfterDeliver,

        BeforeAcquire,
        AfterAcquire,
        BeforeRelease,
        AfterRelease
    };

#if QDBUS_THREAD_DEBUG
    static inline void reportThreadAction(int action, int condition, QDBusConnectionPrivate *ptr)
    { if (qdbusThreadDebug) qdbusThreadDebug(action, condition, ptr); }
#else
    static inline void reportThreadAction(int, int, QDBusConnectionPrivate *) { }
#endif
};

struct QDBusReadLocker: QDBusLockerBase
{
    QDBusConnectionPrivate *self;
    ThreadAction action;
    inline QDBusReadLocker(ThreadAction a, QDBusConnectionPrivate *s)
        : self(s), action(a)
    {
        reportThreadAction(action, BeforeLock, self);
        self->lock.lockForRead();
        reportThreadAction(action, AfterLock, self);
    }

    inline ~QDBusReadLocker()
    {
        reportThreadAction(action, BeforeUnlock, self);
        self->lock.unlock();
        reportThreadAction(action, AfterUnlock, self);
    }
};

struct QDBusWriteLocker: QDBusLockerBase
{
    QDBusConnectionPrivate *self;
    ThreadAction action;
    inline QDBusWriteLocker(ThreadAction a, QDBusConnectionPrivate *s)
        : self(s), action(a)
    {
        reportThreadAction(action, BeforeLock, self);
        self->lock.lockForWrite();
        reportThreadAction(action, AfterLock, self);
    }

    inline ~QDBusWriteLocker()
    {
        reportThreadAction(action, BeforeUnlock, self);
        self->lock.unlock();
        reportThreadAction(action, AfterUnlock, self);
    }
};

struct QDBusMutexLocker: QDBusLockerBase
{
    QDBusConnectionPrivate *self;
    QMutex *mutex;
    ThreadAction action;
    inline QDBusMutexLocker(ThreadAction a, QDBusConnectionPrivate *s,
                            QMutex *m)
        : self(s), mutex(m), action(a)
    {
        reportThreadAction(action, BeforeLock, self);
        mutex->lock();
        reportThreadAction(action, AfterLock, self);
    }

    inline ~QDBusMutexLocker()
    {
        reportThreadAction(action, BeforeUnlock, self);
        mutex->unlock();
        reportThreadAction(action, AfterUnlock, self);
    }
};

struct QDBusDispatchLocker: QDBusMutexLocker
{
    inline QDBusDispatchLocker(ThreadAction a, QDBusConnectionPrivate *s)
        : QDBusMutexLocker(a, s, &s->dispatchLock)
    { }
};

struct QDBusWatchAndTimeoutLocker: QDBusMutexLocker
{
    inline QDBusWatchAndTimeoutLocker(ThreadAction a, QDBusConnectionPrivate *s)
        : QDBusMutexLocker(a, s, &s->watchAndTimeoutLock)
    { }
};

#if QDBUS_THREAD_DEBUG
# define SEM_ACQUIRE(action, sem)                                       \
    do {                                                                \
    QDBusLockerBase::reportThreadAction(action, QDBusLockerBase::BeforeAcquire, this); \
    sem.acquire();                                                      \
    QDBusLockerBase::reportThreadAction(action, QDBusLockerBase::AfterAcquire, this); \
    } while (0)

# define SEM_RELEASE(action, sem)                                       \
    do {                                                                \
    QDBusLockerBase::reportThreadAction(action, QDBusLockerBase::BeforeRelease, that); \
    sem.release();                                                      \
    QDBusLockerBase::reportThreadAction(action, QDBusLockerBase::AfterRelease, that); \
    } while (0)

#else
# define SEM_ACQUIRE(action, sem)       sem.acquire()
# define SEM_RELEASE(action, sem)       sem.release()
#endif

#endif // QT_NO_DBUS
#endif
