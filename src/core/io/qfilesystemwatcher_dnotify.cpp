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

#include <qfilesystemwatcher.h>
#include <qplatformdefs.h>

#include <qfilesystemwatcher_dnotify_p.h>

#ifndef QT_NO_FILESYSTEMWATCHER

#include <dirent.h>
#include <qcoreapplication.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qmutex.h>
#include <qsocketnotifier.h>
#include <qtimer.h>
#include <qwaitcondition.h>

#include <qcore_unix_p.h>

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifdef QT_LINUXBASE

/* LSB doesn't standardize these */
#define F_NOTIFY       1026
#define DN_ACCESS      0x00000001
#define DN_MODIFY      0x00000002
#define DN_CREATE      0x00000004
#define DN_DELETE      0x00000008
#define DN_RENAME      0x00000010
#define DN_ATTRIB      0x00000020
#define DN_MULTISHOT   0x80000000

#endif

static int qfswd_fileChanged_pipe[2];
static void (*qfswd_old_sigio_handler)(int) = nullptr;
static void (*qfswd_old_sigio_action)(int, siginfo_t *, void *) = nullptr;

static void qfswd_sigio_monitor(int signum, siginfo_t *i, void *v)
{
   qt_safe_write(qfswd_fileChanged_pipe[1], reinterpret_cast<char *>(&i->si_fd), sizeof(int));

   if (qfswd_old_sigio_handler && qfswd_old_sigio_handler != SIG_IGN) {
      qfswd_old_sigio_handler(signum);
   }

   if (qfswd_old_sigio_action) {
      qfswd_old_sigio_action(signum, i, v);
   }
}

class QDnotifySignalThread : public QThread
{
   CORE_CS_OBJECT(QDnotifySignalThread)

 public:
   QDnotifySignalThread();
   virtual ~QDnotifySignalThread();

   void startNotify();

   void run() override;

   CORE_CS_SIGNAL_1(Public, void fdChanged(int data))
   CORE_CS_SIGNAL_2(fdChanged, data)

 private:
   QMutex mutex;
   QWaitCondition wait;
   bool isExecing;

   CORE_CS_SLOT_1(Private, void readFromDnotify())
   CORE_CS_SLOT_2(readFromDnotify)

 protected:
   bool event(QEvent *) override;

};

static QDnotifySignalThread *dnotifySignal()
{
   static QDnotifySignalThread retval;
   return &retval;
}

QDnotifySignalThread::QDnotifySignalThread()
   : isExecing(false)
{
   moveToThread(this);

   qt_safe_pipe(qfswd_fileChanged_pipe, O_NONBLOCK);

   struct sigaction oldAction;
   struct sigaction action;
   memset(&action, 0, sizeof(action));
   action.sa_sigaction = qfswd_sigio_monitor;
   action.sa_flags = SA_SIGINFO;
   ::sigaction(SIGIO, &action, &oldAction);

   if (!(oldAction.sa_flags & SA_SIGINFO)) {
      qfswd_old_sigio_handler = oldAction.sa_handler;
   } else {
      qfswd_old_sigio_action = oldAction.sa_sigaction;
   }
}

QDnotifySignalThread::~QDnotifySignalThread()
{
   if (isRunning()) {
      quit();
      QThread::wait();
   }
}

bool QDnotifySignalThread::event(QEvent *e)
{
   if (e->type() == QEvent::User) {
      QMutexLocker locker(&mutex);
      isExecing = true;
      wait.wakeAll();
      return true;
   } else {
      return QThread::event(e);
   }
}

void QDnotifySignalThread::startNotify()
{
   // Note: All this fancy waiting for the thread to enter its event
   // loop is to avoid nasty messages at app shutdown when the
   // QDnotifySignalThread singleton is deleted
   start();
   mutex.lock();

   while (!isExecing) {
      wait.wait(&mutex);
   }

   mutex.unlock();
}

void QDnotifySignalThread::run()
{
   QSocketNotifier sn(qfswd_fileChanged_pipe[0], QSocketNotifier::Read, this);
   connect(&sn, &QSocketNotifier::activated, this, &QDnotifySignalThread::readFromDnotify);

   QCoreApplication::instance()->postEvent(this, new QEvent(QEvent::User));
   (void) exec();
}

void QDnotifySignalThread::readFromDnotify()
{
   int fd;
   int readrv = qt_safe_read(qfswd_fileChanged_pipe[0], reinterpret_cast<char *>(&fd), sizeof(int));

   // Only expect EAGAIN or EINTR.  Other errors are assumed to be impossible.
   if (readrv != -1) {
      Q_ASSERT(readrv == sizeof(int));
      (void) readrv;

      if (0 == fd) {
         quit();
      } else {
         emit fdChanged(fd);
      }
   }
}

QDnotifyFileSystemWatcherEngine::QDnotifyFileSystemWatcherEngine()
{
   QObject::connect(dnotifySignal(), &QDnotifySignalThread::fdChanged, this,
         &QDnotifyFileSystemWatcherEngine::refresh, Qt::DirectConnection);
}

QDnotifyFileSystemWatcherEngine::~QDnotifyFileSystemWatcherEngine()
{
   QMutexLocker locker(&mutex);

   for (auto iter = fdToDirectory.constBegin(); iter != fdToDirectory.constEnd(); ++iter) {
      qt_safe_close(iter->fd);

      if (iter->parentFd) {
         qt_safe_close(iter->parentFd);
      }
   }
}

QDnotifyFileSystemWatcherEngine *QDnotifyFileSystemWatcherEngine::create()
{
   return new QDnotifyFileSystemWatcherEngine();
}

void QDnotifyFileSystemWatcherEngine::run()
{
   qFatal("QDnotifyFileSystemWatcherEngine thread should not be run");
}

QStringList QDnotifyFileSystemWatcherEngine::addPaths(const QStringList &paths, QStringList *files,
      QStringList *directories)
{
   QMutexLocker locker(&mutex);

   QStringList p = paths;
   QMutableListIterator<QString> it(p);

   while (it.hasNext()) {
      QString path = it.next();

      QFileInfo fi(path);

      if (!fi.exists()) {
         continue;
      }

      bool isDir = fi.isDir();

      if (isDir && directories->contains(path)) {
         continue; // Skip monitored directories
      } else if (!isDir && files->contains(path)) {
         continue; // Skip monitored files
      }

      if (!isDir) {
         path = fi.canonicalPath();
      }

      // Locate the directory entry (creating if needed)
      int fd = pathToFD[path];

      if (fd == 0) {

         QT_DIR *d = QT_OPENDIR(path.toUtf8().constData());

         if (! d) {
            continue;   // Could not open directory
         }

         QT_DIR *parent = nullptr;

         QDir parentDir(path);

         if (! parentDir.isRoot()) {
            parentDir.cdUp();
            parent = QT_OPENDIR(parentDir.path().toUtf8().constData());

            if (! parent) {
               QT_CLOSEDIR(d);
               continue;
            }
         }

         fd = qt_safe_dup(::dirfd(d));
         int parentFd = parent ? qt_safe_dup(::dirfd(parent)) : 0;

         QT_CLOSEDIR(d);

         if (parent) {
            QT_CLOSEDIR(parent);
         }

         Q_ASSERT(fd);

         if (::fcntl(fd, F_SETSIG, SIGIO) || ::fcntl(fd, F_NOTIFY, DN_MODIFY | DN_CREATE | DN_DELETE |
               DN_RENAME | DN_ATTRIB | DN_MULTISHOT) ||
               (parent && ::fcntl(parentFd, F_SETSIG, SIGIO)) ||
               (parent && ::fcntl(parentFd, F_NOTIFY, DN_DELETE | DN_RENAME | DN_MULTISHOT))) {
            continue; // Could not set appropriate flags
         }

         Directory dir;
         dir.path = path;
         dir.fd = fd;
         dir.parentFd = parentFd;

         fdToDirectory.insert(fd, dir);
         pathToFD.insert(path, fd);

         if (parentFd) {
            parentToFD.insert(parentFd, fd);
         }
      }

      Directory &directory = fdToDirectory[fd];

      if (isDir) {
         directory.isMonitored = true;
      } else {
         Directory::File file;
         file.path = fi.filePath();
         file.lastWrite = fi.lastModified();
         directory.files.append(file);
         pathToFD.insert(fi.filePath(), fd);
      }

      it.remove();

      if (isDir) {
         directories->append(path);
      } else {
         files->append(fi.filePath());
      }
   }

   dnotifySignal()->startNotify();

   return p;
}

QStringList QDnotifyFileSystemWatcherEngine::removePaths(const QStringList &paths, QStringList *files,
      QStringList *directories)
{
   QMutexLocker locker(&mutex);

   QStringList p = paths;
   QMutableListIterator<QString> it(p);

   while (it.hasNext()) {

      QString path = it.next();
      int fd = pathToFD.take(path);

      if (! fd) {
         continue;
      }

      Directory &directory = fdToDirectory[fd];
      bool isDir = false;

      if (directory.path == path) {
         isDir = true;
         directory.isMonitored = false;

      } else {
         for (int index = 0; index < directory.files.count(); ++index) {
            if (directory.files.at(index).path == path) {
               directory.files.removeAt(index);
               break;
            }
         }
      }

      if (!directory.isMonitored && directory.files.isEmpty()) {
         // No longer needed
         qt_safe_close(directory.fd);
         pathToFD.remove(directory.path);
         fdToDirectory.remove(fd);
      }

      if (isDir) {
         directories->removeAll(path);
      } else {
         files->removeAll(path);
      }

      it.remove();
   }

   return p;
}

void QDnotifyFileSystemWatcherEngine::refresh(int fd)
{
   QMutexLocker locker(&mutex);

   bool wasParent = false;
   QHash<int, Directory>::iterator iter = fdToDirectory.find(fd);

   if (iter == fdToDirectory.end()) {
      QHash<int, int>::iterator pIter = parentToFD.find(fd);

      if (pIter == parentToFD.end()) {
         return;
      }

      iter = fdToDirectory.find(*pIter);

      if (iter == fdToDirectory.end()) {
         return;
      }

      wasParent = true;
   }

   Directory &directory = *iter;

   if (!wasParent) {
      for (int ii = 0; ii < directory.files.count(); ++ii) {
         Directory::File &file = directory.files[ii];

         if (file.updateInfo()) {
            // Emit signal
            QString filePath = file.path;
            bool removed = !QFileInfo(filePath).exists();

            if (removed) {
               directory.files.removeAt(ii);
               --ii;
            }

            emit fileChanged(filePath, removed);
         }
      }
   }

   if (directory.isMonitored) {
      // Emit signal
      bool removed = !QFileInfo(directory.path).exists();
      QString path = directory.path;

      if (removed) {
         directory.isMonitored = false;
      }

      emit directoryChanged(path, removed);
   }

   if (!directory.isMonitored && directory.files.isEmpty()) {
      qt_safe_close(directory.fd);

      if (directory.parentFd) {
         qt_safe_close(directory.parentFd);
         parentToFD.remove(directory.parentFd);
      }

      fdToDirectory.erase(iter);
   }
}

void QDnotifyFileSystemWatcherEngine::stop()
{
}

bool QDnotifyFileSystemWatcherEngine::Directory::File::updateInfo()
{
   QFileInfo fi(path);

   QDateTime nLastWrite = fi.lastModified();
   uint nOwnerId = fi.ownerId();
   uint nGroupId = fi.groupId();

   QFile::Permissions nPermissions = fi.permissions();

   if (nLastWrite != lastWrite || nOwnerId != ownerId || nGroupId != groupId || nPermissions != permissions) {
      ownerId = nOwnerId;
      groupId = nGroupId;
      permissions = nPermissions;
      lastWrite   = nLastWrite;

      return true;

   } else {
      return false;
   }
}

#endif // QT_NO_FILESYSTEMWATCHER
