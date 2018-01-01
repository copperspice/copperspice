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

#include <qsystemlibrary_p.h>

#include <QtCore/qvarlengtharray.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qfileinfo.h>

QT_BEGIN_NAMESPACE

extern QString qAppFileName();


static inline QString qSystemDirectory()
{
   QVarLengthArray<wchar_t, MAX_PATH> fullPath;

   UINT retLen = ::GetSystemDirectory(fullPath.data(), MAX_PATH);
   if (retLen > MAX_PATH) {
      fullPath.resize(retLen);
      retLen = ::GetSystemDirectory(fullPath.data(), retLen);
   }
   // in some rare cases retLen might be 0
   return QString::fromWCharArray(fullPath.constData(), int(retLen));
}

HINSTANCE QSystemLibrary::load(const wchar_t *libraryName, bool onlySystemDirectory /* = true */)
{
   QStringList searchOrder;

   if (!onlySystemDirectory) {
      searchOrder << QFileInfo(qAppFileName()).path();
   }

   searchOrder << qSystemDirectory();

   if (!onlySystemDirectory) {
      const QString PATH = QString::fromWCharArray((const wchar_t *)_wgetenv(L"PATH"));
      searchOrder << PATH.split(QLatin1Char(';'), QString::SkipEmptyParts);
   }

   const QString fileName = QString::fromWCharArray(libraryName) + QLatin1String(".dll");
   // Start looking in the order specified
   for (int i = 0; i < searchOrder.count(); ++i) {
      QString fullPathAttempt = searchOrder.at(i);
      if (!fullPathAttempt.endsWith(QLatin1Char('\\'))) {
         fullPathAttempt.append(QLatin1Char('\\'));
      }
      fullPathAttempt.append(fileName);
      HINSTANCE inst = ::LoadLibrary((const wchar_t *)fullPathAttempt.utf16());
      if (inst != 0) {
         return inst;
      }
   }

   return 0;
}


QT_END_NAMESPACE
