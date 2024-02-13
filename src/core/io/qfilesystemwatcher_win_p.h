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

#ifndef QFILESYSTEMWATCHER_WIN_P_H
#define QFILESYSTEMWATCHER_WIN_P_H

#include <qfilesystemwatcher_p.h>

#ifndef QT_NO_FILESYSTEMWATCHER

#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qhash.h>
#include <qmutex.h>
#include <qt_windows.h>
#include <qvector.h>

class QWindowsFileSystemWatcherEngineThread;

// Even though QWindowsFileSystemWatcherEngine is derived of QThread
// via QFileSystemWatcher, it does not start a thread.
// Instead QWindowsFileSystemWatcher creates QWindowsFileSystemWatcherEngineThreads
// to do the actually watching.
class QWindowsFileSystemWatcherEngine : public QFileSystemWatcherEngine
{
   CORE_CS_OBJECT(QWindowsFileSystemWatcherEngine)

 public:
   QWindowsFileSystemWatcherEngine();
   ~QWindowsFileSystemWatcherEngine();

   QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories) override;
   QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories) override;

   void stop() override;

   class Handle
   {
    public:
      HANDLE handle;
      uint flags;

      Handle()
         : handle(INVALID_HANDLE_VALUE), flags(0u)
      {
      }
   };

   class PathInfo
   {
    public:
      QString absolutePath;
      QString path;
      bool isDir;

      // fileinfo bits
      uint ownerId;
      uint groupId;
      QFile::Permissions permissions;
      QDateTime lastModified;

      PathInfo &operator=(const QFileInfo &fileInfo) {
         ownerId = fileInfo.ownerId();
         groupId = fileInfo.groupId();
         permissions = fileInfo.permissions();
         lastModified = fileInfo.lastModified();
         return *this;
      }

      bool operator!=(const QFileInfo &fileInfo) const {
         return (ownerId != fileInfo.ownerId() || groupId != fileInfo.groupId()
               || permissions != fileInfo.permissions() || lastModified != fileInfo.lastModified());
      }
   };

 private:
   QList<QWindowsFileSystemWatcherEngineThread *> threads;

};

class QWindowsFileSystemWatcherEngineThread : public QThread
{
   CORE_CS_OBJECT(QWindowsFileSystemWatcherEngineThread)

 public:
   QWindowsFileSystemWatcherEngineThread();
   ~QWindowsFileSystemWatcherEngineThread();
   void run() override;
   void stop();
   void wakeup();

   QMutex mutex;
   QVector<HANDLE> handles;
   int msg;

   QHash<QString, QWindowsFileSystemWatcherEngine::Handle> handleForDir;

   QHash<HANDLE, QHash<QString, QWindowsFileSystemWatcherEngine::PathInfo>> pathInfoForHandle;

   CORE_CS_SIGNAL_1(Public, void fileChanged(const QString &path, bool removed))
   CORE_CS_SIGNAL_2(fileChanged, path, removed)
   CORE_CS_SIGNAL_1(Public, void directoryChanged(const QString &path, bool removed))
   CORE_CS_SIGNAL_2(directoryChanged, path, removed)
};

#endif // QT_NO_FILESYSTEMWATCHER

#endif
