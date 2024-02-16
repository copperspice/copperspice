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

#ifndef QTCONCURRENTFILTERKERNEL_H
#define QTCONCURRENTFILTERKERNEL_H

#include <qglobal.h>
#include <qtconcurrentiteratekernel.h>
#include <qtconcurrentmapkernel.h>
#include <qtconcurrentreducekernel.h>

namespace QtConcurrent {

template <typename T>
struct qValueType {
   using value_type = typename T::value_type;
};

template <typename T>
struct qValueType<const T *> {
   using value_type = T;
};

template <typename T>
struct qValueType<T *> {
   using value_type = T;
};

// Implementation of filter
template <typename Sequence, typename KeepFunctor, typename ReduceFunctor>
class FilterKernel : public IterateKernel<typename Sequence::const_iterator, void>
{
   using Reducer           = ReduceKernel<ReduceFunctor, Sequence, typename Sequence::value_type>;
   using IterateKernelType = IterateKernel<typename Sequence::const_iterator, void>;
   using T                 = typename ReduceFunctor::result_type;

   Sequence reducedResult;
   Sequence &sequence;
   KeepFunctor keep;
   ReduceFunctor reduce;
   Reducer reducer;

 public:
   FilterKernel(Sequence &_sequence, KeepFunctor _keep, ReduceFunctor _reduce)
      : IterateKernelType(const_cast<const Sequence &>(_sequence).begin(), const_cast<const Sequence &>(_sequence).end()),
        reducedResult(), sequence(_sequence), keep(_keep), reduce(_reduce), reducer(OrderedReduce)
   {
   }

   bool runIteration(typename Sequence::const_iterator it, int index, T *) {
      IntermediateResults<typename Sequence::value_type> results;
      results.begin = index;
      results.end = index + 1;

      if (keep(*it)) {
         results.vector.append(*it);
      }

      reducer.runReduce(reduce, reducedResult, results);
      return false;
   }

   bool runIterations(typename Sequence::const_iterator sequenceBeginIterator, int begin, int end, T *) {
      IntermediateResults<typename Sequence::value_type> results;
      results.begin = begin;
      results.end = end;
      results.vector.reserve(end - begin);

      typename Sequence::const_iterator it = sequenceBeginIterator;
      advance(it, begin);

      for (int i = begin; i < end; ++i) {
         if (keep(*it)) {
            results.vector.append(*it);
         }

         advance(it, 1);
      }

      reducer.runReduce(reduce, reducedResult, results);
      return false;
   }

   void finish() {
      reducer.finish(reduce, reducedResult);
      sequence = reducedResult;
   }

   bool shouldThrottleThread() {
      return IterateKernelType::shouldThrottleThread() || reducer.shouldThrottle();
   }

   bool shouldStartThread() {
      return IterateKernelType::shouldStartThread() && reducer.shouldStartThread();
   }

   using ReturnType = void;
   using ResultType = void;
};

// Implementation of filter-reduce
template <typename ReducedResultType, typename Iterator, typename KeepFunctor, typename ReduceFunctor,
      typename Reducer = ReduceKernel<ReduceFunctor, ReducedResultType, typename qValueType<Iterator>::value_type>>
class FilteredReducedKernel : public IterateKernel<Iterator, ReducedResultType>
{
   ReducedResultType reducedResult;
   KeepFunctor keep;
   ReduceFunctor reduce;
   Reducer reducer;

   using IterateKernelType = IterateKernel<Iterator, ReducedResultType>;

 public:
   FilteredReducedKernel(Iterator begin, Iterator end, KeepFunctor _keep, ReduceFunctor _reduce,
         ReduceOptions reduceOption)
      : IterateKernelType(begin, end), reducedResult(), keep(_keep), reduce(_reduce), reducer(reduceOption) {
   }

   bool runIteration(Iterator it, int index, ReducedResultType *) {
      IntermediateResults<typename qValueType<Iterator>::value_type> results;
      results.begin = index;
      results.end   = index + 1;

      if (keep(*it)) {
         results.vector.append(*it);
      }

      reducer.runReduce(reduce, reducedResult, results);
      return false;
   }

   bool runIterations(Iterator sequenceBeginIterator, int begin, int end, ReducedResultType *) {
      IntermediateResults<typename qValueType<Iterator>::value_type> results;
      results.begin = begin;
      results.end   = end;
      results.vector.reserve(end - begin);

      Iterator it = sequenceBeginIterator;
      advance(it, begin);

      for (int i = begin; i < end; ++i) {
         if (keep(*it)) {
            results.vector.append(*it);
         }

         advance(it, 1);
      }

      reducer.runReduce(reduce, reducedResult, results);
      return false;
   }

   void finish() {
      reducer.finish(reduce, reducedResult);
   }

   bool shouldThrottleThread() {
      return IterateKernelType::shouldThrottleThread() || reducer.shouldThrottle();
   }

   bool shouldStartThread() {
      return IterateKernelType::shouldStartThread() && reducer.shouldStartThread();
   }

   using ReturnType = ReducedResultType;
   using ResultType = ReducedResultType;

   ReducedResultType *result() {
      return &reducedResult;
   }
};

// Implementation of filter that reports individual results via QFutureInterface
template <typename Iterator, typename KeepFunctor>
class FilteredEachKernel : public IterateKernel<Iterator, typename qValueType<Iterator>::value_type>
{
   using T = typename qValueType<Iterator>::value_type;
   using IterateKernelType = IterateKernel<Iterator, T>;

   KeepFunctor keep;

 public:
   using ReturnType  = T;
   using ResultType  = T;

   FilteredEachKernel(Iterator begin, Iterator end, KeepFunctor _keep)
      : IterateKernelType(begin, end), keep(_keep)
   {
   }

   void start() {
      if (this->futureInterface) {
         this->futureInterface->setFilterMode(true);
      }

      IterateKernelType::start();
   }

   bool runIteration(Iterator it, int index, T *) {
      if (keep(*it)) {
         this->reportResult(&(*it), index);
      } else {
         this->reportResult(0, index);
      }

      return false;
   }

   bool runIterations(Iterator sequenceBeginIterator, int begin, int end, T *) {
      const int count = end - begin;
      IntermediateResults<typename qValueType<Iterator>::value_type> results;
      results.begin = begin;
      results.end = end;
      results.vector.reserve(count);

      Iterator it = sequenceBeginIterator;
      advance(it, begin);

      for (int i = begin; i < end; ++i) {
         if (keep(*it)) {
            results.vector.append(*it);
         }

         advance(it, 1);
      }

      this->reportResults(results.vector, begin, count);
      return false;
   }
};

template <typename Iterator, typename KeepFunctor>
inline ThreadEngineStarter<typename qValueType<Iterator>::value_type> startFiltered(Iterator begin, Iterator end,
      KeepFunctor functor)
{
   return startThreadEngine(new FilteredEachKernel<Iterator, KeepFunctor>(begin, end, functor));
}

template <typename Sequence, typename KeepFunctor>
inline ThreadEngineStarter<typename Sequence::value_type> startFiltered(const Sequence &sequence, KeepFunctor functor)
{
   using SequenceHolderType =
         SequenceHolder1<Sequence, FilteredEachKernel<typename Sequence::const_iterator, KeepFunctor>, KeepFunctor>;

   return startThreadEngine(new SequenceHolderType(sequence, functor));
}

template <typename ResultType, typename Sequence, typename MapFunctor, typename ReduceFunctor>
inline ThreadEngineStarter<ResultType> startFilteredReduced(const Sequence &sequence, MapFunctor mapFunctor,
      ReduceFunctor reduceFunctor, ReduceOptions options)
{
   using Iterator           = typename Sequence::const_iterator;
   using Reducer            = ReduceKernel<ReduceFunctor, ResultType, typename qValueType<Iterator>::value_type>;
   using FilteredReduceType = FilteredReducedKernel<ResultType, Iterator, MapFunctor, ReduceFunctor, Reducer>;
   using SequenceHolderType = SequenceHolder2<Sequence, FilteredReduceType, MapFunctor, ReduceFunctor>;

   return startThreadEngine(new SequenceHolderType(sequence, mapFunctor, reduceFunctor, options));
}

template <typename ResultType, typename Iterator, typename MapFunctor, typename ReduceFunctor>
inline ThreadEngineStarter<ResultType> startFilteredReduced(Iterator begin, Iterator end, MapFunctor mapFunctor,
      ReduceFunctor reduceFunctor, ReduceOptions options)
{
   using Reducer            = ReduceKernel<ReduceFunctor, ResultType, typename qValueType<Iterator>::value_type>;
   using FilteredReduceType = FilteredReducedKernel<ResultType, Iterator, MapFunctor, ReduceFunctor, Reducer>;

   return startThreadEngine(new FilteredReduceType(begin, end, mapFunctor, reduceFunctor, options));
}

}   // end namespace

#endif

