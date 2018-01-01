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

#ifndef QFUTUREINTERFACE_P_H
#define QFUTUREINTERFACE_P_H

#include <qelapsedtimer.h>
#include <qcoreevent.h>
#include <qlist.h>
#include <qwaitcondition.h>
#include <qrunnable.h>
#include <qfutureinterface.h>
#include <qtconcurrentresultstore.h>
#include <qtconcurrentexception.h>

QT_BEGIN_NAMESPACE

class QFutureCallOutEvent : public QEvent
{
 public:
   enum CallOutType {
      Started,
      Finished,
      Canceled,
      Paused,
      Resumed,
      Progress,
      ProgressRange,
      ResultsReady
   };

   QFutureCallOutEvent()
      : QEvent(QEvent::FutureCallOut), callOutType(CallOutType(0)), index1(-1), index2(-1) {
   }
   QFutureCallOutEvent(CallOutType callOutType, int index1 = -1)
      : QEvent(QEvent::FutureCallOut), callOutType(callOutType), index1(index1), index2(-1) {
   }
   QFutureCallOutEvent(CallOutType callOutType, int index1, int index2)
      : QEvent(QEvent::FutureCallOut), callOutType(callOutType), index1(index1), index2(index2) {
   }

   QFutureCallOutEvent(CallOutType callOutType, int index1, const QString &text)
      : QEvent(QEvent::FutureCallOut),
        callOutType(callOutType),
        index1(index1),
        index2(-1),
        text(text) {
   }

   CallOutType callOutType;
   int index1;
   int index2;
   QString text;

   QFutureCallOutEvent *clone() const {
      return new QFutureCallOutEvent(callOutType, index1, index2, text);
   }

 private:
   QFutureCallOutEvent(CallOutType callOutType,
                       int index1,
                       int index2,
                       const QString &text)
      : QEvent(QEvent::FutureCallOut),
        callOutType(callOutType),
        index1(index1),
        index2(index2),
        text(text) {
   }
};

class QFutureCallOutInterface
{
 public:
   virtual ~QFutureCallOutInterface() {}
   virtual void postCallOutEvent(const QFutureCallOutEvent &) = 0;
   virtual void callOutInterfaceDisconnected() = 0;
};

class QFutureInterfaceBasePrivate
{
 public:
   QFutureInterfaceBasePrivate(QFutureInterfaceBase::State initialState);

   QAtomicInt refCount;
   mutable QMutex m_mutex;
   QWaitCondition waitCondition;
   QList<QFutureCallOutInterface *> outputConnections;
   int m_progressValue;
   int m_progressMinimum;
   int m_progressMaximum;
   QFutureInterfaceBase::State state;
   QElapsedTimer progressTime;
   QWaitCondition pausedWaitCondition;
   int pendingResults;
   QtConcurrent::ResultStoreBase m_results;
   bool manualProgress;
   int m_expectedResultCount;
   QtConcurrent::internal::ExceptionStore m_exceptionStore;
   QString m_progressText;
   QRunnable *runnable;

   // Internal functions that does not change the mutex state.
   // The mutex must be locked when calling these.
   int internal_resultCount() const;
   bool internal_isResultReadyAt(int index) const;
   bool internal_waitForNextResult();
   bool internal_updateProgress(int progress, const QString &progressText = QString());
   void internal_setThrottled(bool enable);
   void sendCallOut(const QFutureCallOutEvent &callOut);
   void sendCallOuts(const QFutureCallOutEvent &callOut1, const QFutureCallOutEvent &callOut2);
   void connectOutputInterface(QFutureCallOutInterface *iface);
   void disconnectOutputInterface(QFutureCallOutInterface *iface);

   void setState(QFutureInterfaceBase::State state);
};

QT_END_NAMESPACE

#endif
