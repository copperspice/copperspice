/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qcoreevent.h>
#include <qcoreapplication.h>
#include <qcoreapplication_p.h>

#include <qmutex.h>
#include <qset.h>

QT_BEGIN_NAMESPACE

QEvent::QEvent(Type type)
   : d(0), t(type), posted(false), spont(false), m_accept(true)
{}

/*!
    Destroys the event. If it was \link
    QCoreApplication::postEvent() posted \endlink,
    it will be removed from the list of events to be posted.
*/

QEvent::~QEvent()
{
   if (posted && QCoreApplication::instance()) {
      QCoreApplicationPrivate::removePostedEvent(this);
   }
}

class QEventUserEventRegistration
{
 public:
   QMutex mutex;
   QSet<int> set;
};
Q_GLOBAL_STATIC(QEventUserEventRegistration, userEventRegistrationHelper)

int QEvent::registerEventType(int hint)
{
   QEventUserEventRegistration *userEventRegistration
      = userEventRegistrationHelper();
   if (!userEventRegistration) {
      return -1;
   }

   QMutexLocker locker(&userEventRegistration->mutex);

   // if the type hint hasn't been registered yet, take it
   if (hint >= QEvent::User && hint <= QEvent::MaxUser && !userEventRegistration->set.contains(hint)) {
      userEventRegistration->set.insert(hint);
      return hint;
   }

   // find a free event type, starting at MaxUser and decreasing
   int id = QEvent::MaxUser;
   while (userEventRegistration->set.contains(id) && id >= QEvent::User) {
      --id;
   }
   if (id >= QEvent::User) {
      userEventRegistration->set.insert(id);
      return id;
   }
   return -1;
}

/*!
    \class QTimerEvent
    \brief The QTimerEvent class contains parameters that describe a
    timer event.

    \ingroup events

    Timer events are sent at regular intervals to objects that have
    started one or more timers. Each timer has a unique identifier. A
    timer is started with QObject::startTimer().

    The QTimer class provides a high-level programming interface that
    uses signals instead of events. It also provides single-shot timers.

    The event handler QObject::timerEvent() receives timer events.

    \sa QTimer, QObject::timerEvent(), QObject::startTimer(),
    QObject::killTimer()
*/

/*!
    Constructs a timer event object with the timer identifier set to
    \a timerId.
*/
QTimerEvent::QTimerEvent(int timerId)
   : QEvent(Timer), id(timerId)
{}

/*! \internal
*/
QTimerEvent::~QTimerEvent()
{
}

QChildEvent::QChildEvent(Type type, QObject *child)
   : QEvent(type), c(child)
{}

/*! \internal
*/
QChildEvent::~QChildEvent()
{
}

QDynamicPropertyChangeEvent::QDynamicPropertyChangeEvent(const QByteArray &name)
   : QEvent(QEvent::DynamicPropertyChange), n(name)
{
}

/*!
    \internal
*/
QDynamicPropertyChangeEvent::~QDynamicPropertyChangeEvent()
{
}

/*!
    \fn QByteArray QDynamicPropertyChangeEvent::propertyName() const

    Returns the name of the dynamic property that was added, changed or
    removed.

    \sa QObject::setProperty(), QObject::dynamicPropertyNames()
*/

QT_END_NAMESPACE
