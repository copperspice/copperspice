/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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
// for rand_s, _CRT_RAND_S must be #defined before #including stdlib.h.
// put it at the beginning so some indirect inclusion doesn't break it
#ifndef _CRT_RAND_S
#define _CRT_RAND_S
#endif
#include <stdlib.h>


#include <qhash.h>

#ifdef truncate
#undef truncate
#endif

#include <qbitarray.h>
#include <qstring.h>
#include <qglobal.h>
#include <qbytearray.h>
#include <qdatetime.h>

#include <qcoreapplication.h>

#ifdef Q_OS_UNIX
#include <stdio.h>
#include "private/qcore_unix_p.h"
#endif 

#include <limits.h>


QT_BEGIN_NAMESPACE

static inline uint hash(const uchar *p, int len, uint seed)
{
   uint h = seed;

   for (int i = 0; i < len; ++i) {
      h = 31 * h + p[i];
   }

   return h;
}

static inline uint hash(const QChar *p, int len, uint seed)
{
   uint h = seed;

   for (int i = 0; i < len; ++i) {
      h = 31 * h + p[i].unicode();
   }

   return h;
}

uint qHash(const QByteArray &key, uint seed)
{
   return hash(reinterpret_cast<const uchar *>(key.constData()), key.size(), seed);
}

uint qHash(const QString &key, uint seed)
{
   return hash(key.unicode(), key.size(), seed);
}

uint qHash(const QStringRef &key, uint seed)
{
   return hash(key.unicode(), key.size(), seed);
}

uint qHash(const QBitArray &bitArray, uint seed)
{
   int m = bitArray.d.size() - 1;
   uint result = hash(reinterpret_cast<const uchar *>(bitArray.d.constData()), qMax(0, m), seed);

   // deal with the last 0 to 7 bits manually, because we can't trust that
   // the padding is initialized to 0 in bitArray.d
   int n = bitArray.size();
   if (n & 0x7) {
      result = ((result << 4) + bitArray.d.at(m)) & ((1 << n) - 1);
   }
   return result;
}

uint qHash(const QLatin1String &key, uint seed)
{
   return hash(reinterpret_cast<const uchar *>(key.data()), key.size(), seed);
} 

static uint qt_create_qhash_seed()
{
   uint seed = 0;
	
#if defined(Q_OS_UNIX)
   int randomfd = qt_safe_open("/dev/urandom", O_RDONLY);
   if (randomfd == -1) {
       randomfd = qt_safe_open("/dev/random", O_RDONLY | O_NONBLOCK);
   }
   if (randomfd != -1) {
       if (qt_safe_read(randomfd, reinterpret_cast<char *>(&seed), sizeof(seed)) == sizeof(seed)) {
	   qt_safe_close(randomfd);
	   return seed;
       }
       qt_safe_close(randomfd);
   }
#endif 
   
   // general fallback: initialize from the current timestamp, pid,
   // and address of a stack-local variable
   quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
   seed ^= timestamp;
   seed ^= (timestamp >> 32);
   
   quint64 pid = QCoreApplication::applicationPid();
   seed ^= pid;
   seed ^= (pid >> 32);
   
   quintptr seedPtr = reinterpret_cast<quintptr>(&seed);
   seed ^= seedPtr;
   if(sizeof(int*) == 8) {
       seed ^= (seedPtr >> 32);
   }
   
   return seed;
}


uint qt_hash(const QString &key)
{
   const QChar *p = key.unicode();
   int n = key.size();
   uint h = 0;
   
   while (n--) {
       h = (h << 4) + (*p++).unicode();
       h ^= (h & 0xf0000000) >> 23;
       h &= 0x0fffffff;
   }
   return h;
}

std::atomic<uint> qt_qhash_seed{0};

uint QHashData::hashSeed()
{
    uint value = qt_qhash_seed.load(std::memory_order_relaxed);
    if (value != 0) {
	// Already initialized
	return value;
    }

    value = qt_create_qhash_seed();
    if(value == 0) {
	// Prevent 0 from ever being a valid seed value
	value = 1;
    }
    
    uint expectedValue = 0;
    if(qt_qhash_seed.compare_exchange_strong(expectedValue, value, std::memory_order_relaxed)) {
	// We succeeded, value is correct
	return value;
    } else {
	// We failed, someone else has filled in the value
	return expectedValue;
    }
}

/*
    The prime_deltas array is a table of selected prime values, even
    though it doesn't look like one. The primes we are using are 1,
    2, 5, 11, 17, 37, 67, 131, 257, ..., i.e. primes in the immediate
    surrounding of a power of two.

    The primeForNumBits() function returns the prime associated to a
    power of two. For example, primeForNumBits(8) returns 257.
*/

static const uchar prime_deltas[] = {
   0,  0,  1,  3,  1,  5,  3,  3,  1,  9,  7,  5,  3,  9, 25,  3,
   1, 21,  3, 21,  7, 15,  9,  5,  3, 29, 15,  0,  0,  0,  0,  0
};

static inline int primeForNumBits(int numBits)
{
   return (1 << numBits) + prime_deltas[numBits];
}

/*
    Returns the smallest integer n such that
    primeForNumBits(n) >= hint.
*/
static int countBits(int hint)
{
   int numBits = 0;
   int bits = hint;

   while (bits > 1) {
      bits >>= 1;
      numBits++;
   }

   if (numBits >= (int)sizeof(prime_deltas)) {
      numBits = sizeof(prime_deltas) - 1;
   } else if (primeForNumBits(numBits) < hint) {
      ++numBits;
   }
   return numBits;
}

/*
    A QHash has initially around pow(2, MinNumBits) buckets. For
    example, if MinNumBits is 4, it has 17 buckets.
*/
const int MinNumBits = 4;

QHashData *QHashData::sharedNull()
{
   static const QHashData shared_null = {
       0, 0, Q_REFCOUNT_INITIALIZE_STATIC, 0, 0, MinNumBits, 0, 0, true, false, 0
   };

   return const_cast<QHashData *>(&shared_null);
}

void *QHashData::allocateNode(int nodeAlign)
{
   void *ptr = strictAlignment ? qMallocAligned(nodeSize, nodeAlign) : malloc(nodeSize);
   Q_CHECK_PTR(ptr);
   return ptr;
}

void QHashData::freeNode(void *node)
{
   if (strictAlignment) {
      qFreeAligned(node);
   } else {
      free(node);
   }
}

QHashData *QHashData::detach_helper(void (*node_duplicate)(Node *, void *),
                                    void (*node_delete)(Node *),
                                    int nodeSize,
                                    int nodeAlign)
{
   union {
      QHashData *d;
      Node *e;
   };

   d = new QHashData;
   d->fakeNext = 0;
   d->buckets = 0;
   d->ref.initializeOwned();
   d->size = size;
   d->nodeSize = nodeSize;
   d->userNumBits = userNumBits;
   d->numBits = numBits;
   d->numBuckets = numBuckets;
   d->sharable = true;
   d->strictAlignment = nodeAlign > 8;
   d->reserved = 0;

   if (numBuckets) {
      QT_TRY {
         d->buckets = new Node *[numBuckets];
      } QT_CATCH(...) {
         // restore a consistent state for d
         d->numBuckets = 0;
         // roll back
         d->free_helper(node_delete);
         QT_RETHROW;
      }

      Node *this_e = reinterpret_cast<Node *>(this);
      for (int i = 0; i < numBuckets; ++i) {
         Node **nextNode = &d->buckets[i];
         Node *oldNode = buckets[i];
         while (oldNode != this_e) {
            QT_TRY {
               Node *dup = static_cast<Node *>(allocateNode(nodeAlign));

               QT_TRY {
                  node_duplicate(oldNode, dup);
               } QT_CATCH(...)
               {
                  freeNode( dup );
                  QT_RETHROW;
               }

               dup->h = oldNode->h;
               *nextNode = dup;
               nextNode = &dup->next;
               oldNode = oldNode->next;
            } QT_CATCH(...) {
               // restore a consistent state for d
               *nextNode = e;
               d->numBuckets = i + 1;
               // roll back
               d->free_helper(node_delete);
               QT_RETHROW;
            }
         }
         *nextNode = e;
      }
   }
   return d;
}

void QHashData::free_helper(void (*node_delete)(Node *))
{
   if (node_delete) {
      Node *this_e = reinterpret_cast<Node *>(this);
      Node **bucket = reinterpret_cast<Node **>(this->buckets);

      int n = numBuckets;
      while (n--) {
         Node *cur = *bucket++;
         while (cur != this_e) {
            Node *next = cur->next;
            node_delete(cur);
            freeNode(cur);
            cur = next;
         }
      }
   }
   delete [] buckets;
   delete this;
}

QHashData::Node *QHashData::nextNode(Node *node)
{
   union {
      Node *next;
      Node *e;
      QHashData *d;
   };
   next = node->next;
   Q_ASSERT_X(next, "QHash", "Iterating beyond end()");
   if (next->next) {
      return next;
   }

   int start = (node->h % d->numBuckets) + 1;
   Node **bucket = d->buckets + start;
   int n = d->numBuckets - start;
   while (n--) {
      if (*bucket != e) {
         return *bucket;
      }
      ++bucket;
   }
   return e;
}

QHashData::Node *QHashData::previousNode(Node *node)
{
   union {
      Node *e;
      QHashData *d;
   };

   e = node;
   while (e->next) {
      e = e->next;
   }

   int start;
   if (node == e) {
      start = d->numBuckets - 1;
   } else {
      start = node->h % d->numBuckets;
   }

   Node *sentinel = node;
   Node **bucket = d->buckets + start;
   while (start >= 0) {
      if (*bucket != sentinel) {
         Node *prev = *bucket;
         while (prev->next != sentinel) {
            prev = prev->next;
         }
         return prev;
      }

      sentinel = e;
      --bucket;
      --start;
   }
   Q_ASSERT_X(start >= 0, "QHash", "Iterating backward beyond begin()");
   return e;
}

/*
    If hint is negative, -hint gives the approximate number of
    buckets that should be used for the hash table. If hint is
    nonnegative, (1 << hint) gives the approximate number
    of buckets that should be used.
*/
void QHashData::rehash(int hint)
{
   if (hint < 0) {
      hint = countBits(-hint);
      if (hint < MinNumBits) {
         hint = MinNumBits;
      }
      userNumBits = hint;
      while (primeForNumBits(hint) < (size >> 1)) {
         ++hint;
      }
   } else if (hint < MinNumBits) {
      hint = MinNumBits;
   }

   if (numBits != hint) {
      Node *e = reinterpret_cast<Node *>(this);
      Node **oldBuckets = buckets;
      int oldNumBuckets = numBuckets;

      int nb = primeForNumBits(hint);
      buckets = new Node *[nb];
      numBits = hint;
      numBuckets = nb;
      for (int i = 0; i < numBuckets; ++i) {
         buckets[i] = e;
      }

      for (int i = 0; i < oldNumBuckets; ++i) {
         Node *firstNode = oldBuckets[i];
         while (firstNode != e) {
            uint h = firstNode->h;
            Node *lastNode = firstNode;
            while (lastNode->next != e && lastNode->next->h == h) {
               lastNode = lastNode->next;
            }

            Node *afterLastNode = lastNode->next;
            Node **beforeFirstNode = &buckets[h % numBuckets];
            while (*beforeFirstNode != e) {
               beforeFirstNode = &(*beforeFirstNode)->next;
            }
            lastNode->next = *beforeFirstNode;
            *beforeFirstNode = firstNode;
            firstNode = afterLastNode;
         }
      }
      delete [] oldBuckets;
   }
}

#ifdef QT_QHASH_DEBUG

void QHashData::dump()
{
   qDebug("Hash data (ref = %d, size = %d, nodeSize = %d, userNumBits = %d, numBits = %d, numBuckets = %d)",
          int(ref), size, nodeSize, userNumBits, numBits,
          numBuckets);
   qDebug("    %p (fakeNode = %p)", this, fakeNext);
   for (int i = 0; i < numBuckets; ++i) {
      QString line;
      Node *n = buckets[i];
      if (n != reinterpret_cast<Node *>(this)) {
         line.sprintf("%d:", i);
         while (n != reinterpret_cast<Node *>(this)) {
            line += QString().sprintf(" -> [%p]", n);
            if (!n) {
               line += " (CORRUPT)";
               break;
            }
            n = n->next;
         }
         qDebug(qPrintable(line));
      }
   }
}

void QHashData::checkSanity()
{
   if (fakeNext) {
      qFatal("Fake next isn't 0");
   }

   for (int i = 0; i < numBuckets; ++i) {
      Node *n = buckets[i];
      Node *p = n;
      if (!n) {
         qFatal("%d: Bucket entry is 0", i);
      }
      if (n != reinterpret_cast<Node *>(this)) {
         while (n != reinterpret_cast<Node *>(this)) {
            if (!n->next) {
               qFatal("%d: Next of %p is 0, should be %p", i, n, this);
            }
            n = n->next;
         }
      }
   }
}
#endif
 
QT_END_NAMESPACE
