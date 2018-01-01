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

#include <qtimer.h>
#include <qabstracteventdispatcher.h>
#include <qcoreapplication.h>

QT_BEGIN_NAMESPACE

static const int INV_TIMER = -1;                // invalid timer id

QTimer::QTimer(QObject *parent)
   : QObject(parent), id(INV_TIMER), inter(0), del(0), single(0), nulltimer(0)
{
}

QTimer::~QTimer()
{
   if (id != INV_TIMER) {                      // stop running timer
      stop();
   }
}

void QTimer::start()
{
   if (id != INV_TIMER) {                      // stop running timer
      stop();
   }
   nulltimer = (!inter && single);
   id = QObject::startTimer(inter);
}

void QTimer::start(int msec)
{
   inter = msec;
   start();
}

void QTimer::stop()
{
   if (id != INV_TIMER) {
      QObject::killTimer(id);
      id = INV_TIMER;
   }
}

void QTimer::timerEvent(QTimerEvent *e)
{
   if (e->timerId() == id) {
      if (single) {
         stop();
      }
      emit timeout();
   }
}

class QSingleShotTimer : public QObject
{
   CORE_CS_OBJECT(QSingleShotTimer)
   int timerId;

 public:
   ~QSingleShotTimer();
   QSingleShotTimer(int msec, QObject *r, const char *m);

   CORE_CS_SIGNAL_1(Public, void timeout())
   CORE_CS_SIGNAL_2(timeout)

 protected:
   void timerEvent(QTimerEvent *) override;
};

QSingleShotTimer::QSingleShotTimer(int msec, QObject *receiver, const char *member)
   : QObject(QAbstractEventDispatcher::instance())
{
   connect(this, SIGNAL(timeout()), receiver, member);
   timerId = startTimer(msec);
}

QSingleShotTimer::~QSingleShotTimer()
{
   if (timerId > 0) {
      killTimer(timerId);
   }
}

void QSingleShotTimer::timerEvent(QTimerEvent *)
{
   // need to kill the timer _before_ we emit timeout() in case the
   // slot connected to timeout calls processEvents()
   if (timerId > 0) {
      killTimer(timerId);
   }
   timerId = -1;
   emit timeout();

   // we would like to use delete later here, but it feels like a
   // waste to post a new event to handle this event, so we just unset the flag
   // and explicitly delete
   delete this;
}

void QTimer::singleShot(int msec, QObject *receiver, const char *member)
{
   if (receiver && member) {
      if (msec == 0) {
         // special code shortpath for 0-timers
         const char *bracketPosition = strchr(member, '(');

         if (! bracketPosition) {
            qWarning("QTimer::singleShot: Invalid slot specification");
            return;
         }

         // extract method name
         QByteArray methodName(member, bracketPosition - member);

         QMetaObject::invokeMethod(receiver, methodName.constData(), Qt::QueuedConnection);
         return;
      }

      (void) new QSingleShotTimer(msec, receiver, member);
   }
}

void QTimer::setInterval(int msec)
{
   inter = msec;

   // create new timer
   if (id != INV_TIMER) {
      QObject::killTimer(id);

      // restart timer
      id = QObject::startTimer(msec);
   }
}

QT_END_NAMESPACE
