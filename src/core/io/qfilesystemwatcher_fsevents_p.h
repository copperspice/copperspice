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

#ifndef QFILESYSTEMWATCHER_FSEVENTS_P_H
#define QFILESYSTEMWATCHER_FSEVENTS_P_H

#include <qfilesystemwatcher_p.h>

#ifndef QT_NO_FILESYSTEMWATCHER

#include <qhash.h>
#include <qlinkedlist.h>
#include <qmutex.h>
#include <qthread.h>
#include <qwaitcondition.h>

#include <qcore_mac_p.h>

#include <sys/stat.h>

using FSEventStreamRef        = struct __FSEventStream *;
using ConstFSEventStreamRef   = const struct __FSEventStream *;
using CFArrayRef              = const struct __CFArray *;
using FSEventStreamEventFlags = UInt32;
using FSEventStreamEventId    = uint64_t;

#if ! defined(Q_OS_IOS)

struct PathInfo {
   PathInfo(const QString &path, const QByteArray &absPath)
      : originalPath(path), absolutePath(absPath)
   { }

   QString originalPath;       // The path we need to emit
   QByteArray absolutePath;    // The path we need to stat.
   struct ::stat savedInfo;    // All the info for the path so we can compare it.
};

using PathInfoList = QLinkedList<PathInfo>;
using PathHash     = QHash<QString, PathInfoList>;

#endif

class QFSEventsFileSystemWatcherEngine : public QFileSystemWatcherEngine
{
   CORE_CS_OBJECT(QFSEventsFileSystemWatcherEngine)

 public:
   ~QFSEventsFileSystemWatcherEngine();

   static QFSEventsFileSystemWatcherEngine *create();

   QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories) override;
   QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories) override;

   void stop() override;

 private:
   QFSEventsFileSystemWatcherEngine();
   void warmUpFSEvents();
   void updateFiles();

   static void fseventsCallback(ConstFSEventStreamRef streamRef, void *clientCallBackInfo, size_t numEvents,
         void *eventPaths, const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[]);

   void run() override;
   FSEventStreamRef fsStream;
   CFArrayRef pathsToWatch;
   CFRunLoopRef threadsRunLoop;
   QMutex mutex;
   QWaitCondition waitCondition;
   QWaitCondition waitForStop;

#if ! defined(Q_OS_IOS)
   PathHash filePathInfoHash;
   PathHash dirPathInfoHash;
   void updateHash(PathHash &pathHash);
   void updateList(PathInfoList &list, bool directory, bool emitSignals);
#endif

};

#endif //QT_NO_FILESYSTEMWATCHER

#endif
