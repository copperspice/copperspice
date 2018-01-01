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

#include <qfilesystemiterator_p.h>
#include <qfilesystemengine_p.h>
#include <qplatformdefs.h>
#include <QtCore/qt_windows.h>

QT_BEGIN_NAMESPACE
#ifndef  QT_NO_FILESYSTEMITERATOR

bool done = true;

QFileSystemIterator::QFileSystemIterator(const QFileSystemEntry &entry, QDir::Filters filters,
      const QStringList &nameFilters, QDirIterator::IteratorFlags flags)
   : nativePath(entry.nativeFilePath())
   , dirPath(entry.filePath())
   , findFileHandle(INVALID_HANDLE_VALUE)
   , uncFallback(false)
   , uncShareIndex(0)
   , onlyDirs(false)
{
   Q_UNUSED(nameFilters)
   Q_UNUSED(flags)
   if (nativePath.endsWith(QLatin1String(".lnk"))) {
      QFileSystemMetaData metaData;
      QFileSystemEntry link = QFileSystemEngine::getLinkTarget(entry, metaData);
      nativePath = link.nativeFilePath();
   }
   if (!nativePath.endsWith(QLatin1Char('\\'))) {
      nativePath.append(QLatin1Char('\\'));
   }
   nativePath.append(QLatin1Char('*'));
   if (!dirPath.endsWith(QLatin1Char('/'))) {
      dirPath.append(QLatin1Char('/'));
   }
   if ((filters & (QDir::Dirs | QDir::Drives)) && (!(filters & (QDir::Files)))) {
      onlyDirs = true;
   }
}

QFileSystemIterator::~QFileSystemIterator()
{
   if (findFileHandle != INVALID_HANDLE_VALUE) {
      FindClose(findFileHandle);
   }
}

bool QFileSystemIterator::advance(QFileSystemEntry &fileEntry, QFileSystemMetaData &metaData)
{
   bool haveData = false;
   WIN32_FIND_DATA findData;

   if (findFileHandle == INVALID_HANDLE_VALUE && !uncFallback) {
      haveData = true;
      int infoLevel = 0 ;         // FindExInfoStandard;
      DWORD dwAdditionalFlags  = 0;

      if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7) {
         dwAdditionalFlags = 2;  // FIND_FIRST_EX_LARGE_FETCH
         infoLevel = 1 ;         // FindExInfoBasic;
      }

      int searchOps =  0;         // FindExSearchNameMatch
      if (onlyDirs) {
         searchOps = 1 ;   // FindExSearchLimitToDirectories
      }
      findFileHandle = FindFirstFileEx((const wchar_t *)nativePath.utf16(), FINDEX_INFO_LEVELS(infoLevel), &findData,
                                       FINDEX_SEARCH_OPS(searchOps), 0, dwAdditionalFlags);
      if (findFileHandle == INVALID_HANDLE_VALUE) {
         if (nativePath.startsWith(QLatin1String("\\\\?\\UNC\\"))) {
            QStringList parts = nativePath.split(QLatin1Char('\\'), QString::SkipEmptyParts);
            if (parts.count() == 4 && QFileSystemEngine::uncListSharesOnServer(
                     QLatin1String("\\\\") + parts.at(2), &uncShares)) {
               if (uncShares.isEmpty()) {
                  return false;   // No shares found in the server
               } else {
                  uncFallback = true;
               }
            }
         }
      }
   }
   if (findFileHandle == INVALID_HANDLE_VALUE && !uncFallback) {
      return false;
   }
   // Retrieve the new file information.
   if (!haveData) {
      if (uncFallback) {
         if (++uncShareIndex >= uncShares.count()) {
            return false;
         }
      } else {
         if (!FindNextFile(findFileHandle, &findData)) {
            return false;
         }
      }
   }
   // Create the new file system entry & meta data.
   if (uncFallback) {
      fileEntry = QFileSystemEntry(dirPath + uncShares.at(uncShareIndex));
      metaData.fillFromFileAttribute(FILE_ATTRIBUTE_DIRECTORY);
      return true;
   } else {
      QString fileName = QString::fromWCharArray(findData.cFileName);
      fileEntry = QFileSystemEntry(dirPath + fileName);
      metaData = QFileSystemMetaData();
      if (!fileName.endsWith(QLatin1String(".lnk"))) {
         metaData.fillFromFindData(findData, true);
      }
      return true;
   }
   return false;
}

#endif //  QT_NO_FILESYSTEMITERATOR
QT_END_NAMESPACE
