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

#ifndef QFILESYSTEMWATCHER_P_H
#define QFILESYSTEMWATCHER_P_H

#include <qfilesystemwatcher.h>

#ifndef QT_NO_FILESYSTEMWATCHER

#include <qstringlist.h>
#include <qthread.h>

class QFileSystemWatcherEngine : public QThread
{
   CORE_CS_OBJECT(QFileSystemWatcherEngine)

 protected:
   QFileSystemWatcherEngine(bool move = true) {
      if (move) {
         moveToThread(this);
      }
   }

 public:
   // fills \a files and \a directories with the \a paths it could
   // watch, and returns a list of paths this engine could not watch
   virtual QStringList addPaths(const QStringList &paths, QStringList *files, QStringList *directories) = 0;

   // removes \a paths from \a files and \a directories, and returns
   // a list of paths this engine does not know about (either addPath
   // failed or wasn't called)
   virtual QStringList removePaths(const QStringList &paths, QStringList *files, QStringList *directories) = 0;

   virtual void stop() = 0;

   CORE_CS_SIGNAL_1(Public, void fileChanged(const QString &path, bool removed))
   CORE_CS_SIGNAL_2(fileChanged, path, removed)
   CORE_CS_SIGNAL_1(Public, void directoryChanged(const QString &path, bool removed))
   CORE_CS_SIGNAL_2(directoryChanged, path, removed)
};

class QFileSystemWatcherPrivate
{
   Q_DECLARE_PUBLIC(QFileSystemWatcher)

   static QFileSystemWatcherEngine *createNativeEngine();

 public:
   QFileSystemWatcherPrivate();
   virtual ~QFileSystemWatcherPrivate() {}

   void init();
   void initPollerEngine();
   void initForcedEngine(const QString &);

   QFileSystemWatcherEngine *native, *poller, *forced;
   QStringList files, directories;

   void _q_fileChanged(const QString &path, bool removed);
   void _q_directoryChanged(const QString &path, bool removed);

 protected:
   QFileSystemWatcher *q_ptr;

};

#endif // QT_NO_FILESYSTEMWATCHER

#endif
