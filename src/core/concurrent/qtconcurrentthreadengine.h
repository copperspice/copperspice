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

#ifndef QTCONCURRENTTHREADENGINE_H
#define QTCONCURRENTTHREADENGINE_H

#include <qatomic.h>
#include <qdebug.h>
#include <qfuture.h>
#include <qglobal.h>
#include <qsemaphore.h>
#include <qtconcurrentexception.h>
#include <qthreadpool.h>
#include <qwaitcondition.h>

namespace QtConcurrent {

// The ThreadEngineBarrier counts worker threads, and allows one
// thread to wait for all others to finish. Tested for its use in
// QtConcurrent, requires more testing for use as a general class.
class ThreadEngineBarrier
{
 public:
   ThreadEngineBarrier();
   void acquire();
   int release();
   void wait();
   int currentCount();
   bool releaseUnlessLast();

 private:
   // The thread count is maintained as an integer in the count atomic
   // variable. The count can be either positive or negative - a negative
   // count signals that a thread is waiting on the barrier.

   QAtomicInt count;
   QSemaphore semaphore;
};

enum ThreadFunctionResult {
   ThrottleThread,
   ThreadFinished
};

// ThreadEngine controls the threads used in the computation.
// Can be run in three modes: single threaded, multi-threaded blocking & multi-threaded asynchronous.
// This is the code for the single threaded mode:

class Q_CORE_EXPORT ThreadEngineBase: public QRunnable
{
 public:
   ThreadEngineBase();
   virtual ~ThreadEngineBase();
   void startSingleThreaded();
   void startBlocking();
   void startThread();
   bool isCanceled();
   void waitForResume();
   bool isProgressReportingEnabled();
   void setProgressValue(int progress);
   void setProgressRange(int minimum, int maximum);
   void acquireBarrierSemaphore();

 protected:
   virtual void start() {
   }

   virtual void finish() {
   }

   virtual ThreadFunctionResult threadFunction() {
      return ThreadFinished;
   }

   virtual bool shouldStartThread() {
      return futureInterface ? !futureInterface->isPaused() : true;
   }

   virtual bool shouldThrottleThread() {
      return futureInterface ? futureInterface->isPaused() : false;
   }

   QFutureInterfaceBase *futureInterface;
   QThreadPool *threadPool;
   ThreadEngineBarrier barrier;
   QtConcurrent::cs_internal::ExceptionStore exceptionStore;

 private:
   bool startThreadInternal();
   void startThreads();
   void threadExit();
   bool threadThrottleExit();
   void run() override;
   virtual void asynchronousFinish() = 0;
   void handleException(const QtConcurrent::Exception &exception);
};

template <typename T>
class ThreadEngine : public virtual ThreadEngineBase
{
 public:
   using ResultType = T;

   virtual T *result() {
      return nullptr;
   }

   QFutureInterface<T> *futureInterfaceTyped() {
      return static_cast<QFutureInterface<T> *>(futureInterface);
   }

   // Runs the user algorithm using a single thread.
   T *startSingleThreaded() {
      ThreadEngineBase::startSingleThreaded();
      return result();
   }

   // Runs the user algorithm using multiple threads.
   // This function blocks until the algorithm is finished,
   // and then returns the result.
   T *startBlocking() {
      ThreadEngineBase::startBlocking();
      return result();
   }

   // Runs the user algorithm using multiple threads.
   // Does not block, returns a future.
   QFuture<T> startAsynchronously() {
      futureInterface = new QFutureInterface<T>();

      // reportStart() must be called before starting threads, otherwise the
      // user algorithm might finish while reportStart() is running, which is very bad.
      futureInterface->reportStarted();
      QFuture<T> future = QFuture<T>(futureInterfaceTyped());
      start();

      acquireBarrierSemaphore();
      threadPool->start(this);
      return future;
   }

   void asynchronousFinish() override {
      finish();
      futureInterfaceTyped()->reportFinished(result());
      delete futureInterfaceTyped();
      delete this;
   }

   void reportResult(const T *_result, int index = -1) {
      if (futureInterface) {
         futureInterfaceTyped()->reportResult(_result, index);
      }
   }

   void reportResults(const QVector<T> &_result, int index = -1, int count = -1) {
      if (futureInterface) {
         futureInterfaceTyped()->reportResults(_result, index, count);
      }
   }
};

// The ThreadEngineStarter class ecapsulates the return type fom the thread engine.
// Depending on how the it is used, it will run the engine in either blocking mode or asynchronous mode.
template <typename T>
class ThreadEngineStarterBase
{
 public:
   ThreadEngineStarterBase(ThreadEngine<T> *_threadEngine)
      : threadEngine(_threadEngine)
   { }

   ThreadEngineStarterBase(const ThreadEngineStarterBase &other)
      : threadEngine(other.threadEngine)
   { }

   QFuture<T> startAsynchronously() {
      return threadEngine->startAsynchronously();
   }

   operator QFuture<T>() {
      return startAsynchronously();
   }

 protected:
   ThreadEngine<T> *threadEngine;
};

// factor out the code that dereferences the T pointer, with a specialization where T is void.
// (code that dereferences a void * will not compile)
template <typename T>
class ThreadEngineStarter : public ThreadEngineStarterBase<T>
{
   using Base              = ThreadEngineStarterBase<T>;
   using TypedThreadEngine = ThreadEngine<T>;

 public:
   ThreadEngineStarter(TypedThreadEngine *eng)
      : Base(eng)
   { }

   T startBlocking() {
      T t = *this->threadEngine->startBlocking();
      delete this->threadEngine;
      return t;
   }
};

// Full template specialization where T is void.
template <>
class ThreadEngineStarter<void> : public ThreadEngineStarterBase<void>
{
 public:
   ThreadEngineStarter<void>(ThreadEngine<void> *_threadEngine)
      : ThreadEngineStarterBase<void>(_threadEngine)
   { }

   void startBlocking() {
      this->threadEngine->startBlocking();
      delete this->threadEngine;
   }
};

template <typename ThreadEngine>
inline ThreadEngineStarter<typename ThreadEngine::ResultType> startThreadEngine(ThreadEngine *threadEngine)
{
   return ThreadEngineStarter<typename ThreadEngine::ResultType>(threadEngine);
}

} // namespace QtConcurrent

#endif
