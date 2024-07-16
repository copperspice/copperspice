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

#include <qplatformdefs.h>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qplugin.h>
#include <qpluginloader.h>
#include <qstringview.h>

#include <qlibrary_p.h>

QPluginLoader::QPluginLoader(QObject *parent)
   : QObject(parent), mp_handle(nullptr), mp_loaded(false)
{
}

QPluginLoader::QPluginLoader(const QString &fileName, QObject *parent)
   : QObject(parent), mp_handle(nullptr), mp_loaded(false)
{
   setFileName(fileName);
   setLoadHints(QLibrary::PreventUnloadHint);
}

QPluginLoader::~QPluginLoader()
{
   if (mp_handle) {
      mp_handle->release();
   }
}

QObject *QPluginLoader::instance()
{
   if (! isLoaded() && ! load()) {
      return nullptr;
   }

   if (! mp_handle->pluginObj) {
      mp_handle->pluginObj = mp_handle->m_metaObject->newInstance();
   }

   return mp_handle->pluginObj.data();
}

bool QPluginLoader::load()
{
   if (! mp_handle || mp_handle->fileName.isEmpty()) {
      return false;
   }

   if (mp_loaded) {
      return mp_handle->pHnd && mp_handle->m_metaObject;
   }

   if (! mp_handle->isPlugin()) {
      return false;
   }

   mp_loaded = true;

   return mp_handle->loadPlugin();
}

bool QPluginLoader::unload()
{
   if (mp_loaded) {
      mp_loaded = false;
      return mp_handle->unload();
   }

   if (mp_handle) {
      mp_handle->errorString = tr("Plugin was not loaded.");
   }

   return false;
}

bool QPluginLoader::isLoaded() const
{
   return mp_handle && mp_handle->pHnd && mp_handle->m_metaObject;
}

#if ! defined(QT_STATIC)

static QString locatePlugin(const QString &fileName)
{
   const bool isAbsolute = QDir::isAbsolutePath(fileName);

   if (isAbsolute) {
      QFileInfo fi(fileName);

      if (fi.isFile()) {
         return fi.canonicalFilePath();
      }
   }

   QStringList prefixes = QLibraryHandle::prefixes_sys();
   prefixes.prepend(QString());

   QStringList suffixes = QLibraryHandle::suffixes_sys(QString());
   suffixes.prepend(QString());

   // Split up "subdir/filename"
   const int slash = fileName.lastIndexOf('/');

   const QStringView baseName = fileName.midView(slash + 1);
   const QStringView basePath  = isAbsolute ? QStringView() : fileName.leftView(slash + 1);    // keep the '/'

   QStringList paths;

   if (isAbsolute) {
      paths.append(fileName.left(slash));         // do not include the '/'
   } else {
      paths = QCoreApplication::libraryPaths();
      paths.prepend(".");                         // search in current dir first
   }

   for (const QString &path : paths) {
      for (const QString &prefix : prefixes) {
         for (const QString &suffix : suffixes) {
            const QString fn = path + '/' + basePath + prefix + baseName + suffix;

            if (QFileInfo(fn).isFile()) {
               return fn;
            }
         }
      }
   }

#if defined(CS_SHOW_DEBUG_CORE_PLUGIN)
      qDebug() << "locatePlugin() filename = " << fileName << " was not found";
#endif

   return QString();
}
#endif

void QPluginLoader::setFileName(const QString &fileName)
{
#if ! defined(QT_STATIC)
   QLibrary::LoadHints lh = QLibrary::PreventUnloadHint;

   if (mp_handle) {
      lh = mp_handle->loadHints();
      mp_handle->release();
      mp_handle = nullptr;
      mp_loaded = false;
   }

   const QString fn = locatePlugin(fileName);

   mp_handle = QLibraryHandle::findOrLoad(fn, QString(), lh);

   if (! fn.isEmpty()) {
      mp_handle->updatePluginState();
   }

#else

#if defined(CS_SHOW_DEBUG_CORE_PLUGIN)
   qDebug("QPluginLoader::setFileName() Unable to load %s into a statically linked CopperSpice library",
         QFile::encodeName(fileName).constData() );
#endif

#endif
}

QString QPluginLoader::fileName() const
{
   if (mp_handle) {
      return mp_handle->fileName;
   }

   return QString();
}

QString QPluginLoader::errorString() const
{
   return (! mp_handle || mp_handle->errorString.isEmpty()) ? tr("Unknown error") : mp_handle->errorString;
}

using StaticPluginList = QVector<QMetaObject *>;

static StaticPluginList *staticPluginList()
{
   static StaticPluginList retval;
   return &retval;
}

void QPluginLoader::setLoadHints(QLibrary::LoadHints loadHints)
{
   if (! mp_handle) {
      mp_handle = QLibraryHandle::findOrLoad(QString());
      mp_handle->errorString.clear();
   }

   mp_handle->setLoadHints(loadHints);
}

QLibrary::LoadHints QPluginLoader::loadHints() const
{
   return mp_handle ? mp_handle->loadHints() : QLibrary::LoadHints();
}

/* emerald - static plugins
void Q_CORE_EXPORT qRegisterStaticPluginFunction(QStaticPlugin plugin)
{
   staticPluginList()->append(plugin);
}


*/
QObjectList QPluginLoader::staticInstances()
{
   QObjectList instances;

/* emerald
   const StaticPluginList *plugins = staticPluginList();

   if (plugins) {
      const int numPlugins = plugins->size();

      for (int i = 0; i < numPlugins; ++i) {
         instances += plugins->at(i).instance();
      }
   }
*/

   return instances;
}

QVector<QMetaObject *> QPluginLoader::staticPlugins()
{
   StaticPluginList *plugins = staticPluginList();

   if (plugins) {
      return *plugins;
   }

   return QVector<QMetaObject *>();
}
