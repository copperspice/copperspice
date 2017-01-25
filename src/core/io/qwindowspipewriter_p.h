/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QWINDOWSPIPEWRITER_P_H
#define QWINDOWSPIPEWRITER_P_H

#include <qdatetime.h>
#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qt_windows.h>

QT_BEGIN_NAMESPACE

#define SLEEPMIN 10
#define SLEEPMAX 500

class QIncrementalSleepTimer
{

 public:
   QIncrementalSleepTimer(int msecs)
      : totalTimeOut(msecs)
      , nextSleep(qMin(SLEEPMIN, totalTimeOut)) {
      if (totalTimeOut == -1) {
         nextSleep = SLEEPMIN;
      }
      timer.start();
   }

   int nextSleepTime() {
      int tmp = nextSleep;
      nextSleep = qMin(nextSleep * 2, qMin(SLEEPMAX, timeLeft()));
      return tmp;
   }

   int timeLeft() const {
      if (totalTimeOut == -1) {
         return SLEEPMAX;
      }
      return qMax(totalTimeOut - timer.elapsed(), 0);
   }

   bool hasTimedOut() const {
      if (totalTimeOut == -1) {
         return false;
      }
      return timer.elapsed() >= totalTimeOut;
   }

   void resetIncrements() {
      nextSleep = qMin(SLEEPMIN, timeLeft());
   }

 private:
   QTime timer;
   int totalTimeOut;
   int nextSleep;
};

class Q_CORE_EXPORT QWindowsPipeWriter : public QThread
{
   CORE_CS_OBJECT(QWindowsPipeWriter)

 public:
   CORE_CS_SIGNAL_1(Public, void canWrite())
   CORE_CS_SIGNAL_2(canWrite)
   CORE_CS_SIGNAL_1(Public, void bytesWritten(qint64 bytes))
   CORE_CS_SIGNAL_2(bytesWritten, bytes)

   QWindowsPipeWriter(HANDLE writePipe, QObject *parent = 0);
   ~QWindowsPipeWriter();

   bool waitForWrite(int msecs);
   qint64 write(const char *data, qint64 maxlen);

   qint64 bytesToWrite() const {
      QMutexLocker locker(&lock);
      return data.size();
   }

   bool hadWritten() const {
      return hasWritten;
   }

 protected:
   void run() override;

 private:
   QByteArray data;
   QWaitCondition waitCondition;
   mutable QMutex lock;
   HANDLE writePipe;
   volatile bool quitNow;
   bool hasWritten;
};

QT_END_NAMESPACE

#endif // QT_NO_PROCESS
