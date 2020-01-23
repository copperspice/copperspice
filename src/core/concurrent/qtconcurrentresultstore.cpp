/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qtconcurrentresultstore.h>

QT_BEGIN_NAMESPACE

namespace QtConcurrent {

ResultIteratorBase::ResultIteratorBase()
   : mapIterator(QMap<int, ResultItem>::const_iterator()), m_vectorIndex(0) { }
ResultIteratorBase::ResultIteratorBase(QMap<int, ResultItem>::const_iterator _mapIterator, int _vectorIndex)
   : mapIterator(_mapIterator), m_vectorIndex(_vectorIndex) { }

int ResultIteratorBase::vectorIndex() const
{
   return m_vectorIndex;
}
int ResultIteratorBase::resultIndex() const
{
   return mapIterator.key() + m_vectorIndex;
}

ResultIteratorBase ResultIteratorBase::operator++()
{
   if (canIncrementVectorIndex()) {
      ++m_vectorIndex;
   } else {
      ++mapIterator;
      m_vectorIndex = 0;
   }
   return *this;
}

int ResultIteratorBase::batchSize() const
{
   return mapIterator.value().count();
}

void ResultIteratorBase::batchedAdvance()
{
   ++mapIterator;
   m_vectorIndex = 0;
}

bool ResultIteratorBase::operator==(const ResultIteratorBase &other) const
{
   return (mapIterator == other.mapIterator && m_vectorIndex == other.m_vectorIndex);
}

bool ResultIteratorBase::operator!=(const ResultIteratorBase &other) const
{
   return !operator==(other);
}

bool ResultIteratorBase::isVector() const
{
   return mapIterator.value().isVector();
}

bool ResultIteratorBase::canIncrementVectorIndex() const
{
   return (m_vectorIndex + 1 < mapIterator.value().m_count);
}

ResultStoreBase::ResultStoreBase()
   : insertIndex(0), resultCount(0), m_filterMode(false), filteredResults(0) { }

void ResultStoreBase::setFilterMode(bool enable)
{
   m_filterMode = enable;
}

bool ResultStoreBase::filterMode() const
{
   return m_filterMode;
}

void ResultStoreBase::syncResultCount()
{
   ResultIteratorBase it = resultAt(resultCount);
   while (it != end()) {
      resultCount += it.batchSize();
      it = resultAt(resultCount);
   }
}

void ResultStoreBase::insertResultItemIfValid(int index, ResultItem &resultItem)
{
   if (resultItem.isValid()) {
      m_results[index] = resultItem;
      syncResultCount();
   } else {
      filteredResults += resultItem.count();
   }
}

int ResultStoreBase::insertResultItem(int index, ResultItem &resultItem)
{
   int storeIndex;
   if (m_filterMode && index != -1 && index > insertIndex) {
      pendingResults[index] = resultItem;
      storeIndex = index;
   } else {
      storeIndex = updateInsertIndex(index, resultItem.count());
      insertResultItemIfValid(storeIndex - filteredResults, resultItem);
   }
   syncPendingResults();
   return storeIndex;
}

void ResultStoreBase::syncPendingResults()
{
   // check if we can insert any of the pending results:
   QMap<int, ResultItem>::iterator it = pendingResults.begin();
   while (it != pendingResults.end()) {
      int index = it.key();
      if (index != resultCount + filteredResults) {
         break;
      }

      ResultItem result = it.value();
      insertResultItemIfValid(index - filteredResults, result);
      pendingResults.erase(it);
      it = pendingResults.begin();
   }
}

int ResultStoreBase::addResult(int index, const void *result)
{
   ResultItem resultItem(result, 0); // 0 means "not a vector"
   return insertResultItem(index, resultItem);
}

int ResultStoreBase::addResults(int index, const void *results, int vectorSize, int totalCount)
{
   if (m_filterMode == false || vectorSize == totalCount) {
      ResultItem resultItem(results, vectorSize);
      return insertResultItem(index, resultItem);
   } else {
      if (vectorSize > 0) {
         ResultItem filteredIn(results, vectorSize);
         insertResultItem(index, filteredIn);
      }
      ResultItem filteredAway(0, totalCount - vectorSize);
      return insertResultItem(index + vectorSize, filteredAway);
   }
}

ResultIteratorBase ResultStoreBase::begin() const
{
   return ResultIteratorBase(m_results.begin());
}

ResultIteratorBase ResultStoreBase::end() const
{
   return ResultIteratorBase(m_results.end());
}

bool ResultStoreBase::hasNextResult() const
{
   return begin() != end();
}

ResultIteratorBase ResultStoreBase::resultAt(int index) const
{
   if (m_results.isEmpty()) {
      return ResultIteratorBase(m_results.end());
   }
   QMap<int, ResultItem>::const_iterator it = m_results.lowerBound(index);

   // lowerBound returns either an iterator to the result or an iterator
   // to the nearest greater index. If the latter happens it might be
   // that the result is stored in a vector at the previous index.
   if (it == m_results.end()) {
      --it;
      if (it.value().isVector() == false) {
         return ResultIteratorBase(m_results.end());
      }
   } else {
      if (it.key() > index) {
         if (it == m_results.begin()) {
            return ResultIteratorBase(m_results.end());
         }
         --it;
      }
   }

   const int vectorIndex = index - it.key();

   if (vectorIndex >= it.value().count()) {
      return ResultIteratorBase(m_results.end());
   } else if (it.value().isVector() == false && vectorIndex != 0) {
      return ResultIteratorBase(m_results.end());
   }
   return ResultIteratorBase(it, vectorIndex);
}

bool ResultStoreBase::contains(int index) const
{
   return (resultAt(index) != end());
}

int ResultStoreBase::count() const
{
   return resultCount;
}

// returns the insert index, calling this function with
// index equal to -1 returns the next available index.
int ResultStoreBase::updateInsertIndex(int index, int _count)
{
   if (index == -1) {
      index = insertIndex;
      insertIndex += _count;
   } else {
      insertIndex = qMax(index + _count, insertIndex);
   }
   return index;
}

} // namespace QtConcurrent

QT_END_NAMESPACE

