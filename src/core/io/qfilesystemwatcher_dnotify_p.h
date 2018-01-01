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

#ifndef QFILESYSTEMWATCHER_DNOTIFY_P_H
#define QFILESYSTEMWATCHER_DNOTIFY_P_H

#include <qfilesystemwatcher_p.h>

#ifndef QT_NO_FILESYSTEMWATCHER

#include <qmutex.h>
#include <qhash.h>
#include <qdatetime.h>
#include <qfile.h>

QT_BEGIN_NAMESPACE

class QDnotifyFileSystemWatcherEngine : public QFileSystemWatcherEngine
{
   CORE_CS_OBJECT(QDnotifyFileSystemWatcherEngine)

 public:
   virtual ~QDnotifyFileSystemWatcherEngine();

   static QDnotifyFileSystemWatcherEngine *create();

   void run() override;

   QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories) override;
   QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories) override;

   void stop() override;

 private :
   CORE_CS_SLOT_1(Private, void refresh(int un_named_arg1))
   CORE_CS_SLOT_2(refresh)

   struct Directory {
      Directory() : fd(0), parentFd(0), isMonitored(false) {}
      Directory(const Directory &o) : path(o.path),
         fd(o.fd),
         parentFd(o.parentFd),
         isMonitored(o.isMonitored),
         files(o.files) {}
      QString path;
      int fd;
      int parentFd;
      bool isMonitored;

      struct File {
         File() : ownerId(0u), groupId(0u), permissions(0u) { }
         File(const File &o) : path(o.path),
            ownerId(o.ownerId),
            groupId(o.groupId),
            permissions(o.permissions),
            lastWrite(o.lastWrite) {}
         QString path;

         bool updateInfo();

         uint ownerId;
         uint groupId;
         QFile::Permissions permissions;
         QDateTime lastWrite;
      };

      QList<File> files;
   };

   QDnotifyFileSystemWatcherEngine();

   QMutex mutex;
   QHash<QString, int> pathToFD;
   QHash<int, Directory> fdToDirectory;
   QHash<int, int> parentToFD;
};

QT_END_NAMESPACE

#endif // QT_NO_FILESYSTEMWATCHER

#endif // QFILESYSTEMWATCHER_DNOTIFY_P_H
