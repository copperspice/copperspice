/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QARRAYDATAOPS_H
#define QARRAYDATAOPS_H

#include <qarraydata.h>
#include <new>
#include <string.h>

#include <type_traits>

QT_BEGIN_NAMESPACE

namespace QtPrivate {

template <class T>
struct QPodArrayOps : QTypedArrayData<T> {

   void appendInitialize(size_t newSize) {
      Q_ASSERT(!this->ref.isShared());
      Q_ASSERT(newSize > uint(this->size));
      Q_ASSERT(newSize <= this->alloc);

      ::memset(this->end(), 0, (newSize - this->size) * sizeof(T));
      this->size = newSize;
   }

   void copyAppend(const T *b, const T *e) {
      Q_ASSERT(!this->ref.isShared());
      Q_ASSERT(b < e);
      Q_ASSERT(size_t(e - b) <= this->alloc - uint(this->size));

      ::memcpy(this->end(), b, (e - b) * sizeof(T));
      this->size += e - b;
   }

   void copyAppend(size_t n, const T &t) {
      Q_ASSERT(!this->ref.isShared());
      Q_ASSERT(n <= this->alloc - uint(this->size));

      T *iter = this->end();
      const T *const end = iter + n;
      for (; iter != end; ++iter) {
         ::memcpy(iter, &t, sizeof(T));
      }
      this->size += n;
   }

   void truncate(size_t newSize) {
      Q_ASSERT(!this->ref.isShared());
      Q_ASSERT(newSize < size_t(this->size));

      this->size = newSize;
   }

   void destroyAll() { // Call from destructors, ONLY!
      Q_ASSERT(this->ref == 0);

      // As this is to be called only from destructor, it does not need to be
      // exception safe; size not updated.
   }

   void insert(T *where, const T *b, const T *e) {
      Q_ASSERT(!this->ref.isShared());
      Q_ASSERT(where >= this->begin() && where < this->end()); // Use copyAppend at end
      Q_ASSERT(b < e);
      Q_ASSERT(e <= where || b > this->end()); // No overlap
      Q_ASSERT(size_t(e - b) <= this->alloc - uint(this->size));

      ::memmove(where + (e - b), where, (this->end() - where) * sizeof(T));
      ::memcpy(where, b, (e - b) * sizeof(T));
      this->size += (e - b);
   }

   void erase(T *b, T *e) {
      Q_ASSERT(b < e);
      Q_ASSERT(b >= this->begin() && b < this->end());
      Q_ASSERT(e > this->begin() && e < this->end());

      ::memmove(b, e, (this->end() - e) * sizeof(T));
      this->size -= (e - b);
   }
};

template <class T>
struct QGenericArrayOps : QTypedArrayData<T> {

   void appendInitialize(size_t newSize) {
      Q_ASSERT(!this->ref.isShared());
      Q_ASSERT(newSize > uint(this->size));
      Q_ASSERT(newSize <= this->alloc);

      T *const begin = this->begin();
      do {
         new (begin + this->size) T();
      } while (uint(++this->size) != newSize);
   }

   void copyAppend(const T *b, const T *e) {
      Q_ASSERT(!this->ref.isShared());
      Q_ASSERT(b < e);
      Q_ASSERT(size_t(e - b) <= this->alloc - uint(this->size));

      T *iter = this->end();
      for (; b != e; ++iter, ++b) {
         new (iter) T(*b);
         ++this->size;
      }
   }

   void copyAppend(size_t n, const T &t) {
      Q_ASSERT(!this->ref.isShared());
      Q_ASSERT(n <= this->alloc - uint(this->size));

      T *iter = this->end();
      const T *const end = iter + n;
      for (; iter != end; ++iter) {
         new (iter) T(t);
         ++this->size;
      }
   }

   void truncate(size_t newSize) {
      Q_ASSERT(!this->ref.isShared());
      Q_ASSERT(newSize < size_t(this->size));

      const T *const b = this->begin();
      do {
         (b + --this->size)->~T();
      } while (uint(this->size) != newSize);
   }

   void destroyAll() { // Call from destructors, ONLY
      // As this is to be called only from destructor, it doesn't need to be
      // exception safe; size not updated.

      Q_ASSERT(this->ref == 0);

      const T *const b = this->begin();
      const T *i = this->end();

      while (i != b) {
         (--i)->~T();
      }
   }

   void insert(T *where, const T *b, const T *e) {
      Q_ASSERT(!this->ref.isShared());
      Q_ASSERT(where >= this->begin() && where < this->end()); // Use copyAppend at end
      Q_ASSERT(b < e);
      Q_ASSERT(e <= where || b > this->end()); // No overlap
      Q_ASSERT(size_t(e - b) <= this->alloc - uint(this->size));

      // Array may be truncated at where in case of exceptions

      T *const end = this->end();
      const T *readIter = end;
      T *writeIter = end + (e - b);

      const T *const step1End = where + qMax(e - b, end - where);

      struct Destructor {
         Destructor(T *&iter)
            : iter(&iter)
            , end(iter) {
         }

         void commit() {
            iter = &end;
         }

         ~Destructor() {
            for (; *iter != end; --*iter) {
               (*iter)->~T();
            }
         }

         T **iter;
         T *end;
      } destroyer(writeIter);

      // Construct new elements in array
      do {
         --readIter, --writeIter;
         new (writeIter) T(*readIter);
      } while (writeIter != step1End);

      while (writeIter != end) {
         --e, --writeIter;
         new (writeIter) T(*e);
      }

      destroyer.commit();
      this->size += destroyer.end - end;

      // Copy assign over existing elements
      while (readIter != where) {
         --readIter, --writeIter;
         *writeIter = *readIter;
      }

      while (writeIter != where) {
         --e, --writeIter;
         *writeIter = *e;
      }
   }

   void erase(T *b, T *e) {
      Q_ASSERT(b < e);
      Q_ASSERT(b >= this->begin() && b < this->end());
      Q_ASSERT(e > this->begin() && e < this->end());

      const T *const end = this->end();

      do {
         *b = *e;
         ++b, ++e;
      } while (e != end);

      do {
         (--e)->~T();
         --this->size;
      } while (e != b);
   }
};

template <class T>
struct QMovableArrayOps : QGenericArrayOps<T> {
  
   void insert(T *where, const T *b, const T *e) {
      Q_ASSERT(!this->ref.isShared());
      Q_ASSERT(where >= this->begin() && where < this->end()); // Use copyAppend at end
      Q_ASSERT(b < e);
      Q_ASSERT(e <= where || b > this->end()); // No overlap
      Q_ASSERT(size_t(e - b) <= this->alloc - uint(this->size));

      // Provides strong exception safety guarantee, provided T::~T() nothrow

      struct ReversibleDisplace {
         ReversibleDisplace(T *begin, T *end, size_t displace)
            : begin(begin)
            , end(end)
            , displace(displace) {
            ::memmove(begin + displace, begin, (end - begin) * sizeof(T));
         }

         void commit() {
            displace = 0;
         }

         ~ReversibleDisplace() {
            if (displace) {
               ::memmove(begin, begin + displace, (end - begin) * sizeof(T));
            }
         }

         T *const begin;
         T *const end;
         size_t displace;

      } displace(where, this->end(), size_t(e - b));

      struct CopyConstructor {
         CopyConstructor(T *where) : where(where) {}

         void copy(const T *src, const T *const srcEnd) {
            n = 0;
            for (; src != srcEnd; ++src) {
               new (where + n) T(*src);
               ++n;
            }
            n = 0;
         }

         ~CopyConstructor() {
            while (n) {
               where[--n].~T();
            }
         }

         T *const where;
         size_t n;
      } copier(where);

      copier.copy(b, e);
      displace.commit();
      this->size += (e - b);
   }

   void erase(T *b, T *e) {
      Q_ASSERT(b < e);
      Q_ASSERT(b >= this->begin() && b < this->end());
      Q_ASSERT(e > this->begin() && e < this->end());

      struct Mover {
         Mover(T *&start, const T *finish, int &sz)
            : destination(start)
            , source(start)
            , n(finish - start)
            , size(sz) {
         }

         ~Mover() {
            ::memmove(destination, source, n * sizeof(T));
            size -= (source - destination);
         }

         T *&destination;
         const T *const source;
         size_t n;
         int &size;
      } mover(e, this->end(), this->size);

      do {
         // Exceptions or not, dtor called once per instance
         (--e)->~T();
      } while (e != b);
   }
};

template <class T, class = void>
struct QArrayOpsSelector {
   typedef QGenericArrayOps<T> Type;
};

template <class T>
struct QArrayOpsSelector < T, typename std::enable_if< ! QTypeInfo<T>::isComplex  && ! QTypeInfo<T>::isStatic >::type > {
   typedef QPodArrayOps<T> Type;
};

template <class T>
struct QArrayOpsSelector < T, typename std::enable_if< QTypeInfo<T>::isComplex  &&!QTypeInfo<T>::isStatic >::type > {
   typedef QMovableArrayOps<T> Type;
};

} // namespace QtPrivate


template <class T>
struct QArrayDataOps : QtPrivate::QArrayOpsSelector<T>::Type {
};

QT_END_NAMESPACE

#endif 
