/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QTCONCURRENTMAPKERNEL_H
#define QTCONCURRENTMAPKERNEL_H

#include <QtCore/qglobal.h>

#include <QtCore/qtconcurrentiteratekernel.h>
#include <QtCore/qtconcurrentreducekernel.h>

QT_BEGIN_NAMESPACE

namespace QtConcurrent {

// map kernel, works with both parallel-for and parallel-while
template <typename Iterator, typename MapFunctor>
class MapKernel : public IterateKernel<Iterator, void>
{
   MapFunctor map;

 public:
   typedef void ReturnType;
   MapKernel(Iterator begin, Iterator end, MapFunctor _map)
      : IterateKernel<Iterator, void>(begin, end), map(_map) {
   }

   bool runIteration(Iterator it, int, void *) {
      map(*it);
      return false;
   }

   bool runIterations(Iterator sequenceBeginIterator, int beginIndex, int endIndex, void *) {
      Iterator it = sequenceBeginIterator;
      advance(it, beginIndex);
      for (int i = beginIndex; i < endIndex; ++i) {
         runIteration(it, i, 0);
         advance(it, 1);
      }

      return false;
   }
};

template <typename ReducedResultType, typename Iterator, typename MapFunctor, typename ReduceFunctor,
          typename Reducer = ReduceKernel<ReduceFunctor, ReducedResultType, typename MapFunctor::result_type> >

class MappedReducedKernel : public IterateKernel<Iterator, ReducedResultType>
{
   ReducedResultType reducedResult;
   MapFunctor map;
   ReduceFunctor reduce;
   Reducer reducer;

 public:
   typedef ReducedResultType ReturnType;
   MappedReducedKernel(Iterator begin, Iterator end, MapFunctor _map, ReduceFunctor _reduce, ReduceOptions reduceOptions)
      : IterateKernel<Iterator, ReducedResultType>(begin, end), reducedResult(), map(_map), reduce(_reduce),
        reducer(reduceOptions) {
   }

   MappedReducedKernel(ReducedResultType initialValue, MapFunctor _map, ReduceFunctor _reduce)
      : reducedResult(initialValue), map(_map), reduce(_reduce) {
   }

   bool runIteration(Iterator it, int index, ReducedResultType *) {
      IntermediateResults<typename MapFunctor::result_type> results;
      results.begin = index;
      results.end = index + 1;

      results.vector.append(map(*it));
      reducer.runReduce(reduce, reducedResult, results);
      return false;
   }

   bool runIterations(Iterator sequenceBeginIterator, int begin, int end, ReducedResultType *) {
      IntermediateResults<typename MapFunctor::result_type> results;
      results.begin = begin;
      results.end = end;
      results.vector.reserve(end - begin);

      Iterator it = sequenceBeginIterator;
      advance(it, begin);
      for (int i = begin; i < end; ++i) {
         results.vector.append(map(*(it)));
         advance(it, 1);
      }

      reducer.runReduce(reduce, reducedResult, results);
      return false;
   }

   void finish() {
      reducer.finish(reduce, reducedResult);
   }

   bool shouldThrottleThread() {
      return IterateKernel<Iterator, ReducedResultType>::shouldThrottleThread() || reducer.shouldThrottle();
   }

   bool shouldStartThread() {
      return IterateKernel<Iterator, ReducedResultType>::shouldStartThread() && reducer.shouldStartThread();
   }

   typedef ReducedResultType ResultType;
   ReducedResultType *result() {
      return &reducedResult;
   }
};

template <typename Iterator, typename MapFunctor>
class MappedEachKernel : public IterateKernel<Iterator, typename MapFunctor::result_type>
{
   MapFunctor map;
   typedef typename MapFunctor::result_type T;

 public:
   typedef T ReturnType;
   typedef T ResultType;

   MappedEachKernel(Iterator begin, Iterator end, MapFunctor _map)
      : IterateKernel<Iterator, T>(begin, end), map(_map) { }

   bool runIteration(Iterator it, int,  T *result) {
      *result = map(*it);
      return true;
   }

   bool runIterations(Iterator sequenceBeginIterator, int begin, int end, T *results) {

      Iterator it = sequenceBeginIterator;
      advance(it, begin);
      for (int i = begin; i < end; ++i) {
         runIteration(it, i, results + (i - begin));
         advance(it, 1);
      }

      return true;
   }
};

template <typename Iterator, typename Functor>
inline ThreadEngineStarter<void> startMap(Iterator begin, Iterator end, Functor functor)
{
   return startThreadEngine(new MapKernel<Iterator, Functor>(begin, end, functor));
}

template <typename T, typename Iterator, typename Functor>
inline ThreadEngineStarter<T> startMapped(Iterator begin, Iterator end, Functor functor)
{
   return startThreadEngine(new MappedEachKernel<Iterator, Functor>(begin, end, functor));
}

/*
    The SequnceHolder class is used to hold a reference to the
    sequence we are working on.
*/
template <typename Sequence, typename Base, typename Functor>
struct SequenceHolder1 : public Base {
   SequenceHolder1(const Sequence &_sequence, Functor functor)
      : Base(_sequence.begin(), _sequence.end(), functor), sequence(_sequence) {
   }

   Sequence sequence;

   void finish() {
      Base::finish();
      // Clear the sequence to make sure all temporaries are destroyed
      // before finished is signaled.
      sequence = Sequence();
   }
};

template <typename T, typename Sequence, typename Functor>
inline ThreadEngineStarter<T> startMapped(const Sequence &sequence, Functor functor)
{
   typedef SequenceHolder1<Sequence,
           MappedEachKernel<typename Sequence::const_iterator , Functor>, Functor>
           SequenceHolderType;

   return startThreadEngine(new SequenceHolderType(sequence, functor));
}

template <typename IntermediateType, typename ResultType, typename Sequence, typename MapFunctor, typename ReduceFunctor>
inline ThreadEngineStarter<ResultType> startMappedReduced(const Sequence &sequence,
      MapFunctor mapFunctor, ReduceFunctor reduceFunctor,
      ReduceOptions options)
{
   typedef typename Sequence::const_iterator Iterator;
   typedef ReduceKernel<ReduceFunctor, ResultType, IntermediateType> Reducer;
   typedef MappedReducedKernel<ResultType, Iterator, MapFunctor, ReduceFunctor, Reducer> MappedReduceType;
   typedef SequenceHolder2<Sequence, MappedReduceType, MapFunctor, ReduceFunctor> SequenceHolderType;
   return startThreadEngine(new SequenceHolderType(sequence, mapFunctor, reduceFunctor, options));
}

template <typename IntermediateType, typename ResultType, typename Iterator, typename MapFunctor, typename ReduceFunctor>
inline ThreadEngineStarter<ResultType> startMappedReduced(Iterator begin, Iterator end,
      MapFunctor mapFunctor, ReduceFunctor reduceFunctor,
      ReduceOptions options)
{
   typedef ReduceKernel<ReduceFunctor, ResultType, IntermediateType> Reducer;
   typedef MappedReducedKernel<ResultType, Iterator, MapFunctor, ReduceFunctor, Reducer> MappedReduceType;
   return startThreadEngine(new MappedReduceType(begin, end, mapFunctor, reduceFunctor, options));
}

} // namespace QtConcurrent

QT_END_NAMESPACE

#endif
