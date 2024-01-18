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

#include <qatomic.h>

#include <qmutexpool_p.h>

QMutexPool *globalMutexPool()
{
   static QMutexPool retval;

   return &retval;
}

QMutexPool::QMutexPool(int size)
   : m_mutexArray(size)
{
   for (int index = 0; index < m_mutexArray.count(); ++index) {
      m_mutexArray[index].store(nullptr);
   }
}

QMutexPool::~QMutexPool()
{
   for (int index = 0; index < m_mutexArray.count(); ++index) {
      delete m_mutexArray[index].load();
   }
}

QMutexPool *QMutexPool::instance()
{
   return globalMutexPool();
}

QRecursiveMutex *QMutexPool::createMutex(int index)
{
   QRecursiveMutex *newMutex = new QRecursiveMutex();
   QRecursiveMutex *expected = nullptr;

   if (! m_mutexArray[index].compareExchange(expected, newMutex, std::memory_order_release)) {
      delete newMutex;
   }

   return m_mutexArray[index].load();
}

QRecursiveMutex *QMutexPool::globalInstanceGet(const void *address)
{
   QMutexPool *const globalInstance = globalMutexPool();

   if (globalInstance == nullptr) {
      return nullptr;
   }

   return globalInstance->get(address);
}

