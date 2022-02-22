/***********************************************************************
*
* Copyright (c) 2012-2022 Barbara Geller
* Copyright (c) 2012-2022 Ansel Sermersheim
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

#include <limits>
#include <mutex>
#include <stdlib.h>

#include <qglobal.h>
#include <qlog.h>
#include <qbytearray.h>
#include <qstring.h>
#include <qscopedarraypointer.h>
#include <qsystemlibrary_p.h>
#include <qthreadstorage.h>

#if defined(Q_OS_DARWIN) && ! defined(Q_OS_IOS)
#include <CoreServices/CoreServices.h>
#endif

#if defined(Q_OS_UNIX)
#include <sys/utsname.h>
#include <unistd.h>
#endif

const char *csVersion()
{
   return CS_VERSION_STR;
}

// ** OSX
#if ! defined(QWS) && defined(Q_OS_DARWIN)

#include <qcore_mac_p.h>
#include <qnamespace.h>

static QSysInfo::MacVersion macVersion()
{
   // qcore_mac_objc.mm
   const QAppleOperatingSystemVersion version = qt_apple_os_version();

#if defined(Q_OS_DARWIN) && ! defined(Q_OS_IOS)

   if (version.major == 10) {
      return QSysInfo::MacVersion(version.minor + 2);

   } else {
      return QSysInfo::MV_Unknown;

   }

#elif defined(Q_OS_IOS)
   return QSysInfo::MV_IOS;

#else
   return QSysInfo::MV_Unknown;

#endif

}

const QSysInfo::MacVersion QSysInfo::MacintoshVersion = macVersion();

// ** Windows
#elif defined(Q_OS_WIN)

#include <qt_windows.h>

// determine versions >= 8 by querying the version of kernel32.dll
static inline bool determineWinOsVersionPost8(OSVERSIONINFO *result)
{
   typedef WORD (WINAPI* PtrGetFileVersionInfoSizeW)(LPCWSTR, LPDWORD);
   typedef BOOL (WINAPI* PtrVerQueryValueW)(LPCVOID, LPCWSTR, LPVOID, PUINT);
   typedef BOOL (WINAPI* PtrGetFileVersionInfoW)(LPCWSTR, DWORD, DWORD, LPVOID);

   QSystemLibrary versionLib("version");

   if (! versionLib.load()) {
     return false;
   }

   PtrGetFileVersionInfoSizeW getFileVersionInfoSizeW =
         (PtrGetFileVersionInfoSizeW)versionLib.resolve("GetFileVersionInfoSizeW");

   PtrVerQueryValueW verQueryValueW = (PtrVerQueryValueW)versionLib.resolve("VerQueryValueW");
   PtrGetFileVersionInfoW getFileVersionInfoW = (PtrGetFileVersionInfoW)versionLib.resolve("GetFileVersionInfoW");

   if (! getFileVersionInfoSizeW || ! verQueryValueW || ! getFileVersionInfoW) {
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
   VS_FIXEDFILEINFO *fileInfo = nullptr;

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
             qWarning("Untested Windows version %d.%d detected", int(osver.dwMajorVersion), int(osver.dwMinorVersion));
                   winver = QSysInfo::WV_NT_based;
          }

         }

#ifdef QT_DEBUG
   {
      QByteArray forceWinVer = qgetenv("QT_WINVER_OVERRIDE");

      if (forceWinVer.isEmpty()) {
         return winver;
      }

      if (forceWinVer == "NT") {
         winver = QSysInfo::WV_NT;

      } else if (forceWinVer == "2000") {
         winver = QSysInfo::WV_2000;

      } else if (forceWinVer == "2003") {
         winver = QSysInfo::WV_2003;

      } else if (forceWinVer == "XP") {
          winver = QSysInfo::WV_XP;

      } else if (forceWinVer == "VISTA") {
          winver = QSysInfo::WV_VISTA;

      } else if (forceWinVer == "WINDOWS7") {
          winver = QSysInfo::WV_WINDOWS7;

      } else if (forceWinVer == "WINDOWS8") {
         winver = QSysInfo::WV_WINDOWS8;

      } else if (forceWinVer == "WINDOWS8.1") {
         winver = QSysInfo::WV_WINDOWS8_1;

      } else if (forceWinVer == "WINDOWS10") {
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

#endif  // end of windows block

QString QSysInfo::machineHostName()
{

#if defined(Q_OS_LINUX)
   // gethostname(3) on Linux just calls uname(2)
   struct utsname u;

   if (uname(&u) == 0) {
      return QString::fromUtf8(u.nodename);
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
   return QString::fromUtf8(hostName);
#endif

   return QString();
}

// Q_CHECK_PTR macro calls this function if an allocation check fails

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
    Dijkstra's bisection algorithm to find the square root of an integer.
*/
Q_CORE_EXPORT unsigned int qt_int_sqrt(unsigned int n)
{
   if (n >= (std::numeric_limits<unsigned int>::max() >> 2)) {
      unsigned int r  = 2 * qt_int_sqrt(n / 4);
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

static std::mutex &environmentMutex()
{
   static std::mutex retval;
   return retval;
}

QByteArray qgetenv(const char *varName)
{
   std::lock_guard<std::mutex> lock(environmentMutex());
   return QByteArray(::getenv(varName));
}

bool qputenv(const char *varName, const QByteArray &value)
{
   std::lock_guard<std::mutex> lock(environmentMutex());

#if defined(_POSIX_VERSION) && (_POSIX_VERSION - 0) >= 200112L
    return setenv(varName, value.constData(), true) == 0;

#else
   QByteArray buffer(varName);
   buffer += '=';
   buffer += value;

   char *envVar = qstrdup(buffer.constData());
   int result = putenv(envVar);

   if (result != 0) {
      // error, we have to delete the string.
      delete[] envVar;
   }

   return result == 0;
#endif
}

bool qunsetenv(const char *varName)
{
   std::lock_guard<std::mutex> lock(environmentMutex());

#if defined(_POSIX_VERSION) && (_POSIX_VERSION - 0) >= 200112L
   return unsetenv(varName) == 0;

#elif defined(Q_CC_MINGW)
   // removes "var" from the environment, no memory leak
   QByteArray buffer(varName);
   buffer += '=';

   return putenv(buffer.constData()) == 0;

#else
   // insert an empty var into the environment, memory leak
   QByteArray buffer(varName);
   buffer += '=';
   char *envVar = qstrdup(buffer.constData());

   return putenv(envVar) == 0;
#endif
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
      // global static seed storage should always exist,
      // except after being deleted by QGlobalStaticDeleter.
      // But since it still can be called from destructor of another
      // global static object, fallback to srand(seed)
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
      // global static seed storage should always exist,
      // except after being deleted by QGlobalStaticDeleter.
      // But since it still can be called from destructor of another
      // global static object, fallback to rand()
      return rand();
   }
#else
   return rand();

#endif
}

