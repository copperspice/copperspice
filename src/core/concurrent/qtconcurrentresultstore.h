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

#ifndef QTCONCURRENTRESULTSTORE_H
#define QTCONCURRENTRESULTSTORE_H

#include <qdebug.h>
#include <qglobal.h>
#include <qmap.h>

/*
    ResultStore stores indexed results. Results can be added and retrieved
    either individually batched in a QVector. Retriveing results and checking
    which indexes are in the store can be done either by iterating or by random
    accees. In addition results kan be removed from the front of the store,
    either individually or in batches.
*/

namespace QtConcurrent {

class ResultItem
{
 public:
   ResultItem(const void *_result, int _count)
      : m_count(_count), result(_result)
   { }

   ResultItem(const void *_result)
      : m_count(0), result(_result)
   { }

   ResultItem()
      : m_count(0), result(nullptr)
   { }

   bool isValid() const {
      return result != nullptr;
   }

   bool isVector() const {
      return m_count != 0;
   }

   int count() const {
      return (m_count == 0) ?  1 : m_count;
   }

   int m_count;           // result is either a pointer to a result or to a vector of results,
   const void *result;    // if count is 0 it's a result, otherwise it's a vector.
};

class Q_CORE_EXPORT ResultIteratorBase
{
 public:
   ResultIteratorBase();
   ResultIteratorBase(QMap<int, ResultItem>::const_iterator _mapIterator, int _vectorIndex = 0);
   int vectorIndex() const;
   int resultIndex() const;

   ResultIteratorBase operator++();
   int batchSize() const;
   void batchedAdvance();
   bool operator==(const ResultIteratorBase &other) const;
   bool operator!=(const ResultIteratorBase &other) const;
   bool isVector() const;
   bool canIncrementVectorIndex() const;

 protected:
   QMap<int, ResultItem>::const_iterator mapIterator;
   int m_vectorIndex;
};

template <typename T>
class  ResultIterator : public ResultIteratorBase
{
 public:
   ResultIterator(const ResultIteratorBase &base)
      : ResultIteratorBase(base) { }

   const T &value() const {
      return *pointer();
   }

   const T *pointer() const {
      if (mapIterator.value().isVector()) {
         return &(reinterpret_cast<const QVector<T> *>(mapIterator.value().result)->at(m_vectorIndex));
      } else {
         return reinterpret_cast<const T *>(mapIterator.value().result);
      }
   }
};

class Q_CORE_EXPORT ResultStoreBase
{
 public:
   ResultStoreBase();
   void setFilterMode(bool enable);
   bool filterMode() const;
   int addResult(int index, const void *result);
   int addResults(int index, const void *results, int vectorSize, int logicalCount);
   ResultIteratorBase begin() const;
   ResultIteratorBase end() const;
   bool hasNextResult() const;
   ResultIteratorBase resultAt(int index) const;
   bool contains(int index) const;
   int count() const;
   virtual ~ResultStoreBase() { };

 protected:
   int insertResultItem(int index, ResultItem &resultItem);
   void insertResultItemIfValid(int index, ResultItem &resultItem);
   void syncPendingResults();
   void syncResultCount();
   int updateInsertIndex(int index, int _count);

   QMap<int, ResultItem> m_results;
   int insertIndex;     // The index where the next results(s) will be inserted.
   int resultCount;     // The number of consecutive results stored, starting at index 0.

   bool m_filterMode;
   QMap<int, ResultItem> pendingResults;
   int filteredResults;

};

template <typename T>
class ResultStore : public ResultStoreBase
{
 public:
   ResultStore() { }

   ResultStore(const ResultStoreBase &base)
      : ResultStoreBase(base) { }

   int addResult(int index, const T  *result) {
      if (result == 0) {
         return ResultStoreBase::addResult(index, result);
      } else {
         return ResultStoreBase::addResult(index, new T(*result));
      }
   }

   int addResults(int index, const QVector<T> *results) {
      return ResultStoreBase::addResults(index, new QVector<T>(*results), results->count(), results->count());
   }

   int addResults(int index, const QVector<T> *results, int totalCount) {
      if (m_filterMode && totalCount && !results->count()) {
         return ResultStoreBase::addResults(index, 0, 0, totalCount);
      } else {
         return ResultStoreBase::addResults(index, new QVector<T>(*results), results->count(), totalCount);
      }
   }

   int addCanceledResult(int index) {
      return addResult(index, 0);
   }

   int addCanceledResults(int index, int _count) {
      QVector<T> empty;
      return addResults(index, &empty, _count);
   }

   ResultIterator<T> begin() const {
      return static_cast<ResultIterator<T>>(ResultStoreBase::begin());
   }

   ResultIterator<T> end() const {
      return static_cast<ResultIterator<T>>(ResultStoreBase::end());
   }

   ResultIterator<T> resultAt(int index) const {
      return static_cast<ResultIterator<T>>(ResultStoreBase::resultAt(index));
   }

   void clear() {
      QMap<int, ResultItem>::const_iterator mapIterator = m_results.constBegin();

      while (mapIterator != m_results.constEnd()) {
         if (mapIterator.value().isVector()) {
            delete reinterpret_cast<const QVector<T> *>(mapIterator.value().result);
         } else {
            delete reinterpret_cast<const T *>(mapIterator.value().result);
         }

         ++mapIterator;
      }

      resultCount = 0;
      m_results.clear();
   }

   ~ResultStore() {
      clear();
   }

};

} // namespace QtConcurrent

#endif
