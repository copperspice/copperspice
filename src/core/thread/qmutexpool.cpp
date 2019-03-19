/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

QT_BEGIN_NAMESPACE

// qt_global_mutexpool is here for backwards compatibility only,
// use QMutexpool::instance() in new clode.
Q_CORE_EXPORT QMutexPool *qt_global_mutexpool = 0;
Q_GLOBAL_STATIC_WITH_ARGS(QMutexPool, globalMutexPool, (QMutex::Recursive))

/*!
    \class QMutexPool
    \brief The QMutexPool class provides a pool of QMutex objects.

    \internal

    \ingroup thread

    QMutexPool is a convenience class that provides access to a fixed
    number of QMutex objects.

    Typical use of a QMutexPool is in situations where it is not
    possible or feasible to use one QMutex for every protected object.
    The mutex pool will return a mutex based on the address of the
    object that needs protection.

    For example, consider this simple class:

    \snippet doc/src/snippets/code/src_corelib_thread_qmutexpool.cpp 0

    Adding a QMutex member to the Number class does not make sense,
    because it is so small. However, in order to ensure that access to
    each Number is protected, you need to use a mutex. In this case, a
    QMutexPool would be ideal.

    Code to calculate the square of a number would then look something
    like this:

    \snippet doc/src/snippets/code/src_corelib_thread_qmutexpool.cpp 1

    This function will safely calculate the square of a number, since
    it uses a mutex from a QMutexPool. The mutex is locked and
    unlocked automatically by the QMutexLocker class. See the
    QMutexLocker documentation for more details.
*/

/*!
    Constructs  a QMutexPool, reserving space for \a size QMutexes. All
    mutexes in the pool are created with \a recursionMode. By default,
    all mutexes are non-recursive.

    The QMutexes are created when needed, and deleted when the
    QMutexPool is destructed.
*/
QMutexPool::QMutexPool(QMutex::RecursionMode recursionMode, int size)
   : mutexes(size), recursionMode(recursionMode)
{
   for (int index = 0; index < mutexes.count(); ++index) {
      mutexes[index].store(0);
   }
}

/*!
    Destructs a QMutexPool. All QMutexes that were created by the pool
    are deleted.
*/
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
   if (!mutexes[index].testAndSetRelease(0, newMutex)) {
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
   if (globalInstance == 0) {
      return 0;
   }
   return globalInstance->get(address);
}

QT_END_NAMESPACE

