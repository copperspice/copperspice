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

#ifndef QMUTEXPOOL_P_H
#define QMUTEXPOOL_P_H

#include <qatomic.h>
#include <qmutex.h>
#include <qvarlengtharray.h>

class Q_CORE_EXPORT QMutexPool
{
 public:
   explicit QMutexPool(int size = DEFAULT_SIZE);

   ~QMutexPool();

   QRecursiveMutex *get(const void *address) {
      int index = uint(quintptr(address)) % m_mutexArray.count();
      QRecursiveMutex *mutex = m_mutexArray[index].load();

      if (mutex != nullptr) {
         return mutex;
      } else {
         return createMutex(index);
      }
   }

   static QMutexPool *instance();
   static QRecursiveMutex *globalInstanceGet(const void *address);

   static constexpr const int DEFAULT_SIZE = 131;

 private:
   QRecursiveMutex *createMutex(int index);
   QVarLengthArray<QAtomicPointer<QRecursiveMutex>, DEFAULT_SIZE> m_mutexArray;
};

extern Q_CORE_EXPORT QMutexPool *qt_global_mutexpool;

#endif
