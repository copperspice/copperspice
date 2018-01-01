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

#include <qplatformdefs.h>
#include <qlibrary.h>
#include <qlibrary_p.h>
#include <qstringlist.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmutex.h>
#include <qmap.h>
#include <qsettings.h>
#include <qdatetime.h>
#include <qcoreapplication_p.h>

#ifdef Q_OS_MAC
#  include <qcore_mac_p.h>
#endif

#ifndef NO_ERRNO_H
#include <errno.h>
#endif

#include <qdebug.h>
#include <qvector.h>
#include <qdir.h>
#include <qelfparser_p.h>

QT_BEGIN_NAMESPACE

#ifdef QT_NO_DEBUG
#  define QLIBRARY_AS_DEBUG false
#else
#  define QLIBRARY_AS_DEBUG true
#endif

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
// We don not use separate debug and release libs on UNIX, so we want
// to allow loading plugins, regardless of how they were built.
#  define QT_NO_DEBUG_PLUGIN_CHECK
#endif

Q_GLOBAL_STATIC(QMutex, qt_library_mutex)

#ifndef QT_NO_PLUGIN_CHECK
struct qt_token_info {
   qt_token_info(const char *f, const ulong fc)
      : fields(f), field_count(fc), results(fc), lengths(fc) {
      results.fill(0);
      lengths.fill(0);
   }

   const char *fields;
   const ulong field_count;

   QVector<const char *> results;
   QVector<ulong> lengths;
};

/*
  return values:
       1 parse ok
       0 eos
      -1 parse error
*/
static int qt_tokenize(const char *s, ulong s_len, ulong *advance, qt_token_info &token_info)
{
   if (!s) {
      return -1;
   }

   ulong pos = 0, field = 0, fieldlen = 0;
   char current;
   int ret = -1;
   *advance = 0;
   for (;;) {
      current = s[pos];

      // next char
      ++pos;
      ++fieldlen;
      ++*advance;

      if (! current || pos == s_len + 1) {
         // save result
         token_info.results[(int)field] = s;
         token_info.lengths[(int)field] = fieldlen - 1;

         // end of string
         ret = 0;
         break;
      }

      if (current == token_info.fields[field]) {
         // save result
         token_info.results[(int)field] = s;
         token_info.lengths[(int)field] = fieldlen - 1;

         // end of field
         fieldlen = 0;
         ++field;
         if (field == token_info.field_count - 1) {
            // parse ok
            ret = 1;
         }
         if (field == token_info.field_count) {
            // done parsing
            break;
         }

         // reset string and its length
         s = s + pos;
         s_len -= pos;
         pos = 0;
      }
   }

   return ret;
}

/*
  returns true if the string s was correctly parsed, false otherwise.
*/
static bool qt_parse_pattern(const char *s, uint *version, bool *debug, QByteArray *key)
{
   bool ret = true;

   qt_token_info pinfo("=\n", 2);
   int parse;
   ulong at = 0, advance, parselen = qstrlen(s);

   do {
      parse = qt_tokenize(s + at, parselen, &advance, pinfo);
      if (parse == -1) {
         ret = false;
         break;
      }

      at += advance;
      parselen -= advance;

      if (qstrncmp("version", pinfo.results[0], pinfo.lengths[0]) == 0) {
         // parse version string
         qt_token_info pinfo2("..-", 3);

         if (qt_tokenize(pinfo.results[1], pinfo.lengths[1], &advance, pinfo2) != -1) {
            QByteArray m(pinfo2.results[0], pinfo2.lengths[0]);
            QByteArray n(pinfo2.results[1], pinfo2.lengths[1]);
            QByteArray p(pinfo2.results[2], pinfo2.lengths[2]);
            *version  = (m.toUInt() << 16) | (n.toUInt() << 8) | p.toUInt();

         } else {
            ret = false;
            break;
         }

      } else if (qstrncmp("debug", pinfo.results[0], pinfo.lengths[0]) == 0) {
         *debug = qstrncmp("true", pinfo.results[1], pinfo.lengths[1]) == 0;

      } else if (qstrncmp("buildkey", pinfo.results[0], pinfo.lengths[0]) == 0) {
         // save buildkey
         *key = QByteArray(pinfo.results[1], pinfo.lengths[1]);
      }

   } while (parse == 1 && parselen > 0);

   return ret;
}
#endif // QT_NO_PLUGIN_CHECK

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC) && !defined(QT_NO_PLUGIN_CHECK)

static long qt_find_pattern(const char *s, ulong s_len, const char *pattern, ulong p_len)
{
   /*
     we search from the end of the file because on the supported
     systems, the read-only data/text segments are placed at the end
     of the file.  HOWEVER, when building with debugging enabled, all
     the debug symbols are placed AFTER the data/text segments.

     what does this mean?  when building in release mode, the search
     is fast because the data we are looking for is at the end of the
     file... when building in debug mode, the search is slower
     because we have to skip over all the debugging symbols first
   */

   if (! s || ! pattern || p_len > s_len) {
      return -1;
   }
   ulong i, hs = 0, hp = 0, delta = s_len - p_len;

   for (i = 0; i < p_len; ++i) {
      hs += s[delta + i];
      hp += pattern[i];
   }

   i = delta;
   for (;;) {
      if (hs == hp && qstrncmp(s + i, pattern, p_len) == 0) {
         return i;
      }
      if (i == 0) {
         break;
      }
      --i;
      hs -= s[i + p_len];
      hs += s[i];
   }

   return -1;
}

static bool qt_unix_query(const QString &library, uint *version, bool *debug, QByteArray *key, QLibraryPrivate *lib = 0)
{
   QFile file(library);

   if (! file.open(QIODevice::ReadOnly)) {
      if (lib) {
         lib->errorString = file.errorString();
      }

      if (qt_debug_component()) {
         qWarning("%s: %s", QFile::encodeName(library).constData(), csPrintable(qt_error_string(errno)));
      }
      return false;
   }

   QByteArray data;
   const char *filedata = 0;
   ulong fdlen = file.size();
   filedata = (char *) file.map(0, fdlen);

   if (filedata == 0) {
      // try reading the data into memory instead
      data = file.readAll();
      filedata = data.constData();
      fdlen = data.size();
   }


   // NOTE: if you change the variable "pattern", this MUST also be modified in qlibrary.cpp and qplugin.cpp


   //  ELF binaries on GNU, have .qplugin sections.
   long pos = 0;
   const char pattern[] = "pattern=CS_PLUGIN_VERIFICATION_DATA";
   const ulong plen = qstrlen(pattern);

#if defined (Q_OF_ELF) && defined(Q_CC_GNU)
   int r = QElfParser().parse(filedata, fdlen, library, lib, &pos, &fdlen);

   if (r == QElfParser::NoQtSection) {
      if (pos > 0) {
         // find inside .rodata
         long rel = qt_find_pattern(filedata + pos, fdlen, pattern, plen);
         if (rel < 0) {
            pos = -1;
         } else {
            pos += rel;
         }

      } else {
         pos = qt_find_pattern(filedata, fdlen, pattern, plen);
      }

   } else if (r != QElfParser::Ok) {
      if (lib && qt_debug_component()) {
         qWarning("QElfParser: %s", qPrintable(lib->errorString));
      }
      return false;
   }

#else
   pos = qt_find_pattern(filedata, fdlen, pattern, plen);
#endif // defined(Q_OF_ELF) && defined(Q_CC_GNU)

   bool ret = false;
   if (pos >= 0) {
      ret = qt_parse_pattern(filedata + pos, version, debug, key);
   }

   if (!ret && lib) {
      lib->errorString = QLibrary::tr("Plugin verification data mismatch in '%1'").arg(library);
   }

   file.close();
   return ret;
}

#endif

typedef QMap<QString, QLibraryPrivate *> LibraryMap;

struct LibraryData {
   LibraryMap libraryMap;
   QSet<QLibraryPrivate *> loadedLibs;
};

Q_GLOBAL_STATIC(LibraryData, libraryData)

static LibraryMap *libraryMap()
{
   LibraryData *data = libraryData();
   return data ? &data->libraryMap : 0;
}

QLibraryPrivate::QLibraryPrivate(const QString &canonicalFileName, const QString &version)
   : pHnd(0), fileName(canonicalFileName), fullVersion(version), instance(0), cs_version(0),
     libraryRefCount(1), libraryUnloadCount(0), pluginState(MightBeAPlugin)
{
   libraryMap()->insert(canonicalFileName, this);
}

QLibraryPrivate *QLibraryPrivate::findOrCreate(const QString &fileName, const QString &version)
{
   QMutexLocker locker(qt_library_mutex());
   if (QLibraryPrivate *lib = libraryMap()->value(fileName)) {
      lib->libraryRefCount.ref();
      return lib;
   }

   return new QLibraryPrivate(fileName, version);
}

QLibraryPrivate::~QLibraryPrivate()
{
   LibraryMap *const map = libraryMap();

   if (map) {
      QLibraryPrivate *that = map->take(fileName);
      Q_ASSERT(this == that);
      Q_UNUSED(that);
   }
}

void *QLibraryPrivate::resolve(const char *symbol)
{
   if (! pHnd) {
      return 0;
   }

   return resolve_sys(symbol);
}

bool QLibraryPrivate::load()
{
   libraryUnloadCount.ref();

   if (pHnd) {
      return true;
   }

   if (fileName.isEmpty()) {
      return false;
   }

   bool ret = load_sys();

   if (ret) {
      //when loading a library we add a reference so the QLibraryPrivate will not be deleted
      //this allows the abilitiy to unload the library at a later time

      if (LibraryData *lib = libraryData()) {
         lib->loadedLibs += this;
         libraryRefCount.ref();
      }
   }

   return ret;
}

bool QLibraryPrivate::unload()
{
   if (! pHnd) {
      return false;
   }

   if (! libraryUnloadCount.deref()) {
      // only unload if ALL QLibrary instance wanted to
      delete inst.data();

      if  (unload_sys()) {
         if (qt_debug_component())  {
            qWarning() << "QLibraryPrivate::unload() Unload library succeeded:" << fileName;
         }

         // when the library is unloaded, release the reference on it so 'this' so it can be deleted
         if (LibraryData *lib = libraryData()) {

            if (lib->loadedLibs.remove(this)) {
               libraryRefCount.deref();
            }
         }

         pHnd = 0;
      }
   }

   return (pHnd == 0);
}

void QLibraryPrivate::release()
{
   QMutexLocker locker(qt_library_mutex());

   if (! libraryRefCount.deref()) {
      delete this;
   }
}

bool QLibraryPrivate::loadPlugin()
{
   if (instance) {
      libraryUnloadCount.ref();
      return true;
   }

   if (pluginState == IsNotAPlugin) {
      return false;
   }

   if (! load()) {
      if (qt_debug_component()) {
         qWarning() << "QLibraryPrivate::loadPlugin() Failed on" << fileName << ":" << errorString;
      }

      pluginState = IsNotAPlugin;
      return false;
   }

   void *tmp = this->resolve("qt_plugin_instance");
   instance = reinterpret_cast<QtPluginInstanceFunction>(tmp);

   return instance;
}

bool QLibrary::isLibrary(const QString &fileName)
{

#if defined(Q_OS_WIN32)
   return fileName.endsWith(QLatin1String(".dll"), Qt::CaseInsensitive);

#else
   QString completeSuffix = QFileInfo(fileName).completeSuffix();
   if (completeSuffix.isEmpty()) {
      return false;
   }
   QStringList suffixes = completeSuffix.split(QLatin1Char('.'));

# if defined(Q_OS_DARWIN)

   // On Mac, libs look like libmylib.1.0.0.dylib
   const QString lastSuffix = suffixes.at(suffixes.count() - 1);
   const QString firstSuffix = suffixes.at(0);

   bool valid = (lastSuffix == QLatin1String("dylib")
                 || firstSuffix == QLatin1String("so")
                 || firstSuffix == QLatin1String("bundle"));

   return valid;

# else  // Generic Unix
   QStringList validSuffixList;

#  if defined(Q_OS_HPUX)
   /*
       See "HP-UX Linker and Libraries User's Guide", section "Link-time Differences between PA-RISC and IPF":
       "In PA-RISC (PA-32 and PA-64) shared libraries are suffixed with .sl. In IPF (32-bit and 64-bit),
       the shared libraries are suffixed with .so. For compatibility, the IPF linker also supports the .sl suffix."
    */
   validSuffixList << QLatin1String("sl");
#   if defined __ia64
   validSuffixList << QLatin1String("so");
#   endif

#  elif defined(Q_OS_UNIX)
   validSuffixList << QLatin1String("so");
#  endif

   // Examples of valid library names:
   //  libfoo.so
   //  libfoo.so.0
   //  libfoo.so.0.3
   //  libfoo-0.3.so
   //  libfoo-0.3.so.0.3.0

   int suffix;
   int suffixPos = -1;
   for (suffix = 0; suffix < validSuffixList.count() && suffixPos == -1; ++suffix) {
      suffixPos = suffixes.indexOf(validSuffixList.at(suffix));
   }

   bool valid = suffixPos != -1;
   for (int i = suffixPos + 1; i < suffixes.count() && valid; ++i)
      if (i != suffixPos) {
         suffixes.at(i).toInt(&valid);
      }
   return valid;
# endif

#endif  // Q_OS_WIN

}

typedef const char *(*QtPluginQueryVerificationDataFunction)();

bool qt_get_verificationdata(QtPluginQueryVerificationDataFunction pfn, uint *cs_version, bool *debug, QByteArray *key,
                             bool *exceptionThrown)
{
   *exceptionThrown = false;
   const char *szData = 0;

   if (! pfn) {
      return false;
   }

#ifdef QT_USE_MS_STD_EXCEPTION
   szData = qt_try_versioninfo((void *)pfn, exceptionThrown);

   if (*exceptionThrown) {
      return false;
   }
#else
   szData = pfn();
#endif

#ifdef QT_NO_PLUGIN_CHECK
   return true;
#else
   return qt_parse_pattern(szData, cs_version, debug, key);
#endif
}

bool QLibraryPrivate::isPlugin(QSettings *settings)
{
   errorString.clear();

   if (pluginState != MightBeAPlugin) {
      return pluginState == IsAPlugin;
   }

#ifndef QT_NO_PLUGIN_CHECK
   bool debug = !QLIBRARY_AS_DEBUG;
   QByteArray key;
   bool success = false;

#if defined(Q_OS_UNIX) && ! defined(Q_OS_MAC)
   if (fileName.endsWith(QLatin1String(".debug"))) {
      // refuse to load a file that ends in .debug
      // these are the debug symbols from the libraries
      // the problem is that they are valid shared library files
      // and dlopen is known to crash while opening them

      // pretend we did not see the file
      errorString = QLibrary::tr("The shared library was not found.");
      pluginState = IsNotAPlugin;

      return false;
   }
#endif

   QFileInfo fileinfo(fileName);

#ifndef QT_NO_DATESTRING
   lastModified  = fileinfo.lastModified().toString(Qt::ISODate);
#endif

   QString regkey = QString::fromLatin1("CopperSpice Plugin Cache %1.%2.%3/%4")
                    .arg((CS_VERSION & 0xff0000) >> 16)
                    .arg((CS_VERSION & 0xff00) >> 8)
                    .arg(QLIBRARY_AS_DEBUG ? QLatin1String("debug") : QLatin1String("false"))
                    .arg(fileName);

#ifdef Q_OS_MAC
   // On Mac, add the application arch to the reg key in order to
   // cache plugin information separately for each arch. This prevents
   // Qt from wrongly caching plugin load failures when the archs don't match.

#if defined(__x86_64__)
   regkey += QLatin1String("-x86_64");

#elif defined(__i386__)
   regkey += QLatin1String("-i386");

#elif defined(__ppc64__)
   regkey += QLatin1String("-ppc64");

#elif defined(__ppc__)
   regkey += QLatin1String("-ppc");

#endif

#endif

   QStringList reg;

#ifndef QT_NO_SETTINGS
   if (! settings) {
      settings = QCoreApplicationPrivate::copperspiceConf();
   }
   reg = settings->value(regkey).toStringList();
#endif

   if (reg.count() == 4 && lastModified == reg.at(3)) {
      cs_version = reg.at(0).toUInt(0, 16);
      debug = bool(reg.at(1).toInt());
      key = reg.at(2).toLatin1();
      success = cs_version != 0;

   } else {

#if defined(Q_OS_UNIX) && ! defined(Q_OS_MAC)
      if (! pHnd) {
         // use unix shortcut to avoid loading the library
         success = qt_unix_query(fileName, &cs_version, &debug, &key, this);
      } else
#endif
      {
         bool retryLoadLibrary = false;    // Only used on Windows with MS compiler.(false in other cases)

         do {
            bool temporary_load = false;

#ifdef Q_OS_WIN
            HMODULE hTempModule = 0;

            if (! pHnd) {
               DWORD dwFlags = (retryLoadLibrary) ? 0 : DONT_RESOLVE_DLL_REFERENCES;

               // avoid 'Bad Image' message box
               UINT oldmode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
               hTempModule = ::LoadLibraryEx((wchar_t *)QDir::toNativeSeparators(fileName).utf16(), 0, dwFlags);
               SetErrorMode(oldmode);
            }

#else
            if (! pHnd) {
               temporary_load =  load_sys();
            }
#endif


#ifdef Q_OS_WIN
            QtPluginQueryVerificationDataFunction qtPluginQueryVerificationDataFunction;

            if (hTempModule) {
               qtPluginQueryVerificationDataFunction =
                  reinterpret_cast<QtPluginQueryVerificationDataFunction>
                  (::GetProcAddress(hTempModule, "cs_plugin_query_verification_data"));
            } else {
               qtPluginQueryVerificationDataFunction =
                  reinterpret_cast<QtPluginQueryVerificationDataFunction>
                  (resolve("cs_plugin_query_verification_data"));
            }

#else
            QtPluginQueryVerificationDataFunction qtPluginQueryVerificationDataFunction = NULL;
            qtPluginQueryVerificationDataFunction =
               (QtPluginQueryVerificationDataFunction) resolve("cs_plugin_query_verification_data");
#endif

            bool exceptionThrown = false;
            bool ret = qt_get_verificationdata(qtPluginQueryVerificationDataFunction, &cs_version, &debug, &key, &exceptionThrown);

            if (! exceptionThrown) {
               if (! ret) {
                  cs_version = 0;
                  key = "unknown";

                  if (temporary_load)  {
                     unload_sys();
                  }

               } else {
                  success = true;
               }

               retryLoadLibrary = false;
            }

#ifdef QT_USE_MS_STD_EXCEPTION
            else {
               // An exception was thrown when calling qt_plugin_query_verification_data().
               // This usually happens when plugin is compiled with the /clr compiler flag,
               // & will only work if the dependencies are loaded & DLLMain() is called.
               // LoadLibrary() will do this, try once with this & if it fails don't load.
               retryLoadLibrary = !retryLoadLibrary;
            }
#endif

#ifdef Q_OS_WIN
            if (hTempModule) {
               BOOL ok = ::FreeLibrary(hTempModule);
               if (ok) {
                  hTempModule = 0;
               }

            }
#endif
         } while (retryLoadLibrary); // Will be 'false' in all cases other than when an
         // exception is thrown(will happen only when using a MS compiler)
      }

      // stl does not affect binary compatibility
      key.replace(" no-stl", "");

#ifndef QT_NO_SETTINGS
      QStringList queried;

      queried << QString::number(cs_version, 16)
              << QString::number((int)debug)
              << QLatin1String(key)
              << lastModified;

      settings->setValue(regkey, queried);
#endif

   }

   if (! success) {
      if (errorString.isEmpty()) {
         if (fileName.isEmpty()) {
            errorString = QLibrary::tr("Shared library was not found.");
         } else {
            errorString = QLibrary::tr("File '%1' is not a valid CopperSpice plugin.").arg(fileName);
         }
      }
      return false;
   }

   pluginState = IsNotAPlugin; // be pessimistic

   if ((cs_version & 0x00ff00) > (CS_VERSION & 0x00ff00) || (cs_version & 0xff0000) != (CS_VERSION & 0xff0000)) {
      if (qt_debug_component()) {
         qWarning("In %s:\n"
                  "  Plugin uses incompatible CopperSpice library (%d.%d.%d) [%s]", QFile::encodeName(fileName).constData(),
                  (cs_version & 0xff0000) >> 16, (cs_version & 0xff00) >> 8, cs_version & 0xff, debug ? "debug" : "release");
      }

      errorString = QLibrary::tr("Plugin '%1' uses incompatible CopperSpice library. (%2.%3.%4) [%5]")
                  .arg(fileName).arg((cs_version & 0xff0000) >> 16).arg((cs_version & 0xff00) >> 8).arg(cs_version & 0xff)
                  .arg(debug ? "debug" : "release" );

   } else if (key != QT_BUILD_KEY
              // we may have some compatibility keys, try them too:

#ifdef CS_BUILD_KEY_COMPAT1
              && key != CS_BUILD_KEY_COMPAT1
#endif

#ifdef CS_BUILD_KEY_COMPAT2
              && key != CS_BUILD_KEY_COMPAT2
#endif

#ifdef CS_BUILD_KEY_COMPAT3
              && key != CS_BUILD_KEY_COMPAT3
#endif

             ) {

      if (qt_debug_component()) {
         qWarning("In %s:\n"
                  "  Plugin uses incompatible CopperSpice library\n"
                  "  Expected build key \"%s\", got \"%s\"",
                  QFile::encodeName(fileName).constData(), QT_BUILD_KEY, key.isEmpty() ? "<null>" : key.constData());
      }

      errorString = QLibrary::tr("Plugin '%1' uses incompatible CopperSpice library.\n"
                  " Expected build key \"%2\", got \"%3\"").arg(fileName).arg(QLatin1String(QT_BUILD_KEY))
                  .arg(key.isEmpty() ? "<null>" : key.constData() );

#ifndef QT_NO_DEBUG_PLUGIN_CHECK
   } else if (debug != QLIBRARY_AS_DEBUG) {
      // do not issue a qWarning since there may be no debug support
      errorString = QLibrary::tr("Plugin '%1' uses incompatible CopperSpice library."
                  " (Can not mix debug and release libraries.)").arg(fileName);

#endif
   } else {
      pluginState = IsAPlugin;

   }

   return pluginState == IsAPlugin;

#else
   Q_UNUSED(settings);
   return pluginState == MightBeAPlugin;

#endif   // QT_NO_PLUGIN_CHECK

}

bool QLibrary::load()
{
   if (!d) {
      return false;
   }

   if (did_load) {
      return d->pHnd;
   }

   did_load = true;
   return d->load();
}

bool QLibrary::unload()
{
   if (did_load) {
      did_load = false;
      return d->unload();
   }
   return false;
}

bool QLibrary::isLoaded() const
{
   return d && d->pHnd;
}

QLibrary::QLibrary(QObject *parent)
   : QObject(parent), d(0), did_load(false)
{
}

QLibrary::QLibrary(const QString &fileName, QObject *parent)
   : QObject(parent), d(0), did_load(false)
{
   setFileName(fileName);
}

QLibrary::QLibrary(const QString &fileName, int verNum, QObject *parent)
   : QObject(parent), d(0), did_load(false)
{
   setFileNameAndVersion(fileName, verNum);
}

QLibrary::QLibrary(const QString &fileName, const QString &version, QObject *parent)
   : QObject(parent), d(0), did_load(false)
{
   setFileNameAndVersion(fileName, version);
}

QLibrary::~QLibrary()
{
   if (d) {
      d->release();
   }
}

void QLibrary::setFileName(const QString &fileName)
{
   QLibrary::LoadHints lh;
   if (d) {
      lh = d->loadHints;
      d->release();
      d = 0;
      did_load = false;
   }
   d = QLibraryPrivate::findOrCreate(fileName);
   d->loadHints = lh;
}

QString QLibrary::fileName() const
{
   if (d) {
      return d->qualifiedFileName.isEmpty() ? d->fileName : d->qualifiedFileName;
   }
   return QString();
}

void QLibrary::setFileNameAndVersion(const QString &fileName, int verNum)
{
   QLibrary::LoadHints lh;
   if (d) {
      lh = d->loadHints;
      d->release();
      d = 0;
      did_load = false;
   }
   d = QLibraryPrivate::findOrCreate(fileName, verNum >= 0 ? QString::number(verNum) : QString());
   d->loadHints = lh;
}

void QLibrary::setFileNameAndVersion(const QString &fileName, const QString &version)
{
   QLibrary::LoadHints lh;
   if (d) {
      lh = d->loadHints;
      d->release();
      d = 0;
      did_load = false;
   }
   d = QLibraryPrivate::findOrCreate(fileName, version);
   d->loadHints = lh;
}

void *QLibrary::resolve(const char *symbol)
{
   if (!isLoaded() && !load()) {
      return 0;
   }
   return d->resolve(symbol);
}

void *QLibrary::resolve(const QString &fileName, const char *symbol)
{
   QLibrary library(fileName);
   return library.resolve(symbol);
}

void *QLibrary::resolve(const QString &fileName, int verNum, const char *symbol)
{
   QLibrary library(fileName, verNum);
   return library.resolve(symbol);
}

void *QLibrary::resolve(const QString &fileName, const QString &version, const char *symbol)
{
   QLibrary library(fileName, version);
   return library.resolve(symbol);
}

QString QLibrary::errorString() const
{
   return (!d || d->errorString.isEmpty()) ? tr("Unknown error") : d->errorString;
}

void QLibrary::setLoadHints(LoadHints hints)
{
   if (!d) {
      d = QLibraryPrivate::findOrCreate(QString());   // ugly, but we need a d-ptr
      d->errorString.clear();
   }
   d->loadHints = hints;
}

QLibrary::LoadHints QLibrary::loadHints() const
{
   return d ? d->loadHints : (QLibrary::LoadHints)0;
}

// internal
bool qt_debug_component()
{
   static int debug_env = -1;

   if (debug_env == -1) {
      debug_env = QT_PREPEND_NAMESPACE(qgetenv)("QT_DEBUG_PLUGINS").toInt();
   }

   return debug_env != 0;
}

QT_END_NAMESPACE
