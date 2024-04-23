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

#include <qplatformdefs.h>
#include <qfilesystemwatcher.h>

#include <qfilesystemwatcher_kqueue_p.h>
#include <qcore_unix_p.h>

#ifndef QT_NO_FILESYSTEMWATCHER

#include <qdebug.h>
#include <qfile.h>
#include <qsocketnotifier.h>
#include <qvarlengtharray.h>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>

QKqueueFileSystemWatcherEngine *QKqueueFileSystemWatcherEngine::create()
{
   int kqfd = kqueue();

   if (kqfd == -1) {
      return nullptr;
   }

   return new QKqueueFileSystemWatcherEngine(kqfd);
}

QKqueueFileSystemWatcherEngine::QKqueueFileSystemWatcherEngine(int kqfd)
   : kqfd(kqfd)
{
   fcntl(kqfd, F_SETFD, FD_CLOEXEC);

   if (pipe(kqpipe) == -1) {
      perror("QKqueueFileSystemWatcherEngine: cannot create pipe");
      kqpipe[0] = kqpipe[1] = -1;
      return;
   }

   fcntl(kqpipe[0], F_SETFD, FD_CLOEXEC);
   fcntl(kqpipe[1], F_SETFD, FD_CLOEXEC);

   struct kevent kev;
   EV_SET(&kev, kqpipe[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);

   if (kevent(kqfd, &kev, 1, nullptr, 0, nullptr) == -1) {
      perror("QKqueueFileSystemWatcherEngine: can not watch pipe, kevent returned");
      return;
   }
}

QKqueueFileSystemWatcherEngine::~QKqueueFileSystemWatcherEngine()
{
   stop();
   wait();

   close(kqfd);
   close(kqpipe[0]);
   close(kqpipe[1]);

   for (int id : pathToID) {
      ::close(id < 0 ? -id : id);
   }
}

QStringList QKqueueFileSystemWatcherEngine::addPaths(const QStringList &paths,
      QStringList *files, QStringList *directories)
{
   QStringList p = paths;
   {
      QMutexLocker locker(&mutex);

      QMutableListIterator<QString> it(p);

      while (it.hasNext()) {
         QString path = it.next();
         int fd;

#if defined(O_EVTONLY)
         fd = qt_safe_open(QFile::encodeName(path).constData(), O_EVTONLY);
#else
         fd = qt_safe_open(QFile::encodeName(path).constData(), O_RDONLY);
#endif

         if (fd == -1) {
            perror("QKqueueFileSystemWatcherEngine::addPaths: open");
            continue;
         }

         if (fd >= (int)FD_SETSIZE / 2 && fd < (int)FD_SETSIZE) {
            int fddup = fcntl(fd, F_DUPFD, FD_SETSIZE);

            if (fddup != -1) {
               ::close(fd);
               fd = fddup;
            }
         }

         fcntl(fd, F_SETFD, FD_CLOEXEC);

         QT_STATBUF st;

         if (QT_FSTAT(fd, &st) == -1) {
            perror("QKqueueFileSystemWatcherEngine::addPaths: fstat");
            ::close(fd);
            continue;
         }

         int id = (S_ISDIR(st.st_mode)) ? -fd : fd;

         if (id < 0) {
            if (directories->contains(path)) {
               ::close(fd);
               continue;
            }
         } else {
            if (files->contains(path)) {
               ::close(fd);
               continue;
            }
         }

         struct kevent kev;

         EV_SET(&kev, fd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR,
               NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_RENAME | NOTE_REVOKE, 0, 0);

         if (kevent(kqfd, &kev, 1, nullptr, 0, nullptr) == -1) {
            perror("QKqueueFileSystemWatcherEngine::addPaths: kevent");
            ::close(fd);
            continue;
         }

         it.remove();

         if (id < 0) {
#if defined(CS_SHOW_DEBUG_CORE)
            qDebug() << "QKqueueFileSystemWatcherEngine: Added directory path" << path;
#endif
            directories->append(path);

         } else {
#if defined(CS_SHOW_DEBUG_CORE)
            qDebug() << "QKqueueFileSystemWatcherEngine: Added file path" << path;
#endif
            files->append(path);
         }

         pathToID.insert(path, id);
         idToPath.insert(id, path);
      }
   }

   if (! isRunning()) {
      start();
   } else {
      write(kqpipe[1], "@", 1);
   }

   return p;
}

QStringList QKqueueFileSystemWatcherEngine::removePaths(const QStringList &paths,
      QStringList *files, QStringList *directories)
{
   bool isEmpty;
   QStringList p = paths;
   {
      QMutexLocker locker(&mutex);

      if (pathToID.isEmpty()) {
         return p;
      }

      QMutableListIterator<QString> it(p);

      while (it.hasNext()) {
         QString path = it.next();
         int id = pathToID.take(path);
         QString x = idToPath.take(id);

         if (x.isEmpty() || x != path) {
            continue;
         }

         ::close(id < 0 ? -id : id);

         it.remove();

         if (id < 0) {
            directories->removeAll(path);
         } else {
            files->removeAll(path);
         }
      }

      isEmpty = pathToID.isEmpty();
   }

   if (isEmpty) {
      stop();
      wait();
   } else {
      write(kqpipe[1], "@", 1);
   }

   return p;
}

void QKqueueFileSystemWatcherEngine::stop()
{
   write(kqpipe[1], "q", 1);
}

void QKqueueFileSystemWatcherEngine::run()
{
   while (true) {
      int r;
      struct kevent kev;

      EINTR_LOOP(r, kevent(kqfd, nullptr, 0, &kev, 1, nullptr));

      if (r < 0) {
         perror("QKqueueFileSystemWatcherEngine: error during kevent wait");
         return;

      } else {
         int fd = kev.ident;

#if defined(CS_SHOW_DEBUG_CORE)
         qDebug() << "QKqueueFileSystemWatcherEngine: processing kevent" << kev.ident << kev.filter;
#endif

         if (fd == kqpipe[0]) {
            // read all pending data from the pipe
            QByteArray ba;
            ba.resize(kev.data);

            if (read(kqpipe[0], ba.data(), ba.size()) != ba.size()) {
               perror("QKqueueFileSystemWatcherEngine: error reading from pipe");
               return;
            }

            // read the command from the buffer (but break and return on 'q')
            char cmd = 0;

            for (int i = 0; i < ba.size(); ++i) {
               cmd = ba.constData()[i];

               if (cmd == 'q') {
                  break;
               }
            }

            // handle the command
            switch (cmd) {
               case 'q':
                  return;

               case '@':
                  break;

               default:
                  break;
            }

         } else {
            QMutexLocker locker(&mutex);

            int id = fd;
            QString path = idToPath.value(id);

            if (path.isEmpty()) {
               // perhaps a directory?
               id = -id;
               path = idToPath.value(id);

               if (path.isEmpty()) {
#if defined(CS_SHOW_DEBUG_CORE)
                  qDebug() << "QKqueueFileSystemWatcherEngine: Received a kevent for a file we are not watching";
#endif
                  continue;
               }
            }

            if (kev.filter != EVFILT_VNODE) {
#if defined(CS_SHOW_DEBUG_CORE)
               qDebug() << "QKqueueFileSystemWatcherEngine: received a kevent with the wrong filter";
#endif
               continue;
            }

            if ((kev.fflags & (NOTE_DELETE | NOTE_REVOKE | NOTE_RENAME)) != 0) {
               pathToID.remove(path);
               idToPath.remove(id);
               ::close(fd);

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
   }
}

#endif // QT_NO_FILESYSTEMWATCHER
