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

#include <qlibrary.h>

#include <qstringlist.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmutex.h>
#include <qmap.h>
#include <qdebug.h>
#include <qvector.h>
#include <qdir.h>
#include <qplatformdefs.h>

#include <qlibrary_p.h>
#include <qcoreapplication_p.h>

#ifdef Q_OS_DARWIN
#  include <qcore_mac_p.h>
#endif

#ifndef NO_ERRNO_H
#include <errno.h>
#endif

static QMutex qt_library_mutex;
static QLibraryStore *qt_library_data = nullptr;
static bool qt_library_data_once;

using QPluginMetadataPtr = QMetaObject *(*)();

class QLibraryStore
{
 public:
   inline ~QLibraryStore();

   static inline QLibraryHandle *cs_findLibrary(const QString &fileName, const QString &version, QLibrary::LoadHints loadHints);
   static inline void releaseLibrary(QLibraryHandle *lib);

   static inline void cleanup();

 private:
   static inline QLibraryStore *instance();

   // all members and instance() are protected by qt_library_mutex
   QMap<QString, QLibraryHandle *> libraryMap;
};

QLibraryStore::~QLibraryStore()
{
   qt_library_data = nullptr;
}

inline void QLibraryStore::cleanup()
{
   QLibraryStore *data = qt_library_data;

   if (! data) {
      return;
   }

   for (auto lib : data->libraryMap) {

      if (lib->libraryRefCount.load() == 1) {

         if (lib->libraryUnloadCount.load() > 0) {
            Q_ASSERT(lib->pHnd);

            lib->libraryUnloadCount.store(1);

#ifdef __GLIBC__
            lib->unload(QLibraryHandle::NoUnloadSys);
#else
            lib->unload();
#endif
         }

         delete lib;
         lib = nullptr;
      }
   }

   delete data;
}

static void qlibraryCleanup()
{
   QLibraryStore::cleanup();
}

Q_DESTRUCTOR_FUNCTION(qlibraryCleanup)

QLibraryStore *QLibraryStore::instance()
{
   // this method must be called with a locked mutex

   if (! qt_library_data_once && ! qt_library_data) {
      // only create once per process lifetime
      qt_library_data = new QLibraryStore;
      qt_library_data_once = true;
   }

   return qt_library_data;
}

inline QLibraryHandle *QLibraryStore::cs_findLibrary(const QString &fileName, const QString &version,
      QLibrary::LoadHints loadHints)
{
   QMutexLocker locker(&qt_library_mutex);
   QLibraryStore *data = instance();

   // check if this library is already loaded
   QLibraryHandle *lib = nullptr;

   if (data) {
      lib = data->libraryMap.value(fileName);

      if (lib) {
         lib->mergeLoadHints(loadHints);
      }
   }

   if (! lib) {
      lib = new QLibraryHandle(fileName, version, loadHints);
   }

   // track this library
   if (data && ! fileName.isEmpty()) {
      data->libraryMap.insert(fileName, lib);
   }

   lib->libraryRefCount.ref();

   return lib;
}

inline void QLibraryStore::releaseLibrary(QLibraryHandle *lib)
{
   QMutexLocker locker(&qt_library_mutex);
   QLibraryStore *data = instance();

   if (lib->libraryRefCount.deref()) {
      // still in use
      return;
   }

   // no one else is using
   Q_ASSERT(lib->libraryUnloadCount.load() == 0);

   if (data && ! lib->fileName.isEmpty()) {
      QLibraryHandle *that = data->libraryMap.take(lib->fileName);
      Q_ASSERT(lib == that);
   }

   delete lib;
}

QLibraryHandle::QLibraryHandle(const QString &canonicalFileName, const QString &version, QLibrary::LoadHints loadHints)
   : pHnd(nullptr), fileName(canonicalFileName), fullVersion(version), pluginState(MightBeAPlugin),
     libraryRefCount(0), libraryUnloadCount(0)
{
   loadHintsInt.store(loadHints);

   if (canonicalFileName.isEmpty()) {
      errorString = QLibrary::tr("Shared library was not found.");
   }
}

QLibraryHandle *QLibraryHandle::findOrLoad(const QString &fileName, const QString &version, QLibrary::LoadHints loadHints)
{
   return QLibraryStore::cs_findLibrary(fileName, version, loadHints);
}

QLibraryHandle::~QLibraryHandle()
{
}

void QLibraryHandle::mergeLoadHints(QLibrary::LoadHints lh)
{
   if (pHnd) {
      return;
   }

   loadHintsInt.store(lh);
}

void *QLibraryHandle::resolve(const QString &symbol)
{
   if (! pHnd) {
      return nullptr;
   }

   return resolve_sys(symbol);
}

void QLibraryHandle::setLoadHints(QLibrary::LoadHints lh)
{
   // this locks a global mutex
   QMutexLocker lock(&qt_library_mutex);
   mergeLoadHints(lh);
}

bool QLibraryHandle::tryload()
{
   if (pHnd) {
      libraryUnloadCount.ref();
      return true;
   }

   if (fileName.isEmpty()) {
      return false;
   }

   bool retval = load_sys();

#if defined(CS_SHOW_DEBUG_CORE_PLUGIN)
   qDebug() << "QLibraryHandle::tryload() Loaded library "  << fileName;
#endif

   if (retval) {
      // when loading a library add a reference so the QLibraryHandle will not be deleted
      // this allows the abilitiy to unload the library at a later time

      libraryUnloadCount.ref();
      libraryRefCount.ref();
   }

   return retval;
}

bool QLibraryHandle::unload(UnloadFlag flag)
{
   if (! pHnd) {
      return false;
   }

   if (libraryUnloadCount.load() > 0 && ! libraryUnloadCount.deref()) {
      // only unload if ALL QLibrary instances wanted to
      delete pluginObj.data();

      if (flag == NoUnloadSys || unload_sys()) {
         // when the library is unloaded release the reference so 'this' can get deleted
         libraryRefCount.deref();
         pHnd = nullptr;
      }
   }

   return (pHnd == nullptr);
}

void QLibraryHandle::release()
{
   QLibraryStore::releaseLibrary(this);
}

bool QLibraryHandle::loadPlugin()
{
   if (m_metaObject != nullptr) {
      libraryUnloadCount.ref();
      return true;
   }

   if (pluginState == IsNotAPlugin) {
      return false;
   }

   if (tryload()) {
      return true;
   }

#if defined(CS_SHOW_DEBUG_CORE_PLUGIN)
   qDebug() << "QLibraryHandle::loadPlugin() File name = " << fileName << " error = " << errorString;
#endif

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

   bool valid = (lastSuffix == QString("dylib") || firstSuffix == QString("so") || firstSuffix == QString("bundle"));

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

static bool qt_get_metadata(QPluginMetadataPtr ptrFunc, QLibraryHandle *ptr)
{
   if (ptrFunc == nullptr) {
      return false;
   }

   QMetaObject *meta = ptrFunc();      // calls cs_internal_plugin_metaobject()
   ptr->m_metaObject = meta;

   if (meta == nullptr) {
      return false;
   }

   return true;
}

bool QLibraryHandle::isPlugin()
{
   if (pluginState == MightBeAPlugin) {
      updatePluginState();
   }

   return pluginState == IsAPlugin;
}

void QLibraryHandle::updatePluginState()
{
   errorString.clear();

   if (pluginState != MightBeAPlugin) {
      return;
   }

   bool success = false;

#if defined(Q_OS_UNIX) && ! defined(Q_OS_DARWIN)
   if (fileName.endsWith(".debug")) {
      // do not load a file which ends in .debug, these are the debug symbols from the libraries
      // they are valid shared library files and dlopen is known to crash while opening them

      // pretend we did not see the file
      errorString = QLibrary::tr("Found only a debug version of the library, unable to load plugin.");
      pluginState = IsNotAPlugin;

      return;
   }
#endif

   if (pHnd) {
      // library is already loaded (probably via QLibrary)
      success = true;

   } else {
      // load the plugin library
      success = tryload();
   }

   if (success) {
      QPluginMetadataPtr metaPtr = nullptr;

      // returns a function pointer so we can retrieve the meta data object
      metaPtr = (QPluginMetadataPtr) resolve("cs_internal_plugin_metaobject");

      if (metaPtr) {
         success = qt_get_metadata(metaPtr, this);

      } else {
         success = false;
         errorString = QLibrary::tr("updatePluginState(): Unable to retrieve the meta object for the %1 plugin").formatArg(fileName);

      }
   }

   if (! success) {
      if (errorString.isEmpty()) {

         if (fileName.isEmpty()) {
            errorString = QLibrary::tr("Plugin library was not found.");
         } else {
            errorString = QLibrary::tr("File '%1' is not a valid CopperSpice plugin.").formatArg(fileName);
         }
      }

      pluginState = IsNotAPlugin;

      return;
   }

   pluginState = IsNotAPlugin; // be pessimistic

   int index   = m_metaObject->indexOfClassInfo("plugin_version");
   int version = m_metaObject->classInfo(index).value().toInteger<int>();

   if ((version & 0x00ff00) > (CS_VERSION & 0x00ff00) || (version & 0xff0000) != (CS_VERSION & 0xff0000)) {
      errorString = QLibrary::tr("Plugin '%1' uses an incompatible CopperSpice library (%2.%3.%4)")
            .formatArg(fileName).formatArg((version & 0xff0000) >> 16).formatArg((version & 0xff00) >> 8)
            .formatArg(version & 0xff);

   } else {
      pluginState = IsAPlugin;

   }
}

bool QLibrary::load()
{
   if (m_handle == nullptr) {
      return false;
   }

   if (m_loaded) {
      // return true if previous load good, otherwise return false
      return m_handle->pHnd;
   }

   m_loaded = true;

   // calls method in QLibraryHandle
   bool retval = m_handle->tryload();

   return retval;
}

bool QLibrary::unload()
{
   if (m_loaded) {
      m_loaded = false;
      return m_handle->unload();
   }

   return false;
}

bool QLibrary::isLoaded() const
{
   return m_handle && m_handle->pHnd;
}

QLibrary::QLibrary(QObject *parent)
   : QObject(parent), m_handle(nullptr), m_loaded(false)
{
}

QLibrary::QLibrary(const QString &fileName, QObject *parent)
   : QObject(parent), m_handle(nullptr), m_loaded(false)
{
   setFileName(fileName);
}

QLibrary::QLibrary(const QString &fileName, int verNum, QObject *parent)
   : QObject(parent), m_handle(nullptr), m_loaded(false)
{
   setFileNameAndVersion(fileName, verNum);
}

QLibrary::QLibrary(const QString &fileName, const QString &version, QObject *parent)
   : QObject(parent), m_handle(nullptr), m_loaded(false)
{
   setFileNameAndVersion(fileName, version);
}

QLibrary::~QLibrary()
{
   if (m_handle) {
      m_handle->release();
   }
}

void QLibrary::setFileName(const QString &fileName)
{
   QLibrary::LoadHints lh;

   if (m_handle) {
      lh = m_handle->loadHints();
      m_handle->release();
      m_handle = nullptr;
      m_loaded = false;
   }

   m_handle = QLibraryHandle::findOrLoad(fileName, QString(), lh);
}

QString QLibrary::fileName() const
{
   if (m_handle) {
      return m_handle->qualifiedFileName.isEmpty() ? m_handle->fileName : m_handle->qualifiedFileName;
   }

   return QString();
}

void QLibrary::setFileNameAndVersion(const QString &fileName, int verNum)
{
   QLibrary::LoadHints lh;

   if (m_handle) {
      lh = m_handle->loadHints();
      m_handle->release();
      m_handle = nullptr;
      m_loaded = false;
   }

   m_handle = QLibraryHandle::findOrLoad(fileName, verNum >= 0 ? QString::number(verNum) : QString(), lh);
}

void QLibrary::setFileNameAndVersion(const QString &fileName, const QString &version)
{
   QLibrary::LoadHints lh;

   if (m_handle) {
      lh = m_handle->loadHints();
      m_handle->release();
      m_handle = nullptr;
      m_loaded = false;
   }

   m_handle = QLibraryHandle::findOrLoad(fileName, version, lh);
}

void *QLibrary::resolve(const QString &symbol)
{
   if (! isLoaded() && ! load()) {
      return nullptr;
   }

   return m_handle->resolve(symbol);
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
   return (! m_handle || m_handle->errorString.isEmpty()) ? tr("Unknown error") : m_handle->errorString;
}

void QLibrary::setLoadHints(LoadHints hints)
{
   if (m_handle == nullptr) {
      m_handle = QLibraryHandle::findOrLoad(QString());
      m_handle->errorString.clear();
   }

   m_handle->setLoadHints(hints);
}

QLibrary::LoadHints QLibrary::loadHints() const
{
   if (m_handle) {
      return m_handle->loadHints();

   } else {
      return QLibrary::LoadHints();
   }
}
