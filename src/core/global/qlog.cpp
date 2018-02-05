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

#include <cstdlib>
#include <cstdio>

#include <qlog.h>

#if defined(Q_OS_WIN)
#include <qt_windows.h>
#endif

#if defined(Q_OS_WIN) && defined(QT_BUILD_CORE_LIB)
extern bool usingWinMain;
extern Q_CORE_EXPORT void qWinMsgHandler(QtMsgType t, const char *str);
#endif

static QtMsgHandler s_handler = nullptr;          // pointer to debug handler


#if ! defined(Q_OS_WIN) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && _POSIX_VERSION >= 200112L
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


QString qt_error_string(int errorCode)
{
   if (errorCode == -1) {

#if defined(Q_OS_WIN)
      errorCode = GetLastError();
#else
      errorCode = errno;
#endif
   }

   const char *s = nullptr;
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

#ifdef Q_OS_WIN
         wchar_t *string = 0;
         FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
             NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
             (LPWSTR)&string, 0, NULL);

         ret = QString::fromWCharArray(string);
         LocalFree((HLOCAL)string);

         if (ret.isEmpty() && errorCode == ERROR_MOD_NOT_FOUND) {
            ret = QString::fromLatin1("The specified module could not be found.");
         }

#elif defined(_POSIX_THREAD_SAFE_FUNCTIONS) && _POSIX_VERSION >= 200112L
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

QtMsgHandler qInstallMsgHandler(QtMsgHandler h)
{
   QtMsgHandler old = s_handler;
   s_handler = h;

#if defined(Q_OS_WIN)
   if (! s_handler && usingWinMain) {
      s_handler = qWinMsgHandler;
   }
#endif

   return old;
}


// internal
void qt_message_output(QtMsgType msgType, const char *buf)
{
   if (s_handler) {
      (*s_handler)(msgType, buf);

   } else {
      fprintf(stderr, "%s\n", buf);
      fflush(stderr);
   }

   if (msgType == QtFatalMsg || (msgType == QtWarningMsg && (! qgetenv("QT_FATAL_WARNINGS").isNull())) ) {

#if (defined(Q_OS_UNIX) || defined(Q_CC_MINGW))
      abort();
#else
      exit(1);
#endif

   }
}

// internal
static void qEmergencyOut(QtMsgType msgType, const char *msg, va_list ap)
{
   char emergency_buf[256] = { '\0' };
   emergency_buf[255]      = '\0';

   if (msg) {
      std::vsnprintf(emergency_buf, 255, msg, ap);
   }

   qt_message_output(msgType, emergency_buf);
}

// internal
static void qt_message(QtMsgType msgType, const char *msg, va_list ap)
{
   if (std::uncaught_exception()) {
      qEmergencyOut(msgType, msg, ap);
      return;
   }

   QByteArray buf;

   if (msg) {
      QT_TRY {
         buf = QString().vsprintf(msg, ap).toLocal8Bit();

      } QT_CATCH(const std::bad_alloc &)  {
         qEmergencyOut(msgType, msg, ap);

         // do not rethrow, use qWarning and friends in destructors
         return;
      }
   }

   qt_message_output(msgType, buf.constData());
}

//
void qDebug(const char *msg, ...)
{
   va_list ap;
   va_start(ap, msg); // use variable arg list
   qt_message(QtDebugMsg, msg, ap);
   va_end(ap);
}

void qWarning(const char *msg, ...)
{
   va_list ap;
   va_start(ap, msg); // use variable arg list
   qt_message(QtWarningMsg, msg, ap);
   va_end(ap);
}

void qCritical(const char *msg, ...)
{
   va_list ap;
   va_start(ap, msg); // use variable arg list
   qt_message(QtCriticalMsg, msg, ap);
   va_end(ap);
}

void qErrnoWarning(const char *msg, ...)
{
   // qt_error_string() will allocate anyway, so we don't have
   // to be careful here (like we do in plain qWarning())
   QString buf;

   va_list ap;
   va_start(ap, msg);

   if (msg) {
      buf.vsprintf(msg, ap);
   }
   va_end(ap);

   qCritical("%s (%s)", buf.toLocal8Bit().constData(), qt_error_string(-1).toLocal8Bit().constData());
}

void qErrnoWarning(int code, const char *msg, ...)
{
   // qt_error_string() will allocate anyway, so we don't have
   // to be careful here (like we do in plain qWarning())
   QString buf;

   va_list ap;
   va_start(ap, msg);
   if (msg) {
      buf.vsprintf(msg, ap);
   }
   va_end(ap);

   qCritical("%s (%s)", buf.toLocal8Bit().constData(), qt_error_string(code).toLocal8Bit().constData());
}

void qFatal(const char *msg, ...)
{
   va_list ap;
   va_start(ap, msg); // use variable arg list
   qt_message(QtFatalMsg, msg, ap);
   va_end(ap);
}