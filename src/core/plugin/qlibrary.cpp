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

#include <qlibrary.h>
#include <qlibrary_p.h>

#include <qstringlist.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmutex.h>
#include <qmap.h>
#include <qdebug.h>
#include <qvector.h>
#include <qdir.h>
#include <qcoreapplication_p.h>
#include <qplatformdefs.h>

#include <qjsondocument.h>
#include <qjsonvalue.h>

#include <qelfparser_p.h>
#include <qmachparser_p.h>

#ifdef Q_OS_MAC
#  include <qcore_mac_p.h>
#endif

#ifndef NO_ERRNO_H
#include <errno.h>
#endif

#ifdef QT_NO_DEBUG
#  define QLIBRARY_AS_DEBUG false
#else
#  define QLIBRARY_AS_DEBUG true
#endif

#if defined(Q_OS_UNIX)
// We don not use separate debug and release libs on UNIX, so we want
// to allow loading plugins, regardless of how they were built.
#  define QT_NO_DEBUG_PLUGIN_CHECK
#endif


static long qt_find_pattern(const char *s, ulong s_len,
   const char *pattern, ulong p_len)
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

static bool findPatternUnloaded(const QString &library, QLibraryPrivate *lib)
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
   const char *filedata = nullptr;

   ulong fdlen = file.size();
   filedata    = (char *) file.map(0, fdlen);      // first parameter must be zero, not nullptr

   if (filedata == nullptr) {
      // try reading the data into memory instead
      data     = file.readAll();
      filedata = data.constData();
      fdlen    = data.size();
   }


   //  ELF binaries on GNU, have .qplugin sections.
   bool hasMetaData = false;
   long pos = 0;
   char pattern[] = "qTMETADATA  ";

   pattern[0] = 'Q'; // Ensure the pattern "QTMETADATA" is not found in this library should QPluginLoader ever encounter it.
   const ulong plen     = qstrlen(pattern);

#if defined (Q_OF_ELF) && defined(Q_CC_GNU)
   int r = QElfParser().parse(filedata, fdlen, library, lib, &pos, &fdlen);

   if (r == QElfParser::Corrupt || r == QElfParser::NotElf) {
      if (lib && qt_debug_component()) {
         qWarning("QElfParser: %s", qPrintable(lib->errorString));
      }
      return false;

   } else if (r == QElfParser::QtMetaDataSection) {

      long rel = qt_find_pattern(filedata + pos, fdlen, pattern, plen);
      if (rel < 0) {
         pos = -1;
      } else {
         pos += rel;
      }
      hasMetaData = true;
   }
#elif defined (Q_OF_MACH_O)
   {
      QString errorString;
      int r = QMachOParser::parse(filedata, fdlen, library, &errorString, &pos, &fdlen);
      if (r == QMachOParser::NotSuitable) {
         if (qt_debug_component()) {
            qWarning("QMachOParser: %s", qPrintable(errorString));
         }
         if (lib) {
            lib->errorString = errorString;
         }
         return false;
      }

      long rel = qt_find_pattern(filedata + pos, fdlen, pattern, plen);
      if (rel < 0) {
         pos = -1;
      } else {
         pos += rel;
      }
      hasMetaData = true;
   }
#else
   pos = qt_find_pattern(filedata, fdlen, pattern, plen);
   if (pos > 0) {
      hasMetaData = true;
   }
#endif // defined(Q_OF_ELF) && defined(Q_CC_GNU)

   bool ret = false;
   if (pos >= 0) {
      if (hasMetaData) {
         const char *data = filedata + pos;
         QJsonDocument doc = QLibraryPrivate::fromRawMetaData(data);
         lib->metaData = doc.object();

         if (qt_debug_component()) {
            qWarning("Found metadata in lib %s, metadata=\n%s\n", library.toUtf8().constData(), doc.toJson().constData());
         }

         ret = !doc.isNull();
      }
   }

   if (!ret && lib) {
      lib->errorString = QLibrary::tr("Failed to extract plugin meta data from '%1'").formatArg(library);
   }

   file.close();
   return ret;
}

static void installCoverageTool(QLibraryPrivate *libPrivate)
{
#ifdef __COVERAGESCANNER__
   int ret = __coveragescanner_register_library(libPrivate->fileName.toLocal8Bit());

   if (qt_debug_component()) {
      if (ret >= 0) {
         qDebug("Coverage data for %s registered", csPrintable(libPrivate->fileName));

      } else {
         qWarning("Unable to register %s: error %d; coverage data may be incomplete",
            qPrintable(libPrivate->fileName), ret);
      }
   }

#endif
}

class QLibraryStore
{
 public:
   inline ~QLibraryStore();
   static inline QLibraryPrivate *findOrCreate(const QString &fileName, const QString &version, QLibrary::LoadHints loadHints);
   static inline void releaseLibrary(QLibraryPrivate *lib);

   static inline void cleanup();

 private:
   static inline QLibraryStore *instance();

   // all members and instance() are protected by qt_library_mutex
   typedef QMap<QString, QLibraryPrivate *> LibraryMap;
   LibraryMap libraryMap;
};

static QBasicMutex qt_library_mutex;
static QLibraryStore *qt_library_data = 0;
static bool qt_library_data_once;

QLibraryStore::~QLibraryStore()
{
   qt_library_data = 0;
}

inline void QLibraryStore::cleanup()
{
   QLibraryStore *data = qt_library_data;
   if (! data) {
      return;
   }

   LibraryMap::iterator it = data->libraryMap.begin();

   for (; it != data->libraryMap.end(); ++it) {
      QLibraryPrivate *lib = it.value();

      if (lib->libraryRefCount.load() == 1) {

         if (lib->libraryUnloadCount.load() > 0) {
            Q_ASSERT(lib->pHnd);

            lib->libraryUnloadCount.store(1);
#ifdef __GLIBC__
            lib->unload(QLibraryPrivate::NoUnloadSys);
#else
            lib->unload();
#endif
         }
         delete lib;
         it.value() = 0;
      }
   }

   if (qt_debug_component()) {
      foreach (QLibraryPrivate *lib, data->libraryMap) {
         if (lib)
            qDebug() << "On CsCore unload," << lib->fileName << "was leaked, with"
               << lib->libraryRefCount.load() << "users";
      }
   }

   delete data;
}

static void qlibraryCleanup()
{
   QLibraryStore::cleanup();
}
Q_DESTRUCTOR_FUNCTION(qlibraryCleanup)

// must be called with a locked mutex
QLibraryStore *QLibraryStore::instance()
{
   if (Q_UNLIKELY(!qt_library_data_once && !qt_library_data)) {
      // only create once per process lifetime
      qt_library_data = new QLibraryStore;
      qt_library_data_once = true;
   }
   return qt_library_data;
}

inline QLibraryPrivate *QLibraryStore::findOrCreate(const QString &fileName, const QString &version,
   QLibrary::LoadHints loadHints)
{
   QMutexLocker locker(&qt_library_mutex);
   QLibraryStore *data = instance();

   // check if this library is already loaded
   QLibraryPrivate *lib = nullptr;

   if (data) {
      lib = data->libraryMap.value(fileName);

      if (lib) {
         lib->mergeLoadHints(loadHints);
      }
   }

   if (!lib) {
      lib = new QLibraryPrivate(fileName, version, loadHints);
   }

   // track this library
   if (data && ! fileName.isEmpty()) {
      data->libraryMap.insert(fileName, lib);
   }

   lib->libraryRefCount.ref();
   return lib;
}
inline void QLibraryStore::releaseLibrary(QLibraryPrivate *lib)
{
   QMutexLocker locker(&qt_library_mutex);
   QLibraryStore *data = instance();

   if (lib->libraryRefCount.deref()) {
      // still in use
      return;
   }

   // no one else is using
   Q_ASSERT(lib->libraryUnloadCount.load() == 0);

   if (Q_LIKELY(data) && !lib->fileName.isEmpty()) {
      QLibraryPrivate *that = data->libraryMap.take(lib->fileName);
      Q_ASSERT(lib == that);
      Q_UNUSED(that);
   }
   delete lib;
}
QLibraryPrivate::QLibraryPrivate(const QString &canonicalFileName, const QString &version, QLibrary::LoadHints loadHints)
   : pHnd(0), fileName(canonicalFileName), fullVersion(version), instance(0),
     libraryRefCount(0), libraryUnloadCount(0), pluginState(MightBeAPlugin)
{
   loadHintsInt.store(loadHints);
   if (canonicalFileName.isEmpty()) {
      errorString = QLibrary::tr("The shared library was not found.");
   }
}
QLibraryPrivate *QLibraryPrivate::findOrCreate(const QString &fileName, const QString &version,
   QLibrary::LoadHints loadHints)
{
   return QLibraryStore::findOrCreate(fileName, version, loadHints);
}
QLibraryPrivate::~QLibraryPrivate()
{
}
void QLibraryPrivate::mergeLoadHints(QLibrary::LoadHints lh)
{
   if (pHnd) {
      return;
   }
   loadHintsInt.store(lh);
}

void *QLibraryPrivate::resolve(const QString &symbol)
{
   if (! pHnd) {
      return nullptr;
   }

   return resolve_sys(symbol);
}

void QLibraryPrivate::setLoadHints(QLibrary::LoadHints lh)
{
   // this locks a global mutex
   QMutexLocker lock(&qt_library_mutex);
   mergeLoadHints(lh);
}

bool QLibraryPrivate::load()
{
   if (pHnd) {
      libraryUnloadCount.ref();
      return true;
   }

   if (fileName.isEmpty()) {
      return false;
   }

   bool ret = load_sys();

   if (qt_debug_component()) {
      qDebug() << "loaded library" << fileName;
   }


   if (ret) {
      //when loading a library we add a reference so the QLibraryPrivate will not be deleted
      //this allows the abilitiy to unload the library at a later time
      libraryUnloadCount.ref();
      libraryRefCount.ref();
      installCoverageTool(this);
   }

   return ret;
}

bool QLibraryPrivate::unload(UnloadFlag flag)
{
   if (! pHnd) {
      return false;
   }

   if (libraryUnloadCount.load() > 0 && !libraryUnloadCount.deref()) { // only unload if ALL QLibrary instance wanted to
      // only unload if ALL QLibrary instance wanted to
      delete inst.data();

      if (flag == NoUnloadSys || unload_sys()) {
         if (qt_debug_component())
            qWarning() << "QLibraryPrivate::unload succeeded on" << fileName
               << (flag == NoUnloadSys ? "(faked)" : "");
         //when the library is unloaded, we release the reference on it so that 'this'
         //can get deleted
         libraryRefCount.deref();
         pHnd = 0;
         instance = 0;
      }
   }

   return (pHnd == 0);
}

void QLibraryPrivate::release()
{
   QLibraryStore::releaseLibrary(this);
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

   if (load()) {
      instance = (QtPluginInstanceFunction)resolve("qt_plugin_instance");
      return instance;
   }


   if (qt_debug_component()) {
      qWarning() << "QLibraryPrivate::loadPlugin failed on" << fileName << ":" << errorString;
   }

   pluginState = IsNotAPlugin;

   return false;
}


bool QLibrary::isLibrary(const QString &fileName)
{

#if defined(Q_OS_WIN)
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


#  if defined(Q_OS_UNIX)
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
         suffixes.at(i).toInteger<int>(&valid);
      }
   return valid;
# endif

#endif

}

typedef const char *(*QtPluginQueryVerificationDataFunction)();

static bool qt_get_metadata(QtPluginQueryVerificationDataFunction pfn, QLibraryPrivate *priv)
{
   const char *szData = 0;
   if (!pfn) {
      return false;
   }

   szData = pfn();
   if (!szData) {
      return false;
   }

   QJsonDocument doc = QLibraryPrivate::fromRawMetaData(szData);
   if (doc.isNull()) {
      return false;
   }
   priv->metaData = doc.object();
   return true;
}

bool QLibraryPrivate::isPlugin()
{
   if (pluginState == MightBeAPlugin) {
      updatePluginState();
   }
   return pluginState == IsAPlugin;
}

void QLibraryPrivate::updatePluginState()
{
   errorString.clear();

   if (pluginState != MightBeAPlugin) {
      return;
   }

   bool success = false;

#if defined(Q_OS_UNIX) && ! defined(Q_OS_MAC)
   if (fileName.endsWith(".debug")) {

      // refuse to load a file that ends in .debug
      // these are the debug symbols from the libraries
      // the problem is that they are valid shared library files
      // and dlopen is known to crash while opening them

      // pretend we did not see the file
      errorString = QLibrary::tr("The shared library was not found.");
      pluginState = IsNotAPlugin;

      return;
   }
#endif

   if (! pHnd) {
      // use unix shortcut to avoid loading the library
      success = findPatternUnloaded(fileName, this);


   } else {
      // library is already loaded (probably via QLibrary)



      // simply get the target function and call it.
      QtPluginQueryVerificationDataFunction getMetaData = NULL;
      getMetaData = (QtPluginQueryVerificationDataFunction) resolve("qt_plugin_query_metadata");
      success = qt_get_metadata(getMetaData, this);
   }

   if (! success) {
      if (errorString.isEmpty()) {

         if (fileName.isEmpty()) {
            errorString = QLibrary::tr("Shared library was not found.");
         } else {
            errorString = QLibrary::tr("File '%1' is not a valid CopperSpice plugin.").formatArg(fileName);
         }
      }

      pluginState = IsNotAPlugin;
      return;
   }

   pluginState = IsNotAPlugin; // be pessimistic

   uint cs_version = (uint)metaData.value("version").toDouble();
   bool debug = metaData.value(QLatin1String("debug")).toBool();

   if ((cs_version & 0x00ff00) > (CS_VERSION & 0x00ff00) || (cs_version & 0xff0000) != (CS_VERSION & 0xff0000)) {
      if (qt_debug_component()) {
         qWarning("In %s:\n"
            "  Plugin uses incompatible CopperSpice library (%d.%d.%d) [%s]", QFile::encodeName(fileName).constData(),
            (cs_version & 0xff0000) >> 16, (cs_version & 0xff00) >> 8, cs_version & 0xff, debug ? "debug" : "release" );
      }

      errorString = QLibrary::tr("Plugin '%1' uses incompatible CopperSpice library. (%2.%3.%4) [%5]")
         .formatArg(fileName).formatArg((cs_version & 0xff0000) >> 16).formatArg((cs_version & 0xff00) >> 8)
         .formatArg(cs_version & 0xff).formatArg(debug ? QString("debug") : QString("release") );

#ifndef QT_NO_DEBUG_PLUGIN_CHECK

   } else if (debug != QLIBRARY_AS_DEBUG) {
      // do not issue a qWarning since there may be no debug support
      errorString = QLibrary::tr("Plugin '%1' uses incompatible CopperSpice library."
            " (Can not mix debug and release libraries.)").formatArg(fileName);

#endif


   } else {
      pluginState = IsAPlugin;

   }

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
      lh = d->loadHints();
      d->release();
      d = 0;
      did_load = false;
   }

   d = QLibraryPrivate::findOrCreate(fileName, QString(), lh);
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
      lh = d->loadHints();
      d->release();
      d = 0;
      did_load = false;
   }
   d = QLibraryPrivate::findOrCreate(fileName, verNum >= 0 ? QString::number(verNum) : QString(), lh);

}

void QLibrary::setFileNameAndVersion(const QString &fileName, const QString &version)
{
   QLibrary::LoadHints lh;
   if (d) {
      lh = d->loadHints();
      d->release();
      d = 0;
      did_load = false;
   }

   d = QLibraryPrivate::findOrCreate(fileName, version, lh);
}

void *QLibrary::resolve(const QString &symbol)
{
   if (! isLoaded() && ! load()) {
      return 0;
   }

   return d->resolve(symbol);
}

void *QLibrary::resolve(const QString &fileName, const QString &symbol)
{
   QLibrary library(fileName);
   return library.resolve(symbol);
}

void *QLibrary::resolve(const QString &fileName, int verNum, const QString &symbol)
{
   QLibrary library(fileName, verNum);
   return library.resolve(symbol);
}

void *QLibrary::resolve(const QString &fileName, const QString &version, const QString &symbol)
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

   d->setLoadHints(hints);
}

QLibrary::LoadHints QLibrary::loadHints() const
{
   if (d) {
      return d->loadHints();

   } else {
      return QLibrary::LoadHints();
   }
}

// internal
bool qt_debug_component()
{
   static int debug_env = -1;

   if (debug_env == -1) {
      debug_env = qgetenv("QT_DEBUG_PLUGINS").toInt();
   }

   return debug_env != 0;
}

