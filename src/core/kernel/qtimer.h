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

#ifndef QTIMER_H
#define QTIMER_H

#include <qbasictimer.h>
#include <qobject.h>

class Q_CORE_EXPORT QTimer : public QObject
{
   CORE_CS_OBJECT(QTimer)

   CORE_CS_PROPERTY_READ(singleShot, isSingleShot)
   CORE_CS_PROPERTY_WRITE(singleShot, setSingleShot)

   CORE_CS_PROPERTY_READ(interval, interval)
   CORE_CS_PROPERTY_WRITE(interval, setInterval)

   CORE_CS_PROPERTY_READ(remainingTime, remainingTime)
   CORE_CS_PROPERTY_READ(timerType, timerType)
   CORE_CS_PROPERTY_WRITE(timerType, setTimerType)

   CORE_CS_PROPERTY_READ(active, isActive)

 public:
   explicit QTimer(QObject *parent = nullptr);

   QTimer(const QTimer &) = delete;
   QTimer &operator=(const QTimer &) = delete;

   ~QTimer();

   bool isActive() const {
      return id >= 0;
   }

   int timerId() const {
      return id;
   }

   void setInterval(int msec);
   int interval() const {
      return inter;
   }

   int remainingTime() const;
   void setTimerType(Qt::TimerType timerType) {
      this->type = timerType;
   }

   Qt::TimerType timerType() const {
      return Qt::TimerType(type);
   }

   void setSingleShot(bool value);

   bool isSingleShot() const {
      return single;
   }

   static void singleShot(int msec, const QObject *receiver, const QString &slotMethod);
   static void singleShot(int msec, Qt::TimerType timerType, const QObject *receiver, const QString &slotMethod);

   // **A  slot in a QObject
   template <typename Receiver, typename SlotClass, typename SlotReturn>
   static typename std::enable_if<std::is_base_of<SlotClass, Receiver>::value>::type
   singleShot(int msec, Receiver *receiver, SlotReturn (SlotClass::*slotMethod)()) {
      singleShot(msec, msec >= 2000 ? Qt::CoarseTimer : Qt::PreciseTimer, receiver, slotMethod);
   }

   template <typename Receiver, typename SlotClass, typename SlotReturn>
   static typename std::enable_if<std::is_base_of<SlotClass, Receiver>::value>::type
         singleShot(int msec, Qt::TimerType timerType, Receiver *receiver, SlotReturn (SlotClass::*slotMethod)()) {

      std::unique_ptr<CSBento< SlotReturn (SlotClass::*)() >> slotBento =
            std::make_unique<CSBento< SlotReturn(SlotClass::*)() >>(std::move(slotMethod));

      singleShot_internal(msec, timerType, receiver, std::move(slotBento));
   }

   // ** B  slot is a function pointer (without receiver)
   template <typename T>
   static typename std::enable_if < ! std::is_member_function_pointer<T>::value &&
         ! std::is_convertible<T, QString>::value && ! std::is_convertible<T, const char *>::value, void >::type
         singleShot(int msec, T slotMethod) {
      singleShot(msec, msec >= 2000 ? Qt::CoarseTimer : Qt::PreciseTimer, nullptr, slotMethod);
   }

   template <typename T>
   static typename std::enable_if < ! std::is_member_function_pointer<T>::value &&
         ! std::is_convertible<T, QString>::value && ! std::is_convertible<T, const char *>::value, void >::type
         singleShot(int msec, Qt::TimerType timerType, T slotMethod) {
      singleShot(msec, timerType, nullptr, slotMethod);
   }

   // ** C  slot is a function pointer (with receiver)
   template <typename T>
   static inline typename std::enable_if < ! std::is_member_function_pointer<T>::value &&
         ! std::is_convertible<T, QString>::value && ! std::is_convertible<T, const char *>::value, void >::type
         singleShot(int msec, QObject *receiver, T slotMethod) {
      singleShot(msec, msec >= 2000 ? Qt::CoarseTimer : Qt::PreciseTimer, receiver, slotMethod);
   }

   template <typename T>
   static typename std::enable_if < ! std::is_member_function_pointer<T>::value &&
         ! std::is_convertible<T, QString>::value && ! std::is_convertible<T, const char *>::value, void >::type
         singleShot(int msec, Qt::TimerType timerType, QObject *receiver, T slotMethod) {

      std::unique_ptr<CSBento<T>> slotBento = std::make_unique<CSBento<T>>(std::move(slotMethod));

      singleShot_internal(msec, timerType, receiver, std::move(slotBento));
   }

   CORE_CS_SLOT_1(Public, void start(int msec))
   CORE_CS_SLOT_OVERLOAD(start, (int))

   CORE_CS_SLOT_1(Public, void start())
   CORE_CS_SLOT_OVERLOAD(start, ())

   CORE_CS_SLOT_1(Public, void stop())
   CORE_CS_SLOT_2(stop)

   CORE_CS_SIGNAL_1(Public, void timeout())
   CORE_CS_SIGNAL_2(timeout)

 protected:
   void timerEvent(QTimerEvent *event) override;

 private:
   int startTimer(int) {
      return -1;
   }

   void killTimer(int) {
   }

   static void singleShot_internal(int msec, Qt::TimerType timerType,
         const QObject *receiver, std::unique_ptr<CSBentoAbstract> slotBento);

   int id;
   int inter;
   int del;

   uint single : 1;
   uint nulltimer : 1;
   uint type : 2;
};

inline void QTimer::setSingleShot(bool value)
{
   single = value;
}

#endif
