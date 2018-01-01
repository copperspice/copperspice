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

#include <qsystemsemaphore.h>
#include <qsystemsemaphore_p.h>
#include <qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SYSTEMSEMAPHORE

QSystemSemaphore::QSystemSemaphore(const QString &key, int initialValue, AccessMode mode)
   : d(new QSystemSemaphorePrivate)
{
   setKey(key, initialValue, mode);
}

/*!
  The destructor destroys the QSystemSemaphore object, but the
  underlying system semaphore is not removed from the system unless
  this instance of QSystemSemaphore is the last one existing for that
  system semaphore.

  Two important side effects of the destructor depend on the system.
  In Windows, if acquire() has been called for this semaphore but not
  release(), release() will not be called by the destructor, nor will
  the resource be released when the process exits normally. This would
  be a program bug which could be the cause of a deadlock in another
  process trying to acquire the same resource. In Unix, acquired
  resources that are not released before the destructor is called are
  automatically released when the process exits.
*/
QSystemSemaphore::~QSystemSemaphore()
{
   d->cleanHandle();
}

/*!
  \enum QSystemSemaphore::AccessMode

  This enum is used by the constructor and setKey(). Its purpose is to
  enable handling the problem in Unix implementations of semaphores
  that survive a crash. In Unix, when a semaphore survives a crash, we
  need a way to force it to reset its resource count, when the system
  reuses the semaphore. In Windows and in Symbian, where semaphores can't survive a
  crash, this enum has no effect.

  \value Open If the semaphore already exists, its initial resource
  count is not reset. If the semaphore does not already exist, it is
  created and its initial resource count set.

  \value Create QSystemSemaphore takes ownership of the semaphore and
  sets its resource count to the requested value, regardless of
  whether the semaphore already exists by having survived a crash.
  This value should be passed to the constructor, when the first
  semaphore for a particular key is constructed and you know that if
  the semaphore already exists it could only be because of a crash. In
  Windows and in Symbian, where a semaphore can't survive a crash, Create and Open
  have the same behavior.
*/

/*!
  This function works the same as the constructor. It reconstructs
  this QSystemSemaphore object. If the new \a key is different from
  the old key, calling this function is like calling the destructor of
  the semaphore with the old key, then calling the constructor to
  create a new semaphore with the new \a key. The \a initialValue and
  \a mode parameters are as defined for the constructor.

  \sa QSystemSemaphore(), key()
 */
void QSystemSemaphore::setKey(const QString &key, int initialValue, AccessMode mode)
{
   if (key == d->key && mode == Open) {
      return;
   }
   d->error = NoError;
   d->errorString = QString();
#if !defined(Q_OS_WIN) && !defined(QT_POSIX_IPC)
   // optimization to not destroy/create the file & semaphore
   if (key == d->key && mode == Create && d->createdSemaphore && d->createdFile) {
      d->initialValue = initialValue;
      d->unix_key = -1;
      d->handle(mode);
      return;
   }
#endif
   d->cleanHandle();
   d->key = key;
   d->initialValue = initialValue;
   // cache the file name so it doesn't have to be generated all the time.
   d->fileName = d->makeKeyFileName();
   d->handle(mode);
}

/*!
  Returns the key assigned to this system semaphore. The key is the
  name by which the semaphore can be accessed from other processes.

  \sa setKey()
 */
QString QSystemSemaphore::key() const
{
   return d->key;
}

/*!
  Acquires one of the resources guarded by this semaphore, if there is
  one available, and returns true. If all the resources guarded by this
  semaphore have already been acquired, the call blocks until one of
  them is released by another process or thread having a semaphore
  with the same key.

  If false is returned, a system error has occurred. Call error()
  to get a value of QSystemSemaphore::SystemSemaphoreError that
  indicates which error occurred.

  \sa release()
 */
bool QSystemSemaphore::acquire()
{
   return d->modifySemaphore(-1);
}

/*!
  Releases \a n resources guarded by the semaphore. Returns true
  unless there is a system error.

  Example: Create a system semaphore having five resources; acquire
  them all and then release them all.

  \snippet doc/src/snippets/code/src_corelib_kernel_qsystemsemaphore.cpp 1

  This function can also "create" resources. For example, immediately
  following the sequence of statements above, suppose we add the
  statement:

  \snippet doc/src/snippets/code/src_corelib_kernel_qsystemsemaphore.cpp 2

  Ten new resources are now guarded by the semaphore, in addition to
  the five that already existed. You would not normally use this
  function to create more resources.

  \sa acquire()
 */
bool QSystemSemaphore::release(int n)
{
   if (n == 0) {
      return true;
   }
   if (n < 0) {
      qWarning("QSystemSemaphore::release: n is negative.");
      return false;
   }
   return d->modifySemaphore(n);
}

/*!
  Returns a value indicating whether an error occurred, and, if so,
  which error it was.

  \sa errorString()
 */
QSystemSemaphore::SystemSemaphoreError QSystemSemaphore::error() const
{
   return d->error;
}

/*!
  \enum QSystemSemaphore::SystemSemaphoreError

  \value NoError No error occurred.

  \value PermissionDenied The operation failed because the caller
  didn't have the required permissions.

  \value KeyError The operation failed because of an invalid key.

  \value AlreadyExists The operation failed because a system
  semaphore with the specified key already existed.

  \value NotFound The operation failed because a system semaphore
  with the specified key could not be found.

  \value OutOfResources The operation failed because there was
  not enough memory available to fill the request.

  \value UnknownError Something else happened and it was bad.
*/

/*!
  Returns a text description of the last error that occurred. If
  error() returns an \l {QSystemSemaphore::SystemSemaphoreError} {error
  value}, call this function to get a text string that describes the
  error.

  \sa error()
 */
QString QSystemSemaphore::errorString() const
{
   return d->errorString;
}

#endif // QT_NO_SYSTEMSEMAPHORE

QT_END_NAMESPACE
