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

#include <qfilesystemiterator_p.h>

#include <qplatformdefs.h>
#include <qstringparser.h>
#include <qt_windows.h>

#include <qfilesystemengine_p.h>

#ifndef  QT_NO_FILESYSTEMITERATOR

bool done = true;

QFileSystemIterator::QFileSystemIterator(const QFileSystemEntry &entry, QDir::Filters filters,
      const QStringList &nameFilters, QDirIterator::IteratorFlags flags)
   : nativePath(entry.nativeFilePath()), dirPath(entry.filePath()), findFileHandle(INVALID_HANDLE_VALUE), uncFallback(false),
     uncShareIndex(0), onlyDirs(false)
{
   (void) nameFilters;
   (void) flags;

   if (nativePath.endsWith(".lnk")) {
      QFileSystemMetaData metaData;
      QFileSystemEntry link = QFileSystemEngine::getLinkTarget(entry, metaData);
      nativePath = link.nativeFilePath();
   }

   if (! nativePath.endsWith('\\')) {
      nativePath.append('\\');
   }

   nativePath.append('*');

   if (!dirPath.endsWith('/')) {
      dirPath.append('/');
   }

   if ((filters & (QDir::Dirs | QDir::Drives)) && (! (filters & (QDir::Files)))) {
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
         dwAdditionalFlags = 2;        // FIND_FIRST_EX_LARGE_FETCH
         infoLevel = 1 ;               // FindExInfoBasic;
      }

      int searchOps =  0;              // FindExSearchNameMatch

      if (onlyDirs) {
         searchOps = 1 ;               // FindExSearchLimitToDirectories
      }

      findFileHandle = FindFirstFileEx(&nativePath.toStdWString()[0], FINDEX_INFO_LEVELS(infoLevel), &findData,
            FINDEX_SEARCH_OPS(searchOps), nullptr, dwAdditionalFlags);

      if (findFileHandle == INVALID_HANDLE_VALUE) {

         if (nativePath.startsWith("\\\\?\\UNC\\")) {
            QStringList parts = nativePath.split('\\', QStringParser::SkipEmptyParts);

            if (parts.count() == 4 && QFileSystemEngine::uncListSharesOnServer("\\\\" + parts.at(2), &uncShares)) {
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
      QString fileName = QString::fromStdWString(std::wstring(findData.cFileName));

      fileEntry = QFileSystemEntry(dirPath + fileName);
      metaData  = QFileSystemMetaData();

      if (! fileName.endsWith(QLatin1String(".lnk"))) {
         metaData.fillFromFindData(findData, true);
      }

      return true;
   }

   return false;
}

#endif //  QT_NO_FILESYSTEMITERATOR
