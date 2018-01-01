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

#ifndef QFILESYSTEMWATCHER_FSEVENTS_P_H
#define QFILESYSTEMWATCHER_FSEVENTS_P_H

#include <qfilesystemwatcher_p.h>

#ifndef QT_NO_FILESYSTEMWATCHER

#include <QtCore/qmutex.h>
#include <QtCore/qwaitcondition.h>
#include <QtCore/qthread.h>
#include <QtCore/qhash.h>
#include <QtCore/qlinkedlist.h>
#include <qcore_mac_p.h>
#include <sys/stat.h>

typedef struct __FSEventStream *FSEventStreamRef;
typedef const struct __FSEventStream *ConstFSEventStreamRef;
typedef const struct __CFArray *CFArrayRef;
typedef UInt32 FSEventStreamEventFlags;
typedef uint64_t FSEventStreamEventId;

QT_BEGIN_NAMESPACE

#if ! defined(Q_OS_IOS)

// Yes, I use a stat64 element here. QFileInfo requires too much knowledge about implementation
// details to be used as a long-standing record. Since I'm going to have to store this information, I can
// do the stat myself too.

struct PathInfo {
   PathInfo(const QString &path, const QByteArray &absPath)
      : originalPath(path), absolutePath(absPath) {}

   QString originalPath;       // The path we need to emit
   QByteArray absolutePath;    // The path we need to stat.
   struct ::stat savedInfo;    // All the info for the path so we can compare it.
};

typedef QLinkedList<PathInfo> PathInfoList;
typedef QHash<QString, PathInfoList> PathHash;
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
                                void *eventPaths, const FSEventStreamEventFlags eventFlags[],
                                const FSEventStreamEventId eventIds[]);
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

QT_END_NAMESPACE
