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

#include <qarraydata.h>

#include <qtools_p.h>

#if defined (__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__  >= 406) && !defined(Q_CC_INTEL)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

static QArrayData *qtArray()
{
   static const QArrayData qt_array[3] = {
      { Q_REFCOUNT_INITIALIZE_STATIC, 0, 0, 0, sizeof(QArrayData) },  // shared empty
      { { 0 }, 0, 0, 0, sizeof(QArrayData) },                         // unsharable empty
      { { 0 }, 0, 0, 0, 0 }                                           // zero initialized element
   };

   return const_cast<QArrayData *>(qt_array);
}

QArrayData *QArrayData::sharedNull()
{
   static const QArrayData shared_null[2] = {
      { Q_REFCOUNT_INITIALIZE_STATIC, 0, 0, 0, sizeof(QArrayData) },  // shared null
      { { 0 }, 0, 0, 0, 0 }                                           // zero initialized element
   };

   return const_cast<QArrayData *>(shared_null);
}

#if defined (__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__  >= 406) && ! defined(Q_CC_INTEL)
#pragma GCC diagnostic pop
#endif

static const QArrayData &qt_array_empty()
{
   return qtArray()[0];
}

static const QArrayData &qt_array_unsharable_empty()
{
   return qtArray()[1];
}

QArrayData *QArrayData::allocate(size_t objectSize, size_t alignment, size_t capacity, AllocationOptions options)
{
   // alignment is a power of two
   Q_ASSERT(alignment >= alignof(QArrayData) && ! (alignment & (alignment - 1)));

   // do not allocate empty headers
   if (! (options & RawData) && ! capacity) {

      if (options & Unsharable) {
         return const_cast<QArrayData *>(&qt_array_unsharable_empty());

      } else {
         return const_cast<QArrayData *>(&qt_array_empty());

      }
   }

   size_t headerSize = sizeof(QArrayData);

   // Allocate extra (alignment - alignof(QArrayData)) padding bytes so we
   // can properly align the data array. This assumes malloc is able to
   // provide appropriate alignment for the header -- as it should!
   // Padding is skipped when allocating a header for RawData.
   if (! (options & RawData))  {
      headerSize += (alignment - alignof(QArrayData));
   }

   // Allocate additional space if array is growing
   if (options & Grow) {
      capacity = qAllocMore(objectSize * capacity, headerSize) / objectSize;
   }

   size_t allocSize = headerSize + objectSize * capacity;

   QArrayData *header = static_cast<QArrayData *>(qMalloc(allocSize));
   Q_CHECK_PTR(header);

   if (header) {
      quintptr data = (quintptr(header) + sizeof(QArrayData) + alignment - 1) & ~(alignment - 1);

      header->ref.atomic.store(bool(!(options & Unsharable)));
      header->size   = 0;
      header->alloc  = capacity;

      header->capacityReserved = bool(options & CapacityReserved);
      header->offset = data - quintptr(header);
   }

   return header;
}

void QArrayData::deallocate(QArrayData *data, size_t objectSize, size_t alignment)
{
   // Alignment is a power of two
   Q_ASSERT(alignment >= alignof(QArrayData) && ! (alignment & (alignment - 1)));

   (void) objectSize;
   (void) alignment;

   if (data == &qt_array_unsharable_empty())  {
      return;
   }

   qFree(data);
}
