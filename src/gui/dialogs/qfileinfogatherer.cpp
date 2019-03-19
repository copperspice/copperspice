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

#include <qfileinfogatherer_p.h>
#include <qdebug.h>
#include <qfsfileengine.h>
#include <qdiriterator.h>

#ifndef Q_OS_WIN
#  include <unistd.h>
#  include <sys/types.h>
#endif

QT_BEGIN_NAMESPACE

#ifndef QT_NO_FILESYSTEMMODEL

// Creates thread

QFileInfoGatherer::QFileInfoGatherer(QObject *parent)
   : QThread(parent), abort(false),
#ifndef QT_NO_FILESYSTEMWATCHER
     watcher(0),
#endif
     m_resolveSymlinks(false), m_iconProvider(&defaultProvider)
{
#ifdef Q_OS_WIN
   m_resolveSymlinks = true;
#else
   userId = getuid();
   groupId = getgid();
#endif

#ifndef QT_NO_FILESYSTEMWATCHER
   watcher = new QFileSystemWatcher(this);
   connect(watcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(list(const QString &)));
   connect(watcher, SIGNAL(fileChanged(const QString &)), this, SLOT(updateFile(const QString &)));
#endif

   start(LowPriority);
}

/*!
    Destroys thread
*/
QFileInfoGatherer::~QFileInfoGatherer()
{
   QMutexLocker locker(&mutex);
   abort = true;
   condition.wakeOne();
   locker.unlock();
   wait();
}

void QFileInfoGatherer::setResolveSymlinks(bool enable)
{
   Q_UNUSED(enable);

#ifdef Q_OS_WIN
   QMutexLocker locker(&mutex);
   m_resolveSymlinks = enable;
#endif
}

bool QFileInfoGatherer::resolveSymlinks() const
{
   return m_resolveSymlinks;
}

void QFileInfoGatherer::setIconProvider(QFileIconProvider *provider)
{
   QMutexLocker locker(&mutex);
   m_iconProvider = provider;
}

QFileIconProvider *QFileInfoGatherer::iconProvider() const
{
   return m_iconProvider;
}

/*!
    Fetch extended information for all \a files in \a path

    \sa updateFile(), update(), resolvedName()
*/
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
}

/*!
    Fetch extended information for all \a filePath

    \sa fetchExtendedInformation()
*/
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
   forever {
      bool updateFiles = false;
      QMutexLocker locker(&mutex);

      if (abort) {
         return;
      }

      if (this->path.isEmpty()) {
         condition.wait(&mutex);
      }

      QString path;
      QStringList list;

      if (!this->path.isEmpty()) {
         path = this->path.first();
         list = this->files.first();
         this->path.pop_front();
         this->files.pop_front();
         updateFiles = true;
      }

      locker.unlock();

      if (updateFiles) {
         getFileInfos(path, list);
      }
   }
}

QExtendedInformation QFileInfoGatherer::getInfo(const QFileInfo &fileInfo) const
{
   QExtendedInformation info(fileInfo);
   info.icon = m_iconProvider->icon(fileInfo);
   info.displayType = m_iconProvider->type(fileInfo);

   if (m_resolveSymlinks && info.isSymLink(/* ignoreNtfsSymLinks = */ true)) {
      QFileInfo resolvedInfo(fileInfo.symLinkTarget());
      resolvedInfo = resolvedInfo.canonicalFilePath(); 

      if (resolvedInfo.exists()) {
         // resolves having a const method call a non const Signal 01/09/2014
         emit const_cast<QFileInfoGatherer *> (this)->nameResolved(fileInfo.filePath(), resolvedInfo.fileName());
      }
   }

   return info;
}

QString QFileInfoGatherer::translateDriveName(const QFileInfo &drive) const
{
   QString driveName = drive.absoluteFilePath();

#if defined(Q_OS_WIN)
   if (driveName.startsWith(QLatin1Char('/'))) { 
      // UNC host
      return drive.fileName();
   }
#endif

#if defined(Q_OS_WIN)
   if (driveName.endsWith(QLatin1Char('/'))) {
      driveName.chop(1);
   }
#endif
   return driveName;
}

/*
    Get specific file info's, batch the files so update when we have 100
    items and every 200ms after that
 */
void QFileInfoGatherer::getFileInfos(const QString &path, const QStringList &files)
{

#ifndef QT_NO_FILESYSTEMWATCHER
   if (files.isEmpty() && ! watcher->directories().contains(path)
         && ! path.isEmpty() && ! path.startsWith(QLatin1String("//")) ) {

      // do not watch UNC path
      watcher->addPath(path);
   }
#endif

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
         QList<QPair<QString, QFileInfo> > updatedFiles;

         updatedFiles.append(QPair<QString, QFileInfo>(driveName, infoList.at(i)));
         emit updates(path, updatedFiles);
      }
      return;
   }

   QElapsedTimer base;
   base.start();

   QFileInfo fileInfo;
   bool firstTime = true;

   QList<QPair<QString, QFileInfo> > updatedFiles;
   QStringList filesToCheck = files;

   QString itPath = QDir::fromNativeSeparators(files.isEmpty() ? path : QLatin1String(""));
   QDirIterator dirIt(itPath, QDir::AllEntries | QDir::System | QDir::Hidden);

   QStringList allFiles;

   while (! abort && dirIt.hasNext()) {
      dirIt.next();
      fileInfo = dirIt.fileInfo();
      allFiles.append(fileInfo.fileName());

      fetch(fileInfo, base, firstTime, updatedFiles, path);
   }

   if (! allFiles.isEmpty()) { 
      emit newListOfFiles(path, allFiles);
   }

   QStringList::const_iterator filesIt = filesToCheck.constBegin();

   while (! abort && filesIt != filesToCheck.constEnd()) {
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
                              QList<QPair<QString, QFileInfo> > &updatedFiles, const QString &path)
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

QT_END_NAMESPACE
