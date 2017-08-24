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

#include <qplatformdefs.h>
#include <qstring.h>
#include <qvector.h>
#include <qlist.h>
#include <qthreadstorage.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <qthread_p.h>
#include <qsystemlibrary_p.h>
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

#if defined(Q_OS_UNIX)
#include <sys/utsname.h>
#endif

QT_BEGIN_NAMESPACE

const char *qVersion()
{
   return CS_VERSION_STR;
}

bool qSharedBuild()
{
#ifdef QT_STATIC
   return false;
#else
   return true;
#endif
}

#if ! defined(QWS) && defined(Q_OS_MAC)

QT_BEGIN_INCLUDE_NAMESPACE
#include <qcore_mac_p.h>
#include <qnamespace.h>
QT_END_INCLUDE_NAMESPACE

static QSysInfo::MacVersion macVersion()
{
#if ! defined(Q_OS_IOS)
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
#include <qt_windows.h>
QT_END_INCLUDE_NAMESPACE


// Determine Windows versions >= 8 by querying the version of kernel32.dll
static inline bool determineWinOsVersionPost8(OSVERSIONINFO *result)
{
   typedef WORD (WINAPI* PtrGetFileVersionInfoSizeW)(LPCWSTR, LPDWORD);
   typedef BOOL (WINAPI* PtrVerQueryValueW)(LPCVOID, LPCWSTR, LPVOID, PUINT);
   typedef BOOL (WINAPI* PtrGetFileVersionInfoW)(LPCWSTR, DWORD, DWORD, LPVOID);

   QSystemLibrary versionLib(QLatin1String("version"));

   if (! versionLib.load()) {
     return false;
   }

   PtrGetFileVersionInfoSizeW getFileVersionInfoSizeW = (PtrGetFileVersionInfoSizeW)versionLib.resolve("GetFileVersionInfoSizeW");
   PtrVerQueryValueW verQueryValueW = (PtrVerQueryValueW)versionLib.resolve("VerQueryValueW");
   PtrGetFileVersionInfoW getFileVersionInfoW = (PtrGetFileVersionInfoW)versionLib.resolve("GetFileVersionInfoW");

   if (! getFileVersionInfoSizeW || !verQueryValueW || !getFileVersionInfoW) {
     return false;
   }

   const wchar_t kernel32Dll[] = L"kernel32.dll";
   DWORD handle;

   const DWORD size = getFileVersionInfoSizeW(kernel32Dll, &handle);
   if (! size) {
     return false;
   }

   QScopedArrayPointer<BYTE> versionInfo(new BYTE[size]);

   if (! getFileVersionInfoW(kernel32Dll, handle, size, versionInfo.data())) {
     return false;
   }

   UINT uLen;
   VS_FIXEDFILEINFO *fileInfo = 0;

   if (! verQueryValueW(versionInfo.data(), L"\\", (LPVOID *)&fileInfo, &uLen)) {
     return false;
   }

   const DWORD fileVersionMS = fileInfo->dwFileVersionMS;
   const DWORD fileVersionLS = fileInfo->dwFileVersionLS;

   result->dwMajorVersion = HIWORD(fileVersionMS);
   result->dwMinorVersion = LOWORD(fileVersionMS);
   result->dwBuildNumber  = HIWORD(fileVersionLS);

   return true;
}

// Fallback for determining Windows versions >= 8 by looping using the
// version check macros. Note that it will return build number = 0 to avoid inefficient looping.
static inline void determineWinOsVersionFallbackPost8(OSVERSIONINFO *result)
{
   result->dwBuildNumber   = 0;
   DWORDLONG conditionMask = 0;

   VER_SET_CONDITION(conditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
   VER_SET_CONDITION(conditionMask, VER_PLATFORMID, VER_EQUAL);

   OSVERSIONINFOEX checkVersion = { sizeof(OSVERSIONINFOEX), result->dwMajorVersion, 0,
                                  result->dwBuildNumber, result->dwPlatformId, {'\0'}, 0, 0, 0, 0, 0 };

   while (VerifyVersionInfo(&checkVersion, VER_MAJORVERSION | VER_PLATFORMID, conditionMask)) {
     result->dwMajorVersion = checkVersion.dwMajorVersion;

     ++checkVersion.dwMajorVersion;
   }

   conditionMask = 0;
   checkVersion.dwMajorVersion = result->dwMajorVersion;
   checkVersion.dwMinorVersion = 0;

   VER_SET_CONDITION(conditionMask, VER_MAJORVERSION, VER_EQUAL);
   VER_SET_CONDITION(conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
   VER_SET_CONDITION(conditionMask, VER_PLATFORMID, VER_EQUAL);

   while (VerifyVersionInfo(&checkVersion, VER_MAJORVERSION | VER_MINORVERSION | VER_PLATFORMID, conditionMask)) {
     result->dwMinorVersion = checkVersion.dwMinorVersion;

     ++checkVersion.dwMinorVersion;
   }
}

static inline OSVERSIONINFO winOsVersion()
{
   OSVERSIONINFO result = { sizeof(OSVERSIONINFO), 0, 0, 0, 0, {'\0'}};

   // GetVersionEx() has been deprecated in Windows 8.1 and will return
   // only Windows 8 from that version on.

   GetVersionEx(&result);

   if (result.dwMajorVersion == 6 && result.dwMinorVersion == 2) {

      if (! determineWinOsVersionPost8(&result)) {
         determineWinOsVersionFallbackPost8(&result);
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

         } else if (osver.dwMinorVersion == 10) {
            winver = QSysInfo::WV_98;

         } else {
            winver = QSysInfo::WV_95;
         }

         break;

      default:
          // VER_PLATFORM_WIN32_NT

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

          } else if (osver.dwMajorVersion == 10 && osver.dwMinorVersion == 0) {
             winver = QSysInfo::WV_WINDOWS10;

          } else {
             qWarning("Qt: Untested Windows version %d.%d detected", int(osver.dwMajorVersion), int(osver.dwMinorVersion));
                   winver = QSysInfo::WV_NT_based;
          }

         }

#ifdef QT_DEBUG
   {
      QByteArray override = qgetenv("QT_WINVER_OVERRIDE");

      if (override.isEmpty()) {
         return winver;
      }

      if (override == "NT") {
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

      } else if (override == "WINDOWS8.1") {
         winver = QSysInfo::WV_WINDOWS8_1;

      } else if (override == "WINDOWS10") {
         winver = QSysInfo::WV_WINDOWS10;

      }
   }
#endif

   return winver;
}

const QSysInfo::WinVersion QSysInfo::WindowsVersion = QSysInfo::windowsVersion();


//
class QWindowsSockInit
{
   public:
      QWindowsSockInit();
      ~QWindowsSockInit();

      int version;
};

QWindowsSockInit::QWindowsSockInit()
   :  version(0)
{
   WSAData wsadata;

   // IPv6 requires Winsock v2.0 or better
   if (WSAStartup(MAKEWORD(2,0), &wsadata) != 0) {
      qWarning("QTcpSocketAPI: WinSock v2.0 initialization failed.");
    } else {
       version = 0x20;
    }
}

QWindowsSockInit::~QWindowsSockInit()
{
   WSACleanup();
}

#endif

QString QSysInfo::machineHostName()
{

#if defined(Q_OS_LINUX)
   // gethostname(3) on Linux just calls uname(2)
   struct utsname u;

   if (uname(&u) == 0) {
      return QString::fromLocal8Bit(u.nodename);
   }

#else

#  ifdef Q_OS_WIN
    // QtNetwork depends on machineHostName() initializing ws2_32.dll
    static QWindowsSockInit winSock;
#  endif

   char hostName[512];
   if (gethostname(hostName, sizeof(hostName)) == -1) {
      return QString();
   }

   hostName[sizeof(hostName) - 1] = '\0';
   return QString::fromLocal8Bit(hostName);
#endif

   return QString();
}


/*
  The Q_CHECK_PTR macro calls this function if an allocation check fails.
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
    Deliberately not exported as part of the API, but used in both
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


#if ! defined(Q_OS_WIN) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && _POSIX_VERSION >= 200112L
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

#if defined(Q_OS_WIN) && defined(QT_BUILD_CORE_LIB)
extern bool usingWinMain;
extern Q_CORE_EXPORT void qWinMsgHandler(QtMsgType t, const char *str);
#endif

QtMsgHandler qInstallMsgHandler(QtMsgHandler h)
{
   QtMsgHandler old = handler;
   handler = h;

#if defined(Q_OS_WIN) && defined(QT_BUILD_CORE_LIB)
   if (! handler && usingWinMain) {
      handler = qWinMsgHandler;
   }
#endif

   return old;
}

// internal
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
      abort();    // generates core dump
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
      } QT_CATCH(const std::bad_alloc &)  {

         qEmergencyOut(msgType, msg, ap);

         // do not rethrow - we use qWarning and friends in destructors.
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

      if (! pseed) {
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
