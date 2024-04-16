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

#ifndef QTCONCURRENTREDUCEKERNEL_H
#define QTCONCURRENTREDUCEKERNEL_H

#include <qatomic.h>
#include <qglobal.h>
#include <qlist.h>
#include <qmap.h>
#include <qmutex.h>
#include <qthread.h>
#include <qthreadpool.h>
#include <qvector.h>

namespace QtConcurrent {

/*
    The ReduceQueueStartLimit and ReduceQueueThrottleLimit constants
    limit the reduce queue size for MapReduce. When the number of
    reduce blocks in the queue exceeds ReduceQueueStartLimit,
    MapReduce won't start any new threads, and when it exceeds
    ReduceQueueThrottleLimit running threads will be stopped.
*/

static constexpr const int ReduceQueueStartLimit    = 20;
static constexpr const int ReduceQueueThrottleLimit = 30;

// IntermediateResults holds a block of intermediate results from a
// map or filter functor. The begin/end offsets indicates the origin
// and range of the block.

template <typename T>
class IntermediateResults
{
 public:
   int begin, end;
   QVector<T> vector;
};

enum ReduceOption {
   UnorderedReduce   = 0x1,
   OrderedReduce     = 0x2,
   SequentialReduce  = 0x4,
   // ParallelReduce = 0x8
};

using ReduceOptions = QFlags<ReduceOption>;
Q_DECLARE_OPERATORS_FOR_FLAGS(ReduceOptions)

// supports both ordered and out-of-order reduction
template <typename ReduceFunctor, typename ReduceResultType, typename T>
class ReduceKernel
{
   using ResultsMap = QMap<int, IntermediateResults<T>>;

   const ReduceOptions reduceOptions;

   QMutex mutex;
   int progress, resultsMapSize, threadCount;
   ResultsMap resultsMap;

   bool canReduce(int begin) const {
      return (((reduceOptions & UnorderedReduce) && progress == 0) || ((reduceOptions & OrderedReduce) && progress == begin));
   }

   void reduceResult(ReduceFunctor &reduce, ReduceResultType &r, const IntermediateResults<T> &result) {
      for (int i = 0; i < result.vector.size(); ++i) {
         reduce(r, result.vector.at(i));
      }
   }

   void reduceResults(ReduceFunctor &reduce, ReduceResultType &r, ResultsMap &map) {
      typename ResultsMap::iterator it = map.begin();

      while (it != map.end()) {
         reduceResult(reduce, r, it.value());
         ++it;
      }
   }

 public:
   ReduceKernel(ReduceOptions _reduceOptions)
      : reduceOptions(_reduceOptions), progress(0), resultsMapSize(0),
        threadCount(QThreadPool::globalInstance()->maxThreadCount()) {
   }

   void runReduce(ReduceFunctor &reduce, ReduceResultType &r, const IntermediateResults<T> &result) {
      QMutexLocker locker(&mutex);

      if (!canReduce(result.begin)) {
         ++resultsMapSize;
         resultsMap.insert(result.begin, result);
         return;
      }

      if (reduceOptions & UnorderedReduce) {
         // UnorderedReduce
         progress = -1;

         // reduce this result
         locker.unlock();
         reduceResult(reduce, r, result);
         locker.relock();

         // reduce all stored results as well
         while (!resultsMap.isEmpty()) {
            ResultsMap resultsMapCopy = resultsMap;
            resultsMap.clear();

            locker.unlock();
            reduceResults(reduce, r, resultsMapCopy);
            locker.relock();

            resultsMapSize -= resultsMapCopy.size();
         }

         progress = 0;

      } else {
         // reduce this result
         locker.unlock();
         reduceResult(reduce, r, result);
         locker.relock();

         // OrderedReduce
         progress += result.end - result.begin;

         // reduce as many other results as possible
         typename ResultsMap::iterator it = resultsMap.begin();

         while (it != resultsMap.end()) {
            if (it.value().begin != progress) {
               break;
            }

            locker.unlock();
            reduceResult(reduce, r, it.value());
            locker.relock();

            --resultsMapSize;
            progress += it.value().end - it.value().begin;
            it = resultsMap.erase(it);
         }
      }
   }

   // final reduction
   void finish(ReduceFunctor &reduce, ReduceResultType &r) {
      reduceResults(reduce, r, resultsMap);
   }

   bool shouldThrottle() {
      return (resultsMapSize > (ReduceQueueThrottleLimit * threadCount));
   }

   bool shouldStartThread() {
      return (resultsMapSize <= (ReduceQueueStartLimit * threadCount));
   }
};

template <typename Sequence, typename Base, typename Functor1, typename Functor2>
struct SequenceHolder2 : public Base {
   SequenceHolder2(const Sequence &_sequence, Functor1 functor1, Functor2 functor2, ReduceOptions reduceOptions)
      : Base(_sequence.begin(), _sequence.end(), functor1, functor2, reduceOptions), sequence(_sequence)
   {
   }

   Sequence sequence;

   void finish() {
      Base::finish();
      // Clear the sequence to make sure all temporaries are destroyed
      // before finished is signaled.
      sequence = Sequence();
   }
};

} // namespace QtConcurrent

#endif
