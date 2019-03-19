/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QFILEINFOGATHERER_P_H
#define QFILEINFOGATHERER_P_H

#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <qfilesystemwatcher.h>
#include <qfileiconprovider.h>
#include <qfsfileengine.h>
#include <qpair.h>
#include <qstack.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qelapsedtimer.h>

QT_BEGIN_NAMESPACE

class QFileIconProvider;

class QExtendedInformation
{
 public:
   enum Type { Dir, File, System };

   QExtendedInformation() {}
   QExtendedInformation(const QFileInfo &info) : mFileInfo(info) {}

   inline bool isDir() {
      return type() == Dir;
   }

   inline bool isFile() {
      return type() == File;
   }

   inline bool isSystem() {
      return type() == System;
   }

   bool operator ==(const QExtendedInformation &fileInfo) const {
      return mFileInfo == fileInfo.mFileInfo && displayType == fileInfo.displayType && permissions() == fileInfo.permissions();
   }

#ifndef QT_NO_FSFILEENGINE
   bool isCaseSensitive() const {
      QFSFileEngine fe(mFileInfo.absoluteFilePath());
      return fe.caseSensitive();
   }
#endif

   QFile::Permissions permissions() const {
      return mFileInfo.permissions();
   }

   Type type() const {
      if (mFileInfo.isDir()) {
         return QExtendedInformation::Dir;
      }
      if (mFileInfo.isFile()) {
         return QExtendedInformation::File;
      }
      if (!mFileInfo.exists() && mFileInfo.isSymLink()) {
         return QExtendedInformation::System;
      }
      return QExtendedInformation::System;
   }

   bool isSymLink(bool ignoreNtfsSymLinks = false) const {
      if (ignoreNtfsSymLinks) {

#ifdef Q_OS_WIN
         return !mFileInfo.suffix().compare(QLatin1String("lnk"), Qt::CaseInsensitive);
#endif
      }
      return mFileInfo.isSymLink();
   }

   bool isHidden() const {
      return mFileInfo.isHidden();
   }

   QFileInfo fileInfo() const {
      return mFileInfo;
   }

   QDateTime lastModified() const {
      return mFileInfo.lastModified();
   }

   qint64 size() const {
      qint64 size = -1;
      if (type() == QExtendedInformation::Dir) {
         size = 0;
      }
      if (type() == QExtendedInformation::File) {
         size = mFileInfo.size();
      }
      if (!mFileInfo.exists() && !mFileInfo.isSymLink()) {
         size = -1;
      }
      return size;
   }

   QString displayType;
   QIcon icon;

 private :
   QFileInfo mFileInfo;
};

#ifndef QT_NO_FILESYSTEMMODEL

class QFileInfoGatherer : public QThread
{
   GUI_CS_OBJECT(QFileInfoGatherer)

 public:
   GUI_CS_SIGNAL_1(Public, void updates(const QString &directory, const QList <QPair <QString, QFileInfo>> &updates))
   GUI_CS_SIGNAL_2(updates, directory, updates)

   GUI_CS_SIGNAL_1(Public, void newListOfFiles(const QString &directory, const QStringList &listOfFiles))
   GUI_CS_SIGNAL_2(newListOfFiles, directory, listOfFiles)

   GUI_CS_SIGNAL_1(Public, void nameResolved(const QString &fileName, const QString &resolvedName))
   GUI_CS_SIGNAL_2(nameResolved, fileName, resolvedName)

   GUI_CS_SIGNAL_1(Public, void directoryLoaded(const QString &path))
   GUI_CS_SIGNAL_2(directoryLoaded, path)

   QFileInfoGatherer(QObject *parent = nullptr);
   ~QFileInfoGatherer();

   void clear();
   void removePath(const QString &path);
   QExtendedInformation getInfo(const QFileInfo &info) const;
 
   GUI_CS_SLOT_1(Public, void list(const QString &directoryPath))
   GUI_CS_SLOT_2(list)

   GUI_CS_SLOT_1(Public, void fetchExtendedInformation(const QString &path, const QStringList &files))
   GUI_CS_SLOT_2(fetchExtendedInformation)

   GUI_CS_SLOT_1(Public, void updateFile(const QString &path))
   GUI_CS_SLOT_2(updateFile)

   GUI_CS_SLOT_1(Public, void setResolveSymlinks(bool enable))
   GUI_CS_SLOT_2(setResolveSymlinks)

   GUI_CS_SLOT_1(Public, bool resolveSymlinks() const)
   GUI_CS_SLOT_2(resolveSymlinks)

   GUI_CS_SLOT_1(Public, void setIconProvider(QFileIconProvider *provider))
   GUI_CS_SLOT_2(setIconProvider)

   GUI_CS_SLOT_1(Public, QFileIconProvider *iconProvider() const)
   GUI_CS_SLOT_2(iconProvider)

 protected:
   void run() override;
   void getFileInfos(const QString &path, const QStringList &files);

 private:
   void fetch(const QFileInfo &info, QElapsedTimer &base, bool &firstTime, QList<QPair<QString, QFileInfo> > &updatedFiles,
              const QString &path);
   QString translateDriveName(const QFileInfo &drive) const;

   QMutex mutex;
   QWaitCondition condition;
   volatile bool abort;

   QStack<QString> path;
   QStack<QStringList> files;

#ifndef QT_NO_FILESYSTEMWATCHER
   QFileSystemWatcher *watcher;
#endif

   bool m_resolveSymlinks;
   QFileIconProvider *m_iconProvider;
   QFileIconProvider defaultProvider;

#ifndef Q_OS_WIN
   uint userId;
   uint groupId;
#endif

};
#endif // QT_NO_FILESYSTEMMODEL


QT_END_NAMESPACE
#endif // QFILEINFOGATHERER_H

