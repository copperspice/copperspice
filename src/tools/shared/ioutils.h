/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#ifndef IOUTILS_H
#define IOUTILS_H

#include <QString>

namespace ProFileEvaluatorInternal {

/*!
  This class provides replacement functionality for QFileInfo, QFile & QDir,
  as these are abysmally slow.
*/
class IoUtils
{
 public:
   enum FileType {
      FileNotFound = 0,
      FileIsRegular = 1,
      FileIsDir = 2
   };

   static FileType fileType(const QString &fileName);
   static bool exists(const QString &fileName) {
      return fileType(fileName) != FileNotFound;
   }
   static bool isRelativePath(const QString &fileName);
   static bool isAbsolutePath(const QString &fileName) {
      return !isRelativePath(fileName);
   }
   static QStringView fileName(const QString &fileName); // Requires normalized path
   static QString resolvePath(const QString &baseDir, const QString &fileName);
};

}

#endif // IOUTILS_H
