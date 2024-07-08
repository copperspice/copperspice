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

#ifndef QARRAYDATA_H
#define QARRAYDATA_H

#include <qrefcount.h>

#include <new>

struct Q_CORE_EXPORT QArrayData {
   QtPrivate::RefCount ref;
   int size;
   uint alloc : 31;
   uint capacityReserved : 1;

   qptrdiff offset;             // in bytes from beginning of header

   void *data() {
      Q_ASSERT(size == 0 || offset < 0 || size_t(offset) >= sizeof(QArrayData));
      return std::launder(reinterpret_cast<char *>(this) + offset);
   }

   const void *data() const {
      Q_ASSERT(size == 0 || offset < 0 || size_t(offset) >= sizeof(QArrayData));
      return std::launder(reinterpret_cast<const char *>(this) + offset);
   }

   // This refers to array data mutability, not "header data" represented by
   // data members in QArrayData. Shared data (array and header) must still follow COW principles.
   bool isMutable() const {
      return alloc != 0;
   }

   enum AllocationOption {
      CapacityReserved    = 0x1,
      Unsharable          = 0x2,
      RawData             = 0x4,
      Grow                = 0x8,

      Default = 0
   };

   using AllocationOptions = QFlags<AllocationOption>;

   size_t detachCapacity(size_t newSize) const {
      if (capacityReserved && newSize < alloc) {
         return alloc;
      }

      return newSize;
   }

   AllocationOptions detachFlags() const {
      AllocationOptions result;

      if (!ref.isSharable()) {
         result |= Unsharable;
      }

      if (capacityReserved) {
         result |= CapacityReserved;
      }

      return result;
   }

   AllocationOptions cloneFlags() const {
      AllocationOptions result;

      if (capacityReserved) {
         result |= CapacityReserved;
      }

      return result;
   }

   [[nodiscard]] static QArrayData *allocate(size_t objectSize, size_t alignment,
         size_t capacity, AllocationOptions options = Default);

   static void deallocate(QArrayData *data, size_t objectSize, size_t alignment);

   static QArrayData *sharedNull();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QArrayData::AllocationOptions)

template <class T>
struct QTypedArrayData : QArrayData {
   using iterator       = T *;
   using const_iterator = const T *;

   T *data() {
      return static_cast<T *>(QArrayData::data());
   }

   const T *data() const {
      return static_cast<const T *>(QArrayData::data());
   }

   T *begin() {
      return data();
   }

   T *end() {
      return data() + size;
   }

   const T *begin() const {
      return data();
   }

   const T *end() const {
      return data() + size;
   }

   class AlignmentDummy
   {
      QArrayData header;
      T data;
   };

   [[nodiscard]] static QTypedArrayData *allocate(size_t capacity, AllocationOptions options = Default) {
      return static_cast<QTypedArrayData *>(QArrayData::allocate(sizeof(T),
            alignof(AlignmentDummy), capacity, options));
   }

   static void deallocate(QArrayData *data) {
      QArrayData::deallocate(data, sizeof(T), alignof(AlignmentDummy));
   }

   static QTypedArrayData *fromRawData(const T *data, size_t n, AllocationOptions options = Default) {
      QTypedArrayData *result = allocate(0, options | RawData);

      if (result) {
         Q_ASSERT(! result->ref.isShared());       // dissallow using the special "shared empty value"

         result->offset = reinterpret_cast<const char *>(data) - reinterpret_cast<const char *>(result);
         result->size = n;
      }

      return result;
   }

   static QTypedArrayData *sharedNull() {
      return static_cast<QTypedArrayData *>(QArrayData::sharedNull());
   }

   static QTypedArrayData *sharedEmpty() {
      return allocate(0);
   }

   static QTypedArrayData *unsharableEmpty() {
      return allocate(0, Unsharable);
   }
};

template <class T, size_t N>
struct QStaticArrayData {
   QArrayData header;
   T data[N];
};

// Support for returning QArrayDataPointer<T> from functions
template <class T>
struct QArrayDataPointerRef {
   QTypedArrayData<T> *ptr;
};

#define Q_STATIC_ARRAY_DATA_HEADER_INITIALIZER(type, size) { \
      Q_REFCOUNT_INITIALIZE_STATIC, size, 0, 0, \
      (sizeof(QArrayData) + (alignof(type) - 1)) & ~(alignof(type) - 1) }

#endif
