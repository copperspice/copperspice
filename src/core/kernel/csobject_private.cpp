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
#include <qobject.h>

#include <qthread_p.h>

void CSInternalChildren::deleteChildren(QObject *object)
{
   if (! object) {
      return;
   }

   object->deleteChildren();
}

void CSInternalChildren::moveChildren(QObject *object, int from, int to)
{
   if (! object) {
      return;
   }

   object->m_children.move(from, to);
}

void CSInternalChildren::removeOne(QObject *object, QObject *value)
{
   if (! object) {
      return;
   }

   object->m_children.removeOne(value);
}

void CSInternalChildren::set_mParent(QObject *object, QObject *value)
{
   if (! object) {
      return;
   }

   // hard setting this member!
   object->m_parent = value;
}

// **
CSAbstractDeclarativeData *CSInternalDeclarativeData::get_m_declarativeData(const QObject *object)
{
   if (! object) {
      return nullptr;
   }

   return object->m_declarativeData;
}

void CSInternalDeclarativeData::set_m_declarativeData(QObject *object, CSAbstractDeclarativeData *value)
{
   if (! object) {
      return;
   }

   object->m_declarativeData = value;
}

// **
bool CSInternalEvents::get_m_sendChildEvents(const QObject *object)
{
   if (! object) {
      return false;
   }

   return object->m_sendChildEvents;
}

bool CSInternalEvents::get_m_receiveChildEvents(const QObject *object)
{
   if (! object) {
      return false;
   }

   return object->m_receiveChildEvents;
}

int CSInternalEvents::get_m_PostedEvents(const QObject *object)
{
   if (! object) {
      return 0;
   }

   return object->m_postedEvents;
}

QList<QPointer<QObject>> &CSInternalEvents::get_m_EventFilters(QObject *object)
{
   if (! object) {
      static QList<QPointer<QObject>> emptyList;
      return emptyList;
   }

   return object->m_eventFilters;
}

std::atomic<bool> &CSInternalEvents::get_m_inThreadChangeEvent(QObject *object)
{
   if (! object) {
      static std::atomic<bool> emptyAtomic;
      return emptyAtomic;
   }

   return object->m_inThreadChangeEvent;
}

void CSInternalEvents::set_m_sendChildEvents(QObject *object, bool data)
{
   if (! object) {
      return;
   }

   object->m_sendChildEvents = data;
}

void CSInternalEvents::set_m_receiveChildEvents(QObject *object, bool data)
{
   if (! object) {
      return;
   }

   object->m_receiveChildEvents = data;
}

// **
bool CSInternalRefCount::get_m_wasDeleted(const QObject *object)
{
   if (! object) {
      return false;
   }

   return object->m_wasDeleted;
}

void CSInternalRefCount::set_m_wasDeleted(QObject *object, bool data)
{
   if (! object) {
      return;
   }

   object->m_wasDeleted = data;
}

std::atomic<QtSharedPointer::ExternalRefCountData *> &CSInternalRefCount::get_m_SharedRefCount(const QObject *object)
{
   if (object == nullptr) {
      static std::atomic<QtSharedPointer::ExternalRefCountData *> emptyAtomic;
      return emptyAtomic;
   }

   return object->m_sharedRefCount;
}

// **
bool CSInternalSender::isSender(const QObject *object, const QObject *receiver, const QString &signal)
{
   if (! object) {
      return false;
   }

   return object->isSender(receiver, signal);
}

QList<QObject *> CSInternalSender::receiverList(const QObject *object, const QMetaMethod &signalMetaMethod)
{
   if (object == nullptr) {
      return QList<QObject *>{};
   }

   return object->receiverList(signalMetaMethod);
}

QList<QObject *> CSInternalSender::senderList(const QObject *object)
{
   if (object == nullptr) {
      return QList<QObject *>{};
   }

   return object->senderList();
}

// **
QThreadData *CSInternalThreadData::get_m_ThreadData(const QObject *object)
{
   if (! object) {
      return nullptr;
   }

   // returns a pointer to the threadData
   return object->m_threadData.load();
}

std::atomic<QThreadData *> &CSInternalThreadData::get_AtomicThreadData(QObject *object)
{
   if (! object) {
      static std::atomic<QThreadData *> emptyAtomic;
      return emptyAtomic;
   }

   // returns a reference to the atomic var whichs contains a pointer to the thread data
   return object->m_threadData;
}

void CSInternalEvents::incr_PostedEvents(QObject *object)
{
   if (! object) {
      return;
   }

   ++(object->m_postedEvents);
}

void CSInternalEvents::decr_PostedEvents(QObject *object)
{
   if (! object) {
      return;
   }

   --(object->m_postedEvents);
}

// private slot
void QObject::internal_reregisterTimers(QList<QTimerInfo> timerList)
{
   QAbstractEventDispatcher *eventDispatcher = m_threadData.load()->eventDispatcher.load();

   for (auto &item : timerList) {
      eventDispatcher->registerTimer(item.interval, item.timerType, this);
   }
}
