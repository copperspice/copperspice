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
#include <qfilesystemwatcher_win_p.h>

#ifndef QT_NO_FILESYSTEMWATCHER

#include <qdebug.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qset.h>
#include <qdatetime.h>
#include <qdir.h>

QT_BEGIN_NAMESPACE

void QWindowsFileSystemWatcherEngine::stop()
{
   for (QWindowsFileSystemWatcherEngineThread * thread : threads) {
      thread->stop();
   }
}

QWindowsFileSystemWatcherEngine::QWindowsFileSystemWatcherEngine()
   : QFileSystemWatcherEngine(false)
{
}

QWindowsFileSystemWatcherEngine::~QWindowsFileSystemWatcherEngine()
{
   if (threads.isEmpty()) {
      return;
   }

   for (QWindowsFileSystemWatcherEngineThread * thread : threads) {
      thread->stop();
      thread->wait();
      delete thread;
   }
}

QStringList QWindowsFileSystemWatcherEngine::addPaths(const QStringList &paths,
      QStringList *files, QStringList *directories)
{
   // qDebug()<<"Adding"<<paths.count()<<"to existing"<<(files->count() + directories->count())<<"watchers";
   QStringList p = paths;
   QMutableListIterator<QString> it(p);

   while (it.hasNext()) {
      QString path = it.next();
      QString normalPath = path;

      if ((normalPath.endsWith(QLatin1Char('/')) && !normalPath.endsWith(QLatin1String(":/")))
            || (normalPath.endsWith(QLatin1Char('\\')) && !normalPath.endsWith(QLatin1String(":\\")))  ) {

         normalPath.chop(1);
      }

      QFileInfo fileInfo(normalPath.toLower());
      if (! fileInfo.exists()) {
         continue;
      }

      bool isDir = fileInfo.isDir();
      if (isDir) {
         if (directories->contains(path)) {
            continue;
         }
      } else {
         if (files->contains(path)) {
            continue;
         }
      }

      // qDebug()<<"Looking for a thread/handle for"<<normalPath;

      const QString absolutePath = isDir ? fileInfo.absoluteFilePath() : fileInfo.absolutePath();
      const uint flags = isDir
                         ? (FILE_NOTIFY_CHANGE_DIR_NAME
                            | FILE_NOTIFY_CHANGE_FILE_NAME)
                         : (FILE_NOTIFY_CHANGE_DIR_NAME
                            | FILE_NOTIFY_CHANGE_FILE_NAME
                            | FILE_NOTIFY_CHANGE_ATTRIBUTES
                            | FILE_NOTIFY_CHANGE_SIZE
                            | FILE_NOTIFY_CHANGE_LAST_WRITE
                            | FILE_NOTIFY_CHANGE_SECURITY);

      QWindowsFileSystemWatcherEngine::PathInfo pathInfo;
      pathInfo.absolutePath = absolutePath;
      pathInfo.isDir = isDir;
      pathInfo.path = path;
      pathInfo = fileInfo;

      // Look for a thread
      QWindowsFileSystemWatcherEngineThread *thread = 0;
      QWindowsFileSystemWatcherEngine::Handle handle;
      QList<QWindowsFileSystemWatcherEngineThread *>::const_iterator jt, end;
      end = threads.constEnd();
      for (jt = threads.constBegin(); jt != end; ++jt) {
         thread = *jt;
         QMutexLocker locker(&(thread->mutex));

         handle = thread->handleForDir.value(absolutePath);
         if (handle.handle != INVALID_HANDLE_VALUE && handle.flags == flags) {
            // found a thread now insert...
            // qDebug()<<"  Found a thread"<<thread;

            QHash<QString, QWindowsFileSystemWatcherEngine::PathInfo> &h
               = thread->pathInfoForHandle[handle.handle];
            if (!h.contains(fileInfo.absoluteFilePath())) {
               thread->pathInfoForHandle[handle.handle].insert(fileInfo.absoluteFilePath(), pathInfo);
               if (isDir) {
                  directories->append(path);
               } else {
                  files->append(path);
               }
            }
            it.remove();
            thread->wakeup();
            break;
         }
      }

      // no thread found, first create a handle
      if (handle.handle == INVALID_HANDLE_VALUE || handle.flags != flags) {
         // qDebug()<<"  No thread found";
         // Volume and folder paths need a trailing slash for proper notification
         // (e.g. "c:" -> "c:/").

         const QString effectiveAbsolutePath = isDir ? (absolutePath + '/') : absolutePath;

         handle.handle = FindFirstChangeNotification(&QDir::toNativeSeparators(effectiveAbsolutePath).toStdWString()[0],
                  false, flags);

         handle.flags  = flags;

         if (handle.handle == INVALID_HANDLE_VALUE) {
            continue;
         }

         // now look for a thread to insert
         bool found = false;

         for (QWindowsFileSystemWatcherEngineThread * thread : threads) {
            QMutexLocker(&(thread->mutex));
            if (thread->handles.count() < MAXIMUM_WAIT_OBJECTS) {
               // qDebug() << "  Added handle" << handle.handle << "for" << absolutePath << "to watch" << fileInfo.absoluteFilePath();
               // qDebug()<< "  to existing thread"<<thread;
               thread->handles.append(handle.handle);
               thread->handleForDir.insert(absolutePath, handle);

               thread->pathInfoForHandle[handle.handle].insert(fileInfo.absoluteFilePath(), pathInfo);
               if (isDir) {
                  directories->append(path);
               } else {
                  files->append(path);
               }

               it.remove();
               found = true;
               thread->wakeup();
               break;
            }
         }
         if (!found) {
            QWindowsFileSystemWatcherEngineThread *thread = new QWindowsFileSystemWatcherEngineThread();
            //qDebug()<<"  ###Creating new thread"<<thread<<"("<<(threads.count()+1)<<"threads)";
            thread->handles.append(handle.handle);
            thread->handleForDir.insert(absolutePath, handle);

            thread->pathInfoForHandle[handle.handle].insert(fileInfo.absoluteFilePath(), pathInfo);
            if (isDir) {
               directories->append(path);
            } else {
               files->append(path);
            }

            connect(thread, SIGNAL(fileChanged(const QString &, bool)), this,
                    SLOT(fileChanged(const QString &, bool)));

            connect(thread, SIGNAL(directoryChanged(const QString &, bool)),
                    this, SLOT(directoryChanged(const QString &, bool)));

            thread->msg = '@';
            thread->start();
            threads.append(thread);
            it.remove();
         }
      }
   }
   return p;
}

QStringList QWindowsFileSystemWatcherEngine::removePaths(const QStringList &paths,
      QStringList *files, QStringList *directories)
{
   // qDebug()<<"removePaths"<<paths;
   QStringList p = paths;
   QMutableListIterator<QString> it(p);

   while (it.hasNext()) {
      QString path = it.next();
      QString normalPath = path;

      if (normalPath.endsWith(QLatin1Char('/')) || normalPath.endsWith(QLatin1Char('\\'))) {
         normalPath.chop(1);
      }

      QFileInfo fileInfo(normalPath.toLower());
      // qDebug()<<"removing"<<normalPath;
      QString absolutePath = fileInfo.absoluteFilePath();
      QList<QWindowsFileSystemWatcherEngineThread *>::iterator jt, end;
      end = threads.end();

      for (jt = threads.begin(); jt != end; ++jt) {
         QWindowsFileSystemWatcherEngineThread *thread = *jt;
         if (*jt == 0) {
            continue;
         }

         QMutexLocker locker(&(thread->mutex));

         QWindowsFileSystemWatcherEngine::Handle handle = thread->handleForDir.value(absolutePath);
         if (handle.handle == INVALID_HANDLE_VALUE) {
            // perhaps path is a file?
            absolutePath = fileInfo.absolutePath();
            handle = thread->handleForDir.value(absolutePath);
         }
         if (handle.handle != INVALID_HANDLE_VALUE) {
            QHash<QString, QWindowsFileSystemWatcherEngine::PathInfo> &h =
               thread->pathInfoForHandle[handle.handle];
            if (h.remove(fileInfo.absoluteFilePath())) {
               // ###
               files->removeAll(path);
               directories->removeAll(path);

               if (h.isEmpty()) {
                  // qDebug() << "Closing handle" << handle.handle;
                  FindCloseChangeNotification(handle.handle);    // This one might generate a notification

                  int indexOfHandle = thread->handles.indexOf(handle.handle);
                  Q_ASSERT(indexOfHandle != -1);
                  thread->handles.remove(indexOfHandle);

                  thread->handleForDir.remove(absolutePath);
                  // h is now invalid

                  it.remove();

                  if (thread->handleForDir.isEmpty()) {
                     // qDebug()<<"Stopping thread "<<thread;
                     locker.unlock();
                     thread->stop();
                     thread->wait();
                     locker.relock();
                     // We can't delete the thread until the mutex locker is
                     // out of scope
                  }
               }
            }
            // Found the file, go to next one
            break;
         }
      }
   }

   // Remove all threads that we stopped
   QList<QWindowsFileSystemWatcherEngineThread *>::iterator jt, end;
   end = threads.end();
   for (jt = threads.begin(); jt != end; ++jt) {
      if (!(*jt)->isRunning()) {
         delete *jt;
         *jt = 0;
      }
   }

   threads.removeAll(0);
   return p;
}

///////////
// QWindowsFileSystemWatcherEngineThread
///////////

QWindowsFileSystemWatcherEngineThread::QWindowsFileSystemWatcherEngineThread()
   : msg(0)
{
   if (HANDLE h = CreateEvent(0, false, false, 0)) {
      handles.reserve(MAXIMUM_WAIT_OBJECTS);
      handles.append(h);
   }
   moveToThread(this);
}


QWindowsFileSystemWatcherEngineThread::~QWindowsFileSystemWatcherEngineThread()
{
   CloseHandle(handles.at(0));
   handles[0] = INVALID_HANDLE_VALUE;

   for (HANDLE h : handles) {
      if (h == INVALID_HANDLE_VALUE) {
         continue;
      }
      FindCloseChangeNotification(h);
   }
}

void QWindowsFileSystemWatcherEngineThread::run()
{
   QMutexLocker locker(&mutex);
   forever {
      QVector<HANDLE> handlesCopy = handles;
      locker.unlock();
      // qDebug() << "QWindowsFileSystemWatcherThread"<<this<<"waiting on" << handlesCopy.count() << "handles";
      DWORD r = WaitForMultipleObjects(handlesCopy.count(), handlesCopy.constData(), false, INFINITE);
      locker.relock();
      do {
         if (r == WAIT_OBJECT_0)
         {
            int m = msg;
            msg = 0;
            if (m == 'q') {
               // qDebug() << "thread"<<this<<"told to quit";
               return;
            }
            if (m != '@')  {
               qDebug("QWindowsFileSystemWatcherEngine: unknown message '%c' send to thread", char(m));
            }
            break;
         } else if (r > WAIT_OBJECT_0 && r < WAIT_OBJECT_0 + uint(handlesCopy.count()))
         {
            int at = r - WAIT_OBJECT_0;
            Q_ASSERT(at < handlesCopy.count());
            HANDLE handle = handlesCopy.at(at);

            // When removing a path, FindCloseChangeNotification might actually fire a notification
            // for some reason, so we must check if the handle exist in the handles vector
            if (handles.contains(handle)) {
               // qDebug()<<"thread"<<this<<"Acknowledged handle:"<<at<<handle;
               if (!FindNextChangeNotification(handle)) {
                  qErrnoWarning("QFileSystemWatcher: FindNextChangeNotification failed!!");
               }

               QHash<QString, QWindowsFileSystemWatcherEngine::PathInfo> &h = pathInfoForHandle[handle];
               QMutableHashIterator<QString, QWindowsFileSystemWatcherEngine::PathInfo> it(h);
               while (it.hasNext()) {
                  QHash<QString, QWindowsFileSystemWatcherEngine::PathInfo>::iterator x = it.next();
                  QString absolutePath = x.value().absolutePath;
                  QFileInfo fileInfo(x.value().path);
                  // qDebug() << "checking" << x.key();
                  if (!fileInfo.exists()) {
                     // qDebug() << x.key() << "removed!";
                     if (x.value().isDir) {
                        emit directoryChanged(x.value().path, true);
                     } else {
                        emit fileChanged(x.value().path, true);
                     }
                     h.erase(x);

                     // close the notification handle if the directory has been removed
                     if (h.isEmpty()) {
                        // qDebug() << "Thread closing handle" << handle;
                        FindCloseChangeNotification(handle);    // This one might generate a notification

                        int indexOfHandle = handles.indexOf(handle);
                        Q_ASSERT(indexOfHandle != -1);
                        handles.remove(indexOfHandle);

                        handleForDir.remove(absolutePath);
                        // h is now invalid
                     }
                  } else if (x.value().isDir) {
                     // qDebug() << x.key() << "directory changed!";
                     emit directoryChanged(x.value().path, false);
                     x.value() = fileInfo;
                  } else if (x.value() != fileInfo) {
                     // qDebug() << x.key() << "file changed!";
                     emit fileChanged(x.value().path, false);
                     x.value() = fileInfo;
                  }
               }
            }
         } else {
            // qErrnoWarning("QFileSystemWatcher: error while waiting for change notification");
            break;  // avoid endless loop
         }
         handlesCopy = handles;
         r = WaitForMultipleObjects(handlesCopy.count(), handlesCopy.constData(), false, 0);
      } while (r != WAIT_TIMEOUT);
   }
}


void QWindowsFileSystemWatcherEngineThread::stop()
{
   msg = 'q';
   SetEvent(handles.at(0));
}

void QWindowsFileSystemWatcherEngineThread::wakeup()
{
   msg = '@';
   SetEvent(handles.at(0));
}

QT_END_NAMESPACE
#endif // QT_NO_FILESYSTEMWATCHER
