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

#ifndef QSYSTEMLIBRARY_P_H
#define QSYSTEMLIBRARY_P_H

#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QSystemLibrary
{
 public:
   explicit QSystemLibrary(const QString &libraryName) {
      m_libraryName = libraryName;
      m_handle = 0;
      m_didLoad = false;
   }

   explicit QSystemLibrary(const wchar_t *libraryName) {
      m_libraryName = QString::fromWCharArray(libraryName);
      m_handle = 0;
      m_didLoad = false;
   }

   bool load(bool onlySystemDirectory = true) {
      m_handle = load((const wchar_t *)m_libraryName.utf16(), onlySystemDirectory);
      m_didLoad = true;
      return (m_handle != 0);
   }

   bool isLoaded() {
      return (m_handle != 0);
   }

   void *resolve(const char *symbol) {
      if (!m_didLoad) {
         load();
      }
      if (!m_handle) {
         return 0;
      }

      return (void *)GetProcAddress(m_handle, symbol);

   }

   static void *resolve(const QString &libraryName, const char *symbol) {
      return QSystemLibrary(libraryName).resolve(symbol);
   }

   static Q_CORE_EXPORT HINSTANCE load(const wchar_t *lpFileName, bool onlySystemDirectory = true);

 private:
   HINSTANCE m_handle;
   QString m_libraryName;
   bool m_didLoad;
};

QT_END_NAMESPACE

#endif // Q_OS_WIN

#endif // QSYSTEMLIBRARY_P_H
