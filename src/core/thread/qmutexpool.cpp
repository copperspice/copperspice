/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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
   static QMutexPool retval(QMutex::Recursive);
   return &retval;
}

QMutexPool::QMutexPool(QMutex::RecursionMode recursionMode, int size)
   : mutexes(size), recursionMode(recursionMode)
{
   for (int index = 0; index < mutexes.count(); ++index) {
      mutexes[index].store(nullptr);
   }
}

QMutexPool::~QMutexPool()
{
   for (int index = 0; index < mutexes.count(); ++index) {
      delete mutexes[index].load();
   }
}

/*!
    Returns the global QMutexPool instance.
*/
QMutexPool *QMutexPool::instance()
{
   return globalMutexPool();
}

/*!
    \fn QMutexPool::get(const void *address)
    Returns a QMutex from the pool. QMutexPool uses the value \a address
    to determine which mutex is returned from the pool.
*/

/*! \internal
  create the mutex for the given index
 */
QMutex *QMutexPool::createMutex(int index)
{
   // mutex not created, create one
   QMutex *newMutex = new QMutex(recursionMode);

    QMutex *expected = nullptr;

   if (! mutexes[index].compareExchange(expected, newMutex, std::memory_order_release)) {
      delete newMutex;
   }
   return mutexes[index].load();
}

/*!
    Returns a QMutex from the global mutex pool.
*/
QMutex *QMutexPool::globalInstanceGet(const void *address)
{
   QMutexPool *const globalInstance = globalMutexPool();
   if (globalInstance == nullptr) {
      return nullptr;
   }
   return globalInstance->get(address);
}


