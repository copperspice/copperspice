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

#include <qabstracteventdispatcher.h>
#include <qabstracteventdispatcher_p.h>

#include <qabstractnativeeventfilter.h>
#include <qthread.h>

#include <qcoreapplication_p.h>
#include <qfreelist_p.h>
#include <qthread_p.h>

// we allow for 2^24 = 8^8 = 16777216 simultaneously running timers
struct QtTimerIdFreeListConstants : public QFreeListDefaultConstants
{
   static constexpr const int InitialNextValue = 1;
   static constexpr const int BlockCount       = 6;

   static const int Sizes[BlockCount];
};

enum TimerOffset {
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

using QtTimerIdFreeList = QFreeList<void, QtTimerIdFreeListConstants>;

static QtTimerIdFreeList *timerIdFreeList()
{
   static QtTimerIdFreeList retval;
   return &retval;
}

int QAbstractEventDispatcherPrivate::allocateTimerId()
{
   return timerIdFreeList()->next();
}

void QAbstractEventDispatcherPrivate::releaseTimerId(int timerId)
{
   // may be called by a global destructor after timerIdFreeList() has been destructed
   if (QtTimerIdFreeList *fl = timerIdFreeList()) {
      fl->release(timerId);
   }
}

QAbstractEventDispatcher::QAbstractEventDispatcher(QObject *parent)
   : QObject(parent), d_ptr(new QAbstractEventDispatcherPrivate)
{
   d_ptr->q_ptr = this;
}

// internal
QAbstractEventDispatcher::QAbstractEventDispatcher(QAbstractEventDispatcherPrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

QAbstractEventDispatcher::~QAbstractEventDispatcher()
{
}

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

// ### Are these called when the _application_ starts/stops or just when the current _event loop_ starts/stops?

// internal
void QAbstractEventDispatcher::startingUp()
{ }

// internal
void QAbstractEventDispatcher::closingDown()
{ }

void QAbstractEventDispatcher::installNativeEventFilter(QAbstractNativeEventFilter *filterObj)
{
   Q_D(QAbstractEventDispatcher);

   // clean up unused items in the list
   d->eventFilters.removeAll(nullptr);
   d->eventFilters.removeAll(filterObj);
   d->eventFilters.prepend(filterObj);
}

void QAbstractEventDispatcher::removeNativeEventFilter(QAbstractNativeEventFilter *filter)
{
   Q_D(QAbstractEventDispatcher);

   for (int i = 0; i < d->eventFilters.count(); ++i) {
      if (d->eventFilters.at(i) == filter) {
         d->eventFilters[i] = nullptr;
         break;
      }
   }
}

bool QAbstractEventDispatcher::filterNativeEvent(const QByteArray &eventType, void *message, long *result)
{
   Q_D(QAbstractEventDispatcher);

   QThreadData *threadData = CSInternalThreadData::get_m_ThreadData(this);

   if (! d->eventFilters.isEmpty()) {
      // Raise the loopLevel so that deleteLater() calls in or triggered
      // by event_filter() will be processed from the main event loop.
      QScopedLoopLevelCounter loopLevelCounter(threadData);

      for (int i = 0; i < d->eventFilters.size(); ++i) {
         QAbstractNativeEventFilter *filter = d->eventFilters.at(i);

         if (! filter) {
            continue;
         }

         if (filter->nativeEventFilter(eventType, message, result)) {
            return true;
         }
      }
   }

   return false;
}
