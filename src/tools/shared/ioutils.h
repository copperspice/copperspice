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

#ifndef IOUTILS_H
#define IOUTILS_H

#include <QtCore/QString>

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
   static QStringRef fileName(const QString &fileName); // Requires normalized path
   static QString resolvePath(const QString &baseDir, const QString &fileName);
};

}

#endif // IOUTILS_H
