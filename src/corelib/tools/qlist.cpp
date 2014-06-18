/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <new>
#include "qlist.h"
#include "qtools_p.h"

#include <string.h>
#include <stdlib.h>

QT_BEGIN_NAMESPACE

static int grow(int size)
{
   // dear compiler: don't optimize me out.
   volatile int x = qAllocMore(size * sizeof(void *), QListData::DataHeaderSize) / sizeof(void *);
   return x;
}

QListData::Data *QListData::sharedNull()
{
   static const Data shared_null = {
      Q_REFCOUNT_INITIALIZE_STATIC, 0, 0, 0, true, { 0 }
   };

   return const_cast<Data *>(&shared_null);
}

// internal
QListData::Data *QListData::detach_grow(int *idx, int num)
{
   Data *x = d;
   int l = x->end - x->begin;
   int nl = l + num;
   int alloc = grow(nl);
   Data *t = static_cast<Data *>(::malloc(DataHeaderSize + alloc * sizeof(void *)));
   Q_CHECK_PTR(t);

   t->ref.initializeOwned();
   t->sharable = true;
   t->alloc = alloc;

   // The space reservation algorithm's optimization is biased towards appending:
   // Something which looks like an append will put the data at the beginning,
   // while something which looks like a prepend will put it in the middle
   // instead of at the end. That's based on the assumption that prepending
   // is uncommon and even an initial prepend will eventually be followed by
   // at least some appends.
   int bg;

   if (*idx < 0) {
      *idx = 0;
      bg = (alloc - nl) >> 1;

   } else if (*idx > l) {
      *idx = l;
      bg = 0;

   } else if (*idx < (l >> 1)) {
      bg = (alloc - nl) >> 1;

   } else {
      bg = 0;
   }

   t->begin = bg;
   t->end = bg + nl;
   d = t;

   return x;
}

/*!
 *  Detaches the QListData by allocating new memory for a list which possibly
 *  has a different size than the copied one.
 *  Returns the old (shared) data, it is up to the caller to deref() and free()
 *  For the new data node_copy needs to be called.
 *
 *  \internal
 */
QListData::Data *QListData::detach(int alloc)
{
   Data *x = d;
   Data *t = static_cast<Data *>(::malloc(DataHeaderSize + alloc * sizeof(void *)));
   Q_CHECK_PTR(t);

   t->ref.initializeOwned();
   t->sharable = true;
   t->alloc = alloc;
   if (!alloc) {
      t->begin = 0;
      t->end = 0;
   } else {
      t->begin = x->begin;
      t->end   = x->end;
   }
   d = t;

   return x;
}

void QListData::realloc(int alloc)
{
   Q_ASSERT(! d->ref.isShared());

   Data *x = static_cast<Data *>(::realloc(d, DataHeaderSize + alloc * sizeof(void *)));
   Q_CHECK_PTR(x);

   d = x;
   d->alloc = alloc;

   if (! alloc) {
      d->begin = d->end = 0;
   }
}

// ensures enough space is available to append n elements
void **QListData::append(int n)
{
   Q_ASSERT(!d->ref.isShared());
   int e = d->end;

   if (e + n > d->alloc) {
      int b = d->begin;

      if (b - n >= 2 * d->alloc / 3) {
         // we have enough space. Just not at the end -> move it.
         e -= b;
         ::memmove(d->array, d->array + b, e * sizeof(void *));
         d->begin = 0;

      } else {
         realloc(grow(d->alloc + n));
      }
   }

   d->end = e + n;
   return d->array + e;
}

// ensures that enough space is available to append one element
void **QListData::append()
{
   return append(1);
}

// ensures that enough space is available to append the list
void **QListData::append2(const QListData &l)
{
   return append(l.d->end - l.d->begin);
}

void **QListData::prepend()
{
   Q_ASSERT(! d->ref.isShared());
   if (d->begin == 0) {
      if (d->end >= d->alloc / 3) {
         realloc(grow(d->alloc + 1));
      }

      if (d->end < d->alloc / 3) {
         d->begin = d->alloc - 2 * d->end;
      } else {
         d->begin = d->alloc - d->end;
      }

      ::memmove(d->array + d->begin, d->array, d->end * sizeof(void *));
      d->end += d->begin;
   }
   return d->array + --d->begin;
}

void **QListData::insert(int i)
{
   Q_ASSERT(!d->ref.isShared());
   if (i <= 0) {
      return prepend();
   }
   int size = d->end - d->begin;
   if (i >= size) {
      return append();
   }

   bool leftward = false;

   if (d->begin == 0) {
      if (d->end == d->alloc) {
         // If the array is full, we expand it and move some items rightward
         realloc(grow(d->alloc + 1));
      } else {
         // If there is free space at the end of the array, we move some items rightward
      }
   } else {
      if (d->end == d->alloc) {
         // If there is free space at the beginning of the array, we move some items leftward
         leftward = true;
      } else {
         // If there is free space at both ends, we move as few items as possible
         leftward = (i < size - i);
      }
   }

   if (leftward) {
      --d->begin;
      ::memmove(d->array + d->begin, d->array + d->begin + 1, i * sizeof(void *));
   } else {
      ::memmove(d->array + d->begin + i + 1, d->array + d->begin + i,
                (size - i) * sizeof(void *));
      ++d->end;
   }
   return d->array + d->begin + i;
}

void QListData::remove(int i)
{
   Q_ASSERT(!d->ref.isShared());
   i += d->begin;
   if (i - d->begin < d->end - i) {
      if (int offset = i - d->begin) {
         ::memmove(d->array + d->begin + 1, d->array + d->begin, offset * sizeof(void *));
      }
      d->begin++;
   } else {
      if (int offset = d->end - i - 1) {
         ::memmove(d->array + i, d->array + i + 1, offset * sizeof(void *));
      }
      d->end--;
   }
}

void QListData::remove(int i, int n)
{
   Q_ASSERT(!d->ref.isShared());
   i += d->begin;
   int middle = i + n / 2;
   if (middle - d->begin < d->end - middle) {
      ::memmove(d->array + d->begin + n, d->array + d->begin,
                (i - d->begin) * sizeof(void *));
      d->begin += n;
   } else {
      ::memmove(d->array + i, d->array + i + n,
                (d->end - i - n) * sizeof(void *));
      d->end -= n;
   }
}

void QListData::move(int from, int to)
{
   Q_ASSERT(!d->ref.isShared());
   if (from == to) {
      return;
   }

   from += d->begin;
   to += d->begin;
   void *t = d->array[from];

   if (from < to) {
      if (d->end == d->alloc || 3 * (to - from) < 2 * (d->end - d->begin)) {
         ::memmove(d->array + from, d->array + from + 1, (to - from) * sizeof(void *));
      } else {
         // optimization
         if (int offset = from - d->begin) {
            ::memmove(d->array + d->begin + 1, d->array + d->begin, offset * sizeof(void *));
         }
         if (int offset = d->end - (to + 1)) {
            ::memmove(d->array + to + 2, d->array + to + 1, offset * sizeof(void *));
         }
         ++d->begin;
         ++d->end;
         ++to;
      }
   } else {
      if (d->begin == 0 || 3 * (from - to) < 2 * (d->end - d->begin)) {
         ::memmove(d->array + to + 1, d->array + to, (from - to) * sizeof(void *));
      } else {
         // optimization
         if (int offset = to - d->begin) {
            ::memmove(d->array + d->begin - 1, d->array + d->begin, offset * sizeof(void *));
         }
         if (int offset = d->end - (from + 1)) {
            ::memmove(d->array + from, d->array + from + 1, offset * sizeof(void *));
         }
         --d->begin;
         --d->end;
         --to;
      }
   }
   d->array[to] = t;
}

void **QListData::erase(void **xi)
{
   Q_ASSERT(!d->ref.isShared());
   int i = xi - (d->array + d->begin);
   remove(i);
   return d->array + d->begin + i;
}

QT_END_NAMESPACE
