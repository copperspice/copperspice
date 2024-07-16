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

#ifndef QFILESYSTEMWATCHER_DNOTIFY_P_H
#define QFILESYSTEMWATCHER_DNOTIFY_P_H

#include <qfilesystemwatcher_p.h>

#ifndef QT_NO_FILESYSTEMWATCHER

#include <qdatetime.h>
#include <qfile.h>
#include <qhash.h>
#include <qmutex.h>

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

 private:
   CORE_CS_SLOT_1(Private, void refresh(int fd))
   CORE_CS_SLOT_2(refresh)

   struct Directory {
      Directory()
         : fd(0), parentFd(0), isMonitored(false)
      { }

      struct File {
         File()
            : ownerId(0u), groupId(0u), permissions(Qt::EmptyFlag)
         { }

         bool updateInfo();

         QString path;
         uint ownerId;
         uint groupId;
         QFile::Permissions permissions;
         QDateTime lastWrite;
      };

      QString path;
      int fd;
      int parentFd;
      bool isMonitored;
      QList<File> files;
   };

   QDnotifyFileSystemWatcherEngine();

   QMutex mutex;
   QHash<QString, int> pathToFD;
   QHash<int, Directory> fdToDirectory;
   QHash<int, int> parentToFD;
};

#endif // QT_NO_FILESYSTEMWATCHER

#endif
