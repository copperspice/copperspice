/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#include <qglobal.h>
#include <qlog.h>
#include <qsystemerror_p.h>

#include <errno.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#if !defined(Q_OS_WIN) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && _POSIX_VERSION >= 200112L
namespace {

static inline QString fromstrerror_helper(int, const QByteArray &buf)
{
   return QString::fromLocal8Bit(buf);
}

static inline QString fromstrerror_helper(const char *str, const QByteArray &)
{
   return QString::fromLocal8Bit(str);
}
}
#endif

#ifdef Q_OS_WIN
static QString windowsErrorString(int errorCode)
{
   QString ret;
   wchar_t *string = 0;

   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                 NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 (LPWSTR)&string, 0, NULL);

   ret = QString::fromWCharArray(string);
   LocalFree((HLOCAL)string);

   if (ret.isEmpty() && errorCode == ERROR_MOD_NOT_FOUND) {
      ret = QString::fromLatin1("The specified module could not be found.");
   }
   return ret;
}
#endif

static QString standardLibraryErrorString(int errorCode)
{
   const char *s = 0;
   QString ret;
   switch (errorCode) {
      case 0:
         break;

      case EACCES:
         s = QT_TRANSLATE_NOOP("QIODevice", "Permission denied");
         break;

      case EMFILE:
         s = QT_TRANSLATE_NOOP("QIODevice", "Too many open files");
         break;

      case ENOENT:
         s = QT_TRANSLATE_NOOP("QIODevice", "No such file or directory");
         break;

      case ENOSPC:
         s = QT_TRANSLATE_NOOP("QIODevice", "No space left on device");
         break;

      default: {

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && _POSIX_VERSION >= 200112L
         QByteArray buf(1024, '\0');
         ret = fromstrerror_helper(strerror_r(errorCode, buf.data(), buf.size()), buf);
#else
         ret = QString::fromLocal8Bit(strerror(errorCode));
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
         // unix: fall through as native and standard library are the same
#endif

      case StandardLibraryError:
         return standardLibraryErrorString(errorCode);

      case NoError:
         return QLatin1String("No error");

      default:
         qWarning("Invalid error scope");
         return QLatin1String("Unrecognized error");
   }
}

