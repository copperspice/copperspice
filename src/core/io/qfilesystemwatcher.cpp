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

#include <qfilesystemwatcher.h>
#include <qfilesystemwatcher_p.h>

#ifndef QT_NO_FILESYSTEMWATCHER

#include <qdatetime.h>
#include <qdebug.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qmutex.h>
#include <qset.h>
#include <qtimer.h>

#if defined(Q_OS_WIN)
#  include <qfilesystemwatcher_win_p.h>

#elif defined(Q_OS_LINUX)
#  include <qfilesystemwatcher_inotify_p.h>
#  include <qfilesystemwatcher_dnotify_p.h>

#elif defined(Q_OS_FREEBSD) || defined(Q_OS_MAC)

#  if (defined Q_OS_MAC)
#  include <qfilesystemwatcher_fsevents_p.h>
#  endif

#  include <qfilesystemwatcher_kqueue_p.h>

#endif

QT_BEGIN_NAMESPACE

enum { PollingInterval = 1000 };

class QPollingFileSystemWatcherEngine : public QFileSystemWatcherEngine
{
   CORE_CS_OBJECT(QPollingFileSystemWatcherEngine)

   class FileInfo
   {
      uint ownerId;
      uint groupId;
      QFile::Permissions permissions;
      QDateTime lastModified;
      QStringList entries;

    public:
      FileInfo(const QFileInfo &fileInfo)
         : ownerId(fileInfo.ownerId()),
           groupId(fileInfo.groupId()),
           permissions(fileInfo.permissions()),
           lastModified(fileInfo.lastModified()) {
         if (fileInfo.isDir()) {
            entries = fileInfo.absoluteDir().entryList(QDir::AllEntries);
         }
      }
      FileInfo &operator=(const QFileInfo &fileInfo) {
         *this = FileInfo(fileInfo);
         return *this;
      }

      bool operator!=(const QFileInfo &fileInfo) const {
         if (fileInfo.isDir() && entries != fileInfo.absoluteDir().entryList(QDir::AllEntries)) {
            return true;
         }
         return (ownerId != fileInfo.ownerId()
                 || groupId != fileInfo.groupId()
                 || permissions != fileInfo.permissions()
                 || lastModified != fileInfo.lastModified());
      }
   };

   mutable QMutex mutex;
   QHash<QString, FileInfo> files, directories;

 public:
   QPollingFileSystemWatcherEngine();

   void run()  override;

   QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories)  override;
   QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories) override;

   void stop() override;

 private:

   CORE_CS_SLOT_1(Private, void timeout())
   CORE_CS_SLOT_2(timeout)

};

QPollingFileSystemWatcherEngine::QPollingFileSystemWatcherEngine()
{
   moveToThread(this);
}

void QPollingFileSystemWatcherEngine::run()
{
   QTimer timer;
   connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
   timer.start(PollingInterval);
   (void) exec();
}

QStringList QPollingFileSystemWatcherEngine::addPaths(const QStringList &paths,
      QStringList *files, QStringList *directories)
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
      if (fi.isDir()) {
         if (!directories->contains(path)) {
            directories->append(path);
         }
         if (!path.endsWith(QLatin1Char('/'))) {
            fi = QFileInfo(path + QLatin1Char('/'));
         }
         this->directories.insert(path, fi);
      } else {
         if (!files->contains(path)) {
            files->append(path);
         }
         this->files.insert(path, fi);
      }
      it.remove();
   }
   start();
   return p;
}

QStringList QPollingFileSystemWatcherEngine::removePaths(const QStringList &paths,
      QStringList *files,
      QStringList *directories)
{
   QMutexLocker locker(&mutex);
   QStringList p = paths;
   QMutableListIterator<QString> it(p);
   while (it.hasNext()) {
      QString path = it.next();
      if (this->directories.remove(path)) {
         directories->removeAll(path);
         it.remove();
      } else if (this->files.remove(path)) {
         files->removeAll(path);
         it.remove();
      }
   }
   if (this->files.isEmpty() && this->directories.isEmpty()) {
      locker.unlock();
      stop();
      wait();
   }
   return p;
}

void QPollingFileSystemWatcherEngine::stop()
{
   quit();
}

void QPollingFileSystemWatcherEngine::timeout()
{
   QMutexLocker locker(&mutex);
   QMutableHashIterator<QString, FileInfo> fit(files);
   while (fit.hasNext()) {
      QHash<QString, FileInfo>::iterator x = fit.next();
      QString path = x.key();
      QFileInfo fi(path);
      if (!fi.exists()) {
         fit.remove();
         emit fileChanged(path, true);
      } else if (x.value() != fi) {
         x.value() = fi;
         emit fileChanged(path, false);
      }
   }
   QMutableHashIterator<QString, FileInfo> dit(directories);
   while (dit.hasNext()) {
      QHash<QString, FileInfo>::iterator x = dit.next();
      QString path = x.key();
      QFileInfo fi(path);
      if (!path.endsWith(QLatin1Char('/'))) {
         fi = QFileInfo(path + QLatin1Char('/'));
      }
      if (!fi.exists()) {
         dit.remove();
         emit directoryChanged(path, true);
      } else if (x.value() != fi) {
         fi.refresh();
         if (!fi.exists()) {
            dit.remove();
            emit directoryChanged(path, true);
         } else {
            x.value() = fi;
            emit directoryChanged(path, false);
         }
      }

   }
}




QFileSystemWatcherEngine *QFileSystemWatcherPrivate::createNativeEngine()
{
#if defined(Q_OS_WIN)
   return new QWindowsFileSystemWatcherEngine;

#elif defined(Q_OS_LINUX)
   QFileSystemWatcherEngine *eng = QInotifyFileSystemWatcherEngine::create();

   if (! eng) {
      eng = QDnotifyFileSystemWatcherEngine::create();
   }
   return eng;

#elif defined(Q_OS_FREEBSD) || defined(Q_OS_MAC)
   return QKqueueFileSystemWatcherEngine::create();

#else
   return 0;

#endif
}

QFileSystemWatcherPrivate::QFileSystemWatcherPrivate()
   : native(0), poller(0), forced(0)
{
}

void QFileSystemWatcherPrivate::init()
{
   Q_Q(QFileSystemWatcher);
   native = createNativeEngine();

   if (native) {
      QObject::connect(native, SIGNAL(fileChanged(const QString &, bool)), q, SLOT(_q_fileChanged(const QString &, bool)));
      QObject::connect(native, SIGNAL(directoryChanged(const QString &, bool)), q, SLOT(_q_directoryChanged(const QString &,
                       bool)));
   }
}

void QFileSystemWatcherPrivate::initForcedEngine(const QString &forceName)
{
   if (forced) {
      return;
   }

   Q_Q(QFileSystemWatcher);

#if defined(Q_OS_LINUX)
   if (forceName == QLatin1String("inotify")) {
      forced = QInotifyFileSystemWatcherEngine::create();

   } else if (forceName == QLatin1String("dnotify")) {
      forced = QDnotifyFileSystemWatcherEngine::create();
   }

#else
   Q_UNUSED(forceName);

#endif

   if (forced) {
      QObject::connect(forced, SIGNAL(fileChanged(const QString &, bool)), q, SLOT(_q_fileChanged(const QString &, bool)));
      QObject::connect(forced, SIGNAL(directoryChanged(const QString &, bool)), q, SLOT(_q_directoryChanged(const QString &,
                       bool)));
   }
}

void QFileSystemWatcherPrivate::initPollerEngine()
{
   if (poller) {
      return;
   }

   Q_Q(QFileSystemWatcher);
   poller = new QPollingFileSystemWatcherEngine; // that was a mouthful
   QObject::connect(poller, SIGNAL(fileChanged(const QString &, bool)), q, SLOT(_q_fileChanged(const QString &, bool)));
   QObject::connect(poller, SIGNAL(directoryChanged(const QString &, bool)), q, SLOT(_q_directoryChanged(const QString &,
                    bool)));
}

void QFileSystemWatcherPrivate::_q_fileChanged(const QString &path, bool removed)
{
   Q_Q(QFileSystemWatcher);

   if (!files.contains(path)) {
      // the path was removed after a change was detected, but before we delivered the signal
      return;
   }

   if (removed) {
      files.removeAll(path);
   }
   emit q->fileChanged(path);
}

void QFileSystemWatcherPrivate::_q_directoryChanged(const QString &path, bool removed)
{
   Q_Q(QFileSystemWatcher);
   if (!directories.contains(path)) {
      // perhaps the path was removed after a change was detected, but before we delivered the signal
      return;
   }
   if (removed) {
      directories.removeAll(path);
   }
   emit q->directoryChanged(path);
}

void QFileSystemWatcher::_q_fileChanged(const QString &path, bool removed)
{
   Q_D(QFileSystemWatcher);
   d->_q_fileChanged(path, removed);
}

void QFileSystemWatcher::_q_directoryChanged(const QString &path, bool removed)
{
   Q_D(QFileSystemWatcher);
   d->_q_directoryChanged(path, removed);
}


/*!
    \class QFileSystemWatcher
    \brief The QFileSystemWatcher class provides an interface for monitoring files and directories for modifications.
    \ingroup io
    \since 4.2
    \reentrant

    QFileSystemWatcher monitors the file system for changes to files
    and directories by watching a list of specified paths.

    Call addPath() to watch a particular file or directory. Multiple
    paths can be added using the addPaths() function. Existing paths can
    be removed by using the removePath() and removePaths() functions.

    QFileSystemWatcher examines each path added to it. Files that have
    been added to the QFileSystemWatcher can be accessed using the
    files() function, and directories using the directories() function.

    The fileChanged() signal is emitted when a file has been modified,
    renamed or removed from disk. Similarly, the directoryChanged()
    signal is emitted when a directory or its contents is modified or
    removed.  Note that QFileSystemWatcher stops monitoring files once
    they have been renamed or removed from disk, and directories once
    they have been removed from disk.

    \note On systems running a Linux kernel without inotify support,
    file systems that contain watched paths cannot be unmounted.

    \note Windows CE does not support directory monitoring by
    default as this depends on the file system driver installed.

    \note The act of monitoring files and directories for
    modifications consumes system resources. This implies there is a
    limit to the number of files and directories your process can
    monitor simultaneously. On Mac OS X 10.4 and all BSD variants, for
    example, an open file descriptor is required for each monitored
    file. Some system limits the number of open file descriptors to 256
    by default. This means that addPath() and addPaths() will fail if
    your process tries to add more than 256 files or directories to
    the file system monitor. Also note that your process may have
    other file descriptors open in addition to the ones for files
    being monitored, and these other open descriptors also count in
    the total. Mac OS X 10.5 and up use a different backend and do not
    suffer from this issue.


    \sa QFile, QDir
*/


/*!
    Constructs a new file system watcher object with the given \a parent.
*/
QFileSystemWatcher::QFileSystemWatcher(QObject *parent)
   : QObject(parent), d_ptr(new QFileSystemWatcherPrivate)
{
   d_ptr->q_ptr = this;
   d_func()->init();
}

/*!
    Constructs a new file system watcher object with the given \a parent
    which monitors the specified \a paths list.
*/
QFileSystemWatcher::QFileSystemWatcher(const QStringList &paths, QObject *parent)
   : QObject(parent), d_ptr(new QFileSystemWatcherPrivate)
{
   d_ptr->q_ptr = this;
   d_func()->init();

   addPaths(paths);
}

/*!
    Destroys the file system watcher.
*/
QFileSystemWatcher::~QFileSystemWatcher()
{
   Q_D(QFileSystemWatcher);
   if (d->native) {
      d->native->stop();
      d->native->wait();
      delete d->native;
      d->native = 0;
   }
   if (d->poller) {
      d->poller->stop();
      d->poller->wait();
      delete d->poller;
      d->poller = 0;
   }
   if (d->forced) {
      d->forced->stop();
      d->forced->wait();
      delete d->forced;
      d->forced = 0;
   }
}

/*!
    Adds \a path to the file system watcher if \a path exists. The
    path is not added if it does not exist, or if it is already being
    monitored by the file system watcher.

    If \a path specifies a directory, the directoryChanged() signal
    will be emitted when \a path is modified or removed from disk;
    otherwise the fileChanged() signal is emitted when \a path is
    modified, renamed or removed.

    \note There is a system dependent limit to the number of files and
    directories that can be monitored simultaneously. If this limit
    has been reached, \a path will not be added to the file system
    watcher, and a warning message will be printed to \e{stderr}.

    \sa addPaths(), removePath()
*/
void QFileSystemWatcher::addPath(const QString &path)
{
   if (path.isEmpty()) {
      qWarning("QFileSystemWatcher::addPath: path is empty");
      return;
   }
   addPaths(QStringList(path));
}

/*!
    Adds each path in \a paths to the file system watcher. Paths are
    not added if they not exist, or if they are already being
    monitored by the file system watcher.

    If a path specifies a directory, the directoryChanged() signal
    will be emitted when the path is modified or removed from disk;
    otherwise the fileChanged() signal is emitted when the path is
    modified, renamed, or removed.

    \note There is a system dependent limit to the number of files and
    directories that can be monitored simultaneously. If this limit
    has been reached, the excess \a paths will not be added to the
    file system watcher, and a warning message will be printed to
    \e{stderr} for each path that could not be added.

    \sa addPath(), removePaths()
*/
void QFileSystemWatcher::addPaths(const QStringList &paths)
{
   Q_D(QFileSystemWatcher);
   if (paths.isEmpty()) {
      qWarning("QFileSystemWatcher::addPaths: list is empty");
      return;
   }

   QStringList p = paths;
   QFileSystemWatcherEngine *engine = 0;

   if (!objectName().startsWith(QLatin1String("_qt_autotest_force_engine_"))) {
      // Normal runtime case - search intelligently for best engine
      if (d->native) {
         engine = d->native;
      } else {
         d_func()->initPollerEngine();
         engine = d->poller;
      }

   } else {
      // Autotest override case - use the explicitly selected engine only
      QString forceName = objectName().mid(26);
      if (forceName == QLatin1String("poller")) {
         qDebug() << "QFileSystemWatcher: skipping native engine, using only polling engine";
         d_func()->initPollerEngine();
         engine = d->poller;
      } else if (forceName == QLatin1String("native")) {
         qDebug() << "QFileSystemWatcher: skipping polling engine, using only native engine";
         engine = d->native;
      } else {
         qDebug() << "QFileSystemWatcher: skipping polling and native engine, using only explicit" << forceName << "engine";
         d_func()->initForcedEngine(forceName);
         engine = d->forced;
      }
   }

   if (engine) {
      p = engine->addPaths(p, &d->files, &d->directories);
   }

   if (!p.isEmpty())
      qWarning("QFileSystemWatcher: failed to add paths: %s",
               qPrintable(p.join(QLatin1String(", "))));
}

/*!
    Removes the specified \a path from the file system watcher.

    \sa removePaths(), addPath()
*/
void QFileSystemWatcher::removePath(const QString &path)
{
   if (path.isEmpty()) {
      qWarning("QFileSystemWatcher::removePath: path is empty");
      return;
   }
   removePaths(QStringList(path));
}

/*!
    Removes the specified \a paths from the file system watcher.

    \sa removePath(), addPaths()
*/
void QFileSystemWatcher::removePaths(const QStringList &paths)
{
   if (paths.isEmpty()) {
      qWarning("QFileSystemWatcher::removePaths: list is empty");
      return;
   }
   Q_D(QFileSystemWatcher);
   QStringList p = paths;
   if (d->native) {
      p = d->native->removePaths(p, &d->files, &d->directories);
   }
   if (d->poller) {
      p = d->poller->removePaths(p, &d->files, &d->directories);
   }
   if (d->forced) {
      p = d->forced->removePaths(p, &d->files, &d->directories);
   }
}

/*!
    \fn void QFileSystemWatcher::fileChanged(const QString &path)

    This signal is emitted when the file at the specified \a path is
    modified, renamed or removed from disk.

    \sa directoryChanged()
*/

/*!
    \fn void QFileSystemWatcher::directoryChanged(const QString &path)

    This signal is emitted when the directory at a specified \a path,
    is modified (e.g., when a file is added, modified or deleted) or
    removed from disk. Note that if there are several changes during a
    short period of time, some of the changes might not emit this
    signal. However, the last change in the sequence of changes will
    always generate this signal.

    \sa fileChanged()
*/

/*!
    \fn QStringList QFileSystemWatcher::directories() const

    Returns a list of paths to directories that are being watched.

    \sa files()
*/

/*!
    \fn QStringList QFileSystemWatcher::files() const

    Returns a list of paths to files that are being watched.

    \sa directories()
*/

QStringList QFileSystemWatcher::directories() const
{
   Q_D(const QFileSystemWatcher);
   return d->directories;
}

QStringList QFileSystemWatcher::files() const
{
   Q_D(const QFileSystemWatcher);
   return d->files;
}

QT_END_NAMESPACE

#endif // QT_NO_FILESYSTEMWATCHER

