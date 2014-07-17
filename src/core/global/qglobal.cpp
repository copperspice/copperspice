/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qplatformdefs.h>
#include <qstring.h>
#include <qvector.h>
#include <qlist.h>
#include <qthreadstorage.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <qthread_p.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>

#include <string>
#include <exception>
#include <errno.h>

#if defined(Q_OS_MAC) && ! defined(Q_OS_IOS)
#include <CoreServices/CoreServices.h>
#endif

QT_BEGIN_NAMESPACE

const char *qVersion()
{
   return CS_VERSION_STR;
}

bool qSharedBuild()
{
#ifdef QT_SHARED
   return true;
#else
   return false;
#endif
}

#if !defined(QWS) && defined(Q_OS_MAC)

QT_BEGIN_INCLUDE_NAMESPACE
#include "qcore_mac_p.h"
#include "qnamespace.h"
QT_END_INCLUDE_NAMESPACE

static QSysInfo::MacVersion macVersion()
{
#if !defined(Q_OS_IOS)
   SInt32 gestalt_version;
   if (Gestalt(gestaltSystemVersionMinor, &gestalt_version) == noErr) {
      // add 2 because OS X 10.0 is 0x02 in the enum
      return QSysInfo::MacVersion(gestalt_version + 2);
   }
#endif
   return QSysInfo::MV_Unknown;
}
const QSysInfo::MacVersion QSysInfo::MacintoshVersion = macVersion();

#elif defined(Q_OS_WIN32)

QT_BEGIN_INCLUDE_NAMESPACE
#include "qt_windows.h"
QT_END_INCLUDE_NAMESPACE

static inline OSVERSIONINFO winOsVersion()
{
   OSVERSIONINFO result = { sizeof(OSVERSIONINFO), 0, 0, 0, 0, {'\0'}};
   // GetVersionEx() has been deprecated in Windows 8.1 and will return
   // only Windows 8 from that version on.

#  if defined(_MSC_VER) && _MSC_VER >= 1800
#    pragma warning( push )
#    pragma warning( disable : 4996 )
#  endif

   GetVersionEx(&result);

#  if defined(_MSC_VER) && _MSC_VER >= 1800
#    pragma warning( pop )
#  endif


   if (result.dwMajorVersion == 6 && result.dwMinorVersion == 2) {
      // This could be Windows 8.1 or higher. Note that as of Windows 9,
      // the major version needs to be checked as well.
      DWORDLONG conditionMask = 0;
      VER_SET_CONDITION(conditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
      VER_SET_CONDITION(conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
      VER_SET_CONDITION(conditionMask, VER_PLATFORMID, VER_EQUAL);
      OSVERSIONINFOEX checkVersion = { sizeof(OSVERSIONINFOEX), result.dwMajorVersion, result.dwMinorVersion,
				       result.dwBuildNumber, result.dwPlatformId, {'\0'}, 0, 0, 0, 0, 0
				     };

      for ( ;
	    VerifyVersionInfo(&checkVersion, VER_MAJORVERSION | VER_MINORVERSION | VER_PLATFORMID, conditionMask);
	    ++checkVersion.dwMinorVersion) {

	 result.dwMinorVersion = checkVersion.dwMinorVersion;
      }
   }

   return result;
}

QSysInfo::WinVersion QSysInfo::windowsVersion()
{
   static QSysInfo::WinVersion winver;
   if (winver) {
      return winver;
   }

   winver = QSysInfo::WV_NT;
   const OSVERSIONINFO osver = winOsVersion();
   switch (osver.dwPlatformId) {

      case VER_PLATFORM_WIN32s:
	 winver = QSysInfo::WV_32s;
	 break;

      case VER_PLATFORM_WIN32_WINDOWS:
	 // Windows Me (minor 90) is the same as Windows 98
	 if (osver.dwMinorVersion == 90) {
	    winver = QSysInfo::WV_Me;
	 }

	 else if (osver.dwMinorVersion == 10) {
	    winver = QSysInfo::WV_98;
	 }

	 else {
	    winver = QSysInfo::WV_95;
	 }

	 break;

      default: // VER_PLATFORM_WIN32_NT
	 if (osver.dwMajorVersion < 5) {
	    winver = QSysInfo::WV_NT;
	 } else if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 0) {
	    winver = QSysInfo::WV_2000;
	 } else if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 1) {
	    winver = QSysInfo::WV_XP;
	 } else if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 2) {
	    winver = QSysInfo::WV_2003;
	 } else if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 0) {
	    winver = QSysInfo::WV_VISTA;
	 } else if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 1) {
	    winver = QSysInfo::WV_WINDOWS7;
	 } else if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 2) {
	    winver = QSysInfo::WV_WINDOWS8;
	 } else if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 3) {
	    winver = QSysInfo::WV_WINDOWS8_1;
	 } else {
	    qWarning("Qt: Untested Windows version %d.%d detected!",
		     int(osver.dwMajorVersion), int(osver.dwMinorVersion));

	    winver = QSysInfo::WV_NT_based;
	 }
   }

#ifdef QT_DEBUG
   {
      QByteArray override = qgetenv("QT_WINVER_OVERRIDE");
      if (override.isEmpty()) {
	 return winver;
      }

      if (override == "Me") {
	 winver = QSysInfo::WV_Me;
      }
      if (override == "95") {
	 winver = QSysInfo::WV_95;
      } else if (override == "98") {
	 winver = QSysInfo::WV_98;
      } else if (override == "NT") {
	 winver = QSysInfo::WV_NT;
      } else if (override == "2000") {
	 winver = QSysInfo::WV_2000;
      } else if (override == "2003") {
	 winver = QSysInfo::WV_2003;
      } else if (override == "XP") {
	 winver = QSysInfo::WV_XP;
      } else if (override == "VISTA") {
	 winver = QSysInfo::WV_VISTA;
      } else if (override == "WINDOWS7") {
	 winver = QSysInfo::WV_WINDOWS7;
      } else if (override == "WINDOWS8") {
	 winver = QSysInfo::WV_WINDOWS8;
      }
   }
#endif

   return winver;
}

const QSysInfo::WinVersion QSysInfo::WindowsVersion = QSysInfo::windowsVersion();

#endif



/*
  The Q_CHECK_PTR macro calls this function if an allocation check
  fails.
*/
void qt_check_pointer(const char *n, int l)
{
   qFatal("In file %s, line %d: Out of memory", n, l);
}

/* \internal
   Allows you to throw an exception without including <new>
   Called internally from Q_CHECK_PTR on certain OS combinations
*/
void qBadAlloc()
{
   QT_THROW(std::bad_alloc());
}

/*
  The Q_ASSERT macro calls this function when the test fails.
*/
void qt_assert(const char *assertion, const char *file, int line)
{
   qFatal("ASSERT: \"%s\" in file %s, line %d", assertion, file, line);
}

/*
  The Q_ASSERT_X macro calls this function when the test fails.
*/
void qt_assert_x(const char *where, const char *what, const char *file, int line)
{
   qFatal("ASSERT failure in %s: \"%s\", file %s, line %d", where, what, file, line);
}


/*
    Dijkstra's bisection algorithm to find the square root of an integer.
    Deliberately not exported as part of the Qt API, but used in both
    qsimplerichtext.cpp and qgfxraster_qws.cpp
*/
Q_CORE_EXPORT unsigned int qt_int_sqrt(unsigned int n)
{
   // n must be in the range 0...UINT_MAX/2-1
   if (n >= (UINT_MAX >> 2)) {
      unsigned int r = 2 * qt_int_sqrt(n / 4);
      unsigned int r2 = r + 1;
      return (n >= r2 * r2) ? r2 : r;
   }
   uint h, p = 0, q = 1, r = n;
   while (q <= n) {
      q <<= 2;
   }
   while (q != 1) {
      q >>= 2;
      h = p + q;
      p >>= 1;
      if (r >= h) {
	 p += q;
	 r -= h;
      }
   }
   return p;
}

void *qMemCopy(void *dest, const void *src, size_t n)
{
   return memcpy(dest, src, n);
}
void *qMemSet(void *dest, int c, size_t n)
{
   return memset(dest, c, n);
}

static QtMsgHandler handler = 0;                // pointer to debug handler


#if !defined(Q_OS_WIN) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && _POSIX_VERSION >= 200112L
namespace {
// There are two incompatible versions of strerror_r:
// a) the XSI/POSIX.1 version, which returns an int,
//    indicating success or not
// b) the GNU version, which returns a char*, which may or may not
//    be the beginning of the buffer we used
// The GNU libc manpage for strerror_r says you should use the the XSI
// version in portable code. However, it's impossible to do that if
// _GNU_SOURCE is defined so we use C++ overloading to decide what to do
// depending on the return type
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
   const char *s = 0;
   QString ret;
   if (errorCode == -1) {
#if defined(Q_OS_WIN)
      errorCode = GetLastError();
#else
      errorCode = errno;
#endif
   }
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
		       NULL,
		       errorCode,
		       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		       (LPWSTR)&string,
		       0,
		       NULL);
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

#if defined(Q_OS_WIN) && defined(QT_BUILD_CORE_LIB)
extern bool usingWinMain;
extern Q_CORE_EXPORT void qWinMsgHandler(QtMsgType t, const char *str);
#endif

QtMsgHandler qInstallMsgHandler(QtMsgHandler h)
{
   QtMsgHandler old = handler;
   handler = h;
#if defined(Q_OS_WIN) && defined(QT_BUILD_CORE_LIB)
   if (!handler && usingWinMain) {
      handler = qWinMsgHandler;
   }
#endif
   return old;
}

/*!
    \internal
*/
void qt_message_output(QtMsgType msgType, const char *buf)
{
   if (handler) {
      (*handler)(msgType, buf);
   } else {
      fprintf(stderr, "%s\n", buf);
      fflush(stderr);
   }

   if (msgType == QtFatalMsg || (msgType == QtWarningMsg && (!qgetenv("QT_FATAL_WARNINGS").isNull())) ) {

#if (defined(Q_OS_UNIX) || defined(Q_CC_MINGW))
      abort(); // trap; generates core dump
#else
      exit(1);
#endif

   }
}

// internal
static void qEmergencyOut(QtMsgType msgType, const char *msg, va_list ap)
{
   char emergency_buf[256] = { '\0' };
   emergency_buf[255] = '\0';
   if (msg) {
      qvsnprintf(emergency_buf, 255, msg, ap);
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
      } QT_CATCH(const std::bad_alloc &) {
	 qEmergencyOut(msgType, msg, ap);
	 // don't rethrow - we use qWarning and friends in destructors.
	 return;
      }
   }
   qt_message_output(msgType, buf.constData());
}

#undef qDebug

void qDebug(const char *msg, ...)
{
   va_list ap;
   va_start(ap, msg); // use variable arg list
   qt_message(QtDebugMsg, msg, ap);
   va_end(ap);
}

#undef qWarning

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

QByteArray qgetenv(const char *varName)
{
   return QByteArray(::getenv(varName));
}

bool qputenv(const char *varName, const QByteArray &value)
{
   QByteArray buffer(varName);
   buffer += '=';
   buffer += value;
   char *envVar = qstrdup(buffer.constData());
   int result = putenv(envVar);
   if (result != 0) { // error. we have to delete the string.
      delete[] envVar;
   }

   return result == 0;
}

#if defined(Q_OS_UNIX)

typedef uint SeedStorageType;
typedef QThreadStorage<SeedStorageType *> SeedStorage;
Q_GLOBAL_STATIC(SeedStorage, randTLS)  // Thread Local Storage for seed value

#endif

void qsrand(uint seed)
{
#if defined(Q_OS_UNIX)
   SeedStorage *seedStorage = randTLS();
   if (seedStorage) {
      SeedStorageType *pseed = seedStorage->localData();
      if (!pseed) {
	 seedStorage->setLocalData(pseed = new SeedStorageType);
      }
      *pseed = seed;
   } else {
      //global static seed storage should always exist,
      //except after being deleted by QGlobalStaticDeleter.
      //But since it still can be called from destructor of another
      //global static object, fallback to srand(seed)
      srand(seed);
   }
#else
   srand(seed);
#endif
}


int qrand()
{

#if defined(Q_OS_UNIX)
   SeedStorage *seedStorage = randTLS();
   if (seedStorage) {
      SeedStorageType *pseed = seedStorage->localData();
      if (!pseed) {
	 seedStorage->setLocalData(pseed = new SeedStorageType);
	 *pseed = 1;
      }
      return rand_r(pseed);

   } else {
      //global static seed storage should always exist,
      //except after being deleted by QGlobalStaticDeleter.
      //But since it still can be called from destructor of another
      //global static object, fallback to rand()
      return rand();
   }
#else
   return rand();

#endif
}

QT_END_NAMESPACE
