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

#ifndef QFILESYSTEMWATCHER_KQUEUE_P_H
#define QFILESYSTEMWATCHER_KQUEUE_P_H

#include <qfilesystemwatcher_p.h>

#include <qhash.h>
#include <qmutex.h>
#include <qthread.h>
#include <qvector.h>

#ifndef QT_NO_FILESYSTEMWATCHER
struct kevent;

class QKqueueFileSystemWatcherEngine : public QFileSystemWatcherEngine
{
   CORE_CS_OBJECT(QKqueueFileSystemWatcherEngine)

 public:
   ~QKqueueFileSystemWatcherEngine();

   static QKqueueFileSystemWatcherEngine *create();

   QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories) override;
   QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories) override;

   void stop() override;

 private:
   QKqueueFileSystemWatcherEngine(int kqfd);

   void run() override;

   int kqfd;
   int kqpipe[2];

   QMutex mutex;
   QHash<QString, int> pathToID;
   QHash<int, QString> idToPath;
};

#endif //QT_NO_FILESYSTEMWATCHER

#endif