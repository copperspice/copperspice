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

#include <qsqlcachedresult_p.h>

#include <qvariant.h>
#include <qdatetime.h>
#include <qvector.h>

static const uint initial_cache_size = 128;

class QSqlCachedResultPrivate
{
 public:
   QSqlCachedResultPrivate();
   bool canSeek(int i) const;
   inline int cacheCount() const;
   void init(int count, bool fo);
   void cleanup();
   int nextIndex();
   void revertLast();

   QSqlCachedResult::ValueCache cache;
   int rowCacheEnd;
   int colCount;
   bool forwardOnly;
   bool atEnd;
};

QSqlCachedResultPrivate::QSqlCachedResultPrivate():
   rowCacheEnd(0), colCount(0), forwardOnly(false), atEnd(false)
{
}

void QSqlCachedResultPrivate::cleanup()
{
   cache.clear();
   forwardOnly = false;
   atEnd = false;
   colCount = 0;
   rowCacheEnd = 0;
}

void QSqlCachedResultPrivate::init(int count, bool fo)
{
   Q_ASSERT(count);
   cleanup();
   forwardOnly = fo;
   colCount = count;
   if (fo) {
      cache.resize(count);
      rowCacheEnd = count;
   } else {
      cache.resize(initial_cache_size * count);
   }
}

int QSqlCachedResultPrivate::nextIndex()
{
   if (forwardOnly) {
      return 0;
   }

   int newIdx = rowCacheEnd;
   if (newIdx + colCount > cache.size()) {
      cache.resize(qMin(cache.size() * 2, cache.size() + 10000));
   }

   rowCacheEnd += colCount;

   return newIdx;
}

bool QSqlCachedResultPrivate::canSeek(int i) const
{
   if (forwardOnly || i < 0) {
      return false;
   }

   return rowCacheEnd >= (i + 1) * colCount;
}

void QSqlCachedResultPrivate::revertLast()
{
   if (forwardOnly) {
      return;
   }

   rowCacheEnd -= colCount;
}

inline int QSqlCachedResultPrivate::cacheCount() const
{
   Q_ASSERT(!forwardOnly);
   Q_ASSERT(colCount);
   return rowCacheEnd / colCount;
}

QSqlCachedResult::QSqlCachedResult(const QSqlDriver *db): QSqlResult (db)
{
   d = new QSqlCachedResultPrivate();
}

QSqlCachedResult::~QSqlCachedResult()
{
   delete d;
}

void QSqlCachedResult::init(int colCount)
{
   d->init(colCount, isForwardOnly());
}

bool QSqlCachedResult::fetch(int i)
{
   if ((!isActive()) || (i < 0)) {
      return false;
   }
   if (at() == i) {
      return true;
   }
   if (d->forwardOnly) {
      // speed hack - do not copy values if not needed
      if (at() > i || at() == QSql::AfterLastRow) {
         return false;
      }
      while (at() < i - 1) {
         if (!gotoNext(d->cache, -1)) {
            return false;
         }
         setAt(at() + 1);
      }
      if (!gotoNext(d->cache, 0)) {
         return false;
      }
      setAt(at() + 1);
      return true;
   }
   if (d->canSeek(i)) {
      setAt(i);
      return true;
   }
   if (d->rowCacheEnd > 0) {
      setAt(d->cacheCount());
   }
   while (at() < i + 1) {
      if (!cacheNext()) {
         if (d->canSeek(i)) {
            break;
         }
         return false;
      }
   }
   setAt(i);

   return true;
}

bool QSqlCachedResult::fetchNext()
{
   if (d->canSeek(at() + 1)) {
      setAt(at() + 1);
      return true;
   }
   return cacheNext();
}

bool QSqlCachedResult::fetchPrevious()
{
   return fetch(at() - 1);
}

bool QSqlCachedResult::fetchFirst()
{
   if (d->forwardOnly && at() != QSql::BeforeFirstRow) {
      return false;
   }
   if (d->canSeek(0)) {
      setAt(0);
      return true;
   }
   return cacheNext();
}

bool QSqlCachedResult::fetchLast()
{
   if (d->atEnd) {
      if (d->forwardOnly) {
         return false;
      } else {
         return fetch(d->cacheCount() - 1);
      }
   }

   int i = at();
   while (fetchNext()) {
      ++i;   /* brute force */
   }
   if (d->forwardOnly && at() == QSql::AfterLastRow) {
      setAt(i);
      return true;
   } else {
      return fetch(i);
   }
}

QVariant QSqlCachedResult::data(int i)
{
   int idx = d->forwardOnly ? i : at() * d->colCount + i;
   if (i >= d->colCount || i < 0 || at() < 0 || idx >= d->rowCacheEnd) {
      return QVariant();
   }

   return d->cache.at(idx);
}

bool QSqlCachedResult::isNull(int i)
{
   int idx = d->forwardOnly ? i : at() * d->colCount + i;

   if (i >= d->colCount || i < 0 || at() < 0 || idx >= d->rowCacheEnd) {
      return true;
   }

   return ! d->cache.at(idx).isValid();
}

void QSqlCachedResult::cleanup()
{
   setAt(QSql::BeforeFirstRow);
   setActive(false);
   d->cleanup();
}

void QSqlCachedResult::clearValues()
{
   setAt(QSql::BeforeFirstRow);
   d->rowCacheEnd = 0;
   d->atEnd = false;
}

bool QSqlCachedResult::cacheNext()
{
   if (d->atEnd) {
      return false;
   }

   if (isForwardOnly()) {
      d->cache.clear();
      d->cache.resize(d->colCount);
   }

   if (!gotoNext(d->cache, d->nextIndex())) {
      d->revertLast();
      d->atEnd = true;
      return false;
   }
   setAt(at() + 1);
   return true;
}

int QSqlCachedResult::colCount() const
{
   return d->colCount;
}

QSqlCachedResult::ValueCache &QSqlCachedResult::cache()
{
   return d->cache;
}

void QSqlCachedResult::virtual_hook(int id, void *data)
{
   QSqlResult::virtual_hook(id, data);
}

void QSqlCachedResult::detachFromResultSet()
{
   cleanup();
}

void QSqlCachedResult::setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy policy)
{
   QSqlResult::setNumericalPrecisionPolicy(policy);
   cleanup();
}
