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

#include <qfilesystemengine_p.h>

#include <qabstractfileengine.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qdir.h>
#include <qfile.h>
#include <qplatformdefs.h>
#include <qregularexpression.h>
#include <qstringparser.h>
#include <qt_windows.h>
#include <qvarlengtharray.h>

#include <qfsfileengine_p.h>
#include <qmutexpool_p.h>
#include <qsystemlibrary_p.h>

#include <accctrl.h>
#include <ctype.h>
#include <direct.h>
#include <limits.h>
#include <objbase.h>
#include <shlobj.h>
#include <winioctl.h>

// do not move these two files
#include <sys/types.h>
#include <initguid.h>

#define SECURITY_WIN32
#include <security.h>

#ifndef SPI_GETPLATFORMTYPE
#define SPI_GETPLATFORMTYPE 257
#endif

#ifdef PATH_MAX
#define CS_PATH_LEN  PATH_MAX
#else
#define CS_PATH_LEN  FILENAME_MAX
#endif

#ifndef _INTPTR_T_DEFINED

#ifdef  _WIN64
using intptr_t =  __int64;

#else

#ifdef _W64
using intptr_t =  _W64 int;
#else
using intptr_t = INT_PTR;
#endif

#endif

#define _INTPTR_T_DEFINED
#endif

#ifndef INVALID_FILE_ATTRIBUTES
#  define INVALID_FILE_ATTRIBUTES (DWORD (-1))
#endif

#if ! defined(REPARSE_DATA_BUFFER_HEADER_SIZE)

struct _REPARSE_DATA_BUFFER {
   ULONG  ReparseTag;
   USHORT ReparseDataLength;
   USHORT Reserved;

   union {
      struct {
         USHORT SubstituteNameOffset;
         USHORT SubstituteNameLength;
         USHORT PrintNameOffset;
         USHORT PrintNameLength;
         ULONG  Flags;
         WCHAR  PathBuffer[1];
      } SymbolicLinkReparseBuffer;

      struct {
         USHORT SubstituteNameOffset;
         USHORT SubstituteNameLength;
         USHORT PrintNameOffset;
         USHORT PrintNameLength;
         WCHAR  PathBuffer[1];
      } MountPointReparseBuffer;

      struct {
         UCHAR  DataBuffer[1];
      } GenericReparseBuffer;
   };
};

using REPARSE_DATA_BUFFER  = _REPARSE_DATA_BUFFER;
using PREPARSE_DATA_BUFFER = _REPARSE_DATA_BUFFER *;

#   define REPARSE_DATA_BUFFER_HEADER_SIZE  FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer)
#endif

#ifndef MAXIMUM_REPARSE_DATA_BUFFER_SIZE
#   define MAXIMUM_REPARSE_DATA_BUFFER_SIZE 16384
#endif

#ifndef IO_REPARSE_TAG_SYMLINK
#   define IO_REPARSE_TAG_SYMLINK (0xA000000CL)
#endif

#ifndef FSCTL_GET_REPARSE_POINT
#   define FSCTL_GET_REPARSE_POINT CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 42, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif

Q_CORE_EXPORT int qt_ntfs_permission_lookup = 0;

using PtrGetNamedSecurityInfoW =
   DWORD (WINAPI *)(LPWSTR, SE_OBJECT_TYPE, SECURITY_INFORMATION, PSID *, PSID *, PACL *, PACL *, PSECURITY_DESCRIPTOR *);

static PtrGetNamedSecurityInfoW ptrGetNamedSecurityInfoW = nullptr;

using PtrLookupAccountSidW = BOOL (WINAPI *)(LPCWSTR, PSID, LPWSTR, LPDWORD, LPWSTR, LPDWORD, PSID_NAME_USE);
static PtrLookupAccountSidW ptrLookupAccountSidW = nullptr;

using PtrBuildTrusteeWithSidW = VOID (WINAPI *)(PTRUSTEE_W, PSID);
static PtrBuildTrusteeWithSidW ptrBuildTrusteeWithSidW = nullptr;

using PtrGetEffectiveRightsFromAclW = DWORD (WINAPI *)(PACL, PTRUSTEE_W, OUT PACCESS_MASK);
static PtrGetEffectiveRightsFromAclW ptrGetEffectiveRightsFromAclW = nullptr;

using PtrGetUserProfileDirectoryW = BOOL (WINAPI *)(HANDLE, LPWSTR, LPDWORD);
static PtrGetUserProfileDirectoryW ptrGetUserProfileDirectoryW = nullptr;

using PtrGetVolumePathNamesForVolumeNameW = BOOL (WINAPI *)(LPCWSTR, LPWSTR, DWORD, PDWORD);
static PtrGetVolumePathNamesForVolumeNameW ptrGetVolumePathNamesForVolumeNameW = nullptr;

static TRUSTEE_W currentUserTrusteeW;
static TRUSTEE_W worldTrusteeW;
static PSID currentUserSID = nullptr;
static PSID worldSID = nullptr;

class SidCleanup
{
 public:
   ~SidCleanup();
};

SidCleanup::~SidCleanup()
{
   qFree(currentUserSID);
   currentUserSID = nullptr;

   // worldSID was allocated with AllocateAndInitializeSid so it needs to be freed with FreeSid
   if (worldSID) {
      ::FreeSid(worldSID);
      worldSID = nullptr;
   }
}

static SidCleanup *initSidCleanup()
{
   static SidCleanup retval;
   return &retval;
}

static void resolveLibs()
{
   static bool triedResolve = false;

   if (! triedResolve) {
      // need to resolve the security info functions

      // protect initialization

      QRecursiveMutexLocker locker(QMutexPool::globalInstanceGet(&triedResolve));

      // check triedResolve again, since another thread may have already done the initialization

      if (triedResolve) {
         // another thread did initialize the security function pointers,
         // so we shouldn't do it again
         return;
      }

      QSystemLibrary advapi32("advapi32");

      if (advapi32.load()) {
         ptrGetNamedSecurityInfoW = (PtrGetNamedSecurityInfoW)advapi32.resolve("GetNamedSecurityInfoW");
         ptrLookupAccountSidW = (PtrLookupAccountSidW)advapi32.resolve("LookupAccountSidW");
         ptrBuildTrusteeWithSidW = (PtrBuildTrusteeWithSidW)advapi32.resolve("BuildTrusteeWithSidW");
         ptrGetEffectiveRightsFromAclW = (PtrGetEffectiveRightsFromAclW)advapi32.resolve("GetEffectiveRightsFromAclW");
      }

      if (ptrBuildTrusteeWithSidW) {
         // Create TRUSTEE for current user
         HANDLE hnd = ::GetCurrentProcess();
         HANDLE token = nullptr;
         initSidCleanup();

         if (::OpenProcessToken(hnd, TOKEN_QUERY, &token)) {
            DWORD retsize = 0;
            // GetTokenInformation requires a buffer big enough for the TOKEN_USER struct and
            // the SID struct. Since the SID struct can have variable number of subauthorities
            // tacked at the end, its size is variable. Obtain the required size by first
            // doing a dummy GetTokenInformation call.
            ::GetTokenInformation(token, TokenUser, nullptr, 0, &retsize);

            if (retsize) {
               void *tokenBuffer = qMalloc(retsize);

               if (::GetTokenInformation(token, TokenUser, tokenBuffer, retsize, &retsize)) {
                  PSID tokenSid = reinterpret_cast<PTOKEN_USER>(tokenBuffer)->User.Sid;
                  DWORD sidLen = ::GetLengthSid(tokenSid);
                  currentUserSID = reinterpret_cast<PSID>(qMalloc(sidLen));

                  if (::CopySid(sidLen, currentUserSID, tokenSid)) {
                     ptrBuildTrusteeWithSidW(&currentUserTrusteeW, currentUserSID);
                  }
               }

               qFree(tokenBuffer);
            }

            ::CloseHandle(token);
         }

         using PtrAllocateAndInitializeSid =
               BOOL (WINAPI *)(PSID_IDENTIFIER_AUTHORITY, BYTE, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, PSID *);

         PtrAllocateAndInitializeSid ptrAllocateAndInitializeSid =
               (PtrAllocateAndInitializeSid) advapi32.resolve("AllocateAndInitializeSid");

         if (ptrAllocateAndInitializeSid) {
            // Create TRUSTEE for Everyone (World)
            SID_IDENTIFIER_AUTHORITY worldAuth = { SECURITY_WORLD_SID_AUTHORITY };

            if (ptrAllocateAndInitializeSid(&worldAuth, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &worldSID)) {
               ptrBuildTrusteeWithSidW(&worldTrusteeW, worldSID);
            }
         }
      }

      QSystemLibrary userenv("userenv");

      if (userenv.load()) {
         ptrGetUserProfileDirectoryW = (PtrGetUserProfileDirectoryW)userenv.resolve("GetUserProfileDirectoryW");
      }

      QSystemLibrary kernel32("kernel32");

      if (kernel32.load()) {
         ptrGetVolumePathNamesForVolumeNameW = (PtrGetVolumePathNamesForVolumeNameW) kernel32.resolve("GetVolumePathNamesForVolumeNameW");
      }

      triedResolve = true;
   }
}

using PtrNetShareEnum = DWORD (WINAPI *)(LPWSTR, DWORD, LPBYTE *, DWORD, LPDWORD, LPDWORD, LPDWORD);
static PtrNetShareEnum ptrNetShareEnum = nullptr;

using PtrNetApiBufferFree = DWORD (WINAPI *)(LPVOID);
static PtrNetApiBufferFree ptrNetApiBufferFree = nullptr;

struct _SHARE_INFO_1 {
   LPWSTR shi1_netname;
   DWORD shi1_type;
   LPWSTR shi1_remark;
};
using SHARE_INFO_1 = _SHARE_INFO_1;

static bool resolveUNCLibs()
{
   static bool triedResolve = false;

   if (! triedResolve) {
      QRecursiveMutexLocker locker(QMutexPool::globalInstanceGet(&triedResolve));

      if (triedResolve) {
         return ptrNetShareEnum && ptrNetApiBufferFree;
      }

      QSystemLibrary netapi32("Netapi32");

      if (netapi32.load()) {
         ptrNetShareEnum = (PtrNetShareEnum)netapi32.resolve("NetShareEnum");
         ptrNetApiBufferFree = (PtrNetApiBufferFree)netapi32.resolve("NetApiBufferFree");
      }

      triedResolve = true;
   }

   return ptrNetShareEnum && ptrNetApiBufferFree;
}

static QString readSymLink(const QFileSystemEntry &link)
{
   QString result;

   HANDLE handle = CreateFile( &link.nativeFilePath().toStdWString()[0],
         FILE_READ_EA, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
         FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);

   if (handle != INVALID_HANDLE_VALUE) {

      DWORD bufsize = MAXIMUM_REPARSE_DATA_BUFFER_SIZE;
      REPARSE_DATA_BUFFER *rdb = (REPARSE_DATA_BUFFER *)malloc(bufsize);
      DWORD retsize = 0;

      if (::DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT, nullptr, 0, rdb, bufsize, &retsize, nullptr)) {
         if (rdb->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {
            int length = rdb->MountPointReparseBuffer.SubstituteNameLength / sizeof(wchar_t);
            int offset = rdb->MountPointReparseBuffer.SubstituteNameOffset / sizeof(wchar_t);

            const wchar_t *PathBuffer = &rdb->MountPointReparseBuffer.PathBuffer[offset];
            result = QString::fromStdWString(std::wstring(PathBuffer, length));

         } else if (rdb->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
            int length = rdb->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(wchar_t);
            int offset = rdb->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(wchar_t);

            const wchar_t *PathBuffer = &rdb->SymbolicLinkReparseBuffer.PathBuffer[offset];
            result = QString::fromStdWString(std::wstring(PathBuffer, length));
         }

         // cut-off "//?/" and "/??/"
         if (result.size() > 4 && result.at(0) == '\\' && result.at(2) == '?' && result.at(3) == '\\') {
            result = result.mid(4);
         }
      }

      free(rdb);
      CloseHandle(handle);

      resolveLibs();

      if (ptrGetVolumePathNamesForVolumeNameW) {
         QRegularExpression8 matchVolName("^Volume\\{([a-z]|[0-9]|-)+\\}\\\\", QPatternOption::CaseInsensitiveOption);

         QRegularExpressionMatch8 match = matchVolName.match(result);

         if (match.hasMatch()) {
            DWORD len;
            std::wstring tmpBuffer(MAX_PATH, L'\0');

            QString volumeName = match.captured(0).prepend("\\\\?\\");

            if (ptrGetVolumePathNamesForVolumeNameW(&volumeName.toStdWString()[0], &tmpBuffer[0], MAX_PATH, &len) != 0) {
               result.replace(0, match.captured(0).length(), QString::fromStdWString(tmpBuffer));
            }
         }
      }

   }

   return result;
}

static QString readLink(const QFileSystemEntry &link)
{
   QString retval;

   bool neededCoInit = false;

   IShellLink *psl;                            // pointer to IShellLink i/f
   WIN32_FIND_DATA wfd;

   std::wstring szGotPath(MAX_PATH, L'\0');

   // Get pointer to the IShellLink interface.
   HRESULT hres = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);

   if (hres == CO_E_NOTINITIALIZED) {
      // COM was not initialized
      neededCoInit = true;
      CoInitialize(nullptr);
      hres = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);
   }

   if (SUCCEEDED(hres)) {
      // Get pointer to the IPersistFile interface.
      IPersistFile *ppf;

      hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);

      if (SUCCEEDED(hres))  {
         hres = ppf->Load(&link.nativeFilePath().toStdWString()[0], STGM_READ);

         // original path of the link is retrieved.
         // if the file/folder was moved, the return value still has the old path

         if (SUCCEEDED(hres)) {
            if (psl->GetPath(&szGotPath[0], MAX_PATH, &wfd, SLGP_UNCPRIORITY) == NOERROR) {
               retval = QString::fromStdWString(szGotPath);
            }
         }

         ppf->Release();
      }

      psl->Release();
   }

   if (neededCoInit) {
      CoUninitialize();
   }

   return retval;
}

static bool uncShareExists(const QString &server)
{
   // This code assumes the UNC path is always like \\?\UNC\server...
   QStringList parts = server.split('\\', QStringParser::SkipEmptyParts);

   if (parts.count() >= 3) {
      QStringList shares;

      if (QFileSystemEngine::uncListSharesOnServer("\\\\" + parts.at(2), &shares)) {
         return parts.count() >= 4 ? shares.contains(parts.at(3), Qt::CaseInsensitive) : true;
      }
   }

   return false;
}

static inline bool getFindData(QString path, WIN32_FIND_DATA &findData)
{
   // path should not end with a trailing slash
   while (path.endsWith('\\')) {
      path.chop(1);
   }

   // can not handle drives
   if (! path.endsWith(':')) {
      HANDLE hFind = ::FindFirstFile(&path.toStdWString()[0], &findData);

      if (hFind != INVALID_HANDLE_VALUE) {
         ::FindClose(hFind);
         return true;
      }
   }

   return false;
}

bool QFileSystemEngine::uncListSharesOnServer(const QString &server, QStringList *list)
{
   if (resolveUNCLibs()) {
      SHARE_INFO_1 *BufPtr;
      SHARE_INFO_1 *p;
      DWORD res;
      DWORD er = 0;
      DWORD tr = 0;
      DWORD resume = 0;
      DWORD i;

      do {
         res = ptrNetShareEnum(&server.toStdWString()[0], 1, (LPBYTE *)&BufPtr, DWORD(-1), &er, &tr, &resume);

         if (res == ERROR_SUCCESS || res == ERROR_MORE_DATA) {
            p = BufPtr;

            for (i = 1; i <= er; ++i) {

               if (list && p->shi1_type == 0) {
                  list->append(QString::fromStdWString(p->shi1_netname));
               }

               p++;
            }
         }

         ptrNetApiBufferFree(BufPtr);

      } while (res == ERROR_MORE_DATA);

      return res == ERROR_SUCCESS;
   }

   return false;
}

void QFileSystemEngine::clearWinStatData(QFileSystemMetaData &data)
{
   data.size_           = 0;
   data.fileAttribute_  =  0;
   data.creationTime_   = FILETIME();
   data.lastAccessTime_ = FILETIME();
   data.lastWriteTime_  = FILETIME();
}

QFileSystemEntry QFileSystemEngine::getLinkTarget(const QFileSystemEntry &link, QFileSystemMetaData &data)
{
   if (data.missingFlags(QFileSystemMetaData::LinkType)) {
      QFileSystemEngine::fillMetaData(link, data, QFileSystemMetaData::LinkType);
   }

   QString retval;

   if (data.isLnkFile()) {
      retval = readLink(link);
   } else if (data.isLink()) {
      retval = readSymLink(link);
   }

   return QFileSystemEntry(retval);
}

QFileSystemEntry QFileSystemEngine::canonicalName(const QFileSystemEntry &entry, QFileSystemMetaData &data)
{
   if (data.missingFlags(QFileSystemMetaData::ExistsAttribute)) {
      QFileSystemEngine::fillMetaData(entry, data, QFileSystemMetaData::ExistsAttribute);
   }

   if (data.exists()) {
      return QFileSystemEntry(slowCanonicalized(absoluteName(entry).filePath()));
   } else {
      return QFileSystemEntry();
   }
}

QString QFileSystemEngine::nativeAbsoluteFilePath(const QString &path)
{
   // can be //server or //server/share
   QString absPath;

   std::wstring tmpBuffer(qMax(MAX_PATH, path.size() + 1), L'\0');
   wchar_t *fileName = nullptr;

   DWORD retLen = GetFullPathName(&path.toStdWString()[0], tmpBuffer.size(), &tmpBuffer[0], &fileName);

   if (retLen > (DWORD)tmpBuffer.size()) {
      tmpBuffer.resize(retLen);
      retLen = GetFullPathName(&path.toStdWString()[0], tmpBuffer.size(), &tmpBuffer[0], &fileName);
   }

   if (retLen != 0) {
      absPath = QString::fromStdWString(tmpBuffer, retLen);
   }

   // This is really ugly, but GetFullPathName strips off whitespace at the end.
   // If you for instance write ". " in the lineedit of QFileDialog,
   // (which is an invalid filename) this function will strip the space off and viola,
   // the file is later reported as existing. Therefore, we re-add the whitespace that
   // was at the end of path in order to keep the filename invalid.

   if (! path.isEmpty() && path.at(path.size() - 1) == ' ') {
      absPath.append(' ');
   }

   return absPath;
}

QFileSystemEntry QFileSystemEngine::absoluteName(const QFileSystemEntry &entry)
{
   QString retval;

   if (! entry.isRelative()) {

      if (entry.isAbsolute() && entry.isClean()) {
         retval = entry.filePath();
      }  else {
         retval = QDir::fromNativeSeparators(nativeAbsoluteFilePath(entry.filePath()));
      }

   } else {
      retval = QDir::cleanPath(QDir::currentPath() + '/' + entry.filePath());
   }

   // The path should be absolute at this point.
   // Absolute paths begin with the directory separator "/"
   // (optionally preceded by a drive specification under Windows).

   if (retval.at(0) != '/') {
      Q_ASSERT(retval.length() >= 2);
      Q_ASSERT(retval.at(0).isLetter());
      Q_ASSERT(retval.at(1) == ':');

      // Force uppercase drive letters.
      retval.replace(0, 1, retval.at(0).toUpper());
   }

   return QFileSystemEntry(retval, QFileSystemEntry::FromInternalPath());
}

// FILE_INFO_BY_HANDLE_CLASS has been extended by FileIdInfo = 18
static constexpr const FILE_INFO_BY_HANDLE_CLASS Q_FileIdInfo = static_cast<FILE_INFO_BY_HANDLE_CLASS>(18);

#if defined(Q_CC_MINGW) && ! defined(STORAGE_INFO_OFFSET_UNKNOWN)

#ifndef FILE_SUPPORTS_INTEGRITY_STREAMS

struct _FILE_ID_128 {
   BYTE Identifier[16];
};

using FILE_ID_128  = _FILE_ID_128;
using PFILE_ID_128 = _FILE_ID_128 *;

#endif

struct _FILE_ID_INFO {
   ULONGLONG VolumeSerialNumber;
   FILE_ID_128 FileId;
};

using FILE_ID_INFO  = _FILE_ID_INFO;
using PFILE_ID_INFO = *_FILE_ID_INFO;

#endif

// File ID for Windows up to version 7
static inline QByteArray fileId(HANDLE handle)
{
   QByteArray result;
   BY_HANDLE_FILE_INFORMATION info;

   if (GetFileInformationByHandle(handle, &info)) {
      result  = QByteArray::number(uint(info.nFileIndexLow), 16);
      result += ':';
      result += QByteArray::number(uint(info.nFileIndexHigh), 16);
   }

   return result;
}

// File ID for Windows starting from version 8
QByteArray fileIdWin8(HANDLE handle)
{
   using GetFileInformationByHandleExType = BOOL (WINAPI *)(HANDLE, FILE_INFO_BY_HANDLE_CLASS, void *, DWORD);

   // Dynamically resolve  GetFileInformationByHandleEx (Vista onwards).
   static GetFileInformationByHandleExType getFileInformationByHandleEx = nullptr;

   if (! getFileInformationByHandleEx) {
      QSystemLibrary library("kernel32");
      getFileInformationByHandleEx = (GetFileInformationByHandleExType)library.resolve("GetFileInformationByHandleEx");
   }

   QByteArray result;

   if (getFileInformationByHandleEx) {
      FILE_ID_INFO infoEx;

      if (getFileInformationByHandleEx(handle, Q_FileIdInfo, &infoEx, sizeof(FILE_ID_INFO))) {
         result = QByteArray::number(infoEx.VolumeSerialNumber, 16);
         result += ':';
         result += QByteArray((char *) &infoEx.FileId, sizeof(infoEx.FileId)).toHex();
      }
   }

   return result;
}

QByteArray QFileSystemEngine::id(const QFileSystemEntry &entry)
{
   QByteArray result;
   const HANDLE handle = CreateFile(&entry.nativeFilePath().toStdWString()[0],
         GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

   if (handle) {
      result = QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS8 ? fileIdWin8(handle) : fileId(handle);
      CloseHandle(handle);
   }

   return result;
}

QString QFileSystemEngine::owner(const QFileSystemEntry &entry, QAbstractFileEngine::FileOwner own)
{
   QString name;

   extern int qt_ntfs_permission_lookup;

   if ((qt_ntfs_permission_lookup > 0) && (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)) {
      resolveLibs();

      if (ptrGetNamedSecurityInfoW && ptrLookupAccountSidW) {
         PSID pOwner = nullptr;
         PSECURITY_DESCRIPTOR pSD;

         if (ptrGetNamedSecurityInfoW(&entry.nativeFilePath().toStdWString()[0], SE_FILE_OBJECT,
               own == QAbstractFileEngine::OwnerGroup ? GROUP_SECURITY_INFORMATION : OWNER_SECURITY_INFORMATION,
               own == QAbstractFileEngine::OwnerUser  ? &pOwner : nullptr,
               own == QAbstractFileEngine::OwnerGroup ? &pOwner : nullptr, nullptr, nullptr, &pSD) == ERROR_SUCCESS) {

            DWORD lowner = 64;
            DWORD ldomain = 64;
            QVarLengthArray<wchar_t, 64> owner(lowner);
            QVarLengthArray<wchar_t, 64> domain(ldomain);
            SID_NAME_USE use = SidTypeUnknown;

            // First call, to determine size of the strings (with '\0').
            if (!ptrLookupAccountSidW(nullptr, pOwner, (LPWSTR)owner.data(), &lowner, (LPWSTR)domain.data(), &ldomain, (SID_NAME_USE *)&use)) {

               if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                  if (lowner > (DWORD)owner.size()) {
                     owner.resize(lowner);
                  }

                  if (ldomain > (DWORD)domain.size()) {
                     domain.resize(ldomain);
                  }

                  // Second call, try on resized buf-s
                  if (!ptrLookupAccountSidW(nullptr, pOwner, (LPWSTR)owner.data(), &lowner, (LPWSTR)domain.data(), &ldomain, (SID_NAME_USE *)&use)) {
                     lowner = 0;
                  }

               } else {
                  lowner = 0;
               }
            }

            if (lowner != 0) {
               name = QString::fromStdWString(owner.data());
            }

            LocalFree(pSD);
         }
      }
   }

   return name;
}

bool QFileSystemEngine::fillPermissions(const QFileSystemEntry &entry, QFileSystemMetaData &data, QFileSystemMetaData::MetaDataFlags what)
{
   static constexpr const int ReadMask  = 0x00000001;
   static constexpr const int WriteMask = 0x00000002;
   static constexpr const int ExecMask  = 0x00000020;

   if ((qt_ntfs_permission_lookup > 0) && (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)) {
      resolveLibs();

      if (ptrGetNamedSecurityInfoW && ptrBuildTrusteeWithSidW && ptrGetEffectiveRightsFromAclW) {
         QString fname = entry.nativeFilePath();

         PSID pOwner = nullptr;
         PSID pGroup = nullptr;
         PACL pDacl;
         PSECURITY_DESCRIPTOR pSD;

         DWORD res = ptrGetNamedSecurityInfoW(&fname.toStdWString()[0],
               SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
               &pOwner, &pGroup, &pDacl, nullptr, &pSD);

         if (res == ERROR_SUCCESS) {
            ACCESS_MASK access_mask;
            TRUSTEE_W trustee;

            if (what & QFileSystemMetaData::UserPermissions) {
               // user
               data.knownFlagsMask |= QFileSystemMetaData::UserPermissions;

               if (ptrGetEffectiveRightsFromAclW(pDacl, &currentUserTrusteeW, &access_mask) != ERROR_SUCCESS) {
                  access_mask = (ACCESS_MASK) - 1;
               }

               if (access_mask & ReadMask) {
                  data.entryFlags |= QFileSystemMetaData::UserReadPermission;
               }

               if (access_mask & WriteMask) {
                  data.entryFlags |= QFileSystemMetaData::UserWritePermission;
               }

               if (access_mask & ExecMask) {
                  data.entryFlags |= QFileSystemMetaData::UserExecutePermission;
               }
            }

            if (what & QFileSystemMetaData::OwnerPermissions) {
               // owner
               data.knownFlagsMask |= QFileSystemMetaData::OwnerPermissions;
               ptrBuildTrusteeWithSidW(&trustee, pOwner);

               if (ptrGetEffectiveRightsFromAclW(pDacl, &trustee, &access_mask) != ERROR_SUCCESS) {
                  access_mask = (ACCESS_MASK) - 1;
               }

               if (access_mask & ReadMask) {
                  data.entryFlags |= QFileSystemMetaData::OwnerReadPermission;
               }

               if (access_mask & WriteMask) {
                  data.entryFlags |= QFileSystemMetaData::OwnerWritePermission;
               }

               if (access_mask & ExecMask) {
                  data.entryFlags |= QFileSystemMetaData::OwnerExecutePermission;
               }
            }

            if (what & QFileSystemMetaData::GroupPermissions) {
               // group
               data.knownFlagsMask |= QFileSystemMetaData::GroupPermissions;
               ptrBuildTrusteeWithSidW(&trustee, pGroup);

               if (ptrGetEffectiveRightsFromAclW(pDacl, &trustee, &access_mask) != ERROR_SUCCESS) {
                  access_mask = (ACCESS_MASK) - 1;
               }

               if (access_mask & ReadMask) {
                  data.entryFlags |= QFileSystemMetaData::GroupReadPermission;
               }

               if (access_mask & WriteMask) {
                  data.entryFlags |= QFileSystemMetaData::GroupWritePermission;
               }

               if (access_mask & ExecMask) {
                  data.entryFlags |= QFileSystemMetaData::GroupExecutePermission;
               }
            }

            if (what & QFileSystemMetaData::OtherPermissions) {
               // other (world)
               data.knownFlagsMask |= QFileSystemMetaData::OtherPermissions;

               if (ptrGetEffectiveRightsFromAclW(pDacl, &worldTrusteeW, &access_mask) != ERROR_SUCCESS) {
                  access_mask = (ACCESS_MASK) - 1;   // ###
               }

               if (access_mask & ReadMask) {
                  data.entryFlags |= QFileSystemMetaData::OtherReadPermission;
               }

               if (access_mask & WriteMask) {
                  data.entryFlags |= QFileSystemMetaData::OtherWritePermission;
               }

               if (access_mask & ExecMask) {
                  data.entryFlags |= QFileSystemMetaData::OwnerExecutePermission;
               }
            }

            LocalFree(pSD);
         }
      }

   } else {

      // ### what to do with permissions if we don't use NTFS
      // for now just add all permissions and what about exe missions
      // also qt_ntfs_permission_lookup is now not set by default

      data.entryFlags |= QFileSystemMetaData::OwnerReadPermission |
            QFileSystemMetaData::GroupReadPermission | QFileSystemMetaData::OtherReadPermission;

      if (! (data.fileAttribute_ & FILE_ATTRIBUTE_READONLY)) {
         data.entryFlags |= QFileSystemMetaData::OwnerWritePermission |
               QFileSystemMetaData::GroupWritePermission | QFileSystemMetaData::OtherWritePermission;
      }

      QString fname = entry.filePath();
      QString ext   = fname.right(4).toLower();

      if (data.isDirectory() || ext == ".exe" || ext == ".com" || ext == ".bat" || ext == ".pif" || ext == ".cmd") {

         data.entryFlags |= QFileSystemMetaData::OwnerExecutePermission | QFileSystemMetaData::GroupExecutePermission |
               QFileSystemMetaData::OtherExecutePermission | QFileSystemMetaData::UserExecutePermission;
      }

      data.knownFlagsMask |= QFileSystemMetaData::OwnerPermissions | QFileSystemMetaData::GroupPermissions |
            QFileSystemMetaData::OtherPermissions | QFileSystemMetaData::UserExecutePermission;

      // calculate user permissions
      if (what & QFileSystemMetaData::UserReadPermission) {

         if (::_waccess(&entry.nativeFilePath().toStdWString()[0], R_OK) == 0) {
            data.entryFlags |= QFileSystemMetaData::UserReadPermission;
         }

         data.knownFlagsMask |= QFileSystemMetaData::UserReadPermission;
      }

      if (what & QFileSystemMetaData::UserWritePermission) {
         if (::_waccess(&entry.nativeFilePath().toStdWString()[0], W_OK) == 0) {
            data.entryFlags |= QFileSystemMetaData::UserWritePermission;
         }

         data.knownFlagsMask |= QFileSystemMetaData::UserWritePermission;
      }
   }

   return data.hasFlags(what);
}

static bool tryDriveUNCFallback(const QFileSystemEntry &fname, QFileSystemMetaData &data)
{
   bool entryExists = false;
   DWORD fileAttrib = 0;

   if (fname.isDriveLetter_Root()) {
      // a valid drive letter

      DWORD drivesBitmask = ::GetLogicalDrives();
      int drivebit = 1 << (fname.filePath().at(0).toUpper()[0].unicode() - 'A');

      if (drivesBitmask & drivebit) {
         fileAttrib = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_SYSTEM;
         entryExists = true;
      }

   } else {

      const QString &path = fname.nativeFilePath();
      bool is_dir = false;

      if (path.startsWith("\\\\?\\UNC")) {
         // UNC - stat doesn't work for all cases (Windows bug)

         int s = path.indexOf(path.at(0), 7);

         if (s > 0) {
            // "\\?\UNC\server\..."
            s = path.indexOf(path.at(0), s + 1);

            if (s > 0) {
               // "\\?\UNC\server\share\..."

               if (s == path.size() - 1) {
                  // "\\?\UNC\server\share\"
                  is_dir = true;

               } else {
                  // "\\?\UNC\server\share\notfound"
               }
            } else {
               // "\\?\UNC\server\share"
               is_dir = true;
            }

         } else {
            // "\\?\UNC\server"
            is_dir = true;
         }
      }

      if (is_dir && uncShareExists(path)) {
         // looks like a UNC dir, is a dir.
         fileAttrib = FILE_ATTRIBUTE_DIRECTORY;
         entryExists = true;
      }
   }

   if (entryExists) {
      data.fillFromFileAttribute(fileAttrib);
   }

   return entryExists;
}

static bool tryFindFallback(const QFileSystemEntry &fname, QFileSystemMetaData &data)
{
   bool filledData = false;

   // This assumes the last call to a Windows API failed
   int errorCode = GetLastError();

   if (errorCode == ERROR_ACCESS_DENIED || errorCode == ERROR_SHARING_VIOLATION) {
      WIN32_FIND_DATA findData;

      if (getFindData(fname.nativeFilePath(), findData) && findData.dwFileAttributes != INVALID_FILE_ATTRIBUTES) {
         data.fillFromFindData(findData, true, fname.isRoot());
         filledData = true;
      }
   }

   return filledData;
}

bool QFileSystemEngine::fillMetaData(int fd, QFileSystemMetaData &data, QFileSystemMetaData::MetaDataFlags what)
{
   HANDLE fHandle = (HANDLE)_get_osfhandle(fd);

   if (fHandle  != INVALID_HANDLE_VALUE) {
      return fillMetaData(fHandle, data, what);
   }

   return false;
}

bool QFileSystemEngine::fillMetaData(HANDLE fHandle, QFileSystemMetaData &data, QFileSystemMetaData::MetaDataFlags what)
{
   data.entryFlags &= ~what;

   clearWinStatData(data);
   BY_HANDLE_FILE_INFORMATION fileInfo;
   UINT oldmode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

   if (GetFileInformationByHandle(fHandle, &fileInfo)) {
      data.fillFromFindInfo(fileInfo);
   }

   SetErrorMode(oldmode);
   return data.hasFlags(what);
}

static bool isDirPath(const QString &dirPath, bool *existed);

bool QFileSystemEngine::fillMetaData(const QFileSystemEntry &entry, QFileSystemMetaData &data,
      QFileSystemMetaData::MetaDataFlags what)
{
   what |= QFileSystemMetaData::WinLnkType | QFileSystemMetaData::WinStatFlags;
   data.entryFlags &= ~what;

   QFileSystemEntry fname;
   data.knownFlagsMask |= QFileSystemMetaData::WinLnkType;

   // Check for ".lnk": Directories named ".lnk" should be skipped, corrupted
   // link files should still be detected as links.

   const QString origFilePath = entry.filePath();

   if (origFilePath.endsWith(".lnk") && ! isDirPath(origFilePath, nullptr)) {
      data.entryFlags |= QFileSystemMetaData::WinLnkType;
      fname = QFileSystemEntry(readLink(entry));

   } else {
      fname = entry;
   }

   if (fname.isEmpty()) {
      data.knownFlagsMask |= what;
      clearWinStatData(data);
      return false;
   }

   if (what & QFileSystemMetaData::WinStatFlags) {
      UINT oldmode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
      clearWinStatData(data);
      WIN32_FIND_DATA findData;

      // The memory structure for WIN32_FIND_DATA is same as WIN32_FILE_ATTRIBUTE_DATA
      // for all members used by fillFindData().

      bool ok = ::GetFileAttributesEx(&fname.nativeFilePath().toStdWString()[0],
                  GetFileExInfoStandard, reinterpret_cast<WIN32_FILE_ATTRIBUTE_DATA *>(&findData));

      if (ok) {
         data.fillFromFindData(findData, false, fname.isRoot());

      } else {
         if (! tryFindFallback(fname, data)) {
            tryDriveUNCFallback(fname, data);
         }
      }

      SetErrorMode(oldmode);
   }

   if (what & QFileSystemMetaData::Permissions) {
      fillPermissions(fname, data, what);
   }

   if ((what & QFileSystemMetaData::LinkType) && data.missingFlags(QFileSystemMetaData::LinkType)) {
      data.knownFlagsMask |= QFileSystemMetaData::LinkType;

      if (data.fileAttribute_ & FILE_ATTRIBUTE_REPARSE_POINT) {
         WIN32_FIND_DATA findData;

         if (getFindData(fname.nativeFilePath(), findData)) {
            data.fillFromFindData(findData, true);
         }
      }
   }

   data.knownFlagsMask |= what;
   return data.hasFlags(what);
}

static inline bool mkDir(const QString &path)
{
   return ::CreateDirectory(&QFSFileEnginePrivate::longFileName(path).toStdWString()[0], nullptr);
}

static inline bool rmDir(const QString &path)
{
   return ::RemoveDirectory(&QFSFileEnginePrivate::longFileName(path).toStdWString()[0]);
}

static bool isDirPath(const QString &dirPath, bool *existed)
{
   QString path = dirPath;

   if (path.length() == 2 && path.at(1) == ':') {
      path += QLatin1Char('\\');
   }

   DWORD fileAttrib = ::GetFileAttributes(&QFSFileEnginePrivate::longFileName(path).toStdWString()[0]);

   if (fileAttrib == INVALID_FILE_ATTRIBUTES) {
      int errorCode = GetLastError();

      if (errorCode == ERROR_ACCESS_DENIED || errorCode == ERROR_SHARING_VIOLATION) {
         WIN32_FIND_DATA findData;

         if (getFindData(QFSFileEnginePrivate::longFileName(path), findData)) {
            fileAttrib = findData.dwFileAttributes;
         }
      }
   }

   if (existed) {
      *existed = fileAttrib != INVALID_FILE_ATTRIBUTES;
   }

   if (fileAttrib == INVALID_FILE_ATTRIBUTES) {
      return false;
   }

   return fileAttrib & FILE_ATTRIBUTE_DIRECTORY;
}

bool QFileSystemEngine::createDirectory(const QFileSystemEntry &entry, bool createParents)
{
   QString dirName = entry.filePath();

   if (createParents) {
      dirName = QDir::toNativeSeparators(QDir::cleanPath(dirName));

      // We spefically search for / so \ would break it..
      int oldslash = -1;

      if (dirName.startsWith("\\\\")) {
         // Don't try to create the root path of a UNC path;
         // CreateDirectory() will just return ERROR_INVALID_NAME.
         for (int i = 0; i < dirName.size(); ++i) {
            if (dirName.at(i) != QDir::separator()) {
               oldslash = i;
               break;
            }
         }

         if (oldslash != -1) {
            oldslash = dirName.indexOf(QDir::separator(), oldslash);
         }

      } else if (dirName.size() > 2 && dirName.at(1) == QChar(':')) {
         // do not try to call mkdir with just a drive letter
         oldslash = 2;
      }

      for (int slash = 0; slash != -1; oldslash = slash) {
         slash = dirName.indexOf(QDir::separator(), oldslash + 1);

         if (slash == -1) {
            if (oldslash == dirName.length()) {
               break;
            }

            slash = dirName.length();
         }

         if (slash != 0) {
            QString chunk = dirName.left(slash);

            if (! mkDir(chunk)) {
               if (GetLastError() == ERROR_ALREADY_EXISTS) {
                  bool existed = false;

                  if (isDirPath(chunk, &existed) && existed) {
                     continue;
                  }
               }

               return false;
            }
         }
      }

      return true;
   }

   return mkDir(entry.filePath());
}

bool QFileSystemEngine::removeDirectory(const QFileSystemEntry &entry, bool removeEmptyParents)
{
   QString dirName = entry.filePath();

   if (removeEmptyParents) {
      dirName = QDir::toNativeSeparators(QDir::cleanPath(dirName));

      for (int oldslash = 0, slash = dirName.length(); slash > 0; oldslash = slash) {
         QString chunk = dirName.left(slash);

         if (chunk.length() == 2 && chunk.at(0).isLetter() && chunk.at(1) == ':') {
            break;
         }

         if (!isDirPath(chunk, nullptr)) {
            return false;
         }

         if (!rmDir(chunk)) {
            return oldslash != 0;
         }

         slash = dirName.lastIndexOf(QDir::separator(), oldslash - 1);
      }

      return true;
   }

   return rmDir(entry.filePath());
}

QString QFileSystemEngine::rootPath()
{
   QString retval = "/" + QString::fromLatin1(qgetenv("SystemDrive"));

   if (retval.isEmpty()) {
      retval = "c:";
   }

   retval.append('/');

   return retval;
}

QString QFileSystemEngine::homePath()
{
   QString retval;

   resolveLibs();

   if (ptrGetUserProfileDirectoryW) {
      HANDLE hnd   = ::GetCurrentProcess();
      HANDLE token = nullptr;
      BOOL ok      = ::OpenProcessToken(hnd, TOKEN_QUERY, &token);

      if (ok) {
         DWORD dwBufferSize = 0;

         // First call, to determine size of the strings (with '\0').
         ok = ptrGetUserProfileDirectoryW(token, nullptr, &dwBufferSize);

         if (!ok && dwBufferSize != 0) {
            // We got the required buffer size
            std::wstring userDirectory(dwBufferSize, L'\0');

            // Second call, now we can fill the allocated buffer.
            ok = ptrGetUserProfileDirectoryW(token, &userDirectory[0], &dwBufferSize);

            if (ok) {
               retval = QString::fromStdWString(userDirectory);
            }
         }

         ::CloseHandle(token);
      }
   }

   if (retval.isEmpty() || ! QFile::exists(retval)) {
      retval = QString::fromUtf8(qgetenv("USERPROFILE"));

      if (retval.isEmpty() || ! QFile::exists(retval)) {
         retval = QString::fromUtf8(qgetenv("HOMEDRIVE")) + QString::fromUtf8(qgetenv("HOMEPATH"));

         if (retval.isEmpty() || ! QFile::exists(retval)) {
            retval = QString::fromUtf8(qgetenv("HOME"));

            if (retval.isEmpty() || ! QFile::exists(retval)) {
               retval = rootPath();
            }
         }
      }
   }

   return QDir::fromNativeSeparators(retval);
}

QString QFileSystemEngine::tempPath()
{
   QString retval;

   std::wstring tempPath(MAX_PATH, L'\0');
   DWORD len = GetTempPath(MAX_PATH, &tempPath[0]);

   if (len) {
      retval = QString::fromStdWString(tempPath, len);
   }

   if (! retval.isEmpty()) {
      while (retval.endsWith('\\')) {
         retval.chop(1);
      }

      retval = QDir::fromNativeSeparators(retval);
   }

   if (retval.isEmpty()) {
      retval = "C:/tmp";

   } else if (retval.length() >= 2 && retval[1] == ':' && retval[0].isLower()) {
      // Force uppercase drive letters.
      retval.replace(0, 1, retval.at(0).toUpper());
   }

   return retval;
}

bool QFileSystemEngine::setCurrentPath(const QFileSystemEntry &entry)
{
   QFileSystemMetaData meta;

   fillMetaData(entry, meta, QFileSystemMetaData::ExistsAttribute | QFileSystemMetaData::DirectoryType);

   if (! (meta.exists() && meta.isDirectory())) {
      return false;
   }

   // TODO: this should really be using nativeFilePath(), but that returns a path in long format \\?\c:\foo
   // which causes many problems later on when it's returned through currentPath()
   return ::SetCurrentDirectory(&QDir::toNativeSeparators(entry.filePath()).toStdWString()[0]) != 0;
}

QFileSystemEntry QFileSystemEngine::currentPath()
{
   QString retval;

   DWORD size = 0;
   std::wstring currentName(CS_PATH_LEN, L'\0');
   size = ::GetCurrentDirectory(CS_PATH_LEN, &currentName[0]);

   if (size != 0) {
      if (size > CS_PATH_LEN) {
         currentName.resize(size);

         if (::GetCurrentDirectory(size, &currentName[0]) != 0) {
            retval = QString::fromStdWString(currentName, size);
         }

      } else {
         retval = QString::fromStdWString(currentName, size);
      }
   }

   if (retval.length() >= 2 && retval[1] == ':' && retval[0].isLower()) {
      // Force uppercase drive letters
      retval.replace(0, 1, retval.at(0).toUpper());
   }

   return QFileSystemEntry(retval, QFileSystemEntry::FromNativePath());
}

// static
bool QFileSystemEngine::createLink(const QFileSystemEntry &source, const QFileSystemEntry &target, QSystemError &error)
{
   Q_ASSERT(false);

   (void) source;
   (void) target;
   (void) error;

   return false; // TODO implement - code needs to be moved from qfsfileengine_win.cpp
}

bool QFileSystemEngine::copyFile(const QFileSystemEntry &source, const QFileSystemEntry &target, QSystemError &error)
{
   bool retval = ::CopyFile(&source.nativeFilePath().toStdWString()[0],
               &target.nativeFilePath().toStdWString()[0], true) != 0;

   if (! retval) {
      error = QSystemError(::GetLastError(), QSystemError::NativeError);
   }

   return retval;
}

bool QFileSystemEngine::renameFile(const QFileSystemEntry &source, const QFileSystemEntry &target, QSystemError &error)
{
   bool retval = ::MoveFile(&source.nativeFilePath().toStdWString()[0], &target.nativeFilePath().toStdWString()[0]) != 0;

   if (! retval) {
      error = QSystemError(::GetLastError(), QSystemError::NativeError);
   }

   return retval;
}

bool QFileSystemEngine::removeFile(const QFileSystemEntry &entry, QSystemError &error)
{
   bool retval = ::DeleteFile(&entry.nativeFilePath().toStdWString()[0]) != 0;

   if (! retval) {
      error = QSystemError(::GetLastError(), QSystemError::NativeError);
   }

   return retval;
}

bool QFileSystemEngine::setPermissions(const QFileSystemEntry &entry, QFile::Permissions permissions,
      QSystemError &error, QFileSystemMetaData *data)
{
   (void) data;

   int mode = 0;

   if (permissions & QFile::ReadOwner || permissions & QFile::ReadUser
         || permissions & QFile::ReadGroup || permissions & QFile::ReadOther) {

      mode |= _S_IREAD;
   }

   if (permissions & QFile::WriteOwner || permissions & QFile::WriteUser
         || permissions & QFile::WriteGroup || permissions & QFile::WriteOther) {

      mode |= _S_IWRITE;
   }

   if (mode == 0) {
      // not supported
      return false;
   }

   bool retval = (::_wchmod(&entry.nativeFilePath().toStdWString()[0], mode) == 0);

   if (!retval) {
      error = QSystemError(errno, QSystemError::StandardLibraryError);
   }

   return retval;
}

static inline QDateTime fileTimeToQDateTime(const FILETIME *time)
{
   QDateTime retval;

   SYSTEMTIME sTime, lTime;
   FileTimeToSystemTime(time, &sTime);
   SystemTimeToTzSpecificLocalTime(nullptr, &sTime, &lTime);
   retval.setDate(QDate(lTime.wYear, lTime.wMonth, lTime.wDay));
   retval.setTime(QTime(lTime.wHour, lTime.wMinute, lTime.wSecond, lTime.wMilliseconds));

   return retval;
}

QDateTime QFileSystemMetaData::creationTime() const
{
   return fileTimeToQDateTime(&creationTime_);
}

QDateTime QFileSystemMetaData::modificationTime() const
{
   return fileTimeToQDateTime(&lastWriteTime_);
}

QDateTime QFileSystemMetaData::accessTime() const
{
   return fileTimeToQDateTime(&lastAccessTime_);
}
