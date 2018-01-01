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

#ifndef QTIMER_H
#define QTIMER_H

#include <QtCore/qbasictimer.h>    // conceptual inheritance
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class Q_CORE_EXPORT QTimer : public QObject
{
   CORE_CS_OBJECT(QTimer)

   CORE_CS_PROPERTY_READ(singleShot, isSingleShot)
   CORE_CS_PROPERTY_WRITE(singleShot, setSingleShot)
   CORE_CS_PROPERTY_READ(interval, interval)
   CORE_CS_PROPERTY_WRITE(interval, setInterval)
   CORE_CS_PROPERTY_READ(active, isActive)

 public:
   explicit QTimer(QObject *parent = nullptr);
   ~QTimer();

   inline bool isActive() const;
   inline int timerId() const;

   void setInterval(int msec);
   inline int interval() const;

   inline void setSingleShot(bool singleShot);
   static void singleShot(int msec, QObject *receiver, const char *member);
   inline bool isSingleShot() const;

   CORE_CS_SLOT_1(Public, void start(int msec))
   CORE_CS_SLOT_OVERLOAD(start, (int))

   CORE_CS_SLOT_1(Public, void start())
   CORE_CS_SLOT_OVERLOAD(start, ())

   CORE_CS_SLOT_1(Public, void stop())
   CORE_CS_SLOT_2(stop)

   CORE_CS_SIGNAL_1(Public, void timeout())
   CORE_CS_SIGNAL_2(timeout)

 protected:
   void timerEvent(QTimerEvent *) override;

 private:
   Q_DISABLE_COPY(QTimer)

   inline int startTimer(int) {
      return -1;
   }

   inline void killTimer(int) {}

   int id, inter, del;
   uint single : 1;
   uint nulltimer : 1;
};

bool QTimer::isActive() const
{
   return id >= 0;
}
int  QTimer::timerId() const
{
   return id;
}

int  QTimer::interval() const
{
   return inter;
}

void QTimer::setSingleShot(bool singleShot)
{
   single = singleShot;
}
bool QTimer::isSingleShot() const
{
   return single;
}

QT_END_NAMESPACE

#endif // QTIMER_H
