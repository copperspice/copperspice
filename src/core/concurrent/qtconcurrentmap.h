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

#ifndef QTCONCURRENTMAP_H
#define QTCONCURRENTMAP_H

#include <qglobal.h>
#include <qstringlist.h>
#include <qtconcurrentfunctionwrappers.h>
#include <qtconcurrentmapkernel.h>
#include <qtconcurrentreducekernel.h>

namespace QtConcurrent {

// map() on sequences
template <typename Sequence, typename MapFunctor>
QFuture<void> map(Sequence &sequence, MapFunctor map)
{
   return startMap(sequence.begin(), sequence.end(), QtPrivate::createFunctionWrapper(map));
}

// map() on iterators
template <typename Iterator, typename MapFunctor>
QFuture<void> map(Iterator begin, Iterator end, MapFunctor map)
{
   return startMap(begin, end, QtPrivate::createFunctionWrapper(map));
}

// mappedReduced() for sequences.
template <typename ResultType, typename Sequence, typename MapFunctor, typename ReduceFunctor>
QFuture<ResultType> mappedReduced(const Sequence &sequence, MapFunctor map, ReduceFunctor reduce,
      ReduceOptions reduceOptions = ReduceOptions(UnorderedReduce | SequentialReduce))
{
   return startMappedReduced<typename QtPrivate::MapResultType<void, MapFunctor>::ResultType, ResultType>
         (sequence, QtPrivate::createFunctionWrapper(map), QtPrivate::createFunctionWrapper(reduce), reduceOptions);
}

template <typename Sequence, typename MapFunctor, typename ReduceFunctor>
QFuture<typename QtPrivate::ReduceResultType<ReduceFunctor>::ResultType> mappedReduced(const Sequence &sequence,
      MapFunctor map, ReduceFunctor reduce, ReduceOptions options = ReduceOptions(UnorderedReduce | SequentialReduce))
{
   return startMappedReduced<typename QtPrivate::MapResultType<void, MapFunctor>::ResultType, typename QtPrivate::ReduceResultType<ReduceFunctor>::ResultType>
         (sequence, QtPrivate::createFunctionWrapper(map), QtPrivate::createFunctionWrapper(reduce), options);
}

// mappedReduced() for iterators
template <typename ResultType, typename Iterator, typename MapFunctor, typename ReduceFunctor>
QFuture<ResultType> mappedReduced(Iterator begin, Iterator end, MapFunctor map, ReduceFunctor reduce,
      ReduceOptions reduceOptions = ReduceOptions(UnorderedReduce | SequentialReduce))
{
   return startMappedReduced<typename QtPrivate::MapResultType<void, MapFunctor>::ResultType, ResultType>
         (begin, end, QtPrivate::createFunctionWrapper(map), QtPrivate::createFunctionWrapper(reduce), reduceOptions);
}

template <typename Iterator, typename MapFunctor, typename ReduceFunctor>
QFuture<typename QtPrivate::ReduceResultType<ReduceFunctor>::ResultType> mappedReduced(Iterator begin, Iterator end,
      MapFunctor map, ReduceFunctor reduce, ReduceOptions options = ReduceOptions(UnorderedReduce | SequentialReduce))
{
   return startMappedReduced<typename QtPrivate::MapResultType<void, MapFunctor>::ResultType,
         typename QtPrivate::ReduceResultType<ReduceFunctor>::ResultType> (begin, end, QtPrivate::createFunctionWrapper(map),
         QtPrivate::createFunctionWrapper(reduce), options);
}

// mapped() for sequences
template <typename Sequence, typename MapFunctor>
QFuture<typename QtPrivate::MapResultType<void, MapFunctor>::ResultType> mapped(const Sequence &sequence, MapFunctor map)
{
   return startMapped<typename QtPrivate::MapResultType<void, MapFunctor>::ResultType>(sequence,
         QtPrivate::createFunctionWrapper(map));
}

// mapped() for iterator ranges.
template <typename Iterator, typename MapFunctor>
QFuture<typename QtPrivate::MapResultType<void, MapFunctor>::ResultType> mapped(Iterator begin, Iterator end, MapFunctor map)
{
   return startMapped<typename QtPrivate::MapResultType<void, MapFunctor>::ResultType>(begin, end,
         QtPrivate::createFunctionWrapper(map));
}

// blockingMap() for sequences
template <typename Sequence, typename MapFunctor>
void blockingMap(Sequence &sequence, MapFunctor map)
{
   startMap(sequence.begin(), sequence.end(), QtPrivate::createFunctionWrapper(map)).startBlocking();
}

// blockingMap() for iterator ranges
template <typename Iterator, typename MapFunctor>
void blockingMap(Iterator begin, Iterator end, MapFunctor map)
{
   startMap(begin, end, QtPrivate::createFunctionWrapper(map)).startBlocking();
}

// blockingMappedReduced() for sequences
template <typename ResultType, typename Sequence, typename MapFunctor, typename ReduceFunctor>
ResultType blockingMappedReduced(const Sequence &sequence, MapFunctor map, ReduceFunctor reduce,
      ReduceOptions reduceOptions = ReduceOptions(UnorderedReduce | SequentialReduce))
{
   return QtConcurrent::startMappedReduced<typename QtPrivate::MapResultType<void, MapFunctor>::ResultType, ResultType>
         (sequence, QtPrivate::createFunctionWrapper(map), QtPrivate::createFunctionWrapper(reduce), reduceOptions).startBlocking();
}

template <typename MapFunctor, typename ReduceFunctor, typename Sequence>
typename QtPrivate::ReduceResultType<ReduceFunctor>::ResultType blockingMappedReduced(const Sequence &sequence,
      MapFunctor map, ReduceFunctor reduce, ReduceOptions options = ReduceOptions(UnorderedReduce | SequentialReduce))
{
   return QtConcurrent::startMappedReduced<typename QtPrivate::MapResultType<void, MapFunctor>::ResultType,
         typename QtPrivate::ReduceResultType<ReduceFunctor>::ResultType> (sequence, QtPrivate::createFunctionWrapper(map),
         QtPrivate::createFunctionWrapper(reduce), options).startBlocking();
}

// blockingMappedReduced() for iterator ranges
template <typename ResultType, typename Iterator, typename MapFunctor, typename ReduceFunctor>
ResultType blockingMappedReduced(Iterator begin, Iterator end, MapFunctor map, ReduceFunctor reduce,
      QtConcurrent::ReduceOptions reduceOptions = QtConcurrent::ReduceOptions(QtConcurrent::UnorderedReduce |
            QtConcurrent::SequentialReduce))
{
   return QtConcurrent::startMappedReduced<typename QtPrivate::MapResultType<void, MapFunctor>::ResultType, ResultType>
         (begin, end, QtPrivate::createFunctionWrapper(map), QtPrivate::createFunctionWrapper(reduce), reduceOptions).startBlocking();
}

template <typename Iterator, typename MapFunctor, typename ReduceFunctor>
typename QtPrivate::ReduceResultType<ReduceFunctor>::ResultType blockingMappedReduced(Iterator begin,
      Iterator end, MapFunctor map, ReduceFunctor reduce,
      QtConcurrent::ReduceOptions options = QtConcurrent::ReduceOptions(QtConcurrent::UnorderedReduce |
            QtConcurrent::SequentialReduce))
{
   return QtConcurrent::startMappedReduced<typename QtPrivate::MapResultType<void, MapFunctor>::ResultType,
      typename QtPrivate::ReduceResultType<ReduceFunctor>::ResultType> (begin, end, QtPrivate::createFunctionWrapper(map),
      QtPrivate::createFunctionWrapper(reduce), options).startBlocking();
}

// mapped() for sequences with a different putput sequence type.
template <typename OutputSequence, typename InputSequence, typename MapFunctor>
OutputSequence blockingMapped(const InputSequence &sequence, MapFunctor map)
{
   return blockingMappedReduced<OutputSequence> (sequence, QtPrivate::createFunctionWrapper(map),
         QtPrivate::PushBackWrapper(), QtConcurrent::OrderedReduce);
}

template <typename MapFunctor, typename InputSequence>
typename QtPrivate::MapResultType<InputSequence, MapFunctor>::ResultType blockingMapped(const InputSequence &sequence,
      MapFunctor map)
{
   using OutputSequence = typename QtPrivate::MapResultType<InputSequence, MapFunctor>::ResultType;

   return blockingMappedReduced<OutputSequence> (sequence, QtPrivate::createFunctionWrapper(map),
          QtPrivate::PushBackWrapper(), QtConcurrent::OrderedReduce);
}

// mapped()  for iterator ranges
template <typename Sequence, typename Iterator, typename MapFunctor>
Sequence blockingMapped(Iterator begin, Iterator end, MapFunctor map)
{
   return blockingMappedReduced<Sequence> (begin, end, QtPrivate::createFunctionWrapper(map), QtPrivate::PushBackWrapper(),
         QtConcurrent::OrderedReduce);
}

template <typename Iterator, typename MapFunctor>
typename QtPrivate::MapResultType<Iterator, MapFunctor>::ResultType blockingMapped(Iterator begin, Iterator end,
      MapFunctor map)
{
   using OutputSequence = typename QtPrivate::MapResultType<Iterator, MapFunctor>::ResultType;

   return blockingMappedReduced<OutputSequence> (begin, end, QtPrivate::createFunctionWrapper(map),
         QtPrivate::PushBackWrapper(), QtConcurrent::OrderedReduce);
}

} // namespace QtConcurrent

#endif
