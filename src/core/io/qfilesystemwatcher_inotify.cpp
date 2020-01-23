/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qfilesystemwatcher.h>
#include <qfilesystemwatcher_inotify_p.h>

#ifndef QT_NO_FILESYSTEMWATCHER

#include <qcore_unix_p.h>
#include <qdebug.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qsocketnotifier.h>
#include <qvarlengtharray.h>

#if defined(Q_OS_LINUX)
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#if defined(QT_NO_INOTIFY)
#include <linux/types.h>

#if defined(__i386__)
# define __NR_inotify_init      291
# define __NR_inotify_add_watch 292
# define __NR_inotify_rm_watch  293
# define __NR_inotify_init1     332
#elif defined(__x86_64__)
# define __NR_inotify_init      253
# define __NR_inotify_add_watch 254
# define __NR_inotify_rm_watch  255
# define __NR_inotify_init1     294
#elif defined(__powerpc__) || defined(__powerpc64__)
# define __NR_inotify_init      275
# define __NR_inotify_add_watch 276
# define __NR_inotify_rm_watch  277
# define __NR_inotify_init1     318
#elif defined (__ia64__)
# define __NR_inotify_init      1277
# define __NR_inotify_add_watch 1278
# define __NR_inotify_rm_watch  1279
# define __NR_inotify_init1     1318
#elif defined (__s390__) || defined (__s390x__)
# define __NR_inotify_init      284
# define __NR_inotify_add_watch 285
# define __NR_inotify_rm_watch  286
# define __NR_inotify_init1     324
#elif defined (__alpha__)
# define __NR_inotify_init      444
# define __NR_inotify_add_watch 445
# define __NR_inotify_rm_watch  446
// no inotify_init1 for the Alpha
#elif defined (__sparc__) || defined (__sparc64__)
# define __NR_inotify_init      151
# define __NR_inotify_add_watch 152
# define __NR_inotify_rm_watch  156
# define __NR_inotify_init1     322
#elif defined (__arm__)
# define __NR_inotify_init      316
# define __NR_inotify_add_watch 317
# define __NR_inotify_rm_watch  318
# define __NR_inotify_init1     360
#elif defined (__sh__)
# define __NR_inotify_init      290
# define __NR_inotify_add_watch 291
# define __NR_inotify_rm_watch  292
# define __NR_inotify_init1     332
#elif defined (__sh64__)
# define __NR_inotify_init      318
# define __NR_inotify_add_watch 319
# define __NR_inotify_rm_watch  320
# define __NR_inotify_init1     360
#elif defined (__mips__)
# define __NR_inotify_init      284
# define __NR_inotify_add_watch 285
# define __NR_inotify_rm_watch  286
# define __NR_inotify_init1     329
#elif defined (__hppa__)
# define __NR_inotify_init      269
# define __NR_inotify_add_watch 270
# define __NR_inotify_rm_watch  271
# define __NR_inotify_init1     314
#elif defined (__avr32__)
# define __NR_inotify_init	240
# define __NR_inotify_add_watch	241
# define __NR_inotify_rm_watch	242
// no inotify_init1 for AVR32
#elif defined (__mc68000__)
# define __NR_inotify_init      284
# define __NR_inotify_add_watch 285
# define __NR_inotify_rm_watch  286
# define __NR_inotify_init1     328
#elif defined (__aarch64__)
# define __NR_inotify_init1     26
# define __NR_inotify_add_watch 27
# define __NR_inotify_rm_watch  28
// no inotify_init for aarch64
#else
# error "This architecture is not supported."
#endif

#if !defined(IN_CLOEXEC) && defined(O_CLOEXEC) && defined(__NR_inotify_init1)
# define IN_CLOEXEC              O_CLOEXEC
#endif

QT_BEGIN_NAMESPACE

#ifdef QT_LINUXBASE
// ### the LSB doesn't standardize syscall, need to wait until glib2.4 is standardized
static inline int syscall(...)
{
   return -1;
}
#endif

static inline int inotify_init()
{
#ifdef __NR_inotify_init
   return syscall(__NR_inotify_init);
#else
   return syscall(__NR_inotify_init1, 0);
#endif
}

static inline int inotify_add_watch(int fd, const char *name, __u32 mask)
{
   return syscall(__NR_inotify_add_watch, fd, name, mask);
}

static inline int inotify_rm_watch(int fd, __u32 wd)
{
   return syscall(__NR_inotify_rm_watch, fd, wd);
}

#ifdef IN_CLOEXEC
static inline int inotify_init1(int flags)
{
   return syscall(__NR_inotify_init1, flags);
}
#endif

// the following struct and values are documented in linux/inotify.h
extern "C" {

   struct inotify_event {
      __s32           wd;
      __u32           mask;
      __u32           cookie;
      __u32           len;
      char            name[0];
   };

#define IN_ACCESS               0x00000001
#define IN_MODIFY               0x00000002
#define IN_ATTRIB               0x00000004
#define IN_CLOSE_WRITE          0x00000008
#define IN_CLOSE_NOWRITE        0x00000010
#define IN_OPEN                 0x00000020
#define IN_MOVED_FROM           0x00000040
#define IN_MOVED_TO             0x00000080
#define IN_CREATE               0x00000100
#define IN_DELETE               0x00000200
#define IN_DELETE_SELF          0x00000400
#define IN_MOVE_SELF            0x00000800
#define IN_UNMOUNT              0x00002000
#define IN_Q_OVERFLOW           0x00004000
#define IN_IGNORED              0x00008000

#define IN_CLOSE                (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE)
#define IN_MOVE                 (IN_MOVED_FROM | IN_MOVED_TO)
}

QT_END_NAMESPACE

// --------- inotify.h end ----------

#else /* QT_NO_INOTIFY */

#include <sys/inotify.h>

#endif

QT_BEGIN_NAMESPACE

QInotifyFileSystemWatcherEngine *QInotifyFileSystemWatcherEngine::create()
{
   int fd = -1;
#ifdef IN_CLOEXEC
   fd = inotify_init1(IN_CLOEXEC);
#endif
   if (fd == -1) {
      fd = inotify_init();
      if (fd == -1) {
         return 0;
      }
      ::fcntl(fd, F_SETFD, FD_CLOEXEC);
   }
   return new QInotifyFileSystemWatcherEngine(fd);
}

QInotifyFileSystemWatcherEngine::QInotifyFileSystemWatcherEngine(int fd)
   : inotifyFd(fd)
{
   fcntl(inotifyFd, F_SETFD, FD_CLOEXEC);

   moveToThread(this);
}

QInotifyFileSystemWatcherEngine::~QInotifyFileSystemWatcherEngine()
{
   for (int id : pathToID) {
      inotify_rm_watch(inotifyFd, id < 0 ? -id : id);
   }

   ::close(inotifyFd);
}

void QInotifyFileSystemWatcherEngine::run()
{
   QSocketNotifier sn(inotifyFd, QSocketNotifier::Read, this);
   connect(&sn, SIGNAL(activated(int)), this, SLOT(readFromInotify()));
   (void) exec();
}

QStringList QInotifyFileSystemWatcherEngine::addPaths(const QStringList &paths,
      QStringList *files,
      QStringList *directories)
{
   QMutexLocker locker(&mutex);

   QStringList p = paths;
   QMutableListIterator<QString> it(p);
   while (it.hasNext()) {
      QString path = it.next();
      QFileInfo fi(path);
      bool isDir = fi.isDir();
      if (isDir) {
         if (directories->contains(path)) {
            continue;
         }
      } else {
         if (files->contains(path)) {
            continue;
         }
      }

      int wd = inotify_add_watch(inotifyFd, QFile::encodeName(path).constData(),
                  (isDir ? (0 | IN_ATTRIB | IN_MOVE | IN_CREATE | IN_DELETE | IN_DELETE_SELF)
                         : (0 | IN_ATTRIB | IN_MODIFY | IN_MOVE | IN_MOVE_SELF | IN_DELETE_SELF)));

      if (wd <= 0) {
         perror("QInotifyFileSystemWatcherEngine::addPaths: inotify_add_watch failed");
         continue;
      }

      it.remove();

      int id = isDir ? -wd : wd;
      if (id < 0) {
         directories->append(path);
      } else {
         files->append(path);
      }

      pathToID.insert(path, id);
      idToPath.insert(id, path);
   }

   start();

   return p;
}

QStringList QInotifyFileSystemWatcherEngine::removePaths(const QStringList &paths,
      QStringList *files,
      QStringList *directories)
{
   QMutexLocker locker(&mutex);

   QStringList p = paths;
   QMutableListIterator<QString> it(p);
   while (it.hasNext()) {
      QString path = it.next();
      int id = pathToID.take(path);
      QString x = idToPath.take(id);
      if (x.isEmpty() || x != path) {
         continue;
      }

      int wd = id < 0 ? -id : id;
      // qDebug() << "removing watch for path" << path << "wd" << wd;
      inotify_rm_watch(inotifyFd, wd);

      it.remove();
      if (id < 0) {
         directories->removeAll(path);
      } else {
         files->removeAll(path);
      }
   }

   return p;
}

void QInotifyFileSystemWatcherEngine::stop()
{
   quit();
}

void QInotifyFileSystemWatcherEngine::readFromInotify()
{
   QMutexLocker locker(&mutex);

   // qDebug() << "QInotifyFileSystemWatcherEngine::readFromInotify";

   int buffSize = 0;
   ioctl(inotifyFd, FIONREAD, (char *) &buffSize);
   QVarLengthArray<char, 4096> buffer(buffSize);
   buffSize = read(inotifyFd, buffer.data(), buffSize);
   char *at = buffer.data();
   char *const end = at + buffSize;

   QHash<int, inotify_event *> eventForId;
   while (at < end) {
      inotify_event *event = reinterpret_cast<inotify_event *>(at);

      if (eventForId.contains(event->wd)) {
         eventForId[event->wd]->mask |= event->mask;
      } else {
         eventForId.insert(event->wd, event);
      }

      at += sizeof(inotify_event) + event->len;
   }

   QHash<int, inotify_event *>::const_iterator it = eventForId.constBegin();
   while (it != eventForId.constEnd()) {
      const inotify_event &event = **it;
      ++it;

      // qDebug() << "inotify event, wd" << event.wd << "mask" << hex << event.mask;

      int id = event.wd;
      QString path = idToPath.value(id);
      if (path.isEmpty()) {
         // perhaps a directory?
         id = -id;
         path = idToPath.value(id);
         if (path.isEmpty()) {
            continue;
         }
      }

      // qDebug() << "event for path" << path;

      if ((event.mask & (IN_DELETE_SELF | IN_MOVE_SELF | IN_UNMOUNT)) != 0) {
         pathToID.remove(path);
         idToPath.remove(id);
         inotify_rm_watch(inotifyFd, event.wd);

         if (id < 0) {
            emit directoryChanged(path, true);
         } else {
            emit fileChanged(path, true);
         }
      } else {
         if (id < 0) {
            emit directoryChanged(path, false);
         } else {
            emit fileChanged(path, false);
         }
      }
   }
}

QT_END_NAMESPACE

#endif // QT_NO_FILESYSTEMWATCHER
