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

#include <errno.h>

#include <qglobal.h>
#include <qlog.h>

#include <qsystemerror_p.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#if ! defined(Q_OS_WIN) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && _POSIX_VERSION >= 200112L

namespace {

static inline QString fromstrerror_helper(int, const QByteArray &buf)
{
   return QString::fromUtf8(buf);
}

static inline QString fromstrerror_helper(const char *str, const QByteArray &)
{
   return QString::fromUtf8(str);
}

}     // end namespace
#endif

#ifdef Q_OS_WIN
static QString windowsErrorString(int errorCode)
{
   QString ret;
   wchar_t *buffer = nullptr;

   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
         nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&buffer, 0, nullptr);

   if (buffer != nullptr) {
      ret = QString::fromStdWString(std::wstring(buffer));
      LocalFree((HLOCAL)buffer);
   }

   if (ret.isEmpty() && errorCode == ERROR_MOD_NOT_FOUND) {
      ret = QString("The specified module could not be found.");
   }

   return ret;
}
#endif

static QString standardLibraryErrorString(int errorCode)
{
   const char *s = nullptr;
   QString ret;

   switch (errorCode) {
      case 0:
         break;

      case EACCES:
         s = cs_mark_tr("QIODevice", "Permission denied");
         break;

      case EMFILE:
         s = cs_mark_tr("QIODevice", "Too many open files");
         break;

      case ENOENT:
         s = cs_mark_tr("QIODevice", "No such file or directory");
         break;

      case ENOSPC:
         s = cs_mark_tr("QIODevice", "No space left on device");
         break;

      default: {

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && _POSIX_VERSION >= 200112L
         QByteArray buf(1024, '\0');
         ret = fromstrerror_helper(strerror_r(errorCode, buf.data(), buf.size()), buf);
#else
         ret = QString::fromUtf8(strerror(errorCode));
#endif

         break;
      }
   }

   if (s) {
      ret = QString::fromLatin1(s);
   }

   return ret.trimmed();
}

QString QSystemError::toString()
{
   switch (errorScope) {
      case NativeError:

#if defined (Q_OS_WIN)
         return windowsErrorString(errorCode);
#else
         // unix: native and standard library are the same
         [[fallthrough]];
#endif

      case StandardLibraryError:
         return standardLibraryErrorString(errorCode);

      case NoError:
         return QString("No error");

      default:
         qWarning("QSystemError::toString() Invalid error scope");
         return QString("Unrecognized error");
   }
}
