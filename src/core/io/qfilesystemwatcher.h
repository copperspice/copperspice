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

#ifndef QFILESYSTEMWATCHER_H
#define QFILESYSTEMWATCHER_H

#include <qobject.h>
#include <qscopedpointer.h>

#ifndef QT_NO_FILESYSTEMWATCHER

class QFileSystemWatcherPrivate;

class Q_CORE_EXPORT QFileSystemWatcher : public QObject
{
   CORE_CS_OBJECT(QFileSystemWatcher)
   Q_DECLARE_PRIVATE(QFileSystemWatcher)

 public:
   QFileSystemWatcher(QObject *parent = nullptr);
   QFileSystemWatcher(const QStringList &pathList, QObject *parent = nullptr);

   ~QFileSystemWatcher();

   void addPath(const QString &path);
   void addPaths(const QStringList &pathList);
   void removePath(const QString &path);
   void removePaths(const QStringList &pathList);

   QStringList files() const;
   QStringList directories() const;

   CORE_CS_SIGNAL_1(Public, void fileChanged(const QString &path))
   CORE_CS_SIGNAL_2(fileChanged, path)

   CORE_CS_SIGNAL_1(Public, void directoryChanged(const QString &path))
   CORE_CS_SIGNAL_2(directoryChanged, path)

 private:
   CORE_CS_SLOT_1(Private, void _q_fileChanged(const QString &path, bool removed))
   CORE_CS_SLOT_2(_q_fileChanged)

   CORE_CS_SLOT_1(Private, void _q_directoryChanged(const QString &path, bool removed))
   CORE_CS_SLOT_2(_q_directoryChanged)

 protected:
   QScopedPointer<QFileSystemWatcherPrivate> d_ptr;

};

#endif // QT_NO_FILESYSTEMWATCHER

#endif
