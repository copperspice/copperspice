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

/*****************************************************************
** Copyright (c) 2013 David Faure <faure+bluesystems@kde.org>
*****************************************************************/

#include <qlockfile_p.h>

#include <qcache.h>
#include <qcoreapplication.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qfileinfo.h>
#include <qmutex.h>
#include <qtemporaryfile.h>

#include <qcore_unix_p.h>
#include <qabstractfileengine_p.h>
#include <qtemporaryfile_p.h>

#include <signal.h>    // kill
#include <sys/file.h>  // flock
#include <sys/types.h> // kill
#include <unistd.h>    // gethostname

#if defined(Q_OS_DARWIN)
#   include <libproc.h>

#elif defined(Q_OS_LINUX)
#   include <unistd.h>
#   include <cstdio>

#elif defined(Q_OS_BSD4)
#   include <sys/cdefs.h>
#   include <sys/param.h>
#   include <sys/sysctl.h>

# if ! defined(Q_OS_NETBSD)
#   include <sys/user.h>
# endif

#endif

static QByteArray localHostName()
{
   QByteArray hostName(512, Qt::NoData);

   if (gethostname(hostName.data(), hostName.size()) == -1) {
      return QByteArray();
   }

   hostName.truncate(strlen(hostName.data()));

   return hostName;
}

// ### merge into qt_safe_write?
static qint64 qt_write_loop(int fd, const char *data, qint64 len)
{
   qint64 pos = 0;

   while (pos < len) {
      const qint64 ret = qt_safe_write(fd, data + pos, len - pos);

      if (ret == -1) {
         // for example, partition full
         return pos;
      }

      pos += ret;
   }

   return pos;
}

int QLockFilePrivate::checkFcntlWorksAfterFlock(const QString &fn)
{
#ifndef QT_NO_TEMPORARYFILE
   QTemporaryFile file(fn);

   if (! file.open()) {
      return 0;
   }

   const int fd = file.d_func()->engine()->handle();

#if defined(LOCK_EX) && defined(LOCK_NB)

   if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
      // other threads, and other processes on a local fs
      return 0;
   }

#endif

   struct flock flockData;
   flockData.l_type   = F_WRLCK;
   flockData.l_whence = SEEK_SET;
   flockData.l_start  = 0;
   flockData.l_len    = 0; // 0 = entire file
   flockData.l_pid    = getpid();

   if (fcntl(fd, F_SETLK, &flockData) == -1) {
      // for networked filesystems
      return 0;
   }

   return 1;

#else
   return 0;
#endif
}

// Cache the result of checkFcntlWorksAfterFlock for each directory a lock file is
// created in because in some filesystems, like NFS, both locks are the same.
// This does not take into account a filesystem changing. QCache is set to hold a
// maximum of 10 entries, this is to avoid unbounded growth, this is caching directories
// of files and it is assumed a low number will be sufficient.

static QCache<QString, bool> *fcntlOK()
{
   static QCache<QString, bool> retval(10);
   return &retval;
}

static QMutex fcntlLock;

/*!
  \internal
  Checks that the OS is not using POSIX locks to emulate flock(), this happens on macos
*/
static bool fcntlWorksAfterFlock(const QString &fn)
{
   QMutexLocker lock(&fcntlLock);

   bool *worksPtr = fcntlOK()->object(fn);

   if (worksPtr) {
      return *worksPtr;
   }

   const bool val = QLockFilePrivate::checkFcntlWorksAfterFlock(fn);
   worksPtr = new bool(val);
   fcntlOK()->insert(fn, worksPtr);

   return val;
}

static bool setNativeLocks(const QString &fileName, int fd)
{
#if defined(LOCK_EX) && defined(LOCK_NB)

   if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
      // other threads, and other processes on a local fs
      return false;
   }

#endif

   struct flock flockData;
   flockData.l_type   = F_WRLCK;
   flockData.l_whence = SEEK_SET;
   flockData.l_start  = 0;
   flockData.l_len    = 0; // 0 = entire file
   flockData.l_pid    = getpid();

   if (fcntlWorksAfterFlock(QDir::cleanPath(QFileInfo(fileName).absolutePath()) + '/') &&
         fcntl(fd, F_SETLK, &flockData) == -1) {
      // for networked filesystems

      return false;
   }

   return true;
}

QLockFile::LockError QLockFilePrivate::tryLock_sys()
{
   // Assemble data, to write in a single call to write
   // (otherwise we'd have to check every write call)
   // Use operator% from the fast builder to avoid multiple memory allocations.

   QByteArray fileData = QByteArray::number(QCoreApplication::applicationPid()) + '\n'
         + QCoreApplication::applicationName().toUtf8() + '\n' + localHostName() + '\n';

   const QByteArray lockFileName = QFile::encodeName(fileName);
   const int fd = qt_safe_open(lockFileName.constData(), O_WRONLY | O_CREAT | O_EXCL, 0666);

   if (fd < 0) {
      switch (errno) {
         case EEXIST:
            return QLockFile::LockFailedError;

         case EACCES:
         case EROFS:
            return QLockFile::PermissionError;

         default:
            return QLockFile::UnknownError;
      }
   }

   // Ensure nobody else can delete the file while we have it
   if (! setNativeLocks(fileName, fd)) {
      const int errnoSaved = errno;
      qWarning() << "QLockFile::tryLock_sys() setNativeLocks failed, " << qt_error_string(errnoSaved);
   }

   if (qt_write_loop(fd, fileData.constData(), fileData.size()) < fileData.size()) {
      close(fd);

      if (! QFile::remove(fileName)) {
         qWarning("QLockFile::tryLock_sys() Unable to remove lock file %s", csPrintable(fileName));
      }

      return QLockFile::UnknownError; // partition full
   }

   // We hold the lock, continue.
   fileHandle = fd;

   // Sync to disk if possible. Ignore errors (e.g. not supported).
#if defined(_POSIX_SYNCHRONIZED_IO) && _POSIX_SYNCHRONIZED_IO > 0
   fdatasync(fileHandle);
#else
   fsync(fileHandle);
#endif

   return QLockFile::NoError;
}

bool QLockFilePrivate::removeStaleLock()
{
   const QByteArray lockFileName = QFile::encodeName(fileName);
   const int fd = qt_safe_open(lockFileName.constData(), O_WRONLY, 0666);

   if (fd < 0) {
      // gone already?
      return false;
   }

   bool success = setNativeLocks(fileName, fd) && (::unlink(lockFileName.constData()) == 0);
   close(fd);

   return success;
}

bool QLockFilePrivate::isApparentlyStale() const
{
   qint64 pid;
   QString hostname, appname;

   if (getLockInfo(&pid, &hostname, &appname)) {
      if (hostname.isEmpty() || hostname == QString::fromUtf8(localHostName())) {

         if (::kill(pid, 0) == -1 && errno == ESRCH) {
            // PID does not exist anymore
            return true;
         }

         const QString processName = processNameByPid(pid);

         if (! processName.isEmpty()) {
            QFileInfo fi(appname);

            if (fi.isSymLink()) {
               fi.setFile(fi.symLinkTarget());
            }

            if (processName != fi.fileName()) {
               return true;   // PID got reused by a different application.
            }
         }
      }
   }

   const qint64 age = QFileInfo(fileName).lastModified().msecsTo(QDateTime::currentDateTime());
   return staleLockTime > 0 && age > staleLockTime;
}

QString QLockFilePrivate::processNameByPid(qint64 pid)
{
#if defined(Q_OS_DARWIN)
   char name[1024];
   proc_name(pid, name, sizeof(name) / sizeof(char));

   return QFile::decodeName(name);

#elif defined(Q_OS_LINUX)

   if (! QFile::exists("/proc/version")) {
      return QString();
   }

   char exePath[64];
   char buf[PATH_MAX + 1];
   sprintf(exePath, "/proc/%lld/exe", pid);
   size_t len = (size_t)readlink(exePath, buf, sizeof(buf));

   if (len >= sizeof(buf)) {
      // The pid is gone. Return some invalid process name to fail the test.
      return QString("/ERROR/");
   }

   buf[len] = 0;
   return QFileInfo(QFile::decodeName(buf)).fileName();

#elif defined(Q_OS_BSD4)

# if defined(Q_OS_NETBSD)
   struct kinfo_proc2 kp;
   int mib[6] = { CTL_KERN, KERN_PROC2, KERN_PROC_PID, (int)pid, sizeof(struct kinfo_proc2), 1 };

# elif defined(Q_OS_OPENBSD)
   struct kinfo_proc kp;
   int mib[6] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, (int)pid, sizeof(struct kinfo_proc), 1 };

# else
   struct kinfo_proc kp;
   int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, (int)pid };
# endif

   size_t len = sizeof(kp);
   u_int mib_len = sizeof(mib) / sizeof(u_int);

   if (sysctl(mib, mib_len, &kp, &len, nullptr, 0) < 0) {
      return QString();
   }

# if defined(Q_OS_OPENBSD) || defined(Q_OS_NETBSD)

   if (kp.p_pid != pid) {
      return QString();
   }

   QString name = QFile::decodeName(kp.p_comm);
# else

   if (kp.ki_pid != pid) {
      return QString();
   }

   QString name = QFile::decodeName(kp.ki_comm);
# endif
   return name;

#else
   (void) pid;
   return QString();
#endif
}

void QLockFile::unlock()
{
   Q_D(QLockFile);

   if (! d->isLocked) {
      return;
   }

   close(d->fileHandle);
   d->fileHandle = -1;

   if (! QFile::remove(d->fileName)) {
      qWarning("QLockFile::unlock() Unable to remove lock file %s", csPrintable(d->fileName));
   }

   d->lockError = QLockFile::NoError;
   d->isLocked  = false;
}
