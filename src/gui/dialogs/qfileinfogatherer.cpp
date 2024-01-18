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

#include <qfileinfogatherer_p.h>
#include <qdebug.h>
#include <qdiriterator.h>

#ifndef Q_OS_WIN
#  include <unistd.h>
#  include <sys/types.h>
#endif

#ifndef QT_NO_FILESYSTEMMODEL

// Creates thread

QFileInfoGatherer::QFileInfoGatherer(QObject *parent)
   : QThread(parent), abort(false),

#ifndef QT_NO_FILESYSTEMWATCHER
     watcher(nullptr),
#endif
     m_iconProvider(&defaultProvider)
{
#ifdef Q_OS_WIN
   m_resolveSymlinks = true;
#endif

#ifndef QT_NO_FILESYSTEMWATCHER
   watcher = new QFileSystemWatcher(this);

   connect(watcher, &QFileSystemWatcher::directoryChanged, this, &QFileInfoGatherer::list);
   connect(watcher, &QFileSystemWatcher::fileChanged,      this, &QFileInfoGatherer::updateFile);
#endif

   start(LowPriority);
}

/*!
    Destroys thread
*/
QFileInfoGatherer::~QFileInfoGatherer()
{
   abort.store(true);

   QMutexLocker locker(&mutex);

   condition.wakeAll();
   locker.unlock();
   wait();
}

void QFileInfoGatherer::setResolveSymlinks(bool enable)
{
#ifdef Q_OS_WIN
   m_resolveSymlinks = enable;
#else
   (void) enable;
#endif
}

bool QFileInfoGatherer::resolveSymlinks() const
{
#ifdef Q_OS_WIN
   return m_resolveSymlinks;
#else
   return false;
#endif
}

void QFileInfoGatherer::setIconProvider(QFileIconProvider *provider)
{
   m_iconProvider = provider;
}

QFileIconProvider *QFileInfoGatherer::iconProvider() const
{
   return m_iconProvider;
}


void QFileInfoGatherer::fetchExtendedInformation(const QString &path, const QStringList &files)
{
   QMutexLocker locker(&mutex);

   // See if we already have this dir/file in our queue
   int loc = this->path.lastIndexOf(path);

   while (loc > 0)  {
      if (this->files.at(loc) == files) {
         return;
      }
      loc = this->path.lastIndexOf(path, loc - 1);
   }

   this->path.push(path);
   this->files.push(files);

   condition.wakeAll();

#ifndef QT_NO_FILESYSTEMWATCHER
   if (files.isEmpty() && ! path.isEmpty() && ! path.startsWith("//") ) {
      if (! watcher->directories().contains(path)) {
         watcher->addPath(path);
      }
   }
#endif
}


void QFileInfoGatherer::updateFile(const QString &filePath)
{
   QString dir = filePath.mid(0, filePath.lastIndexOf(QDir::separator()));
   QString fileName = filePath.mid(dir.length() + 1);
   fetchExtendedInformation(dir, QStringList(fileName));
}

/*
    List all files in \a directoryPath

    \sa listed()
*/
void QFileInfoGatherer::clear()
{
#ifndef QT_NO_FILESYSTEMWATCHER
   QMutexLocker locker(&mutex);
   watcher->removePaths(watcher->files());
   watcher->removePaths(watcher->directories());
#endif
}

/*
    Remove a \a path from the watcher

    \sa listed()
*/
void QFileInfoGatherer::removePath(const QString &path)
{
#ifndef QT_NO_FILESYSTEMWATCHER
   QMutexLocker locker(&mutex);
   watcher->removePath(path);
#endif
}

/*
    List all files in \a directoryPath

    \sa listed()
*/
void QFileInfoGatherer::list(const QString &directoryPath)
{
   fetchExtendedInformation(directoryPath, QStringList());
}

/*
    Until aborted wait to fetch a directory or files
*/
void QFileInfoGatherer::run()
{
   while (true) {
      QMutexLocker locker(&mutex);

      while (! abort.load() && path.isEmpty()) {

         condition.wait(&mutex);
      }

      if (abort.load()) {
         return;
      }


      const QString thisPath = path.front();
      path.pop_front();
      const QStringList thisList = files.front();
      files.pop_front();
      locker.unlock();
      getFileInfos(thisPath, thisList);
   }
}

QExtendedInformation QFileInfoGatherer::getInfo(const QFileInfo &fileInfo) const
{
   QExtendedInformation info(fileInfo);
   info.icon = m_iconProvider->icon(fileInfo);
   info.displayType = m_iconProvider->type(fileInfo);

#ifdef Q_OS_WIN
   if (m_resolveSymlinks && info.isSymLink(/* ignoreNtfsSymLinks = */ true)) {
      QFileInfo resolvedInfo(fileInfo.symLinkTarget());
      resolvedInfo = resolvedInfo.canonicalFilePath();

      if (resolvedInfo.exists()) {
         // resolves having a const method call a non const Signal 01/09/2014
         emit const_cast<QFileInfoGatherer *> (this)->nameResolved(fileInfo.filePath(), resolvedInfo.fileName());
      }
   }
#endif

   return info;
}

static QString translateDriveName(const QFileInfo &drive)
{
   QString driveName = drive.absoluteFilePath();

#if defined(Q_OS_WIN)
   if (driveName.startsWith('/')) {
      // UNC host
      return drive.fileName();
   }

   if (driveName.endsWith('/')) {
      driveName.chop(1);
   }
#endif

   return driveName;
}


void QFileInfoGatherer::getFileInfos(const QString &path, const QStringList &files)
{
   // List drives
   if (path.isEmpty()) {

      QFileInfoList infoList;

      if (files.isEmpty()) {
         infoList = QDir::drives();

      } else {
         for (int i = 0; i < files.count(); ++i) {
            infoList << QFileInfo(files.at(i));
         }
      }

      for (int i = infoList.count() - 1; i >= 0; --i) {
         QString driveName = translateDriveName(infoList.at(i));
         QVector<QPair<QString, QFileInfo>> updatedFiles;

         updatedFiles.append(QPair<QString, QFileInfo>(driveName, infoList.at(i)));
         emit updates(path, updatedFiles);
      }
      return;
   }

   QElapsedTimer base;
   base.start();

   QFileInfo fileInfo;
   bool firstTime = true;

   QVector<QPair<QString, QFileInfo>> updatedFiles;
   QStringList filesToCheck = files;

   QString itPath = QDir::fromNativeSeparators(files.isEmpty() ? path : QString(""));
   QDirIterator dirIt(itPath, QDir::AllEntries | QDir::System | QDir::Hidden);

   QStringList allFiles;

   while (! abort.load() && dirIt.hasNext()) {
      dirIt.next();
      fileInfo = dirIt.fileInfo();
      allFiles.append(fileInfo.fileName());

      fetch(fileInfo, base, firstTime, updatedFiles, path);
   }

   if (! allFiles.isEmpty()) {
      emit newListOfFiles(path, allFiles);
   }

   QStringList::const_iterator filesIt = filesToCheck.constBegin();

   while (! abort.load() && filesIt != filesToCheck.constEnd()) {
      fileInfo.setFile(path + QDir::separator() + *filesIt);
      ++filesIt;
      fetch(fileInfo, base, firstTime, updatedFiles, path);
   }

   if (! updatedFiles.isEmpty()) {
      emit updates(path, updatedFiles);
   }

   emit directoryLoaded(path);
}

void QFileInfoGatherer::fetch(const QFileInfo &fileInfo, QElapsedTimer &base, bool &firstTime,
   QVector<QPair<QString, QFileInfo>> &updatedFiles, const QString &path)
{
   updatedFiles.append(QPair<QString, QFileInfo>(fileInfo.fileName(), fileInfo));
   QElapsedTimer current;
   current.start();

   if ((firstTime && updatedFiles.count() > 100) || base.msecsTo(current) > 1000) {
      emit updates(path, updatedFiles);

      updatedFiles.clear();
      base = current;
      firstTime = false;
   }
}

#endif // QT_NO_FILESYSTEMMODEL

