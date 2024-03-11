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

#include <qfsfileengine_p.h>

#include <qabstractfileengine.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qdir.h>
#include <qfile.h>
#include <qplatformdefs.h>
#include <qt_windows.h>
#include <qvarlengtharray.h>

#include <qfilesystemengine_p.h>
#include <qmutexpool_p.h>

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

#ifdef PATH_MAX
#define CS_PATH_LEN  PATH_MAX
#else
#define CS_PATH_LEN  FILENAME_MAX
#endif

static inline bool isUncPath(const QString &path)
{
   // Starts with \\, but not \\.
   return (path.startsWith("\\\\") && path.size() > 2 && path.at(2) != '.');
}

QString QFSFileEnginePrivate::longFileName(const QString &path)
{
   if (path.startsWith("\\\\.\\")) {
      return path;
   }

   QString absPath = QFileSystemEngine::nativeAbsoluteFilePath(path);
   QString prefix  = "\\\\?\\";

   if (isUncPath(absPath)) {
      prefix.append("UNC\\");                         // "\\\\?\\UNC\\"
      absPath.remove(0, 2);
   }

   return prefix + absPath;
}

bool QFSFileEnginePrivate::nativeOpen(QIODevice::OpenMode openMode)
{
   Q_Q(QFSFileEngine);

   // All files are opened in share mode (both read and write).
   DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;

   int accessRights = 0;

   if (openMode & QIODevice::ReadOnly) {
      accessRights |= GENERIC_READ;
   }

   if (openMode & QIODevice::WriteOnly) {
      accessRights |= GENERIC_WRITE;
   }

   SECURITY_ATTRIBUTES securityAtts = { sizeof(SECURITY_ATTRIBUTES), nullptr, FALSE };

   // WriteOnly can create files, ReadOnly cannot
   DWORD creationDisp = (openMode & QIODevice::WriteOnly) ? OPEN_ALWAYS : OPEN_EXISTING;

   // Create the file handle
   fileHandle = CreateFile(&fileEntry.nativeFilePath().toStdWString()[0],
         accessRights, shareMode, &securityAtts, creationDisp, FILE_ATTRIBUTE_NORMAL, nullptr);

   // Bail out on error
   if (fileHandle == INVALID_HANDLE_VALUE) {
      q->setError(QFile::OpenError, qt_error_string());
      return false;
   }

   // Truncate the file after successfully opening it if Truncate is passed
   if (openMode & QIODevice::Truncate) {
      q->setSize(0);
   }

   return true;
}

bool QFSFileEnginePrivate::nativeClose()
{
   Q_Q(QFSFileEngine);

   if (fh || fd != -1) {
      // stdlib / stdio mode
      return closeFdFh();
   }

   // Windows native mode
   bool ok = true;

   if (cachedFd != -1) {
      if (::_close(cachedFd) && !::CloseHandle(fileHandle)) {
         q->setError(QFile::UnspecifiedError, qt_error_string());
         ok = false;
      }

      // System handle is closed with associated file descriptor.
      fileHandle = INVALID_HANDLE_VALUE;
      cachedFd = -1;

      return ok;
   }

   if ((fileHandle == INVALID_HANDLE_VALUE || !::CloseHandle(fileHandle))) {
      q->setError(QFile::UnspecifiedError, qt_error_string());
      ok = false;
   }

   fileHandle = INVALID_HANDLE_VALUE;
   return ok;
}

bool QFSFileEnginePrivate::nativeFlush()
{
   if (fh) {
      // Buffered stdlib mode.
      return flushFh();
   }

   if (fd != -1) {
      // Unbuffered stdio mode; always succeeds (no buffer).
      return true;
   }

   // Windows native mode; flushing is unnecessary.
   return true;
}

bool QFSFileEnginePrivate::nativeSyncToDisk()
{
   if (fh || fd != -1) {
      // stdlib / stdio mode. No API available.
      return false;
   }

   return FlushFileBuffers(fileHandle);
}

qint64 QFSFileEnginePrivate::nativeSize() const
{
   Q_Q(const QFSFileEngine);

   QFSFileEngine *self_FileEngine = const_cast<QFSFileEngine *>(q);

   // do not flush, for buffered files, we should get away with ftell
   self_FileEngine->flush();

   // Always retrive the current information
   metaData.clearFlags(QFileSystemMetaData::SizeAttribute);

   bool filled = false;

   if (fileHandle != INVALID_HANDLE_VALUE && openMode != QIODevice::NotOpen) {
      filled = QFileSystemEngine::fillMetaData(fileHandle, metaData, QFileSystemMetaData::SizeAttribute);

   } else {
      filled = doStat(QFileSystemMetaData::SizeAttribute);
   }

   if (! filled) {
      // file is most likely not open

      self_FileEngine->setError(QFile::UnspecifiedError, qt_error_string(errno));
      return 0;
   }

   return metaData.size();
}

qint64 QFSFileEnginePrivate::nativePos() const
{
   Q_Q(const QFSFileEngine);
   QFSFileEngine *self_FileEngine = const_cast<QFSFileEngine *>(q);

   if (fh || fd != -1) {
      // stdlib / stido mode.
      return posFdFh();
   }

   // Windows native mode.
   if (fileHandle == INVALID_HANDLE_VALUE) {
      return 0;
   }

   LARGE_INTEGER currentFilePos;
   LARGE_INTEGER offset;
   offset.QuadPart = 0;

   if (! ::SetFilePointerEx(fileHandle, offset, &currentFilePos, FILE_CURRENT)) {
      self_FileEngine->setError(QFile::UnspecifiedError, qt_error_string());
      return 0;
   }

   return qint64(currentFilePos.QuadPart);
}

bool QFSFileEnginePrivate::nativeSeek(qint64 pos)
{
   Q_Q(QFSFileEngine);

   if (fh || fd != -1) {
      // stdlib / stdio mode.
      return seekFdFh(pos);
   }

   LARGE_INTEGER currentFilePos;
   LARGE_INTEGER offset;
   offset.QuadPart = pos;

   if (!::SetFilePointerEx(fileHandle, offset, &currentFilePos, FILE_BEGIN)) {
      q->setError(QFile::UnspecifiedError, qt_error_string());
      return false;
   }

   return true;
}

qint64 QFSFileEnginePrivate::nativeRead(char *data, qint64 maxlen)
{
   Q_Q(QFSFileEngine);

   if (fh || fd != -1) {
      // stdio / stdlib mode.
      if (fh && nativeIsSequential() && feof(fh)) {
         q->setError(QFile::ReadError, qt_error_string(int(errno)));
         return -1;
      }

      return readFdFh(data, maxlen);
   }

   // Windows native mode.
   if (fileHandle == INVALID_HANDLE_VALUE) {
      return -1;
   }

   qint64 bytesToRead = maxlen;

   // Reading on Windows fails with ERROR_NO_SYSTEM_RESOURCES when
   // the chunks are too large, so we limit the block size to 32MB.
   static constexpr const qint64 maxBlockSize = 32 * 1024 * 1024;

   qint64 totalRead = 0;

   do {
      DWORD blockSize = qMin(bytesToRead, maxBlockSize);
      DWORD bytesRead;

      if (! ReadFile(fileHandle, data + totalRead, blockSize, &bytesRead, nullptr)) {
         if (totalRead == 0) {
            // Note: only return failure if the first ReadFile fails.
            q->setError(QFile::ReadError, qt_error_string());
            return -1;
         }

         break;
      }

      if (bytesRead == 0) {
         break;
      }

      totalRead += bytesRead;
      bytesToRead -= bytesRead;

   } while (totalRead < maxlen);

   return totalRead;
}

qint64 QFSFileEnginePrivate::nativeReadLine(char *data, qint64 maxlen)
{
   Q_Q(QFSFileEngine);

   if (fh || fd != -1) {
      // stdio / stdlib mode.
      return readLineFdFh(data, maxlen);
   }

   // Windows native mode.
   if (fileHandle == INVALID_HANDLE_VALUE) {
      return -1;
   }

   // ### No equivalent in Win32?
   return q->QAbstractFileEngine::readLine(data, maxlen);
}

qint64 QFSFileEnginePrivate::nativeWrite(const char *data, qint64 len)
{
   Q_Q(QFSFileEngine);

   if (fh || fd != -1) {
      // stdio / stdlib mode.
      return writeFdFh(data, len);
   }

   // Windows native mode.
   if (fileHandle == INVALID_HANDLE_VALUE) {
      return -1;
   }

   qint64 bytesToWrite = len;

   // Writing on Windows fails with ERROR_NO_SYSTEM_RESOURCES when
   // the chunks are too large, so we limit the block size to 32MB.
   static constexpr const qint64 maxBlockSize = 32 * 1024 * 1024;

   qint64 totalWritten = 0;

   do {
      DWORD blockSize = qMin(bytesToWrite, maxBlockSize);
      DWORD bytesWritten;

      if (!WriteFile(fileHandle, data + totalWritten, blockSize, &bytesWritten, nullptr)) {
         if (totalWritten == 0) {
            // Note: Only return error if the first WriteFile failed.
            q->setError(QFile::WriteError, qt_error_string());
            return -1;
         }

         break;
      }

      if (bytesWritten == 0) {
         break;
      }

      totalWritten += bytesWritten;
      bytesToWrite -= bytesWritten;
   } while (totalWritten < len);

   return qint64(totalWritten);
}

int QFSFileEnginePrivate::nativeHandle() const
{
   if (fh || fd != -1) {
      return fh ? QT_FILENO(fh) : fd;
   }

   if (cachedFd != -1) {
      return cachedFd;
   }

   int flags = 0;

   if (openMode & QIODevice::Append) {
      flags |= _O_APPEND;
   }

   if (!(openMode & QIODevice::WriteOnly)) {
      flags |= _O_RDONLY;
   }

   cachedFd = _open_osfhandle((intptr_t) fileHandle, flags);

   return cachedFd;

}

bool QFSFileEnginePrivate::nativeIsSequential() const
{
   HANDLE handle = fileHandle;

   if (fh || fd != -1) {
      handle = (HANDLE)_get_osfhandle(fh ? QT_FILENO(fh) : fd);
   }

   if (handle == INVALID_HANDLE_VALUE) {
      return false;
   }

   DWORD fileType = GetFileType(handle);

   return (fileType == FILE_TYPE_CHAR) || (fileType == FILE_TYPE_PIPE);
}

bool QFSFileEngine::remove()
{
   Q_D(QFSFileEngine);

   QSystemError error;
   bool retval = QFileSystemEngine::removeFile(d->fileEntry, error);

   if (! retval) {
      setError(QFile::RemoveError, error.toString());
   }

   return retval;
}

bool QFSFileEngine::copy(const QString &copyName)
{
   Q_D(QFSFileEngine);

   QSystemError error;
   bool retval = QFileSystemEngine::copyFile(d->fileEntry, QFileSystemEntry(copyName), error);

   if (! retval) {
      setError(QFile::CopyError, error.toString());
   }

   return retval;
}

bool QFSFileEngine::rename(const QString &newName)
{
   Q_D(QFSFileEngine);

   QSystemError error;
   bool retval = QFileSystemEngine::renameFile(d->fileEntry, QFileSystemEntry(newName), error);

   if (! retval) {
      setError(QFile::RenameError, error.toString());
   }

   return retval;
}

bool QFSFileEngine::renameOverwrite(const QString &newName)
{
   Q_D(QFSFileEngine);

   bool retval = ::MoveFileEx(&d->fileEntry.nativeFilePath().toStdWString()[0],
         &QFileSystemEntry(newName).nativeFilePath().toStdWString()[0], MOVEFILE_REPLACE_EXISTING) != 0;

   if (! retval) {
      setError(QFile::RenameError, QSystemError(::GetLastError(), QSystemError::NativeError).toString());
   }

   return retval;
}

bool QFSFileEngine::mkdir(const QString &name, bool createParentDirectories) const
{
   return QFileSystemEngine::createDirectory(QFileSystemEntry(name), createParentDirectories);
}

bool QFSFileEngine::rmdir(const QString &name, bool recurseParentDirectories) const
{
   return QFileSystemEngine::removeDirectory(QFileSystemEntry(name), recurseParentDirectories);
}

bool QFSFileEngine::caseSensitive() const
{
   return false;
}

bool QFSFileEngine::setCurrentPath(const QString &path)
{
   return QFileSystemEngine::setCurrentPath(QFileSystemEntry(path));
}

QString QFSFileEngine::currentPath(const QString &fileName)
{
   QString retval;

   // if filename is a drive: then get the pwd of that drive
   if (fileName.length() >= 2 && fileName.at(0).isLetter() && fileName.at(1) == ':') {
      int drv = fileName.toUpper().at(0).toLatin1() - 'A' + 1;

      if (_getdrive() != drv) {
         std::wstring buffer(CS_PATH_LEN, L'\0');
         ::_wgetdcwd(drv, &buffer[0], CS_PATH_LEN);

         retval = QString::fromStdWString(buffer);
      }
   }

   if (retval.isEmpty()) {
      // just the pwd
      retval = QFileSystemEngine::currentPath().filePath();
   }

   if (retval.length() >= 2 && retval[1] == ':' && retval[0].isLower()) {
      // Force uppercase drive letters
      retval.replace(0, 1, retval.at(0).toUpper());
   }

   return retval;
}

QString QFSFileEngine::homePath()
{
   return QFileSystemEngine::homePath();
}

QString QFSFileEngine::rootPath()
{
   return QFileSystemEngine::rootPath();
}

QString QFSFileEngine::tempPath()
{
   return QFileSystemEngine::tempPath();
}

QFileInfoList QFSFileEngine::drives()
{
   QFileInfoList retval;

#if defined(Q_OS_WIN)
   const UINT oldErrorMode = ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
   quint32 driveBits = (quint32) GetLogicalDrives() & 0x3ffffff;
   ::SetErrorMode(oldErrorMode);
#endif

   char driveName[] = "A:/";

   while (driveBits) {
      if (driveBits & 1) {
         retval.append(QFileInfo(driveName));
      }

      driveName[0]++;
      driveBits = driveBits >> 1;
   }

   return retval;
}

bool QFSFileEnginePrivate::doStat(QFileSystemMetaData::MetaDataFlags flags) const
{
   if (! tried_stat || ! metaData.hasFlags(flags)) {
      tried_stat = true;

      int localFd = fd;

      if (fh && fileEntry.isEmpty()) {
         localFd = QT_FILENO(fh);
      }

      if (localFd != -1) {
         QFileSystemEngine::fillMetaData(localFd, metaData, flags);
      }

      if (metaData.missingFlags(flags) && !fileEntry.isEmpty()) {
         QFileSystemEngine::fillMetaData(fileEntry, metaData, metaData.missingFlags(flags));
      }
   }

   return metaData.exists();
}

bool QFSFileEngine::link(const QString &newName)
{
   bool retval      = false;
   QString linkName = newName;

   IShellLink *psl;
   bool neededCoInit = false;

   HRESULT hres = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);

   if (hres == CO_E_NOTINITIALIZED) {
      // COM was not initialized
      neededCoInit = true;
      CoInitialize(nullptr);
      hres = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
   }

   if (SUCCEEDED(hres)) {
      QString tmp = this->fileName(AbsoluteName).replace('/', '\\');

      hres = psl->SetPath(&tmp.toStdWString()[0]);

      if (SUCCEEDED(hres)) {
         tmp = this->fileName(AbsolutePathName).replace('/', '\\');

         hres = psl->SetWorkingDirectory(&tmp.toStdWString()[0]);

         if (SUCCEEDED(hres)) {
            IPersistFile *ppf;
            hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);

            if (SUCCEEDED(hres)) {
               hres = ppf->Save(&linkName.toStdWString()[0], TRUE);

               if (SUCCEEDED(hres)) {
                  retval = true;
               }

               ppf->Release();
            }
         }
      }

      psl->Release();
   }

   if (! retval) {
      setError(QFile::RenameError, qt_error_string());
   }

   if (neededCoInit) {
      CoUninitialize();
   }

   return retval;
}

QAbstractFileEngine::FileFlags QFSFileEngine::fileFlags(QAbstractFileEngine::FileFlags type) const
{
   Q_D(const QFSFileEngine);

   if (type & Refresh) {
      d->metaData.clear();
   }

   QAbstractFileEngine::FileFlags retval = Qt::EmptyFlag;

   if (type & FlagsMask) {
      retval |= LocalDiskFlag;
   }

   bool exists;
   {
      QFileSystemMetaData::MetaDataFlags queryFlags = Qt::EmptyFlag;
      queryFlags |= QFileSystemMetaData::MetaDataFlags(uint(type)) & QFileSystemMetaData::Permissions;

      // AliasType and BundleType are 0x0
      if (type & TypesMask)
         queryFlags |= QFileSystemMetaData::AliasType
               | QFileSystemMetaData::LinkType
               | QFileSystemMetaData::FileType
               | QFileSystemMetaData::DirectoryType
               | QFileSystemMetaData::BundleType;

      if (type & FlagsMask) {
         queryFlags |= QFileSystemMetaData::HiddenAttribute | QFileSystemMetaData::ExistsAttribute;
      }

      queryFlags |= QFileSystemMetaData::LinkType;

      exists = d->doStat(queryFlags);
   }

   if (exists && (type & PermsMask)) {
      retval |= FileFlags(uint(d->metaData.permissions()));
   }

   if (type & TypesMask) {
      if ((type & LinkType) && d->metaData.isLegacyLink()) {
         retval |= LinkType;
      }

      if (d->metaData.isDirectory()) {
         retval |= DirectoryType;
      } else {
         retval |= FileType;
      }
   }

   if (type & FlagsMask) {
      if (d->metaData.exists()) {
         retval |= ExistsFlag;

         if (d->fileEntry.isRoot()) {
            retval |= RootFlag;

         } else if (d->metaData.isHidden()) {
            retval |= HiddenFlag;
         }
      }
   }

   return retval;
}

QString QFSFileEngine::fileName(FileName file) const
{
   Q_D(const QFSFileEngine);

   if (file == BaseName) {
      return d->fileEntry.fileName();

   } else if (file == PathName) {
      return d->fileEntry.path();

   } else if (file == AbsoluteName || file == AbsolutePathName) {
      QString retval;

      if (! isRelativePath()) {

         if (d->fileEntry.filePath().startsWith('/') ||  d->fileEntry.filePath().size() == 2 ||
               (d->fileEntry.filePath().size() > 2 && d->fileEntry.filePath().at(2) != '/') ||
               d->fileEntry.filePath().contains("/../") || d->fileEntry.filePath().contains("/./") ||
               d->fileEntry.filePath().endsWith("/..")  || d->fileEntry.filePath().endsWith("/.")) {

            // an absolute path to the current drive, so \a.txt -> Z:\a.txt
            // a drive letter that needs to get a working dir appended
            // a drive-relative path, so Z:a.txt -> Z:\currentpath\a.txt

            retval = QDir::fromNativeSeparators(QFileSystemEngine::nativeAbsoluteFilePath(d->fileEntry.filePath()));

         } else

         {
            retval = d->fileEntry.filePath();
         }

      } else {
         retval = QDir::cleanPath(QDir::currentPath() + '/' + d->fileEntry.filePath());
      }

      // The path should be absolute at this point.
      // Absolute paths begin with the directory separator "/"
      // (optionally preceded by a drive specification under Windows).

      if (retval.at(0) != '/' && retval[0].isLower()) {
         Q_ASSERT(retval.length() >= 2);
         Q_ASSERT(retval.at(0).isLetter());
         Q_ASSERT(retval.at(1) == ':');

         // Force uppercase drive letters.
         retval.replace(0, 1, retval.at(0).toUpper());
      }

      if (file == AbsolutePathName) {
         int slash = retval.lastIndexOf('/');

         if (slash < 0) {
            return retval;

         } else if (retval.at(0) != '/' && slash == 2) {
            return retval.left(3);   // include the slash

         } else {
            return retval.left(slash > 0 ? slash : 1);
         }
      }

      return retval;

   } else if (file == CanonicalName || file == CanonicalPathName) {
      if (! (fileFlags(ExistsFlag) & ExistsFlag)) {
         return QString();
      }

      QFileSystemEntry entry(QFileSystemEngine::canonicalName(QFileSystemEntry(fileName(AbsoluteName)), d->metaData));

      if (file == CanonicalPathName) {
         return entry.path();
      }

      return entry.filePath();

   } else if (file == LinkName) {
      return QFileSystemEngine::getLinkTarget(d->fileEntry, d->metaData).filePath();

   } else if (file == BundleName) {
      return QString();
   }

   return d->fileEntry.filePath();
}

bool QFSFileEngine::isRelativePath() const
{
   Q_D(const QFSFileEngine);

   // drive, e.g. "a:", or UNC root, e.q. "//"
   return d->fileEntry.isRelative();
}

uint QFSFileEngine::ownerId(FileOwner) const
{
   static constexpr const uint nobodyID = (uint) - 2;
   return nobodyID;
}

QString QFSFileEngine::owner(FileOwner own) const
{
   Q_D(const QFSFileEngine);
   return QFileSystemEngine::owner(d->fileEntry, own);
}

bool QFSFileEngine::setPermissions(uint perms)
{
   Q_D(QFSFileEngine);

   QSystemError error;
   bool retval = QFileSystemEngine::setPermissions(d->fileEntry, QFile::Permissions(perms), error);

   if (! retval) {
      setError(QFile::PermissionsError, error.toString());
   }

   return retval;
}

bool QFSFileEngine::setSize(qint64 size)
{
   Q_D(QFSFileEngine);

   if (d->fileHandle != INVALID_HANDLE_VALUE || d->fd != -1 || d->fh) {
      // resize open file
      HANDLE fh = d->fileHandle;

      if (fh == INVALID_HANDLE_VALUE) {
         if (d->fh) {
            fh = (HANDLE)_get_osfhandle(QT_FILENO(d->fh));
         } else {
            fh = (HANDLE)_get_osfhandle(d->fd);
         }
      }

      if (fh == INVALID_HANDLE_VALUE) {
         return false;
      }

      qint64 currentPos = pos();

      if (seek(size) && SetEndOfFile(fh)) {
         seek(qMin(currentPos, size));
         return true;
      }

      seek(currentPos);
      return false;
   }

   if (!d->fileEntry.isEmpty()) {
      // resize file on disk
      QFile file(d->fileEntry.filePath());

      if (file.open(QFile::ReadWrite)) {
         bool retval = file.resize(size);

         if (!retval) {
            setError(QFile::ResizeError, file.errorString());
         }

         return retval;
      }
   }

   return false;
}

QDateTime QFSFileEngine::fileTime(FileTime time) const
{
   Q_D(const QFSFileEngine);

   if (d->doStat(QFileSystemMetaData::Times)) {
      return d->metaData.fileTime(time);
   }

   return QDateTime();
}

uchar *QFSFileEnginePrivate::map(qint64 offset, qint64 size, QFile::MemoryMapFlags flags)
{
   (void) flags;

   Q_Q(QFSFileEngine);

   if (openMode == QFile::NotOpen) {
      q->setError(QFile::PermissionsError, qt_error_string(ERROR_ACCESS_DENIED));
      return nullptr;
   }

   if (offset == 0 && size == 0) {
      q->setError(QFile::UnspecifiedError, qt_error_string(ERROR_INVALID_PARAMETER));
      return nullptr;
   }

   if (mapHandle == nullptr) {
      // get handle to the file
      HANDLE handle = fileHandle;

      if (handle == INVALID_HANDLE_VALUE && fh) {
         handle = (HANDLE)::_get_osfhandle(QT_FILENO(fh));
      }

      if (handle == INVALID_HANDLE_VALUE) {
         q->setError(QFile::PermissionsError, qt_error_string(ERROR_ACCESS_DENIED));
         return nullptr;
      }

      // first create the file mapping handle
      DWORD protection = (openMode & QIODevice::WriteOnly) ? PAGE_READWRITE : PAGE_READONLY;
      mapHandle = ::CreateFileMapping(handle, nullptr, protection, 0, 0, nullptr);

      if (mapHandle == nullptr) {
         q->setError(QFile::PermissionsError, qt_error_string());
         return nullptr;
      }
   }

   // setup args to map
   DWORD access = 0;

   if (openMode & QIODevice::ReadOnly) {
      access = FILE_MAP_READ;
   }

   if (openMode & QIODevice::WriteOnly) {
      access = FILE_MAP_WRITE;
   }

   DWORD offsetHi = offset >> 32;
   DWORD offsetLo = offset & Q_UINT64_C(0xffffffff);
   SYSTEM_INFO sysinfo;
   ::GetSystemInfo(&sysinfo);
   DWORD mask = sysinfo.dwAllocationGranularity - 1;
   DWORD extra = offset & mask;

   if (extra) {
      offsetLo &= ~mask;
   }

   // attempt to create the map
   LPVOID mapAddress = ::MapViewOfFile(mapHandle, access, offsetHi, offsetLo, size + extra);

   if (mapAddress) {
      uchar *address = extra + static_cast<uchar *>(mapAddress);
      maps[address] = extra;
      return address;
   }

   switch (GetLastError()) {
      case ERROR_ACCESS_DENIED:
         q->setError(QFile::PermissionsError, qt_error_string());
         break;

      case ERROR_INVALID_PARAMETER:
         // size is out of bounds
         [[fallthrough]];

      default:
         q->setError(QFile::UnspecifiedError, qt_error_string());
   }

   ::CloseHandle(mapHandle);
   mapHandle = nullptr;

   return nullptr;
}

bool QFSFileEnginePrivate::unmap(uchar *ptr)
{
   Q_Q(QFSFileEngine);

   if (! maps.contains(ptr)) {
      q->setError(QFile::PermissionsError, qt_error_string(ERROR_ACCESS_DENIED));
      return false;
   }

   uchar *start = ptr - maps[ptr];

   if (!UnmapViewOfFile(start)) {
      q->setError(QFile::PermissionsError, qt_error_string());
      return false;
   }

   maps.remove(ptr);

   if (maps.isEmpty()) {
      ::CloseHandle(mapHandle);
      mapHandle = nullptr;
   }

   return true;
}
