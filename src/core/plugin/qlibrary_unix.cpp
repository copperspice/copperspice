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

#include <qcoreapplication.h>
#include <qfile.h>
#include <qplatformdefs.h>

#include <qfilesystementry_p.h>
#include <qlibrary_p.h>

#ifdef Q_OS_DARWIN
#  include <qcore_mac_p.h>
#endif

#if defined (Q_OS_NACL)
#define QT_NO_DYNAMIC_LIBRARY
#endif

#if ! defined(QT_NO_DYNAMIC_LIBRARY)
#include <dlfcn.h>
#endif

static QString qdlerror()
{
#if defined(QT_NO_DYNAMIC_LIBRARY)
   const char *err = "This platform does not support dynamic libraries.";

#else
   const char *err = strerror(errno);
#endif

   return err ? QString('(' + QString::fromUtf8(err) + ')') : QString();
}

QStringList QLibraryHandle::suffixes_sys(const QString &fullVersion)
{
   QStringList suffixes;

   if (! fullVersion.isEmpty()) {
      suffixes << QString::fromLatin1(".so.%1").formatArg(fullVersion);
   } else {
      suffixes << QLatin1String(".so");
   }

# ifdef Q_OS_DARWIN

   if (! fullVersion.isEmpty()) {
      suffixes << QString(".%1.bundle").formatArg(fullVersion);
      suffixes << QString(".%1.dylib").formatArg(fullVersion);
   } else {
      suffixes << ".bundle" << ".dylib";
   }

#endif

   return suffixes;
}

QStringList QLibraryHandle::prefixes_sys()
{
   return QStringList() << "lib";
}

bool QLibraryHandle::load_sys()
{
   QString attempt;

#if ! defined(QT_NO_DYNAMIC_LIBRARY)
   QFileSystemEntry fsEntry(fileName);

   QString path = fsEntry.path();
   QString name = fsEntry.fileName();

   if (path == "." && ! fileName.startsWith(path)) {
      path.clear();

   } else {
      path += '/';
   }

   QStringList suffixes;
   QStringList prefixes;

   if (pluginState != IsAPlugin) {
      prefixes = prefixes_sys();
      suffixes = suffixes_sys(fullVersion);
   }

   int dlFlags = 0;

   int loadHints = this->loadHints();

   if (loadHints & QLibrary::ResolveAllSymbolsHint) {
      dlFlags |= RTLD_NOW;
   } else {
      dlFlags |= RTLD_LAZY;
   }

   if (loadHints & QLibrary::ExportExternalSymbolsHint) {
      dlFlags |= RTLD_GLOBAL;

   } else {
      dlFlags |= RTLD_LOCAL;
   }

#if defined(RTLD_DEEPBIND)

   if (loadHints & QLibrary::DeepBindHint) {
      dlFlags |= RTLD_DEEPBIND;
   }

#endif

   // Provide access to RTLD_NODELETE flag on Unix
   // From GNU documentation on RTLD_NODELETE:
   // Do not unload the library during dlclose(). Consequently, the
   // library's specific static variables are not reinitialized if the
   // library is reloaded with dlopen() at a later time.

#ifdef RTLD_NODELETE

   if (loadHints & QLibrary::PreventUnloadHint) {
      dlFlags |= RTLD_NODELETE;
   }

#endif

   // If the filename is an absolute path then we want to try that first as it is most likely
   // what the callee wants. If we have been given a non-absolute path then lets try the
   // native library name first to avoid unnecessary calls to dlopen().

   if (fsEntry.isAbsolute()) {
      suffixes.prepend(QString());
      prefixes.prepend(QString());

   } else {
      suffixes.append(QString());
      prefixes.append(QString());
   }

   bool retry = true;

   for (int prefix = 0; retry && !pHnd && prefix < prefixes.size(); prefix++) {
      for (int suffix = 0; retry && !pHnd && suffix < suffixes.size(); suffix++) {
         if (! prefixes.at(prefix).isEmpty() && name.startsWith(prefixes.at(prefix))) {
            continue;
         }

         if (!suffixes.at(suffix).isEmpty() && name.endsWith(suffixes.at(suffix))) {
            continue;
         }

         if (loadHints & QLibrary::LoadArchiveMemberHint) {
            attempt = name;
            int lparen = attempt.indexOf('(');

            if (lparen == -1) {
               lparen = attempt.count();
            }

            attempt = path + prefixes.at(prefix) + attempt.insert(lparen, suffixes.at(suffix));
         } else {
            attempt = path + prefixes.at(prefix) + name + suffixes.at(suffix);
         }

         pHnd = dlopen(QFile::encodeName(attempt).constData(), dlFlags);

         if (!pHnd && fileName.startsWith(QLatin1Char('/')) && QFile::exists(attempt)) {
            // We only want to continue if dlopen failed due to that the shared library did not exist.
            // However, we are only able to apply this check for absolute filenames (since they are
            // not influenced by the content of LD_LIBRARY_PATH, /etc/ld.so.cache, DT_RPATH etc...)
            // This is all because dlerror is flawed and cannot tell us the reason why it failed.
            retry = false;
         }
      }
   }

#ifdef Q_OS_DARWIN

   if (!pHnd) {
      QByteArray utf8Bundle = fileName.toUtf8();
      QCFType<CFURLRef> bundleUrl = CFURLCreateFromFileSystemRepresentation(nullptr,
                  reinterpret_cast<const UInt8 *>(utf8Bundle.data()), utf8Bundle.length(), true);

      QCFType<CFBundleRef> bundle = CFBundleCreate(nullptr, bundleUrl);

      if (bundle) {
         QCFType<CFURLRef> url = CFBundleCopyExecutableURL(bundle);
         char executableFile[FILENAME_MAX];

         CFURLGetFileSystemRepresentation(url, true, reinterpret_cast<UInt8 *>(executableFile), FILENAME_MAX);

         attempt = QString::fromUtf8(executableFile);
         pHnd = dlopen(QFile::encodeName(attempt).constData(), dlFlags);
      }
   }

#endif

#endif // ! defined(QT_NO_DYNAMIC_LIBRARY)

   if (pHnd) {
      qualifiedFileName = attempt;
      errorString.clear();
   } else {
      errorString = QLibrary::tr("Unable to load library %1: %2").formatArg(fileName).formatArg(qdlerror());
   }

   return (pHnd != nullptr);
}

bool QLibraryHandle::unload_sys()
{
#if ! defined(QT_NO_DYNAMIC_LIBRARY)

   if (dlclose(pHnd)) {
      errorString = QLibrary::tr("Unable to unload library %1: %2").formatArg(fileName).formatArg(qdlerror());
      return false;
   }

#endif
   errorString.clear();
   return true;
}

#if defined(Q_OS_LINUX) && ! defined(QT_NO_DYNAMIC_LIBRARY)
Q_CORE_EXPORT void *qt_linux_find_symbol_sys(const char *symbol)
{
   return dlsym(RTLD_DEFAULT, symbol);
}
#endif

#ifdef Q_OS_DARWIN
Q_CORE_EXPORT void *qt_mac_resolve_sys(void *handle, const char *symbol)
{
   return dlsym(handle, symbol);
}
#endif

void *QLibraryHandle::resolve_sys(const QString &symbol)
{

#if defined (QT_NO_DYNAMIC_LIBRARY)
   void *address = nullptr;

#else
   void *address = dlsym(pHnd, symbol.constData());

#endif

   if (! address) {
      errorString = QLibrary::tr("Unable to resolve symbol \"%1\" in %2:%3").formatArg(symbol).formatArg(fileName).formatArg(qdlerror());
   } else {
      errorString.clear();
   }

   return address;
}
