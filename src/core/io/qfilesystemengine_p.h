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

#ifndef QFILESYSTEMENGINE_P_H
#define QFILESYSTEMENGINE_P_H

#include <qfile.h>
#include <qfilesystementry_p.h>
#include <qfilesystemmetadata_p.h>
#include <qsystemerror_p.h>

QT_BEGIN_NAMESPACE

class QFileSystemEngine
{
 public:
   static bool isCaseSensitive();

   static QFileSystemEntry getLinkTarget(const QFileSystemEntry &link, QFileSystemMetaData &data);
   static QFileSystemEntry canonicalName(const QFileSystemEntry &entry, QFileSystemMetaData &data);
   static QFileSystemEntry absoluteName(const QFileSystemEntry &entry);
   static QByteArray id(const QFileSystemEntry &entry);
   static QString resolveUserName(const QFileSystemEntry &entry, QFileSystemMetaData &data);
   static QString resolveGroupName(const QFileSystemEntry &entry, QFileSystemMetaData &data);

#if defined(Q_OS_UNIX)
   static QString resolveUserName(uint userId);
   static QString resolveGroupName(uint groupId);
#endif

#if !defined(QWS) && !defined(Q_WS_QPA) && defined(Q_OS_MAC)
   static QString bundleName(const QFileSystemEntry &entry);
#else
   static QString bundleName(const QFileSystemEntry &entry) {
      Q_UNUSED(entry) return QString();
   }
#endif

   static bool fillMetaData(const QFileSystemEntry &entry, QFileSystemMetaData &data, QFileSystemMetaData::MetaDataFlags what);
#if defined(Q_OS_UNIX)
   static bool fillMetaData(int fd, QFileSystemMetaData &data); // what = PosixStatFlags
#endif

#if defined(Q_OS_WIN)
   static bool uncListSharesOnServer(const QString &server,QStringList *list); 
   static bool fillMetaData(int fd, QFileSystemMetaData &data,QFileSystemMetaData::MetaDataFlags what);
   static bool fillMetaData(HANDLE fHandle, QFileSystemMetaData &data, QFileSystemMetaData::MetaDataFlags what);
   static bool fillPermissions(const QFileSystemEntry &entry, QFileSystemMetaData &data, QFileSystemMetaData::MetaDataFlags what);
   static QString owner(const QFileSystemEntry &entry, QAbstractFileEngine::FileOwner own);
   static QString nativeAbsoluteFilePath(const QString &path);
#endif

   //homePath, rootPath and tempPath shall return clean paths
   static QString homePath();
   static QString rootPath();
   static QString tempPath();

   static bool createDirectory(const QFileSystemEntry &entry, bool createParents);
   static bool removeDirectory(const QFileSystemEntry &entry, bool removeEmptyParents);

   static bool createLink(const QFileSystemEntry &source, const QFileSystemEntry &target, QSystemError &error);

   static bool copyFile(const QFileSystemEntry &source, const QFileSystemEntry &target, QSystemError &error);
   static bool renameFile(const QFileSystemEntry &source, const QFileSystemEntry &target, QSystemError &error);
   static bool removeFile(const QFileSystemEntry &entry, QSystemError &error);

   static bool setPermissions(const QFileSystemEntry &entry, QFile::Permissions permissions, 
         QSystemError &error, QFileSystemMetaData *data = 0);

   static bool setCurrentPath(const QFileSystemEntry &entry);
   static QFileSystemEntry currentPath();

   static QAbstractFileEngine *resolveEntryAndCreateLegacyEngine(QFileSystemEntry &entry,QFileSystemMetaData &data);

 private:
   static QString slowCanonicalized(const QString &path);

#if defined(Q_OS_WIN)
   static void clearWinStatData(QFileSystemMetaData &data);
#endif
};

QT_END_NAMESPACE

#endif // include guard
