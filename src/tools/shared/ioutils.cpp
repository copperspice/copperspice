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

#include "ioutils.h"

#include <QtCore/QDir>
#include <QtCore/QFile>

#ifdef Q_OS_WIN
#  include <windows.h>
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <unistd.h>
#endif

using namespace ProFileEvaluatorInternal;

IoUtils::FileType IoUtils::fileType(const QString &fileName)
{
   Q_ASSERT(fileName.isEmpty() || isAbsolutePath(fileName));
#ifdef Q_OS_WIN
   DWORD attr = GetFileAttributesW((WCHAR *)fileName.utf16());
   if (attr == INVALID_FILE_ATTRIBUTES) {
      return FileNotFound;
   }
   return (attr & FILE_ATTRIBUTE_DIRECTORY) ? FileIsDir : FileIsRegular;
#else
   struct ::stat st;
   if (::stat(fileName.toLocal8Bit().constData(), &st)) {
      return FileNotFound;
   }
   return S_ISDIR(st.st_mode) ? FileIsDir : FileIsRegular;
#endif
}

bool IoUtils::isRelativePath(const QString &path)
{
   if (path.startsWith(QLatin1Char('/'))) {
      return false;
   }
#ifdef Q_OS_WIN
   if (path.startsWith(QLatin1Char('\\'))) {
      return false;
   }
   // Unlike QFileInfo, this won't accept a relative path with a drive letter.
   // Such paths result in a royal mess anyway ...
   if (path.length() >= 3 && path.at(1) == QLatin1Char(':') && path.at(0).isLetter()
         && (path.at(2) == QLatin1Char('/') || path.at(2) == QLatin1Char('\\'))) {
      return false;
   }
#endif
   return true;
}

QStringRef IoUtils::fileName(const QString &fileName)
{
   return fileName.midRef(fileName.lastIndexOf(QLatin1Char('/')) + 1);
}

QString IoUtils::resolvePath(const QString &baseDir, const QString &fileName)
{
   if (fileName.isEmpty()) {
      return QString();
   }
   if (isAbsolutePath(fileName)) {
      return QDir::cleanPath(fileName);
   }
   return QDir::cleanPath(baseDir + QLatin1Char('/') + fileName);
}
