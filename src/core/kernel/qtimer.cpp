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

#include <qtimer.h>

#include <qabstracteventdispatcher.h>
#include <qcoreapplication.h>

static constexpr const int INV_TIMER = -1;                // invalid timer id

QTimer::QTimer(QObject *parent)
   : QObject(parent), id(INV_TIMER), inter(0), del(0), single(0), nulltimer(0), type(Qt::CoarseTimer)
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
   id = QObject::startTimer(inter, Qt::TimerType(type));
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

 public:
   QSingleShotTimer(int msec, Qt::TimerType timerType, const QObject *receiver, const QString &slotMethod);
   QSingleShotTimer(int msec, Qt::TimerType timerType, const QObject *receiver, std::unique_ptr<CSBentoAbstract> slotBento);

   ~QSingleShotTimer();

   CORE_CS_SIGNAL_1(Public, void timeout())
   CORE_CS_SIGNAL_2(timeout)

 protected:
   void timerEvent(QTimerEvent *) override;

 private:
   int timerId;
   bool hasValidReceiver;

   QPointer<const QObject> m_receiver;
   std::unique_ptr<CSBentoAbstract> m_slotBento;
};

QSingleShotTimer::QSingleShotTimer(int msec, Qt::TimerType timerType, const QObject *receiver, const QString &slotMethod)
   : QObject(QAbstractEventDispatcher::instance()), hasValidReceiver(true)
{
   timerId = startTimer(msec, timerType);
   connect(this, SIGNAL(timeout()), receiver, slotMethod);
}

QSingleShotTimer::QSingleShotTimer(int msec, Qt::TimerType timerType, const QObject *receiver,
      std::unique_ptr<CSBentoAbstract> slotBento)
   : QObject(QAbstractEventDispatcher::instance()), hasValidReceiver(receiver != nullptr),
     m_receiver(receiver), m_slotBento(std::move(slotBento))
{
   timerId = startTimer(msec, timerType);

   if (m_receiver && thread() != m_receiver->thread()) {
      // Avoid leaking the QSingleShotTimer instance in case the application exits before the timer fires
      connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &QObject::deleteLater);
      setParent(nullptr);
      moveToThread(m_receiver->thread());
   }
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

   if (m_slotBento) {
      // if the receiver was destroyed, skip this part

      if ( ! m_receiver.isNull() || ! hasValidReceiver) {
         // allocate only the return type, previously checked the function had no arguments

         auto args = CsSignal::Internal::TeaCup_Data<>(false);

         m_slotBento->invoke(const_cast<QObject *>(m_receiver.data()), &args);
      }

   } else {
      emit timeout();
   }

   delete this;
}

void QTimer::singleShot_internal(int msec, Qt::TimerType timerType, const QObject *receiver,
      std::unique_ptr<CSBentoAbstract> slotBento)
{
   new QSingleShotTimer(msec, timerType, receiver, std::move(slotBento));
}

void QTimer::singleShot(int msec, const QObject *receiver, const QString &slotMethod)
{
   singleShot(msec, msec >= 2000 ? Qt::CoarseTimer : Qt::PreciseTimer, receiver, slotMethod);
}

void QTimer::singleShot(int msec, Qt::TimerType timerType, const QObject *receiver, const QString &slotMethod)
{
   if (msec < 0) {
      qWarning("QTimer::singleShot() Timer duration can not be negative");
      return;
   }

   if (receiver != nullptr && ! slotMethod.isEmpty()) {
      if (msec == 0) {
         // special code shortpath for 0 timers
         int bracketPosition = slotMethod.indexOf('(');

         if (bracketPosition == -1) {
            qWarning("QTimer::singleShot() Invalid slot specification");
            return;
         }

         // extract method name
         QString methodName = slotMethod.left(bracketPosition);
         QMetaObject::invokeMethod(const_cast<QObject *>(receiver), methodName, Qt::QueuedConnection);

         return;
      }

      (void) new QSingleShotTimer(msec, timerType, receiver, slotMethod);
   }
}

void QTimer::setInterval(int msec)
{
   inter = msec;

   // create new timer
   if (id != INV_TIMER) {
      QObject::killTimer(id);

      // restart timer
      id = QObject::startTimer(msec, Qt::TimerType(type));
   }
}

int QTimer::remainingTime() const
{
   if (id != INV_TIMER) {
      return QAbstractEventDispatcher::instance()->remainingTime(id);
   }

   return -1;
}
