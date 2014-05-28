/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

/****************************************************************************
**
** Copyright (c) 2007-2008, Apple, Inc.
**
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
**   * Redistributions of source code must retain the above copyright notice,
**     this list of conditions and the following disclaimer.
**
**   * Redistributions in binary form must reproduce the above copyright notice,
**     this list of conditions and the following disclaimer in the documentation
**     and/or other materials provided with the distribution.
**
**   * Neither the name of Apple, Inc. nor the names of its contributors
**     may be used to endorse or promote products derived from this software
**     without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
** CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
** EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#ifndef QEVENTDISPATCHER_MAC_P_H
#define QEVENTDISPATCHER_MAC_P_H

#include <QtGui/qwindowdefs.h>
#include <QtCore/qhash.h>
#include <QtCore/qstack.h>
#include "qabstracteventdispatcher_p.h"
#include "qt_mac_p.h"

QT_BEGIN_NAMESPACE

typedef struct _NSModalSession *NSModalSession;
typedef struct _QCocoaModalSessionInfo {
    QPointer<QWidget> widget;
    NSModalSession session;
    void *nswindow;
} QCocoaModalSessionInfo;


class QEventDispatcherMacPrivate;

class QEventDispatcherMac : public QAbstractEventDispatcher
{
    CS_OBJECT(QEventDispatcherMac)
    Q_DECLARE_PRIVATE(QEventDispatcherMac)

public:
    explicit QEventDispatcherMac(QObject *parent = 0);
    ~QEventDispatcherMac();

    bool processEvents(QEventLoop::ProcessEventsFlags flags);
    bool hasPendingEvents();

    void registerSocketNotifier(QSocketNotifier *notifier);
    void unregisterSocketNotifier(QSocketNotifier *notifier);

    void registerTimer(int timerId, int interval, QObject *object);
    bool unregisterTimer(int timerId);
    bool unregisterTimers(QObject *object);
    QList<TimerInfo> registeredTimers(QObject *object) const;

    void wakeUp();
    void flush();
    void interrupt();

private:
    friend void qt_mac_select_timer_callbk(__EventLoopTimer*, void*);
    friend class QApplicationPrivate;
};

struct MacTimerInfo {
    int id;
    int interval;
    QObject *obj;
    bool pending;
    CFRunLoopTimerRef runLoopTimer;
    bool operator==(const MacTimerInfo &other)
    {
        return (id == other.id);
    }
};
typedef QHash<int, MacTimerInfo *> MacTimerHash;

struct MacSocketInfo {
    MacSocketInfo() : socket(0), runloop(0), readNotifier(0), writeNotifier(0) {}
    CFSocketRef socket;
    CFRunLoopSourceRef runloop;
    QObject *readNotifier;
    QObject *writeNotifier;
};
typedef QHash<int, MacSocketInfo *> MacSocketHash;

class QEventDispatcherMacPrivate : public QAbstractEventDispatcherPrivate
{
    Q_DECLARE_PUBLIC(QEventDispatcherMac)

public:
    QEventDispatcherMacPrivate();

    static MacTimerHash macTimerHash;
    // Set 'blockSendPostedEvents' to true if you _really_ need
    // to make sure that qt events are not posted while calling
    // low-level cocoa functions (like beginModalForWindow). And
    // use a QBoolBlocker to be safe:
    static bool blockSendPostedEvents;

    QThreadData * internal_get_ThreadData();

    // The following variables help organizing modal sessions:
    static QStack<QCocoaModalSessionInfo> cocoaModalSessionStack;
    static bool currentExecIsNSAppRun;
    static bool nsAppRunCalledByQt;
    static bool cleanupModalSessionsNeeded;
    static NSModalSession currentModalSessionCached;
    static NSModalSession currentModalSession();
    static void updateChildrenWorksWhenModal();
    static void temporarilyStopAllModalSessions();
    static void beginModalSession(QWidget *widget);
    static void endModalSession(QWidget *widget);
    static void cancelWaitForMoreEvents();
    static void cleanupModalSessions();
    static void ensureNSAppInitialized();

    MacSocketHash macSockets;
    QList<void *> queuedUserInputEvents; // List of EventRef in Carbon, and NSEvent * in Cocoa
    CFRunLoopSourceRef postedEventsSource;
    CFRunLoopObserverRef waitingObserver;
    CFRunLoopObserverRef firstTimeObserver;
    QAtomicInt serialNumber;
    int lastSerial;
    static bool interrupt;

private:
    static Boolean postedEventSourceEqualCallback(const void *info1, const void *info2);
    static void postedEventsSourcePerformCallback(void *info);
    static void activateTimer(CFRunLoopTimerRef, void *info);
    static void waitingObserverCallback(CFRunLoopObserverRef observer,
                                        CFRunLoopActivity activity, void *info);
    static void firstLoopEntry(CFRunLoopObserverRef ref, CFRunLoopActivity activity, void *info);
};


class QtMacInterruptDispatcherHelp : public QObject
{
    static QtMacInterruptDispatcherHelp *instance;
    bool cancelled;

    QtMacInterruptDispatcherHelp();
    ~QtMacInterruptDispatcherHelp();

    public:
    static void interruptLater();
    static void cancelInterruptLater();
};

QT_END_NAMESPACE

#endif // QEVENTDISPATCHER_MAC_P_H
