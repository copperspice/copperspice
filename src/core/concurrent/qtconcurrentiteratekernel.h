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

#ifndef QTCONCURRENTITERATEKERNEL_H
#define QTCONCURRENTITERATEKERNEL_H

#include <qatomic.h>
#include <qglobal.h>
#include <qtconcurrentmedian.h>
#include <qtconcurrentthreadengine.h>

#include <iterator>

namespace QtConcurrent {

using std::advance;

/*
    The BlockSizeManager class manages how many iterations a thread should
    reserve and process at a time. This is done by measuring the time spent
    in the user code versus the control part code, and then increasing
    the block size if the ratio between them is to small. The block size
    management is done on the basis of the median of several timing measuremens,
    and it is done induvidualy for each thread.
*/
class Q_CORE_EXPORT BlockSizeManager
{
 public:
   BlockSizeManager(int iterationCount);
   void timeBeforeUser();
   void timeAfterUser();
   int blockSize();

 private:
   bool blockSizeMaxed() {
      return (m_blockSize >= maxBlockSize);
   }

   const int maxBlockSize;
   qint64 beforeUser;
   qint64 afterUser;
   Median<double> controlPartElapsed;
   Median<double> userPartElapsed;
   int m_blockSize;
};

template <typename T>
class ResultReporter
{
 public:
   ResultReporter(ThreadEngine<T> *_threadEngine)
      : threadEngine(_threadEngine)
   {
   }

   void reserveSpace(int resultCount) {
      currentResultCount = resultCount;
      vector.resize(qMax(resultCount, vector.count()));
   }

   void reportResults(int begin) {
      const int useVectorThreshold = 4; // Tunable parameter.

      if (currentResultCount > useVectorThreshold) {
         vector.resize(currentResultCount);
         threadEngine->reportResults(vector, begin);
      } else {
         for (int i = 0; i < currentResultCount; ++i) {
            threadEngine->reportResult(&vector.at(i), begin + i);
         }
      }
   }

   T *getPointer() {
      return vector.data();
   }

   int currentResultCount;
   ThreadEngine<T> *threadEngine;
   QVector<T> vector;
};

template <>
class ResultReporter<void>
{
 public:
   ResultReporter(ThreadEngine<void> *) {
   }

   void reserveSpace(int) {
   };

   void reportResults(int) {
   };

   void *getPointer() {
      return nullptr;
   }
};

inline bool selectIteration(std::bidirectional_iterator_tag)
{
   return false; // while
}

inline bool selectIteration(std::forward_iterator_tag)
{
   return false; // while
}

inline bool selectIteration(std::random_access_iterator_tag)
{
   return true; // for
}

template <typename Iterator, typename T>
class IterateKernel : public ThreadEngine<T>
{
 public:
   using ResultType = T;

   IterateKernel(Iterator _begin, Iterator _end)
      : begin(_begin), end(_end), current(_begin), currentIndex(0),
        forIteration(selectIteration(typename std::iterator_traits<Iterator>::iterator_category())),
        progressReportingEnabled(true)

   {
      iterationCount =  forIteration ? std::distance(_begin, _end) : 0;

   }

   virtual ~IterateKernel() { }

   virtual bool runIteration(Iterator it, int index, T *result) {
      (void) it;
      (void) index;
      (void) result;
      return false;
   }

   virtual bool runIterations(Iterator _begin, int beginIndex, int endIndex, T *results) {
      (void) _begin;
      (void) beginIndex;
      (void) endIndex;
      (void) results;
      return false;
   }

   void start() {
      progressReportingEnabled = this->isProgressReportingEnabled();

      if (progressReportingEnabled && iterationCount > 0) {
         this->setProgressRange(0, iterationCount);
      }
   }

   bool shouldStartThread() {
      if (forIteration) {
         return (currentIndex.load() < iterationCount) && !this->shouldThrottleThread();
      } else { // whileIteration
         return (iteratorThreads.load() == 0);
      }
   }

   ThreadFunctionResult threadFunction() {
      if (forIteration) {
         return this->forThreadFunction();
      } else { // whileIteration
         return this->whileThreadFunction();
      }
   }

   ThreadFunctionResult forThreadFunction() {
      BlockSizeManager blockSizeManager(iterationCount);
      ResultReporter<T> resultReporter(this);

      for (;;) {
         if (this->isCanceled()) {
            break;
         }

         const int currentBlockSize = blockSizeManager.blockSize();

         if (currentIndex.load() >= iterationCount) {
            break;
         }

         // Atomically reserve a block of iterationCount for this thread.
         const int beginIndex = currentIndex.fetchAndAddRelease(currentBlockSize);
         const int endIndex = qMin(beginIndex + currentBlockSize, iterationCount);

         if (beginIndex >= endIndex) {
            // No more work
            break;
         }

         this->waitForResume(); // (only waits if the qfuture is paused.)

         if (shouldStartThread()) {
            this->startThread();
         }

         const int finalBlockSize = endIndex - beginIndex; // block size adjusted for possible end-of-range
         resultReporter.reserveSpace(finalBlockSize);

         // Call user code with the current iteration range.
         blockSizeManager.timeBeforeUser();
         const bool resultsAvailable = this->runIterations(begin, beginIndex, endIndex, resultReporter.getPointer());
         blockSizeManager.timeAfterUser();

         if (resultsAvailable) {
            resultReporter.reportResults(beginIndex);
         }

         // Report progress if progress reporting enabled.
         if (progressReportingEnabled) {
            completed.fetchAndAddAcquire(finalBlockSize);
            this->setProgressValue(this->completed.load());
         }

         if (this->shouldThrottleThread()) {
            return ThrottleThread;
         }
      }

      return ThreadFinished;
   }

   ThreadFunctionResult whileThreadFunction() {
      int expected = 0;

      if (iteratorThreads.compareExchange(expected, 1, std::memory_order_acquire) == false) {
         return ThreadFinished;
      }

      ResultReporter<T> resultReporter(this);
      resultReporter.reserveSpace(1);

      while (current != end) {
         // The following two lines breaks support for input iterators according to
         // the sgi docs: dereferencing prev after calling ++current is not allowed
         // on input iterators. (prev is dereferenced inside user.runIteration())
         Iterator prev = current;
         ++current;

         int index = currentIndex.fetchAndAddRelaxed(1);

         expected = 1;
         iteratorThreads.compareExchange(expected, 0, std::memory_order_release);

         this->waitForResume(); // (only waits if the qfuture is paused.)

         if (shouldStartThread()) {
            this->startThread();
         }

         const bool resultAavailable = this->runIteration(prev, index, resultReporter.getPointer());

         if (resultAavailable) {
            resultReporter.reportResults(index);
         }

         if (this->shouldThrottleThread()) {
            return ThrottleThread;
         }

         expected = 0;

         if (iteratorThreads.compareExchange(expected, 1, std::memory_order_acquire) == false) {
            return ThreadFinished;
         }
      }

      return ThreadFinished;
   }

 public:
   const Iterator begin;
   const Iterator end;
   Iterator current;
   QAtomicInt currentIndex;
   bool forIteration;
   QAtomicInt iteratorThreads;
   int iterationCount;

   bool progressReportingEnabled;
   QAtomicInt completed;
};

} // namespace QtConcurrent

#endif
