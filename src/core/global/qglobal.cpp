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

#include <qbytearray.h>
#include <qglobal.h>
#include <qlog.h>
#include <qscopedarraypointer.h>
#include <qstring.h>
#include <qthreadstorage.h>

#include <qsystemlibrary_p.h>

#include <limits>
#include <mutex>
#include <random>
#include <stdlib.h>

#if defined(Q_OS_DARWIN)
#include <qcore_mac_p.h>
#include <qnamespace.h>

#include <CoreServices/CoreServices.h>
#endif

#if defined(Q_OS_UNIX)
#include <sys/utsname.h>
#include <unistd.h>
#endif

#if defined(Q_OS_WIN)
#include <qt_windows.h>

// from <ddk/ntddk.h>
extern "C" __declspec(dllimport) NTSTATUS NTAPI RtlGetVersion(IN OUT PRTL_OSVERSIONINFOW lpVersionInformation);

#endif

const char *csVersion()
{
   return CS_VERSION_STR;
}

// ** OSX
#if defined(Q_OS_DARWIN)

QSysInfo::MacVersion QSysInfo::macVersion()
{
   // kernel/qcore_mac_objc.mm
   const QAppleOperatingSystemVersion osVersion = qt_apple_os_version();

   QSysInfo::MacVersion retval;

   if (osVersion.major == 10 && osVersion.minor == 11) {
      retval = MacVersion::MV_10_11;

   } else if (osVersion.major == 10 && osVersion.minor == 12) {
      retval = MacVersion::MV_10_12;

   } else if (osVersion.major == 10 && osVersion.minor == 13) {
      retval = MacVersion::MV_10_13;

   } else if (osVersion.major == 10 && osVersion.minor == 14) {
      retval = MacVersion::MV_10_14;

   } else if (osVersion.major == 10 && osVersion.minor == 15) {
      retval = MacVersion::MV_10_15;

   } else if (osVersion.major == 10 && osVersion.minor == 16) {
      retval = MacVersion::MV_10_16;

   // **
   } else if (osVersion.major == 11) {
      retval = MacVersion::MV_11;

   } else if (osVersion.major == 12) {
      retval = MacVersion::MV_12;

   } else if (osVersion.major == 13) {
      retval = MacVersion::MV_13;

   } else if (osVersion.major == 14) {
      retval = MacVersion::MV_14;

   } else {
      retval = QSysInfo::MV_Unknown;

   }

   return retval;
}

const QSysInfo::MacVersion QSysInfo::MacintoshVersion = QSysInfo::macVersion();

QString QSysInfo::macEdition(MacVersion macVersion)
{
   QString retval = "Unknown Version";

   switch (macVersion) {
      case QSysInfo::MacVersion::MV_10_11:
         retval = "El Capitan";
         break;

      case QSysInfo::MacVersion::MV_10_12:
         retval = "Sierra";
         break;

      case QSysInfo::MacVersion::MV_10_13:
         retval = "High Sierra";
         break;

      case QSysInfo::MacVersion::MV_10_14:
         retval = "Mojave";
         break;

      case QSysInfo::MacVersion::MV_10_15:
         retval = "Catalina";
         break;

      case QSysInfo::MacVersion::MV_11:
         retval = "Big Sur";
         break;

      case QSysInfo::MacVersion::MV_12:
         retval = "Monterey";
         break;

      case QSysInfo::MacVersion::MV_13:
         retval = "Ventura";
         break;

      case QSysInfo::MacVersion::MV_14:
         retval = "Sonoma";
         break;

      default:
         break;
   }

   return retval;
}


// ** Windows
#elif defined(Q_OS_WIN)

static OSVERSIONINFO winOsVersion()
{
   OSVERSIONINFO result = { };
   result.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

   RtlGetVersion(&result);

   return result;
}

QSysInfo::WinVersion QSysInfo::windowsVersion()
{
   static QSysInfo::WinVersion winVersion;

   if (winVersion) {
      return winVersion;
   }

   winVersion = QSysInfo::WV_NT;
   const OSVERSIONINFO osVersion = winOsVersion();

   switch (osVersion.dwPlatformId) {

      case VER_PLATFORM_WIN32_WINDOWS:
         if (osVersion.dwMinorVersion == 90) {
            winVersion = QSysInfo::WV_Me;

         } else if (osVersion.dwMinorVersion == 10) {
            winVersion = QSysInfo::WV_98;

         } else {
            winVersion = QSysInfo::WV_95;
         }

         break;

      default:
          // VER_PLATFORM_WIN32_NT

          if (osVersion.dwMajorVersion < 5) {
             winVersion = QSysInfo::WV_NT;

          } else if (osVersion.dwMajorVersion == 5 && osVersion.dwMinorVersion == 0) {
             winVersion = QSysInfo::WV_2000;

          } else if (osVersion.dwMajorVersion == 5 && osVersion.dwMinorVersion == 1) {
             winVersion = QSysInfo::WV_XP;

          } else if (osVersion.dwMajorVersion == 5 && osVersion.dwMinorVersion == 2) {
             winVersion = QSysInfo::WV_2003;

          } else if (osVersion.dwMajorVersion == 6 && osVersion.dwMinorVersion == 0) {
             winVersion = QSysInfo::WV_VISTA;

          } else if (osVersion.dwMajorVersion == 6 && osVersion.dwMinorVersion == 1) {
             winVersion = QSysInfo::WV_WINDOWS7;

          } else if (osVersion.dwMajorVersion == 6 && osVersion.dwMinorVersion == 2) {
             winVersion = QSysInfo::WV_WINDOWS8;

          } else if (osVersion.dwMajorVersion == 6 && osVersion.dwMinorVersion == 3) {
             winVersion = QSysInfo::WV_WINDOWS8_1;

          } else if (osVersion.dwMajorVersion == 10 && osVersion.dwMinorVersion == 0) {
             winVersion = QSysInfo::WV_WINDOWS10;

          } else if (osVersion.dwMajorVersion == 11 && osVersion.dwMinorVersion == 0) {
             winVersion = QSysInfo::WV_WINDOWS11;

          } else {
             qWarning("QSysInfo::windowsVersion() Untested Windows version %d.%d detected",
                   int(osVersion.dwMajorVersion), int(osVersion.dwMinorVersion));

             winVersion = QSysInfo::WV_NT_based;
          }

         }

#if defined(CS_SHOW_DEBUG_CORE)
   {
      QByteArray forceWinVersion = qgetenv("QT_WINVER_OVERRIDE");

      if (forceWinVersion.isEmpty()) {
         return winVersion;
      }

      if (forceWinVersion == "NT") {
         winVersion = QSysInfo::WV_NT;

      } else if (forceWinVersion == "2000") {
         winVersion = QSysInfo::WV_2000;

      } else if (forceWinVersion == "2003") {
         winVersion = QSysInfo::WV_2003;

      } else if (forceWinVersion == "XP") {
          winVersion = QSysInfo::WV_XP;

      } else if (forceWinVersion == "VISTA") {
          winVersion = QSysInfo::WV_VISTA;

      } else if (forceWinVersion == "WINDOWS7") {
          winVersion = QSysInfo::WV_WINDOWS7;

      } else if (forceWinVersion == "WINDOWS8") {
         winVersion = QSysInfo::WV_WINDOWS8;

      } else if (forceWinVersion == "WINDOWS8.1") {
         winVersion = QSysInfo::WV_WINDOWS8_1;

      } else if (forceWinVersion == "WINDOWS10") {
         winVersion = QSysInfo::WV_WINDOWS10;

      } else if (forceWinVersion == "WINDOWS11") {
         winVersion = QSysInfo::WV_WINDOWS11;

      }
   }
#endif

   return winVersion;
}

QString QSysInfo::windowsEdition(WinVersion winVersion)
{
   QString retval = "Unknown Version";

   switch (winVersion) {
      case QSysInfo::WV_95:
         retval = "Windows 95";
         break;

      case QSysInfo::WV_98:
         retval = "Windows 98";
         break;

      case QSysInfo::WV_Me:
         retval = "Windows ME";
         break;

      case QSysInfo::WV_NT:
         retval = "Windows NT";
         break;

      case QSysInfo::WV_2000:
         retval = "Windows 2000";
         break;

      case  QSysInfo::WV_2003:
         retval = "Windows Server 2003";
         break;

      case QSysInfo::WV_XP:
         retval = "Windows XP";
         break;

      case QSysInfo::WV_VISTA:
         retval = "Windows Vista";
         break;

      case QSysInfo::WV_WINDOWS7:
         retval = "Windows 7";
         break;

      case QSysInfo::WV_WINDOWS8:
         retval = "Windows 8";
         break;

      case QSysInfo::WV_WINDOWS8_1:
         retval = "Windows 8.1";
         break;

      case QSysInfo::WV_WINDOWS10:
         retval = "Windows 10";
         break;

      case QSysInfo::WV_WINDOWS11:
         retval = "Windows 11";
         break;

      case QSysInfo::WV_NT_based:
         retval = "Unknown NT Kernel";
         break;
   }

   return retval;
}

const QSysInfo::WinVersion QSysInfo::WindowsVersion = QSysInfo::windowsVersion();

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
      qWarning("QTcpSocketAPI() WinSock v2.0 initialization failed");
    } else {
      version = 0x20;
    }
}

QWindowsSockInit::~QWindowsSockInit()
{
   WSACleanup();
}

#endif  // end of OS_X_WIN


QString QSysInfo::machineHostName()
{

#if defined(Q_OS_LINUX)
   // gethostname(3) on Linux just calls uname(2)
   struct utsname u;

   if (uname(&u) == 0) {
      return QString::fromUtf8(u.nodename);
   }

#else

#ifdef Q_OS_WIN
    // network depends on machineHostName() initializing ws2_32.dll
    static QWindowsSockInit winSock;
#endif

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

void qBadAlloc()
{
   throw(std::bad_alloc());
}

// Dijkstra's bisection algorithm to find the square root of an integer.
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
      // error, we have to delete the string
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

thread_local static std::random_device s_randomDevice;
thread_local static std::mt19937 s_rand(s_randomDevice());
thread_local static std::uniform_int_distribution<int> s_dist;

void qsrand(uint seed)
{
   s_rand = std::mt19937(seed);
}

int qrand()
{
   return s_dist(s_rand);
}
