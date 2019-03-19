/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qplatformdefs.h>
#include <qfile.h>
#include <qlibrary_p.h>
#include <qcoreapplication.h>
#include <qfilesystementry_p.h>

#ifdef Q_OS_DARWIN
#include <qcore_mac_p.h>
#endif

#if defined (Q_OS_NACL)
#define QT_NO_DYNAMIC_LIBRARY
#endif

#if !defined(QT_HPUX_LD) && !defined(QT_NO_DYNAMIC_LIBRARY)
#include <dlfcn.h>
#endif

static QString qdlerror()
{
#if defined(QT_NO_DYNAMIC_LIBRARY)
   const char *err = "This platform does not support dynamic libraries.";
#elif !defined(QT_HPUX_LD)
   const char *err = dlerror();
#else
   const char *err = strerror(errno);
#endif
   return err ? QString('(' + QString::fromUtf8(err) + ')') : QString();
}

bool QLibraryPrivate::load_sys()
{
   QString attempt;
#if !defined(QT_NO_DYNAMIC_LIBRARY)
   QFileSystemEntry fsEntry(fileName);

   QString path = fsEntry.path();
   QString name = fsEntry.fileName();

   if (path == QLatin1String(".") && !fileName.startsWith(path)) {
      path.clear();
   } else {
      path += QLatin1Char('/');
   }

   // The first filename we want to attempt to load is the filename as the callee specified.
   // Thus, the first attempt we do must be with an empty prefix and empty suffix.
   QStringList suffixes(QLatin1String("")), prefixes(QLatin1String(""));
   if (pluginState != IsAPlugin) {

      prefixes << QLatin1String("lib");

#if defined(Q_OS_HPUX)
      // according to
      // http://docs.hp.com/en/B2355-90968/linkerdifferencesiapa.htm

      // In PA-RISC (PA-32 and PA-64) shared libraries are suffixed
      // with .sl. In IPF (32-bit and 64-bit), the shared libraries
      // are suffixed with .so. For compatibility, the IPF linker
      // also supports the .sl suffix.

      // But since we don't know if we are built on HPUX or HPUXi,
      // we support both .sl (and .<version>) and .so suffixes but
      // .so is preferred.
# if defined(__ia64)
      if (!fullVersion.isEmpty()) {
         suffixes << QString::fromLatin1(".so.%1").formatArg(fullVersion);
      } else {
         suffixes << QLatin1String(".so");
      }
# endif
      if (!fullVersion.isEmpty()) {
         suffixes << QString::fromLatin1(".sl.%1").formatArg(fullVersion);
         suffixes << QString::fromLatin1(".%1").formatArg(fullVersion);
      } else {
         suffixes << QLatin1String(".sl");
      }
#else

      if (!fullVersion.isEmpty()) {
         suffixes << QString::fromLatin1(".so.%1").formatArg(fullVersion);
      } else {
         suffixes << QLatin1String(".so");
      }
#endif // Q_OS_HPUX

# ifdef Q_OS_MAC
      if (!fullVersion.isEmpty()) {
         suffixes << QString::fromLatin1(".%1.bundle").formatArg(fullVersion);
         suffixes << QString::fromLatin1(".%1.dylib").formatArg(fullVersion);
      } else {
         suffixes << QLatin1String(".bundle") << QLatin1String(".dylib");
      }
#endif
   }
   int dlFlags = 0;

#if defined(QT_HPUX_LD)
   dlFlags = DYNAMIC_PATH | BIND_NONFATAL;

   if (loadHints & QLibrary::ResolveAllSymbolsHint) {
      dlFlags |= BIND_IMMEDIATE;
   } else {
      dlFlags |= BIND_DEFERRED;
   }

#else
   if (loadHints & QLibrary::ResolveAllSymbolsHint) {
      dlFlags |= RTLD_NOW;
   } else {
      dlFlags |= RTLD_LAZY;
   }

   if (loadHints & QLibrary::ExportExternalSymbolsHint) {
      dlFlags |= RTLD_GLOBAL;
   }

   else {

#if defined(Q_OS_MAC)
      if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4)
#endif
         dlFlags |= RTLD_LOCAL;
   }


#endif // QT_HPUX_LD
   // If using the new search heuristics we do:
   //
   //   If the filename is an absolute path then we want to try that first as it is most likely
   //   what the callee wants. If we have been given a non-absolute path then lets try the
   //   native library name first to avoid unnecessary calls to dlopen().
   //
   // otherwise:
   //
   //   We use the old behaviour which is to always try the specified filename first
   if ((loadHints & QLibrary::ImprovedSearchHeuristics) && !fsEntry.isAbsolute()) {
      suffixes.append(QLatin1String(""));
      prefixes.append(QLatin1String(""));
   } else {
      suffixes.prepend(QLatin1String(""));
      prefixes.prepend(QLatin1String(""));
   }

   bool retry = true;
   for (int prefix = 0; retry && !pHnd && prefix < prefixes.size(); prefix++) {
      for (int suffix = 0; retry && !pHnd && suffix < suffixes.size(); suffix++) {
         if (!prefixes.at(prefix).isEmpty() && name.startsWith(prefixes.at(prefix))) {
            continue;
         }
         if (!suffixes.at(suffix).isEmpty() && name.endsWith(suffixes.at(suffix))) {
            continue;
         }
         if (loadHints & QLibrary::LoadArchiveMemberHint) {
            attempt = name;
            int lparen = attempt.indexOf(QLatin1Char('('));
            if (lparen == -1) {
               lparen = attempt.count();
            }
            attempt = path + prefixes.at(prefix) + attempt.insert(lparen, suffixes.at(suffix));
         } else {
            attempt = path + prefixes.at(prefix) + name + suffixes.at(suffix);
         }
#if defined(QT_HPUX_LD)
         pHnd = (void *)shl_load(QFile::encodeName(attempt), dlFlags, 0);
#else
         pHnd = dlopen(QFile::encodeName(attempt).constData(), dlFlags);
#endif

         if (!pHnd && fileName.startsWith(QLatin1Char('/')) && QFile::exists(attempt)) {
            // We only want to continue if dlopen failed due to that the shared library did not exist.
            // However, we are only able to apply this check for absolute filenames (since they are
            // not influenced by the content of LD_LIBRARY_PATH, /etc/ld.so.cache, DT_RPATH etc...)
            // This is all because dlerror is flawed and cannot tell us the reason why it failed.
            retry = false;
         }

      }
   }

#ifdef Q_OS_MAC
   if (!pHnd) {
      QByteArray utf8Bundle = fileName.toUtf8();
      QCFType<CFURLRef> bundleUrl = CFURLCreateFromFileSystemRepresentation(NULL,
                                    reinterpret_cast<const UInt8 *>(utf8Bundle.data()), utf8Bundle.length(), true);

      QCFType<CFBundleRef> bundle = CFBundleCreate(NULL, bundleUrl);

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
      errorString = QLibrary::tr("Can not load library %1: %2").formatArg(fileName).formatArg(qdlerror());
   }
   return (pHnd != 0);
}

bool QLibraryPrivate::unload_sys()
{
#if !defined(QT_NO_DYNAMIC_LIBRARY)
#  if defined(QT_HPUX_LD)
   if (shl_unload((shl_t)pHnd)) {
#  else
   if (dlclose(pHnd)) {
#  endif
      errorString = QLibrary::tr("Cannot unload library %1: %2").formatArg(fileName).formatArg(qdlerror());
      return false;
   }
#endif
   errorString.clear();
   return true;
}

#ifdef Q_OS_MAC
Q_CORE_EXPORT void *qt_mac_resolve_sys(void *handle, const char *symbol)
{
   return dlsym(handle, symbol);
}
#endif

void *QLibraryPrivate::resolve_sys(const QString &symbol)
{

#if defined(QT_HPUX_LD)
   void *address = 0;
   if (shl_findsym((shl_t *)&pHnd, symbol.constData(), TYPE_UNDEFINED, &address) < 0) {
      address = 0;
   }

#elif defined (QT_NO_DYNAMIC_LIBRARY)
   void *address = 0;

#else
   void *address = dlsym(pHnd, symbol.constData());

#endif

   if (! address) {
      errorString = QLibrary::tr("Can not resolve symbol \"%1\" in %2: %3").formatArg(symbol).formatArg(fileName).formatArg(qdlerror());
   } else {
      errorString.clear();
   }

   return address;
}

