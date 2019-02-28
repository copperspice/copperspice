/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QEVENTDISPATCHER_CF_P_H
#define QEVENTDISPATCHER_CF_P_H

#define DEBUG_EVENT_DISPATCHER 0

#include <qabstracteventdispatcher.h>
#include <qtimerinfo_unix_p.h>
#include <qcfsocketnotifier_p.h>
#include <qdebug.h>

#include <CoreFoundation/CoreFoundation.h>

#ifdef __OBJC__
@class RunLoopModeTracker;
#else
typedef struct objc_object RunLoopModeTracker;
#endif

class QEventDispatcherCoreFoundation;

template <class T = QEventDispatcherCoreFoundation>
class RunLoopSource
{
public:
    typedef bool (T::*CallbackFunction)();

    enum { kHighestPriority = 0 } RunLoopSourcePriority;

    RunLoopSource(T *delegate, CallbackFunction callback)
        : m_delegate(delegate), m_callback(callback)
    {
        CFRunLoopSourceContext context = {};
        context.info = this;
        context.perform = RunLoopSource::process;

        m_source = CFRunLoopSourceCreate(kCFAllocatorDefault, kHighestPriority, &context);
        Q_ASSERT(m_source);
    }

    ~RunLoopSource()
    {
        CFRunLoopSourceInvalidate(m_source);
        CFRelease(m_source);
    }

    void addToMode(CFStringRef mode, CFRunLoopRef runLoop = 0)
    {
        if (!runLoop)
            runLoop = CFRunLoopGetCurrent();

        CFRunLoopAddSource(runLoop, m_source, mode);
    }

    void signal() { CFRunLoopSourceSignal(m_source); }

private:
    static void process(void *info)
    {
        RunLoopSource *self = static_cast<RunLoopSource *>(info);
        ((self->m_delegate)->*(self->m_callback))();
    }

    T *m_delegate;
    CallbackFunction m_callback;
    CFRunLoopSourceRef m_source;
};

template <class T = QEventDispatcherCoreFoundation>
class RunLoopObserver
{
public:
    typedef void (T::*CallbackFunction) (CFRunLoopActivity activity);

    RunLoopObserver(T *delegate, CallbackFunction callback, CFOptionFlags activities)
        : m_delegate(delegate), m_callback(callback)
    {
        CFRunLoopObserverContext context = {};
        context.info = this;

        m_observer = CFRunLoopObserverCreate(kCFAllocatorDefault, activities, true, 0, process, &context);
        Q_ASSERT(m_observer);
    }

    ~RunLoopObserver()
    {
        CFRunLoopObserverInvalidate(m_observer);
        CFRelease(m_observer);
    }

    void addToMode(CFStringRef mode, CFRunLoopRef runLoop = 0)
    {
        if (!runLoop)
            runLoop = CFRunLoopGetCurrent();

        if (!CFRunLoopContainsObserver(runLoop, m_observer, mode))
            CFRunLoopAddObserver(runLoop, m_observer, mode);
    }

    void removeFromMode(CFStringRef mode, CFRunLoopRef runLoop = 0)
    {
        if (!runLoop)
            runLoop = CFRunLoopGetCurrent();

        if (CFRunLoopContainsObserver(runLoop, m_observer, mode))
            CFRunLoopRemoveObserver(runLoop, m_observer, mode);
    }

private:
    static void process(CFRunLoopObserverRef, CFRunLoopActivity activity, void *info)
    {
        RunLoopObserver *self = static_cast<RunLoopObserver *>(info);
        ((self->m_delegate)->*(self->m_callback))(activity);
    }

    T *m_delegate;
    CallbackFunction m_callback;
    CFRunLoopObserverRef m_observer;
};

class Q_CORE_EXPORT QEventDispatcherCoreFoundation : public QAbstractEventDispatcher
{
    CORE_CS_OBJECT(QEventDispatcherCoreFoundation)

public:
    explicit QEventDispatcherCoreFoundation(QObject *parent = nullptr);
    ~QEventDispatcherCoreFoundation();

    bool processEvents(QEventLoop::ProcessEventsFlags flags) override;
    bool hasPendingEvents() override;

    void registerSocketNotifier(QSocketNotifier *notifier) override;
    void unregisterSocketNotifier(QSocketNotifier *notifier) override;

    void registerTimer(int timerId, int interval, Qt::TimerType timerType, QObject *object) override;
    bool unregisterTimer(int timerId) override;
    bool unregisterTimers(QObject *object) override;
    QList<QAbstractEventDispatcher::TimerInfo> registeredTimers(QObject *object) const override;

    int remainingTime(int timerId) override;

    void wakeUp() override;
    void interrupt() override;
    void flush() override;

protected:
    virtual bool processPostedEvents();

    struct ProcessEventsState
    {
        ProcessEventsState(QEventLoop::ProcessEventsFlags f)
         : flags(f), wasInterrupted(false)
         , processedPostedEvents(false), processedTimers(false)
         , deferredWakeUp(false), deferredUpdateTimers(false) {}

        QEventLoop::ProcessEventsFlags flags;
        bool wasInterrupted;
        bool processedPostedEvents;
        bool processedTimers;
        bool deferredWakeUp;
        bool deferredUpdateTimers;
    };

    ProcessEventsState m_processEvents;

private:
    RunLoopSource<> m_postedEventsRunLoopSource;
    RunLoopObserver<> m_runLoopActivityObserver;

    RunLoopModeTracker *m_runLoopModeTracker;

    QTimerInfoList m_timerInfoList;
    CFRunLoopTimerRef m_runLoopTimer;
    CFRunLoopTimerRef m_blockedRunLoopTimer;
    bool m_overdueTimerScheduled;

    QCFSocketNotifier m_cfSocketNotifier;

    void processTimers(CFRunLoopTimerRef);

    void handleRunLoopActivity(CFRunLoopActivity activity);

    void updateTimers();
    void invalidateTimer();
};


#if DEBUG_EVENT_DISPATCHER
extern uint g_eventDispatcherIndentationLevel;
#define qEventDispatcherDebug() qDebug().nospace() \
            << qPrintable(QString(QLatin1String("| ")).repeated(g_eventDispatcherIndentationLevel)) \
            << __FUNCTION__ << "(): "
#define qIndent() ++g_eventDispatcherIndentationLevel
#define qUnIndent() --g_eventDispatcherIndentationLevel
#else
#define qEventDispatcherDebug() QT_NO_QDEBUG_MACRO()
#define qIndent()
#define qUnIndent()
#endif

#endif // QEVENTDISPATCHER_CF_P_H
