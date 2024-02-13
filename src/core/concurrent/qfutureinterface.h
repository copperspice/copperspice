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

#ifndef QFUTUREINTERFACE_H
#define QFUTUREINTERFACE_H

#include <qglobal.h>
#include <qrunnable.h>
#include <qmutex.h>
#include <qtconcurrentexception.h>
#include <qtconcurrentresultstore.h>

template <typename T>
class QFuture;

class QFutureInterfaceBasePrivate;

class QFutureWatcherBase;
class QFutureWatcherBasePrivate;

class Q_CORE_EXPORT QFutureInterfaceBase
{
 public:
   enum State {
      NoState   = 0x00,
      Running   = 0x01,
      Started   = 0x02,
      Finished  = 0x04,
      Canceled  = 0x08,
      Paused    = 0x10,
      Throttled = 0x20
   };

   QFutureInterfaceBase(State initialState = NoState);
   QFutureInterfaceBase(const QFutureInterfaceBase &other);
   virtual ~QFutureInterfaceBase();

   // reporting functions available to the engine author:
   void reportStarted();
   void reportFinished();
   void reportCanceled();
   void reportException(const QtConcurrent::Exception &e);
   void reportResultsReady(int beginIndex, int endIndex);

   void setRunnable(QRunnable *runnable);
   void setFilterMode(bool enable);
   void setProgressRange(int minimum, int maximum);
   int progressMinimum() const;
   int progressMaximum() const;
   bool isProgressUpdateNeeded() const;
   void setProgressValue(int progressValue);
   int progressValue() const;
   void setProgressValueAndText(int progressValue, const QString &progressText);
   QString progressText() const;

   void setExpectedResultCount(int resultCount);
   int expectedResultCount();
   int resultCount() const;

   bool queryState(State state) const;
   bool isRunning() const;
   bool isStarted() const;
   bool isCanceled() const;
   bool isFinished() const;
   bool isPaused() const;
   bool isThrottled() const;
   bool isResultReadyAt(int index) const;

   void cancel();
   void setPaused(bool paused);
   void togglePaused();
   void setThrottled(bool enable);

   void waitForFinished();
   bool waitForNextResult();
   void waitForResult(int resultIndex);
   void waitForResume();

   QMutex *mutex() const;
   QtConcurrent::cs_internal::ExceptionStore &exceptionStore();
   QtConcurrent::ResultStoreBase &resultStoreBase();
   const QtConcurrent::ResultStoreBase &resultStoreBase() const;

   bool operator==(const QFutureInterfaceBase &other) const {
      return d == other.d;
   }

   bool operator!=(const QFutureInterfaceBase &other) const {
      return d != other.d;
   }

   QFutureInterfaceBase &operator=(const QFutureInterfaceBase &other);

 protected:
   bool referenceCountIsOne() const;

 private:
   QFutureInterfaceBasePrivate *d;

   friend class QFutureWatcherBase;
   friend class QFutureWatcherBasePrivate;
};

template <typename T>
class QFutureInterface : public QFutureInterfaceBase
{
 public:
   QFutureInterface(State initialState = NoState)
      : QFutureInterfaceBase(initialState) {
   }
   QFutureInterface(const QFutureInterface &other)
      : QFutureInterfaceBase(other) {
   }
   ~QFutureInterface() {
      if (referenceCountIsOne()) {
         resultStore().clear();
      }
   }

   static QFutureInterface canceledResult() {
      return QFutureInterface(State(Started | Finished | Canceled));
   }

   QFutureInterface &operator=(const QFutureInterface &other) {
      if (referenceCountIsOne()) {
         resultStore().clear();
      }

      QFutureInterfaceBase::operator=(other);
      return *this;
   }

   inline QFuture<T> future(); // implemented in qfuture.h

   inline void reportResult(const T *result, int index = -1);
   inline void reportResult(const T &result, int index = -1);
   inline void reportResults(const QVector<T> &results, int beginIndex = -1, int count = -1);
   inline void reportFinished(const T *result = 0);

   inline const T &resultReference(int index) const;
   inline const T *resultPointer(int index) const;
   inline QList<T> results();

 private:
   QtConcurrent::ResultStore<T> &resultStore() {
      return static_cast<QtConcurrent::ResultStore<T> &>(resultStoreBase());
   }
   const QtConcurrent::ResultStore<T> &resultStore() const {
      return static_cast<const QtConcurrent::ResultStore<T> &>(resultStoreBase());
   }
};

template <typename T>
inline void QFutureInterface<T>::reportResult(const T *result, int index)
{
   QMutexLocker locker(mutex());

   if (this->queryState(Canceled) || this->queryState(Finished)) {
      return;
   }

   QtConcurrent::ResultStore<T> &store = resultStore();

   if (store.filterMode()) {
      const int resultCountBefore = store.count();
      store.addResult(index, result);
      this->reportResultsReady(resultCountBefore, resultCountBefore + store.count());
   } else {
      const int insertIndex = store.addResult(index, result);
      this->reportResultsReady(insertIndex, insertIndex + 1);
   }
}

template <typename T>
inline void QFutureInterface<T>::reportResult(const T &result, int index)
{
   reportResult(&result, index);
}

template <typename T>
inline void QFutureInterface<T>::reportResults(const QVector<T> &_results, int beginIndex, int count)
{
   QMutexLocker locker(mutex());

   if (this->queryState(Canceled) || this->queryState(Finished)) {
      return;
   }

   QtConcurrent::ResultStore<T> &store = resultStore();

   if (store.filterMode()) {
      const int resultCountBefore = store.count();
      store.addResults(beginIndex, &_results, count);
      this->reportResultsReady(resultCountBefore, store.count());
   } else {
      const int insertIndex = store.addResults(beginIndex, &_results, count);
      this->reportResultsReady(insertIndex, insertIndex + _results.count());
   }
}

template <typename T>
inline void QFutureInterface<T>::reportFinished(const T *result)
{
   if (result) {
      reportResult(result);
   }

   QFutureInterfaceBase::reportFinished();
}

template <typename T>
inline const T &QFutureInterface<T>::resultReference(int index) const
{
   QMutexLocker lock(mutex());
   return resultStore().resultAt(index).value();
}

template <typename T>
inline const T *QFutureInterface<T>::resultPointer(int index) const
{
   QMutexLocker lock(mutex());
   return resultStore().resultAt(index).pointer();
}

template <typename T>
inline QList<T> QFutureInterface<T>::results()
{
   if (this->isCanceled()) {
      exceptionStore().throwPossibleException();
      return QList<T>();
   }

   QFutureInterfaceBase::waitForResult(-1);

   QList<T> res;
   QMutexLocker lock(mutex());

   QtConcurrent::ResultIterator<T> it = resultStore().begin();

   while (it != resultStore().end()) {
      res.append(it.value());
      ++it;
   }

   return res;
}

template <>
class QFutureInterface<void> : public QFutureInterfaceBase
{
 public:
   QFutureInterface(State initialState = State::NoState)
      : QFutureInterfaceBase(initialState)
   {
   }

   QFutureInterface(const QFutureInterface &other)
      : QFutureInterfaceBase(other)
   {
   }

   static QFutureInterface canceledResult() {
      return QFutureInterface(State(State::Started | State::Finished | State::Canceled));
   }

   QFutureInterface &operator=(const QFutureInterface &other) {
      QFutureInterfaceBase::operator=(other);
      return *this;
   }

   inline QFuture<void> future(); // implemented in qfuture.h

   void reportResult(const void *, int) {
   }

   void reportResults(const QVector<void> &, int) {
   }

   void reportFinished(void * = nullptr) {
      QFutureInterfaceBase::reportFinished();
   }
};

#endif
