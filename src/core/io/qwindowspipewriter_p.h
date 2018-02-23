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

#ifndef QWINDOWSPIPEWRITER_P_H
#define QWINDOWSPIPEWRITER_P_H

#include <qelapsedtimer.h>
#include <qthread.h>
#include <qmutex.h>
#include <qt_windows.h>

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
   QElapsedTimer timer;
   int totalTimeOut;
   int nextSleep;
};

class Q_CORE_EXPORT QWindowsPipeWriter : public QObject
{
   CORE_CS_OBJECT(QWindowsPipeWriter)

 public:
   explicit QWindowsPipeWriter(HANDLE pipeWriteEnd, QObject *parent = nullptr);
   ~QWindowsPipeWriter();

   bool write(const QByteArray &ba);
   void stop();
   bool waitForWrite(int msecs);

   bool isWriteOperationActive() const { return writeSequenceStarted; }
   qint64 bytesToWrite() const;

   CORE_CS_SIGNAL_1(Public, void canWrite())
   CORE_CS_SIGNAL_2(canWrite)

   CORE_CS_SIGNAL_1(Public, void bytesWritten(qint64 bytes))
   CORE_CS_SIGNAL_2(bytesWritten, bytes)

   CORE_CS_SIGNAL_1(Public, void _q_queueBytesWritten())
   CORE_CS_SIGNAL_2(_q_queueBytesWritten)


 private:
    static void CALLBACK writeFileCompleted(DWORD errorCode, DWORD numberOfBytesTransfered,
                                            OVERLAPPED *overlappedBase);
    void notified(DWORD errorCode, DWORD numberOfBytesWritten);
    bool waitForNotification(int timeout);
    void emitPendingBytesWrittenValue();
    class Overlapped : public OVERLAPPED
    {
        Q_DISABLE_COPY(Overlapped)
    public:
        explicit Overlapped(QWindowsPipeWriter *pipeWriter);
        void clear();
        QWindowsPipeWriter *pipeWriter;
    };
    HANDLE handle;
    Overlapped overlapped;
    QByteArray buffer;
    qint64 numberOfBytesToWrite;
    qint64 pendingBytesWrittenValue;
    bool stopped;
    bool writeSequenceStarted;
    bool notifiedCalled;
    bool bytesWrittenPending;
    bool inBytesWritten;
};

#endif // QT_NO_PROCESS
