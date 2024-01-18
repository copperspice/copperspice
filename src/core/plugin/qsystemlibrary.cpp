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

#include <qsystemlibrary_p.h>

#include <qvarlengtharray.h>
#include <qstringlist.h>
#include <qstringparser.h>
#include <qfileinfo.h>

extern QString qAppFileName();

static QString qSystemDirectory()
{
   std::wstring fullPath(MAX_PATH, L'\0');

   UINT retLen = ::GetSystemDirectory(&fullPath[0], MAX_PATH);

   if (retLen > MAX_PATH) {
      fullPath.resize(retLen);
      retLen = ::GetSystemDirectory(&fullPath[0], retLen);
   }

   // in some rare cases retLen might be 0
   return QString::fromStdWString(fullPath, int(retLen));
}

HINSTANCE QSystemLibrary::load(const QString &libraryName, bool onlySystemDirectory)
{
   QStringList searchOrder;

   if (! onlySystemDirectory) {
      searchOrder << QFileInfo(qAppFileName()).path();
   }

   searchOrder << qSystemDirectory();

   if (! onlySystemDirectory) {
      const QString path = QString::fromUtf16((const char16_t *)_wgetenv(L"PATH"));
      searchOrder << path.split(';', QStringParser::SkipEmptyParts);
   }

   const QString fileName = libraryName + ".dll";

   // Start looking in the order specified
   for (int i = 0; i < searchOrder.count(); ++i) {

      QString fullPathAttempt = searchOrder.at(i);

      if (! fullPathAttempt.endsWith('\\')) {
         fullPathAttempt.append('\\');
      }

      fullPathAttempt.append(fileName);
      HINSTANCE inst = ::LoadLibrary(&fullPathAttempt.toStdWString()[0]);

      if (inst != nullptr) {
         return inst;
      }
   }

   return nullptr;
}

