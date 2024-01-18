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

#include <qplatformdefs.h>
#include <stdlib.h>

// Define the container allocation functions in a separate file, so users can easily override them.

void *qMalloc(size_t size)
{
   return ::malloc(size);
}

void qFree(void *ptr)
{
   ::free(ptr);
}

void *qRealloc(void *ptr, size_t size)
{
   return ::realloc(ptr, size);
}

void *qMallocAligned(size_t size, size_t alignment)
{
   return qReallocAligned(nullptr, size, 0, alignment);
}

void *qReallocAligned(void *oldptr, size_t newsize, size_t oldsize, size_t alignment)
{
   // fake an aligned allocation
   (void) oldsize;

   void *actualptr = oldptr ? static_cast<void **>(oldptr)[-1] : nullptr;

   if (alignment <= sizeof(void *)) {
      // special, fast case
      void **newptr = static_cast<void **>(realloc(actualptr, newsize + sizeof(void *)));
      if (! newptr) {
         return nullptr;
      }

      if (newptr == actualptr) {
         // realloc succeeded without reallocating
         return oldptr;
      }

      *newptr = newptr;
      return newptr + 1;
   }

   // malloc returns pointers aligned at least at sizeof(size_t) boundaries
   // but usually more (8- or 16-byte boundaries).
   // So we overallocate by alignment-sizeof(size_t) bytes, so we're guaranteed to find a
   // somewhere within the first alignment-sizeof(size_t) that is properly aligned.

   // However, we need to store the actual pointer, so we need to allocate actually size +
   // alignment anyway.

   void *real = realloc(actualptr, newsize + alignment);
   if (! real) {
      return nullptr;
   }

   quintptr faked = reinterpret_cast<quintptr>(real) + alignment;
   faked &= ~(alignment - 1);

   void **faked_ptr = reinterpret_cast<void **>(faked);

   // now save the value of the real pointer at faked-sizeof(void*)
   // by construction, alignment > sizeof(void*) and is a power of 2, so
   // faked-sizeof(void*) is properly aligned for a pointer
   faked_ptr[-1] = real;

   return faked_ptr;
}

void qFreeAligned(void *ptr)
{
   if (!ptr) {
      return;
   }
   void **ptr2 = static_cast<void **>(ptr);
   free(ptr2[-1]);
}

