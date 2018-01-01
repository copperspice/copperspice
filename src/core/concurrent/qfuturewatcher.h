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

#ifndef QFUTUREWATCHER_H
#define QFUTUREWATCHER_H

#include <QtCore/qfuture.h>
#include <QtCore/qobject.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QEvent;
class QFutureWatcherBasePrivate;

class Q_CORE_EXPORT QFutureWatcherBase : public QObject
{
   CORE_CS_OBJECT(QFutureWatcherBase)
   Q_DECLARE_PRIVATE(QFutureWatcherBase)

 public:
   QFutureWatcherBase(QObject *parent = nullptr);
   ~QFutureWatcherBase();

   int progressValue() const;
   int progressMinimum() const;
   int progressMaximum() const;
   QString progressText() const;

   bool isStarted() const;
   bool isFinished() const;
   bool isRunning() const;
   bool isCanceled() const;
   bool isPaused() const;

   void waitForFinished();

   void setPendingResultsLimit(int limit);

   bool event(QEvent *event) override;

 public:
   CORE_CS_SIGNAL_1(Public, void started())
   CORE_CS_SIGNAL_2(started)
   CORE_CS_SIGNAL_1(Public, void finished())
   CORE_CS_SIGNAL_2(finished)
   CORE_CS_SIGNAL_1(Public, void canceled())
   CORE_CS_SIGNAL_2(canceled)
   CORE_CS_SIGNAL_1(Public, void paused())
   CORE_CS_SIGNAL_2(paused)
   CORE_CS_SIGNAL_1(Public, void resumed())
   CORE_CS_SIGNAL_2(resumed)
   CORE_CS_SIGNAL_1(Public, void resultReadyAt(int resultIndex))
   CORE_CS_SIGNAL_2(resultReadyAt, resultIndex)
   CORE_CS_SIGNAL_1(Public, void resultsReadyAt(int beginIndex, int endIndex))
   CORE_CS_SIGNAL_2(resultsReadyAt, beginIndex, endIndex)
   CORE_CS_SIGNAL_1(Public, void progressRangeChanged(int minimum, int maximum))
   CORE_CS_SIGNAL_2(progressRangeChanged, minimum, maximum)
   CORE_CS_SIGNAL_1(Public, void progressValueChanged(int progressValue))
   CORE_CS_SIGNAL_2(progressValueChanged, progressValue)
   CORE_CS_SIGNAL_1(Public, void progressTextChanged(const QString &progressText))
   CORE_CS_SIGNAL_2(progressTextChanged, progressText)

   CORE_CS_SLOT_1(Public, void cancel())
   CORE_CS_SLOT_2(cancel)
   CORE_CS_SLOT_1(Public, void setPaused(bool paused))
   CORE_CS_SLOT_2(setPaused)
   CORE_CS_SLOT_1(Public, void pause())
   CORE_CS_SLOT_2(pause)
   CORE_CS_SLOT_1(Public, void resume())
   CORE_CS_SLOT_2(resume)
   CORE_CS_SLOT_1(Public, void togglePaused())
   CORE_CS_SLOT_2(togglePaused)

 protected:
   void connectNotify (const char *signal);
   void disconnectNotify (const char *signal);

   // called from setFuture() implemented in template sub-classes
   void connectOutputInterface();
   void disconnectOutputInterface(bool pendingAssignment = false);

   QScopedPointer<QFutureWatcherBasePrivate> d_ptr;

 private:
   // implemented in the template sub-classes
   virtual const QFutureInterfaceBase &futureInterface() const = 0;
   virtual QFutureInterfaceBase &futureInterface() = 0;
};

template <typename T>
class QFutureWatcher : public QFutureWatcherBase
{
 public:
   QFutureWatcher(QObject *_parent = 0)
      : QFutureWatcherBase(_parent) {
   }

   ~QFutureWatcher() {
      disconnectOutputInterface();
   }

   void setFuture(const QFuture<T> &future);
   QFuture<T> future() const {
      return m_future;
   }

   T result() const {
      return m_future.result();
   }
   T resultAt(int index) const {
      return m_future.resultAt(index);
   }

 private:
   QFuture<T> m_future;
   const QFutureInterfaceBase &futureInterface() const {
      return m_future.d;
   }
   QFutureInterfaceBase &futureInterface() {
      return m_future.d;
   }
};

template <typename T>
Q_INLINE_TEMPLATE void QFutureWatcher<T>::setFuture(const QFuture<T> &_future)
{
   if (_future == m_future) {
      return;
   }

   disconnectOutputInterface(true);
   m_future = _future;
   connectOutputInterface();
}

template <>
class QFutureWatcher<void> : public QFutureWatcherBase
{
 public:
   QFutureWatcher(QObject *_parent = 0)
      : QFutureWatcherBase(_parent) {
   }

   ~QFutureWatcher() {
      disconnectOutputInterface();
   }

   void setFuture(const QFuture<void> &future);
   QFuture<void> future() const {
      return m_future;
   }

 private:
   QFuture<void> m_future;
   const QFutureInterfaceBase &futureInterface() const override {
      return m_future.d;
   }

   QFutureInterfaceBase &futureInterface() override {
      return m_future.d;
   }
};

Q_INLINE_TEMPLATE void QFutureWatcher<void>::setFuture(const QFuture<void> &_future)
{
   if (_future == m_future) {
      return;
   }

   disconnectOutputInterface(true);
   m_future = _future;
   connectOutputInterface();
}

QT_END_NAMESPACE

#endif // QFUTUREWATCHER_H
