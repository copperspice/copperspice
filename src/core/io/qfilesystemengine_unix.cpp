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

#include <qfile.h>
#include <qfileinfo.h>
#include <qfsfileengine.h>
#include <qplatformdefs.h>
#include <qvarlengtharray.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef Q_OS_DARWIN
#include <qcore_mac_p.h>
#include <CoreFoundation/CFBundle.h>

#ifndef Q_OS_IOS
#include <CoreServices/CoreServices.h>

#else
#include <MobileCoreServices/MobileCoreServices.h>

#endif

// unable to include <Foundation/Foundation.h> (it is an Objective-C header), but we need these declarations:
using NSString = struct objc_object;
extern "C" NSString *NSTemporaryDirectory();

static inline bool hasResourcePropertyFlag(const QFileSystemMetaData &data, const QFileSystemEntry &entry, CFStringRef key)
{
   QCFString path = CFStringCreateWithFileSystemRepresentation(nullptr, entry.nativeFilePath().constData());

   if (path.toCFStringRef() == nullptr) {
      return false;
   }

   QCFType<CFURLRef> url = CFURLCreateWithFileSystemPath(nullptr, path.toCFStringRef(), kCFURLPOSIXPathStyle,
               data.hasFlags(QFileSystemMetaData::DirectoryType));

   if (! url) {
      return false;
   }

   CFBooleanRef value;

   if (CFURLCopyResourcePropertyForKey(url, key, &value, nullptr)) {
      if (value == kCFBooleanTrue) {
         return true;
      }
   }

   return false;
}

static bool isPackage(const QFileSystemMetaData &data, const QFileSystemEntry &entry)
{
   if (! data.isDirectory()) {
      return false;
   }

   QFileInfo info(entry.filePath());
   QString suffix = info.suffix();

   if (suffix.length() > 0) {
      // First step: is the extension known ?
      QCFType<CFStringRef> extensionRef = QCFString::toCFStringRef(suffix);
      QCFType<CFStringRef> uniformTypeIdentifier = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, extensionRef, nullptr);

      if (UTTypeConformsTo(uniformTypeIdentifier, kUTTypeBundle)) {
         return true;
      }

      // Second step: check if an application knows the package type
      QCFType<CFStringRef> path = QCFString::toCFStringRef(entry.filePath());
      QCFType<CFURLRef> url = CFURLCreateWithFileSystemPath(nullptr, path, kCFURLPOSIXPathStyle, true);

      UInt32 type, creator;

      // Well created packages have the PkgInfo file
      if (CFBundleGetPackageInfoInDirectory(url, &type, &creator)) {
         return true;
      }

      // Find if an application other than Finder claims to know how to handle the package
      QCFType<CFURLRef> application;
      application = LSCopyDefaultApplicationURLForURL(url, kLSRolesEditor | kLSRolesViewer | kLSRolesViewer, nullptr);

      if (application) {
         QCFType<CFBundleRef> bundle = CFBundleCreate(kCFAllocatorDefault, application);
         CFStringRef identifier = CFBundleGetIdentifier(bundle);
         QString applicationId  = QCFString::toQString(identifier);

         if (applicationId != "com.apple.finder") {
            return true;
         }
      }
   }

   // Third step: check if the directory has the package bit set
   return hasResourcePropertyFlag(data, entry, kCFURLIsPackageKey);
}
#endif  // Q_OS_DARWIN

QFileSystemEntry QFileSystemEngine::getLinkTarget(const QFileSystemEntry &link, QFileSystemMetaData &data)
{

#if defined(__GLIBC__) && ! defined(PATH_MAX)

#define PATH_CHUNK_SIZE 256
   char *s  = 0;
   int len  = -1;
   int size = PATH_CHUNK_SIZE;

   while (true) {
      s = (char *) ::realloc(s, size);
      Q_CHECK_PTR(s);
      len = ::readlink(link.nativeFilePath().constData(), s, size);

      if (len < 0) {
         ::free(s);
         break;
      }

      if (len < size) {
         break;
      }

      size *= 2;
   }

#else
   char s[PATH_MAX + 1];
   int len = readlink(link.nativeFilePath().constData(), s, PATH_MAX);
#endif

   if (len > 0) {
      QString ret;

      if (! data.hasFlags(QFileSystemMetaData::DirectoryType)) {
         fillMetaData(link, data, QFileSystemMetaData::DirectoryType);
      }

      if (data.isDirectory() && s[0] != '/') {
         QDir parent(link.filePath());
         parent.cdUp();
         ret = parent.path();

         if (!ret.isEmpty() && ! ret.endsWith('/')) {
            ret += '/';
         }
      }

      s[len] = '\0';
      ret += QFile::decodeName(QByteArray(s));

#if defined(__GLIBC__) && !defined(PATH_MAX)
      ::free(s);
#endif

      if (! ret.startsWith('/')) {
         if (link.filePath().startsWith('/')) {
            ret.prepend(link.filePath().left(link.filePath().lastIndexOf('/')) + '/');

         } else {
            ret.prepend(QDir::currentPath() + '/');
         }
      }

      ret = QDir::cleanPath(ret);

      if (ret.size() > 1 && ret.endsWith('/')) {
         ret.chop(1);
      }

      return QFileSystemEntry(ret);
   }

#if defined(Q_OS_DARWIN)
   {
      QCFString path = CFStringCreateWithFileSystemRepresentation(nullptr,
                  QFile::encodeName(QDir::cleanPath(link.filePath())).data());

      if (path.toCFStringRef() == nullptr) {
         return QFileSystemEntry();
      }

      QCFType<CFURLRef> url = CFURLCreateWithFileSystemPath(nullptr, path.toCFStringRef(), kCFURLPOSIXPathStyle,
                  data.hasFlags(QFileSystemMetaData::DirectoryType));

      if (! url) {
         return QFileSystemEntry();
      }

      QCFType<CFDataRef> bookmarkData = CFURLCreateBookmarkDataFromFile(nullptr, url, nullptr);

      if (! bookmarkData) {
         return QFileSystemEntry();
      }

      QCFType<CFURLRef> resolvedUrl = CFURLCreateByResolvingBookmarkData(nullptr, bookmarkData,
            (CFURLBookmarkResolutionOptions)(kCFBookmarkResolutionWithoutUIMask
            | kCFBookmarkResolutionWithoutMountingMask), nullptr, nullptr, nullptr, nullptr);

      if (! resolvedUrl) {
         return QFileSystemEntry();
      }

      QCFString cfstr(CFURLCopyFileSystemPath(resolvedUrl, kCFURLPOSIXPathStyle));

      if (cfstr.toCFStringRef() != nullptr) {
         return QFileSystemEntry();
      }

      return QFileSystemEntry(cfstr.toQString());
   }
#endif

   return QFileSystemEntry();
}

QFileSystemEntry QFileSystemEngine::canonicalName(const QFileSystemEntry &entry, QFileSystemMetaData &data)
{
   if (entry.isEmpty() || entry.isRoot()) {
      return entry;
   }

#if ! defined(Q_OS_DARWIN) && ! defined(Q_OS_ANDROID) && _POSIX_VERSION < 200809L
   // realpath(X,0) is not supported

   (void) data;
   return QFileSystemEntry(slowCanonicalized(absoluteName(entry).filePath()));
#else
   char *ret = nullptr;

#ifdef Q_OS_DARWIN

   ret = (char *)malloc(PATH_MAX + 1);

   if (ret && realpath(entry.nativeFilePath().constData(), (char * )ret) == nullptr) {
      const int savedErrno = errno; // errno is checked below, and free() might change it
      free(ret);
      errno = savedErrno;
      ret = nullptr;
   }

#elif defined(Q_OS_ANDROID)

   if (! data.hasFlags(QFileSystemMetaData::ExistsAttribute))  {
      fillMetaData(entry, data, QFileSystemMetaData::ExistsAttribute);
   }

   if (! data.exists()) {
      ret   = nullptr;
      errno = ENOENT;

   } else {
      ret = (char *)malloc(PATH_MAX + 1);

      if (realpath(entry.nativeFilePath().constData(), (char * )ret) == nullptr) {
         const int savedErrno = errno; // errno is checked below, and free() might change it
         free(ret);

         errno = savedErrno;
         ret   = nullptr;
      }
   }

# else

#if _POSIX_VERSION >= 200801L
   ret = realpath(entry.nativeFilePath().constData(), (char *)nullptr);

#else
   ret = (char *)malloc(PATH_MAX + 1);

   if (realpath(entry.nativeFilePath().constData(), (char * )ret) == nullptr) {
      const int savedErrno = errno; // errno is checked below, and free() might change it
      free(ret);
      errno = savedErrno;
      ret   = nullptr;
   }

#endif

#endif

   if (ret) {
      data.knownFlagsMask |= QFileSystemMetaData::ExistsAttribute;
      data.entryFlags |= QFileSystemMetaData::ExistsAttribute;
      QString canonicalPath = QDir::cleanPath(QFile::decodeName(ret));
      free(ret);

      return QFileSystemEntry(canonicalPath);

   } else if (errno == ENOENT) {
      // file doesn't exist
      data.knownFlagsMask |= QFileSystemMetaData::ExistsAttribute;
      data.entryFlags &= ~(QFileSystemMetaData::ExistsAttribute);
      return QFileSystemEntry();
   }

   return entry;
#endif

}

QFileSystemEntry QFileSystemEngine::absoluteName(const QFileSystemEntry &entry)
{
   if (entry.isAbsolute() && entry.isClean()) {
      return entry;
   }

   QString orig = entry.nativeFilePath();
   QString result;

   if (orig.isEmpty() || ! orig.startsWith('/')) {
      QFileSystemEntry cur(currentPath());
      result = cur.nativeFilePath();
   }

   if (! orig.isEmpty() && ! (orig.length() == 1 && orig[0] == '.')) {
      if (! result.isEmpty() && ! result.endsWith('/')) {
         result.append('/');
      }

      result.append(orig);
   }

   if (result.length() == 1 && result[0] == '/') {
      return QFileSystemEntry(result, QFileSystemEntry::FromNativePath());
   }

   const bool isDir = result.endsWith('/');

   /* as long as QDir::cleanPath() operates on a QString we have to convert to a string here.
    * ideally we never convert to a string since that loses information. Please fix after
    * we get a QByteArray version of QDir::cleanPath()
    */
   QFileSystemEntry resultingEntry(result, QFileSystemEntry::FromNativePath());
   QString stringVersion = QDir::cleanPath(resultingEntry.filePath());

   if (isDir) {
      stringVersion.append('/');
   }

   return QFileSystemEntry(stringVersion);
}

QByteArray QFileSystemEngine::id(const QFileSystemEntry &entry)
{
   struct stat statResult;

   if (stat(entry.nativeFilePath().constData(), &statResult)) {
      qErrnoWarning("stat() failed for '%s'", entry.nativeFilePath().constData());
      return QByteArray();
   }

   QByteArray result = QByteArray::number(quint64(statResult.st_dev), 16);
   result += ':';
   result += QByteArray::number(quint64(statResult.st_ino), 16);

   return result;
}

QString QFileSystemEngine::resolveUserName(uint userId)
{
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && ! defined(Q_OS_OPENBSD)
   int size_max = sysconf(_SC_GETPW_R_SIZE_MAX);

   if (size_max == -1) {
      size_max = 1024;
   }

   QVarLengthArray<char, 1024> buf(size_max);
#endif

   struct passwd *pw = nullptr;

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && ! defined(Q_OS_OPENBSD)
   struct passwd entry;
   getpwuid_r(userId, &entry, buf.data(), buf.size(), &pw);
#else
   pw = getpwuid(userId);
#endif

   if (pw) {
      return QFile::decodeName(QByteArray(pw->pw_name));
   }

   return QString();
}

QString QFileSystemEngine::resolveGroupName(uint groupId)
{
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_OPENBSD)
   int size_max = sysconf(_SC_GETPW_R_SIZE_MAX);

   if (size_max == -1) {
      size_max = 1024;
   }

   QVarLengthArray<char, 1024> buf(size_max);
#endif

   struct group *gr = nullptr;

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_OPENBSD)
   size_max = sysconf(_SC_GETGR_R_SIZE_MAX);

   if (size_max == -1) {
      size_max = 1024;
   }

   buf.resize(size_max);
   struct group entry;

   // Some large systems have more members than the POSIX max size
   // Loop over by doubling the buffer size (upper limit 250k)
   for (unsigned size = size_max; size < 256000; size += size) {
      buf.resize(size);

      // ERANGE indicates that the buffer was too small
      if (!getgrgid_r(groupId, &entry, buf.data(), buf.size(), &gr)
            || errno != ERANGE) {
         break;
      }
   }

#else
   gr = getgrgid(groupId);
#endif

   if (gr) {
      return QFile::decodeName(QByteArray(gr->gr_name));
   }

   return QString();
}

#ifdef Q_OS_DARWIN

QString QFileSystemEngine::bundleName(const QFileSystemEntry &entry)
{
   QCFType<CFURLRef> url = CFURLCreateWithFileSystemPath(nullptr, QCFString(entry.filePath()).toCFStringRef(),
         kCFURLPOSIXPathStyle, true);

   if (QCFType<CFDictionaryRef> dict = CFBundleCopyInfoDictionaryForURL(url)) {
      if (CFTypeRef name = (CFTypeRef)CFDictionaryGetValue(dict, kCFBundleNameKey)) {
         if (CFGetTypeID(name) == CFStringGetTypeID()) {
            return QCFString::toQString((CFStringRef)name);
         }
      }
   }

   return QString();
}
#endif

bool QFileSystemEngine::fillMetaData(const QFileSystemEntry &entry, QFileSystemMetaData &data,
      QFileSystemMetaData::MetaDataFlags what)
{
#ifdef Q_OS_DARWIN

   if (what & QFileSystemMetaData::BundleType) {
      if (! data.hasFlags(QFileSystemMetaData::DirectoryType)) {
         what |= QFileSystemMetaData::DirectoryType;
      }
   }

   if (what & QFileSystemMetaData::HiddenAttribute) {
      // Mac OS >= 10.5: st_flags & UF_HIDDEN
      what |= QFileSystemMetaData::PosixStatFlags;
   }

#endif

   if (what & QFileSystemMetaData::PosixStatFlags) {
      what |= QFileSystemMetaData::PosixStatFlags;
   }

   if (what & QFileSystemMetaData::ExistsAttribute) {
      //  FIXME:  Would other queries being performed provide this bit?
      what |= QFileSystemMetaData::PosixStatFlags;
   }

   data.entryFlags &= ~what;

   QString nativeFilePath = entry.nativeFilePath();

   bool entryExists = true;

   QT_STATBUF statBuffer;
   bool statBufferValid = false;

   if (what & QFileSystemMetaData::LinkType) {
      if (QT_LSTAT(nativeFilePath.constData(), &statBuffer) == 0) {

         if (S_ISLNK(statBuffer.st_mode)) {
            data.entryFlags |= QFileSystemMetaData::LinkType;
         } else {
            statBufferValid = true;
            data.entryFlags &= ~QFileSystemMetaData::PosixStatFlags;
         }

      } else {
         entryExists = false;
      }

      data.knownFlagsMask |= QFileSystemMetaData::LinkType;
   }

   if (statBufferValid || (what & QFileSystemMetaData::PosixStatFlags)) {
      if (entryExists && ! statBufferValid) {
         statBufferValid = (QT_STAT(nativeFilePath.constData(), &statBuffer) == 0);
      }

      if (statBufferValid) {
         data.fillFromStatBuf(statBuffer);
      } else {
         entryExists = false;
         data.creationTime_ = 0;
         data.modificationTime_ = 0;
         data.accessTime_ = 0;
         data.size_ = 0;
         data.userId_ = (uint) - 2;
         data.groupId_ = (uint) - 2;
      }

      // reset the mask
      data.knownFlagsMask |= QFileSystemMetaData::PosixStatFlags | QFileSystemMetaData::ExistsAttribute;
   }

#ifdef Q_OS_DARWIN

   if (what & QFileSystemMetaData::AliasType) {
      if (entryExists && hasResourcePropertyFlag(data, entry, kCFURLIsAliasFileKey)) {
         data.entryFlags |= QFileSystemMetaData::AliasType;
      }

      data.knownFlagsMask |= QFileSystemMetaData::AliasType;
   }

#endif

   if (what & QFileSystemMetaData::UserPermissions) {
      // calculate user permissions

      if (entryExists) {
         if (what & QFileSystemMetaData::UserReadPermission) {
            if (QT_ACCESS(nativeFilePath.constData(), R_OK) == 0) {
               data.entryFlags |= QFileSystemMetaData::UserReadPermission;
            }
         }

         if (what & QFileSystemMetaData::UserWritePermission) {
            if (QT_ACCESS(nativeFilePath.constData(), W_OK) == 0) {
               data.entryFlags |= QFileSystemMetaData::UserWritePermission;
            }
         }

         if (what & QFileSystemMetaData::UserExecutePermission) {
            if (QT_ACCESS(nativeFilePath.constData(), X_OK) == 0) {
               data.entryFlags |= QFileSystemMetaData::UserExecutePermission;
            }
         }
      }

      data.knownFlagsMask |= (what & QFileSystemMetaData::UserPermissions);
   }

   if (what & QFileSystemMetaData::HiddenAttribute && ! data.isHidden()) {
      QString fileName = entry.fileName();

      if ((fileName.size() > 0 && fileName.at(0) == '.')

#ifdef Q_OS_DARWIN
            || (entryExists && hasResourcePropertyFlag(data, entry, kCFURLIsHiddenKey))
#endif
      )  {
         data.entryFlags |= QFileSystemMetaData::HiddenAttribute;
      }

      data.knownFlagsMask |= QFileSystemMetaData::HiddenAttribute;
   }

#ifdef Q_OS_DARWIN

   if (what & QFileSystemMetaData::BundleType) {
      if (entryExists && isPackage(data, entry)) {
         data.entryFlags |= QFileSystemMetaData::BundleType;
      }

      data.knownFlagsMask |= QFileSystemMetaData::BundleType;
   }

#endif

   if (! entryExists) {
      data.clearFlags(what);
      return false;
   }

   return data.hasFlags(what);
}

static bool pathIsDir(const QByteArray &nativeName)
{
   // helper function to check if a given path is a directory, since mkdir can
   // fail if the dir already exists (it may have been created by another
   // thread or another process)

   QT_STATBUF st;
   return QT_STAT(nativeName.constData(), &st) == 0 && (st.st_mode & S_IFMT) == S_IFDIR;
}

static bool createDirectoryWithParents(const QByteArray &nativeName, bool shouldMkdirFirst = true)
{
   // if shouldMkdirFirst is false, assume the caller tried to mkdir before calling this function

   if (shouldMkdirFirst && QT_MKDIR(nativeName.constData(), 0777) == 0) {
      return true;
   }

   if (errno == EEXIST) {
      return pathIsDir(nativeName);
   }

   if (errno != ENOENT) {
      return false;
   }

   // mkdir failed because the parent dir doesn't exist, so try to create it
   int slash = nativeName.lastIndexOf('/');

   if (slash < 1) {
      return false;
   }

   QByteArray parentNativeName = nativeName.left(slash);

   if (! createDirectoryWithParents(parentNativeName)) {
      return false;
   }

   // try again
   if (QT_MKDIR(nativeName.constData(), 0777) == 0) {
      return true;
   }

   return errno == EEXIST && pathIsDir(nativeName);
}

bool QFileSystemEngine::createDirectory(const QFileSystemEntry &entry, bool createParents)
{
   QString dirName = entry.filePath();

   // Darwin does not support trailing /'s, so remove for everyone
   while (dirName.size() > 1 && dirName.endsWith('/')) {
      dirName.chop(1);
   }

   // try to mkdir this directory
   QByteArray nativeName = QFile::encodeName(dirName);

   if (QT_MKDIR(nativeName.constData(), 0777) == 0) {
      return true;
   }

   if (! createParents) {
      return false;
   }

   // we need the cleaned path in order to create the parents
   // and we save errno just in case encodeName needs to load codecs
   int savedErrno   = errno;
   bool pathChanged = false;

   QString cleanName = QDir::cleanPath(dirName);

   if (cleanName != dirName) {
      pathChanged = true;
      nativeName = QFile::encodeName(cleanName);
   }

   errno = savedErrno;

   return createDirectoryWithParents(nativeName, pathChanged);
}

bool QFileSystemEngine::removeDirectory(const QFileSystemEntry &entry, bool removeEmptyParents)
{
   if (removeEmptyParents) {
      QString dirName = QDir::cleanPath(entry.filePath());

      for (int oldslash = 0, slash = dirName.length(); slash > 0; oldslash = slash) {
         const QByteArray chunk = QFile::encodeName(dirName.left(slash));
         QT_STATBUF st;

         if (QT_STAT(chunk.constData(), &st) != -1) {
            if ((st.st_mode & S_IFMT) != S_IFDIR) {
               return false;
            }

            if (::rmdir(chunk.constData()) != 0) {
               return oldslash != 0;
            }

         } else {
            return false;
         }

         slash = dirName.lastIndexOf(QDir::separator(), oldslash - 1);
      }

      return true;
   }

   return rmdir(QFile::encodeName(entry.filePath()).constData()) == 0;
}

bool QFileSystemEngine::createLink(const QFileSystemEntry &source, const QFileSystemEntry &target, QSystemError &error)
{
   if (::symlink(source.nativeFilePath().constData(), target.nativeFilePath().constData()) == 0) {
      return true;
   }

   error = QSystemError(errno, QSystemError::StandardLibraryError);
   return false;
}

bool QFileSystemEngine::copyFile(const QFileSystemEntry &source, const QFileSystemEntry &target, QSystemError &error)
{
   (void) source;
   (void) target;

   error = QSystemError(ENOSYS, QSystemError::StandardLibraryError); //Function not implemented

   return false;
}

bool QFileSystemEngine::renameFile(const QFileSystemEntry &source, const QFileSystemEntry &target, QSystemError &error)
{
   if (::rename(source.nativeFilePath().constData(), target.nativeFilePath().constData()) == 0) {
      return true;
   }

   error = QSystemError(errno, QSystemError::StandardLibraryError);

   return false;
}

bool QFileSystemEngine::removeFile(const QFileSystemEntry &entry, QSystemError &error)
{
   if (unlink(entry.nativeFilePath().constData()) == 0) {
      return true;
   }

   error = QSystemError(errno, QSystemError::StandardLibraryError);

   return false;
}

bool QFileSystemEngine::setPermissions(const QFileSystemEntry &entry, QFile::Permissions permissions,
      QSystemError &error, QFileSystemMetaData *data)
{
   mode_t mode = 0;

   if (permissions & (QFile::ReadOwner | QFile::ReadUser)) {
      mode |= S_IRUSR;
   }

   if (permissions & (QFile::WriteOwner | QFile::WriteUser)) {
      mode |= S_IWUSR;
   }

   if (permissions & (QFile::ExeOwner | QFile::ExeUser)) {
      mode |= S_IXUSR;
   }

   if (permissions & QFile::ReadGroup) {
      mode |= S_IRGRP;
   }

   if (permissions & QFile::WriteGroup) {
      mode |= S_IWGRP;
   }

   if (permissions & QFile::ExeGroup) {
      mode |= S_IXGRP;
   }

   if (permissions & QFile::ReadOther) {
      mode |= S_IROTH;
   }

   if (permissions & QFile::WriteOther) {
      mode |= S_IWOTH;
   }

   if (permissions & QFile::ExeOther) {
      mode |= S_IXOTH;
   }

   bool success = ::chmod(entry.nativeFilePath().constData(), mode) == 0;

   if (success && data) {
      data->entryFlags &= ~QFileSystemMetaData::Permissions;
      data->entryFlags |= QFileSystemMetaData::MetaDataFlag(uint(permissions));
      data->knownFlagsMask |= QFileSystemMetaData::Permissions;
   }

   if (!success) {
      error = QSystemError(errno, QSystemError::StandardLibraryError);
   }

   return success;
}

QString QFileSystemEngine::homePath()
{
   QString home = QFile::decodeName(qgetenv("HOME"));

   if (home.isEmpty()) {
      home = rootPath();
   }

   return QDir::cleanPath(home);
}

QString QFileSystemEngine::rootPath()
{
   return QString("/");
}

QString QFileSystemEngine::tempPath()
{

#ifdef QT_UNIX_TEMP_PATH_OVERRIDE
   return QT_UNIX_TEMP_PATH_OVERRIDE;

#else
   QString tmp = QFile::decodeName(qgetenv("TMPDIR"));

   if (tmp.isEmpty()) {

#if defined(Q_OS_DARWIN)

      if (NSString *nsPath = NSTemporaryDirectory()) {
         tmp = QString::fromCFString((CFStringRef)nsPath);
      } else {
         tmp = "/tmp";
      }

#else
      tmp = "/tmp";

#endif

   }

   return QDir::cleanPath(tmp);

#endif
}

bool QFileSystemEngine::setCurrentPath(const QFileSystemEntry &path)
{
   int r;
   r = QT_CHDIR(path.nativeFilePath().constData());
   return r >= 0;
}

QFileSystemEntry QFileSystemEngine::currentPath()
{
   QFileSystemEntry result;

#if defined(__GLIBC__) && ! defined(PATH_MAX)
   char *currentName = ::get_current_dir_name();

   if (currentName) {
      result = QFileSystemEntry(QByteArray(currentName), QFileSystemEntry::FromNativePath());
      ::free(currentName);
   }

#else
   char currentName[PATH_MAX + 1];

   if (::getcwd(currentName, PATH_MAX)) {
      result = QFileSystemEntry(QByteArray(currentName), QFileSystemEntry::FromNativePath());
   }

#if defined(CS_SHOW_DEBUG_CORE)
   if (result.isEmpty()) {
      qDebug("QFileSystemEngine::currentPath() Call to getcwd() failed");
   }
#endif

#endif

   return result;
}
