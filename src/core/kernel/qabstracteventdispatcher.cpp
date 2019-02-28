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

#include <qabstracteventdispatcher.h>
#include <qabstracteventdispatcher_p.h>
#include <qabstractnativeeventfilter.h>

#include <qthread.h>
#include <qthread_p.h>
#include <qcoreapplication_p.h>
#include <qfreelist_p.h>


// we allow for 2^24 = 8^8 = 16777216 simultaneously running timers
struct QtTimerIdFreeListConstants : public QFreeListDefaultConstants
{
    enum
    {
        InitialNextValue = 1,
        BlockCount = 6
    };

    static const int Sizes[BlockCount];
};

enum {
    Offset0 = 0x00000000,
    Offset1 = 0x00000040,
    Offset2 = 0x00000100,
    Offset3 = 0x00001000,
    Offset4 = 0x00010000,
    Offset5 = 0x00100000,

    Size0 = Offset1  - Offset0,
    Size1 = Offset2  - Offset1,
    Size2 = Offset3  - Offset2,
    Size3 = Offset4  - Offset3,
    Size4 = Offset5  - Offset4,
    Size5 = QtTimerIdFreeListConstants::MaxIndex - Offset5
};
const int QtTimerIdFreeListConstants::Sizes[QtTimerIdFreeListConstants::BlockCount] = {
    Size0,
    Size1,
    Size2,
    Size3,
    Size4,
    Size5
};

typedef QFreeList<void, QtTimerIdFreeListConstants> QtTimerIdFreeList;
Q_GLOBAL_STATIC(QtTimerIdFreeList, timerIdFreeList)

int QAbstractEventDispatcherPrivate::allocateTimerId()
{
   return timerIdFreeList()->next();
}


void QAbstractEventDispatcherPrivate::releaseTimerId(int timerId)
{
    // this function may be called by a global destructor after
    // timerIdFreeList() has been destructed
    if (QtTimerIdFreeList *fl = timerIdFreeList())
        fl->release(timerId);
}

QAbstractEventDispatcher::QAbstractEventDispatcher(QObject *parent)
   : QObject(parent), d_ptr(new QAbstractEventDispatcherPrivate)
{
   d_ptr->q_ptr = this;
}

/*!
    \internal
*/
QAbstractEventDispatcher::QAbstractEventDispatcher(QAbstractEventDispatcherPrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}


QAbstractEventDispatcher::~QAbstractEventDispatcher()
{ }


QAbstractEventDispatcher *QAbstractEventDispatcher::instance(QThread *thread)
{
   QThreadData *data = thread ? QThreadData::get2(thread) : QThreadData::current();
   return data->eventDispatcher.load();
}

int QAbstractEventDispatcher::registerTimer(int interval, Qt::TimerType timerType, QObject *object)
{
   int id = QAbstractEventDispatcherPrivate::allocateTimerId();
   registerTimer(id, interval, timerType, object);

   return id;
}

/*!
    \fn void QAbstractEventDispatcher::registerTimer(int timerId, int interval, QObject *object)

    Register a timer with the specified \a timerId and \a interval for
    the given \a object.
*/

/*!
    \fn bool QAbstractEventDispatcher::unregisterTimer(int timerId)

    Unregisters the timer with the given \a timerId.
    Returns true if successful; otherwise returns false.

    \sa registerTimer(), unregisterTimers()
*/

/*!
    \fn bool QAbstractEventDispatcher::unregisterTimers(QObject *object)

    Unregisters all the timers associated with the given \a object.
    Returns true if all timers were successful removed; otherwise returns false.

    \sa unregisterTimer(), registeredTimers()
*/

/*!
    \fn QList<TimerInfo> QAbstractEventDispatcher::registeredTimers(QObject *object) const

    Returns a list of registered timers for \a object. The timer ID
    is the first member in each pair; the interval is the second.
*/

/*! \fn void QAbstractEventDispatcher::wakeUp()
    \threadsafe

    Wakes up the event loop.

    \sa awake()
*/

/*!
    \fn void QAbstractEventDispatcher::interrupt()

    Interrupts event dispatching; i.e. the event dispatcher will
    return from processEvents() as soon as possible.
*/

/*! \fn void QAbstractEventDispatcher::flush()

    Flushes the event queue. This normally returns almost
    immediately. Does nothing on platforms other than X11.
*/

// ### DOC: Are these called when the _application_ starts/stops or just
// when the current _event loop_ starts/stops?
/*! \internal */
void QAbstractEventDispatcher::startingUp()
{ }

/*! \internal */
void QAbstractEventDispatcher::closingDown()
{ }

/*!
    \typedef QAbstractEventDispatcher::TimerInfo

    Typedef for QPair<int, int>. The first component of
    the pair is the timer ID; the second component is
    the interval.

    \sa registeredTimers()
*/

/*!
    \typedef QAbstractEventDispatcher::EventFilter

    Typedef for a function with the signature

    \snippet doc/src/snippets/code/src_corelib_kernel_qabstracteventdispatcher.cpp 0

    Note that the type of the \a message is platform dependent. The
    following table shows the \a {message}'s type on Windows, Mac, and
    X11. You can do a static cast to these types.

    \table
        \header
            \o Platform
            \o type
        \row
            \o Windows
            \o MSG
        \row
            \o X11
            \o XEvent
        \row
            \o Mac
            \o NSEvent
    \endtable



    \sa setEventFilter(), filterEvent()
*/

/*!
    Replaces the event filter function for this
    QAbstractEventDispatcher with \a filter and returns the replaced
    event filter function. Only the current event filter function is
    called. If you want to use both filter functions, save the
    replaced EventFilter in a place where yours can call it.

    The event filter function set here is called for all messages
    taken from the system event loop before the event is dispatched to
    the respective target, including the messages not meant for Qt
    objects.

    The event filter function should return true if the message should
    be filtered, (i.e. stopped). It should return false to allow
    processing the message to continue.

    By default, no event filter function is set (i.e., this function
    returns a null EventFilter the first time it is called).
*/

void QAbstractEventDispatcher::installNativeEventFilter(QAbstractNativeEventFilter *filterObj)
{
    Q_D(QAbstractEventDispatcher);

    // clean up unused items in the list
    d->eventFilters.removeAll(0);
    d->eventFilters.removeAll(filterObj);
    d->eventFilters.prepend(filterObj);
}

void QAbstractEventDispatcher::removeNativeEventFilter(QAbstractNativeEventFilter *filter)
{
    Q_D(QAbstractEventDispatcher);
    for (int i = 0; i < d->eventFilters.count(); ++i) {
        if (d->eventFilters.at(i) == filter) {
            d->eventFilters[i] = 0;
            break;
        }
    }
}

bool QAbstractEventDispatcher::filterNativeEvent(const QByteArray &eventType, void *message, long *result)
{
   Q_D(QAbstractEventDispatcher);

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

   if (!d->eventFilters.isEmpty()) {
      // Raise the loopLevel so that deleteLater() calls in or triggered
      // by event_filter() will be processed from the main event loop.
      QScopedLoopLevelCounter loopLevelCounter(threadData);

      for (int i = 0; i < d->eventFilters.size(); ++i) {
            QAbstractNativeEventFilter *filter = d->eventFilters.at(i);
            if (!filter)
                continue;

            if (filter->nativeEventFilter(eventType, message, result))
                return true;
       }
   }

   return false;
}

