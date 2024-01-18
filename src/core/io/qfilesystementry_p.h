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

#ifndef QFILESYSTEMENTRY_P_H
#define QFILESYSTEMENTRY_P_H

#include <qglobal.h>
#include <qstring.h>
#include <qbytearray.h>

class QFileSystemEntry
{
 public:
   struct FromNativePath
   {
   };

   struct FromInternalPath
   {
   };

   QFileSystemEntry();
   explicit QFileSystemEntry(const QString &filePath);

   QFileSystemEntry(const QString &filePath, FromInternalPath dummy);
   QFileSystemEntry(const QString &nativeFilePath, FromNativePath dummy);
   QFileSystemEntry(const QString &filePath, const QString &nativeFilePath);

   QString filePath() const;
   QString fileName() const;
   QString path() const;
   QString nativeFilePath() const;
   QString baseName() const;
   QString completeBaseName() const;
   QString suffix() const;
   QString completeSuffix() const;

   bool isAbsolute() const;
   bool isRelative() const;
   bool isClean() const;

#if defined(Q_OS_WIN)
   bool isDriveLetter_Root() const;
#endif

   bool isRoot() const;

   bool isEmpty() const;
   void clear() {
      *this = QFileSystemEntry();
   }

 private:
   // creates the QString version out of the bytearray version
   void resolveFilePath() const;

   // creates the bytearray version out of the QString version
   void resolveNativeFilePath() const;

   // resolves the separator
   void findLastSeparator() const;

   // resolves the dots and the separator
   void findFileNameSeparators() const;

   mutable QString m_filePath;            // always has slashes as separator
   mutable QString m_nativeFilePath;      // native encoding and separators

   mutable qint16 m_lastSeparator;        // index in m_filePath of last separator
   mutable qint16 m_firstDotInFileName;   // index after m_filePath for first dot (.)
   mutable qint16 m_lastDotInFileName;    // index after m_firstDotInFileName for last dot (.)
};

#endif
