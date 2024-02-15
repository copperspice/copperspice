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

#ifndef QTCONCURRENTFILTER_H
#define QTCONCURRENTFILTER_H

#include <qglobal.h>
#include <qtconcurrentfilterkernel.h>
#include <qtconcurrentfunctionwrappers.h>

namespace QtConcurrent {

template <typename Sequence, typename KeepFunctor, typename ReduceFunctor>
ThreadEngineStarter<void> filterInternal(Sequence &sequence, KeepFunctor keep, ReduceFunctor reduce)
{
   using KernelType = FilterKernel<Sequence, KeepFunctor, ReduceFunctor>;
   return startThreadEngine(new KernelType(sequence, keep, reduce));
}

// filter() on sequences
template <typename Sequence, typename KeepFunctor>
QFuture<void> filter(Sequence &sequence, KeepFunctor keep)
{
   return filterInternal(sequence, QtPrivate::createFunctionWrapper(keep), QtPrivate::PushBackWrapper());
}

// filteredReduced() on sequences
template <typename ResultType, typename Sequence, typename KeepFunctor, typename ReduceFunctor>
QFuture<ResultType> filteredReduced(const Sequence &sequence, KeepFunctor keep, ReduceFunctor reduce,
      ReduceOptions reduceOptions = ReduceOptions(UnorderedReduce | SequentialReduce))
{
   return startFilteredReduced<ResultType>(sequence, QtPrivate::createFunctionWrapper(keep),
      QtPrivate::createFunctionWrapper(reduce), reduceOptions);
}

template <typename Sequence, typename KeepFunctor, typename ReduceFunctor>
QFuture<typename QtPrivate::ReduceResultType<ReduceFunctor>::ResultType> filteredReduced(const Sequence &sequence,
      KeepFunctor keep, ReduceFunctor reduce, ReduceOptions options = ReduceOptions(UnorderedReduce | SequentialReduce))
{
   return startFilteredReduced<typename QtPrivate::ReduceResultType<ReduceFunctor>::ResultType>
         (sequence, QtPrivate::createFunctionWrapper(keep), QtPrivate::createFunctionWrapper(reduce), options);
}

// filteredReduced() on iterators
template <typename ResultType, typename Iterator, typename KeepFunctor, typename ReduceFunctor>
QFuture<ResultType> filteredReduced(Iterator begin, Iterator end, KeepFunctor keep, ReduceFunctor reduce,
      ReduceOptions reduceOptions = ReduceOptions(UnorderedReduce | SequentialReduce))
{
   return startFilteredReduced<ResultType>(begin, end, QtPrivate::createFunctionWrapper(keep),
         QtPrivate::createFunctionWrapper(reduce), reduceOptions);
}

template <typename Iterator, typename KeepFunctor, typename ReduceFunctor>
QFuture<typename QtPrivate::ReduceResultType<ReduceFunctor>::ResultType> filteredReduced(Iterator begin,
      Iterator end, KeepFunctor keep, ReduceFunctor reduce,
      ReduceOptions options = ReduceOptions(UnorderedReduce | SequentialReduce))
{
   return startFilteredReduced<typename QtPrivate::ReduceResultType<ReduceFunctor>::ResultType>
         (begin, end, QtPrivate::createFunctionWrapper(keep), QtPrivate::createFunctionWrapper(reduce), options);
}

// filtered() on sequences
template <typename Sequence, typename KeepFunctor>
QFuture<typename Sequence::value_type> filtered(const Sequence &sequence, KeepFunctor keep)
{
   return startFiltered(sequence, QtPrivate::createFunctionWrapper(keep));
}

// filtered() on iterators
template <typename Iterator, typename KeepFunctor>
QFuture<typename qValueType<Iterator>::value_type> filtered(Iterator begin, Iterator end, KeepFunctor keep)
{
   return startFiltered(begin, end, QtPrivate::createFunctionWrapper(keep));
}

// blocking filter() on sequences
template <typename Sequence, typename KeepFunctor>
void blockingFilter(Sequence &sequence, KeepFunctor keep)
{
   filterInternal(sequence, QtPrivate::createFunctionWrapper(keep), QtPrivate::PushBackWrapper()).startBlocking();
}

// blocking filteredReduced() on sequences
template <typename ResultType, typename Sequence, typename KeepFunctor, typename ReduceFunctor>
ResultType blockingFilteredReduced(const Sequence &sequence, KeepFunctor keep, ReduceFunctor reduce,
      ReduceOptions reduceOptions = ReduceOptions(UnorderedReduce | SequentialReduce))
{
   return startFilteredReduced<ResultType>(sequence, QtPrivate::createFunctionWrapper(keep),
         QtPrivate::createFunctionWrapper(reduce), reduceOptions).startBlocking();
}

template <typename Sequence, typename KeepFunctor, typename ReduceFunctor>
typename QtPrivate::ReduceResultType<ReduceFunctor>::ResultType blockingFilteredReduced(const Sequence &sequence,
      KeepFunctor keep, ReduceFunctor reduce, ReduceOptions options = ReduceOptions(UnorderedReduce | SequentialReduce))
{
   return blockingFilteredReduced<typename QtPrivate::ReduceResultType<ReduceFunctor>::ResultType>
         (sequence, QtPrivate::createFunctionWrapper(keep), QtPrivate::createFunctionWrapper(reduce), options);
}

// blocking filteredReduced() on iterators
template <typename ResultType, typename Iterator, typename KeepFunctor, typename ReduceFunctor>
ResultType blockingFilteredReduced(Iterator begin, Iterator end, KeepFunctor keep, ReduceFunctor reduce,
      ReduceOptions reduceOptions = ReduceOptions(UnorderedReduce | SequentialReduce))
{
   return startFilteredReduced<ResultType> (begin, end, QtPrivate::createFunctionWrapper(keep),
         QtPrivate::createFunctionWrapper(reduce), reduceOptions).startBlocking();
}

template <typename Iterator, typename KeepFunctor, typename ReduceFunctor>
typename QtPrivate::ReduceResultType<ReduceFunctor>::ResultType blockingFilteredReduced(Iterator begin, Iterator end,
      KeepFunctor keep, ReduceFunctor reduce, ReduceOptions options = ReduceOptions(UnorderedReduce | SequentialReduce))
{
   return startFilteredReduced<typename QtPrivate::ReduceResultType<ReduceFunctor>::ResultType>
         (begin, end, QtPrivate::createFunctionWrapper(keep), QtPrivate::createFunctionWrapper(reduce), options).startBlocking();
}

// blocking filtered() on sequences
template <typename Sequence, typename KeepFunctor>
Sequence blockingFiltered(const Sequence &sequence, KeepFunctor keep)
{
   return startFilteredReduced<Sequence>(sequence, QtPrivate::createFunctionWrapper(keep), QtPrivate::PushBackWrapper(),
         OrderedReduce).startBlocking();
}

// blocking filtered() on iterators
template <typename OutputSequence, typename Iterator, typename KeepFunctor>
OutputSequence blockingFiltered(Iterator begin, Iterator end, KeepFunctor keep)
{
   return startFilteredReduced<OutputSequence>(begin, end, QtPrivate::createFunctionWrapper(keep),
         QtPrivate::PushBackWrapper(), OrderedReduce).startBlocking();
}

} // namespace QtConcurrent

#endif
