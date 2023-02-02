/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#include <qsharedmemory.h>
#include <qsharedmemory_p.h>
#include <qsystemsemaphore.h>
#include <qdir.h>
#include <qcryptographichash.h>
#include <qregularexpression.h>

#include <qdebug.h>

#if !(defined(QT_NO_SHAREDMEMORY) && defined(QT_NO_SYSTEMSEMAPHORE))
/*!
    \internal

    Generate a string from the key which can be any unicode string into
    the subset that the win/unix kernel allows.

    On Unix this will be a file name.
  */
QString QSharedMemoryPrivate::makePlatformSafeKey(const QString &key, const QString &prefix)
{
   if (key.isEmpty()) {
      return QString();
   }

   QString result = prefix;

   QString part1 = key;
   part1.replace(QRegularExpression8("[^A-Za-z]"), QString());

   result.append(part1);

   QByteArray hex = QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha1).toHex();
   result.append(QString(hex));

#ifdef Q_OS_WIN
   return result;

#elif defined(QT_POSIX_IPC)
   return '/' + result;

#else
   return QDir::tempPath() + '/' + result;

#endif

}
#endif // QT_NO_SHAREDMEMORY && QT_NO_SHAREDMEMORY

#ifndef QT_NO_SHAREDMEMORY

QSharedMemory::QSharedMemory(QObject *parent)
   : QObject(parent), d_ptr(new QSharedMemoryPrivate)
{
   d_ptr->q_ptr = this;
}


QSharedMemory::QSharedMemory(const QString &key, QObject *parent)
   : QObject(parent), d_ptr(new QSharedMemoryPrivate)
{
   d_ptr->q_ptr = this;
   setKey(key);
}

QSharedMemory::~QSharedMemory()
{
   setKey(QString());
}

void QSharedMemory::setKey(const QString &key)
{
   Q_D(QSharedMemory);
   if (key == d->key && d->makePlatformSafeKey(key) == d->nativeKey) {
      return;
   }

   if (isAttached()) {
      detach();
   }

   d->cleanHandle();
   d->key = key;
   d->nativeKey = d->makePlatformSafeKey(key);
}

void QSharedMemory::setNativeKey(const QString &key)
{
   Q_D(QSharedMemory);

   if (key == d->nativeKey && d->key.isEmpty()) {
      return;
   }

   if (isAttached()) {
      detach();
   }

   d->cleanHandle();
   d->key.clear();
   d->nativeKey = key;
}

bool QSharedMemoryPrivate::initKey()
{
   cleanHandle();

#ifndef QT_NO_SYSTEMSEMAPHORE
   systemSemaphore.setKey(QString(), 1);
   systemSemaphore.setKey(key, 1);

   if (systemSemaphore.error() != QSystemSemaphore::NoError) {
      QString function = "QSharedMemoryPrivate::initKey";

      errorString = QSharedMemory::tr("%1: unable to set key on lock").formatArg(function);

      switch (systemSemaphore.error()) {
         case QSystemSemaphore::PermissionDenied:
            error = QSharedMemory::PermissionDenied;
            break;
         case QSystemSemaphore::KeyError:
            error = QSharedMemory::KeyError;
            break;
         case QSystemSemaphore::AlreadyExists:
            error = QSharedMemory::AlreadyExists;
            break;
         case QSystemSemaphore::NotFound:
            error = QSharedMemory::NotFound;
            break;
         case QSystemSemaphore::OutOfResources:
            error = QSharedMemory::OutOfResources;
            break;
         case QSystemSemaphore::UnknownError:
         default:
            error = QSharedMemory::UnknownError;
            break;
      }
      return false;
   }
#endif
   errorString.clear();
   error = QSharedMemory::NoError;
   return true;
}

/*!
  Returns the key assigned with setKey() to this shared memory, or a null key
  if no key has been assigned, or if the segment is using a nativeKey(). The
  key is the identifier used by Qt applications to identify the shared memory
  segment.

  You can find the native, platform specific, key used by the operating system
  by calling nativeKey().

  \sa setKey() setNativeKey()
 */
QString QSharedMemory::key() const
{
   Q_D(const QSharedMemory);
   return d->key;
}

QString QSharedMemory::nativeKey() const
{
   Q_D(const QSharedMemory);
   return d->nativeKey;
}

bool QSharedMemory::create(int size, AccessMode mode)
{
   Q_D(QSharedMemory);

   if (! d->initKey()) {
      return false;
   }

   if (size <= 0) {
      d->error = QSharedMemory::InvalidSize;
      d->errorString = QSharedMemory::tr("%1: Create size is less than zero").formatArg("QSharedMemory::create");
      return false;
   }

#ifndef QT_NO_SYSTEMSEMAPHORE

#ifndef Q_OS_WIN
   // Take ownership and force set initialValue because the semaphore
   // might have already existed from a previous crash.
   d->systemSemaphore.setKey(d->key, 1, QSystemSemaphore::Create);
#endif

   QSharedMemoryLocker lock(this);

   if (! d->key.isEmpty() && ! d->tryLocker(&lock, "QSharedMemory::create")) {
      return false;
   }
#endif

   if (!d->create(size)) {
      return false;
   }

   return d->attach(mode);
}

/*!
  Returns the size of the attached shared memory segment. If no shared
  memory segment is attached, 0 is returned.

  \sa create() attach()
 */
int QSharedMemory::size() const
{
   Q_D(const QSharedMemory);
   return d->size;
}

bool QSharedMemory::attach(AccessMode mode)
{
   Q_D(QSharedMemory);

   if (isAttached() || !d->initKey()) {
      return false;
   }
#ifndef QT_NO_SYSTEMSEMAPHORE
   QSharedMemoryLocker lock(this);

   if (! d->key.isEmpty() && !d->tryLocker(&lock, "QSharedMemory::attach")) {
      return false;
   }
#endif

   if (isAttached() || !d->handle()) {
      return false;
   }

   return d->attach(mode);
}

/*!
  Returns true if this process is attached to the shared memory
  segment.

  \sa attach(), detach()
 */
bool QSharedMemory::isAttached() const
{
   Q_D(const QSharedMemory);
   return (d->memory != nullptr);
}

/*!
  Detaches the process from the shared memory segment. If this was the
  last process attached to the shared memory segment, then the shared
  memory segment is released by the system, i.e., the contents are
  destroyed. The function returns true if it detaches the shared
  memory segment. If it returns false, it usually means the segment
  either isn't attached, or it is locked by another process.

  \sa attach(), isAttached()
 */
bool QSharedMemory::detach()
{
   Q_D(QSharedMemory);
   if (!isAttached()) {
      return false;
   }

#ifndef QT_NO_SYSTEMSEMAPHORE
   QSharedMemoryLocker lock(this);

   if (! d->key.isEmpty() && !d->tryLocker(&lock, "QSharedMemory::detach")) {
      return false;
   }
#endif

   return d->detach();
}

/*!
  Returns a pointer to the contents of the shared memory segment, if
  one is attached. Otherwise it returns null. Remember to lock the
  shared memory with lock() before reading from or writing to the
  shared memory, and remember to release the lock with unlock() after
  you are done.

  \sa attach()
 */
void *QSharedMemory::data()
{
   Q_D(QSharedMemory);
   return d->memory;
}

/*!
  Returns a const pointer to the contents of the shared memory
  segment, if one is attached. Otherwise it returns null. Remember to
  lock the shared memory with lock() before reading from or writing to
  the shared memory, and remember to release the lock with unlock()
  after you are done.

  \sa attach() create()
 */
const void *QSharedMemory::constData() const
{
   Q_D(const QSharedMemory);
   return d->memory;
}

/*!
  \overload data()
 */
const void *QSharedMemory::data() const
{
   Q_D(const QSharedMemory);
   return d->memory;
}

#ifndef QT_NO_SYSTEMSEMAPHORE
/*!
  This is a semaphore that locks the shared memory segment for access
  by this process and returns true. If another process has locked the
  segment, this function blocks until the lock is released. Then it
  acquires the lock and returns true. If this function returns false,
  it means that you have ignored a false return from create() or attach(),
  that you have set the key with setNativeKey() or that
  QSystemSemaphore::acquire() failed due to an unknown system error.

  \sa unlock(), data(), QSystemSemaphore::acquire()
 */
bool QSharedMemory::lock()
{
   Q_D(QSharedMemory);
   if (d->lockedByMe) {
      qWarning("QSharedMemory::lock: already locked");
      return true;
   }
   if (d->systemSemaphore.acquire()) {
      d->lockedByMe = true;
      return true;
   }

   QString function = "QSharedMemory::lock";
   d->errorString = QSharedMemory::tr("%1: unable to lock").formatArg(function);
   d->error = QSharedMemory::LockError;

   return false;
}

/*!
  Releases the lock on the shared memory segment and returns true, if
  the lock is currently held by this process. If the segment is not
  locked, or if the lock is held by another process, nothing happens
  and false is returned.

  \sa lock()
 */
bool QSharedMemory::unlock()
{
   Q_D(QSharedMemory);

   if (!d->lockedByMe) {
      return false;
   }

   d->lockedByMe = false;
   if (d->systemSemaphore.release()) {
      return true;
   }

   QString function = "QSharedMemory::unlock";
   d->errorString = QSharedMemory::tr("%1: unable to unlock").formatArg(function);
   d->error = QSharedMemory::LockError;

   return false;
}
#endif // QT_NO_SYSTEMSEMAPHORE

QSharedMemory::SharedMemoryError QSharedMemory::error() const
{
   Q_D(const QSharedMemory);
   return d->error;
}

QString QSharedMemory::errorString() const
{
   Q_D(const QSharedMemory);
   return d->errorString;
}

#endif // QT_NO_SHAREDMEMORY

