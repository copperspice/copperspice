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

#include <qtconcurrentthreadengine.h>

namespace QtConcurrent {

ThreadEngineBarrier::ThreadEngineBarrier()
   : count(0)
{
}

void ThreadEngineBarrier::acquire()
{
   while (true) {
      int localCount = count.load();

      if (localCount < 0) {
         int expected = localCount;

         if (count.compareExchange(expected, localCount - 1)) {
            return;
         }

      } else {
         int expected = localCount;

         if (count.compareExchange(expected, localCount + 1)) {
            return;
         }
      }
   }
}

int ThreadEngineBarrier::release()
{
   while (true)  {
      int localCount = count.load();

      if (localCount == -1) {
         int expected = -1;

         if (count.compareExchange(expected, 0)) {
            semaphore.release();
            return 0;
         }

      } else if (localCount < 0) {
         int expected = localCount;

         if (count.compareExchange(expected, localCount + 1)) {
            return qAbs(localCount + 1);
         }

      } else {
         int expected = localCount;

         if (count.compareExchange(expected, localCount - 1)) {
            return localCount - 1;
         }
      }
   }
}

// Wait until all threads have been released
void ThreadEngineBarrier::wait()
{
   while (true)  {
      int localCount = count.load();

      if (localCount == 0) {
         return;
      }

      Q_ASSERT(localCount > 0); // multiple waiters are not allowed

      int expected = localCount;

      if (count.compareExchange(expected, -localCount)) {
         semaphore.acquire();
         return;
      }
   }
}

int ThreadEngineBarrier::currentCount()
{
   return count.load();
}

// releases a thread, unless this is the last thread.
// returns true if the thread was released.
bool ThreadEngineBarrier::releaseUnlessLast()
{
   while (true)  {
      int localCount = count.load();

      if (qAbs(localCount) == 1) {
         return false;

      } else if (localCount < 0) {
         int expected = localCount;

         if (count.compareExchange(expected, localCount + 1)) {
            return true;
         }

      } else {
         int expected = localCount;

         if (count.compareExchange(expected, localCount - 1)) {
            return true;
         }
      }
   }
}

ThreadEngineBase::ThreadEngineBase()
   : futureInterface(nullptr), threadPool(QThreadPool::globalInstance())
{
   setAutoDelete(false);
}

ThreadEngineBase::~ThreadEngineBase()
{
}

void ThreadEngineBase::startSingleThreaded()
{
   start();

   while (threadFunction() != ThreadFinished) {
      ;
   }

   finish();
}

void ThreadEngineBase::startBlocking()
{
   start();
   barrier.acquire();
   startThreads();

   bool throttled = false;

   try {
      while (threadFunction() == ThrottleThread) {
         if (threadThrottleExit()) {
            throttled = true;
            break;
         }
      }

   } catch (QtConcurrent::Exception &e) {
      handleException(e);
   } catch (...) {
      handleException(QtConcurrent::UnhandledException());
   }

   if (throttled == false) {
      barrier.release();
   }

   barrier.wait();
   finish();
   exceptionStore.throwPossibleException();
}

void ThreadEngineBase::startThread()
{
   startThreadInternal();
}

void ThreadEngineBase::acquireBarrierSemaphore()
{
   barrier.acquire();
}

bool ThreadEngineBase::isCanceled()
{
   if (futureInterface) {
      return futureInterface->isCanceled();
   } else {
      return false;
   }
}

void ThreadEngineBase::waitForResume()
{
   if (futureInterface) {
      futureInterface->waitForResume();
   }
}

bool ThreadEngineBase::isProgressReportingEnabled()
{
   // if we do not have a QFuture, there is no-one to report the progress to.
   return (futureInterface != nullptr);
}

void ThreadEngineBase::setProgressValue(int progress)
{
   if (futureInterface) {
      futureInterface->setProgressValue(progress);
   }
}

void ThreadEngineBase::setProgressRange(int minimum, int maximum)
{
   if (futureInterface) {
      futureInterface->setProgressRange(minimum, maximum);
   }
}

bool ThreadEngineBase::startThreadInternal()
{
   if (this->isCanceled()) {
      return false;
   }

   barrier.acquire();

   if (!threadPool->tryStart(this)) {
      barrier.release();
      return false;
   }

   return true;
}

void ThreadEngineBase::startThreads()
{
   while (shouldStartThread() && startThreadInternal()) {
      ;
   }
}

void ThreadEngineBase::threadExit()
{
   const bool asynchronous = futureInterface != nullptr;
   const int lastThread    = (barrier.release() == 0);

   if (lastThread && asynchronous) {
      this->asynchronousFinish();
   }
}

// Called by a worker thread that wants to be throttled. If the current number
// of running threads is larger than one the thread is allowed to exit and
// this function returns one.
bool ThreadEngineBase::threadThrottleExit()
{
   return barrier.releaseUnlessLast();
}

void ThreadEngineBase::run()
{
   // implements QRunnable

   if (this->isCanceled()) {
      threadExit();
      return;
   }

   startThreads();

   try {

      while (threadFunction() == ThrottleThread) {
         // threadFunction returning ThrottleThread means it that the user
         // struct wants to be throttled by making a worker thread exit.
         // Respect that request unless this is the only worker thread left
         // running, in which case it has to keep going.
         if (threadThrottleExit()) {
            return;
         }
      }

   } catch (QtConcurrent::Exception &e) {
      handleException(e);

   } catch (...) {
      handleException(QtConcurrent::UnhandledException());
   }

   threadExit();
}

void ThreadEngineBase::handleException(const QtConcurrent::Exception &exception)
{
   if (futureInterface) {
      futureInterface->reportException(exception);
   } else {
      exceptionStore.setException(exception);
   }
}

} // namespace QtConcurrent

