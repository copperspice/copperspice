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

#include <qfactoryloader_p.h>

#include <qdebug.h>
#include <qdir.h>
#include <qfactoryinterface.h>
#include <qmap.h>
#include <qmutex.h>
#include <qplugin.h>
#include <qpluginloader.h>

#include <qcoreapplication_p.h>

QList<QFactoryLoader *> *qt_factory_loaders()
{
   static QList<QFactoryLoader *> retval;
   return &retval;
}

QRecursiveMutex *qt_factoryloader_mutex()
{
   static QRecursiveMutex retval;
   return &retval;
}

class QFactoryLoaderPrivate
{
   Q_DECLARE_PUBLIC(QFactoryLoader)

 public:
   QFactoryLoaderPrivate() {}
   virtual ~QFactoryLoaderPrivate();

   Qt::CaseSensitivity cs;

   QString iid;
   QString suffix;
   QStringList loadedPaths;

   mutable QMutex mutex;

   QList<QLibraryHandle *> libraryList;
   QMap<QString, QLibraryHandle *> libraryMap;

 protected:
   QFactoryLoader *q_ptr;
};

QFactoryLoaderPrivate::~QFactoryLoaderPrivate()
{
   for (auto item : libraryList) {
      QLibraryHandle *library = item;

      library->unload();
      library->release();
   }
}

QFactoryLoader::QFactoryLoader(const QString &iid, const QString &pluginDir, Qt::CaseSensitivity cs)
   : QObject(nullptr), d_ptr(new QFactoryLoaderPrivate)
{
   d_ptr->q_ptr = this;

   moveToThread(QCoreApplicationPrivate::mainThread());
   Q_D(QFactoryLoader);

   d->iid    = iid;
   d->cs     = cs;
   d->suffix = pluginDir;

   QRecursiveMutexLocker locker(qt_factoryloader_mutex());
   setup();

   qt_factory_loaders()->append(this);
}

void QFactoryLoader::setup()
{
#ifdef QT_SHARED
   Q_D(QFactoryLoader);

   QStringList libDirs = QCoreApplication::libraryPaths();
   // mp_pluginsFound.clear();

   for (const QString &pluginDir : libDirs)  {

      // already looked in this path
      if (d->loadedPaths.contains(pluginDir)) {
         continue;
      }

      d->loadedPaths.append(pluginDir);

      QString path = pluginDir + d->suffix;
      // mp_pluginsFound.append( PluginStatus{path} );

      if (! QDir(path).exists(".")) {
         continue;
      }

      QStringList plugins = QDir(path).entryList(QDir::Files);
      QLibraryHandle *library = nullptr;

      for (int j = 0; j < plugins.count(); ++j) {
         QString fname = QDir::cleanPath(path + '/' + plugins.at(j));

/*       if (j > 0) {
            mp_pluginsFound.append( PluginStatus{path} );
         }

         mp_pluginsFound.last().fileName = plugins.at(j);
*/

         library = QLibraryHandle::findOrLoad(QFileInfo(fname).canonicalFilePath());

         if (! library->isPlugin()) {
            // show the full error message
            qWarning("QFactoryLoader::setup() %s", csPrintable(library->errorString));

            library->release();
            continue;
         }

         if (library->m_metaObject == nullptr) {
            qWarning("QFactoryLoader::setup() Invalid meta object (nullptr)");
            library->release();
            continue;
         }

         QString iid;
         QString keyString;
         QString version;

         int index = library->m_metaObject->indexOfClassInfo("plugin_iid");

         if (index == -1) {
            library->release();
            continue;
         }

         iid = library->m_metaObject->classInfo(index).value();

         if (iid != d->iid) {
            // no match
            library->release();
            continue;
         }

         index = library->m_metaObject->indexOfClassInfo("plugin_key");

         if (index != -1) {
            keyString = library->m_metaObject->classInfo(index).value();
         }

         index = library->m_metaObject->indexOfClassInfo("plugin_version");

         if (index == -1) {
            library->release();
            continue;
         }

         version = library->m_metaObject->classInfo(index).value();

         //
         QStringList keyList;

         if (iid == d->iid && ! keyString.isEmpty()) {
            // found an iid for the plugin and there are keys

            // there may be multiple keys
            QStringList tmpKeyList = keyString.split(",");

            for (const QString &item : tmpKeyList) {
               const QString key = item.trimmed();

               if (d->cs) {
                  keyList.append(key);

               } else {
                  keyList.append(key.toLower());

               }
            }

/*          if (keyList.count() > 0) {
               mp_pluginsFound.append( PluginStatus{path} );
            }

            mp_pluginsFound.last().keyFound = keyList[0];
*/
         }

         int keyUsageCount = 0;

         for (const auto &item : keyList) {
            QLibraryHandle *lib_other = d->libraryMap.value(item);

            int other_version = 0;

            if (lib_other) {
               int index     = lib_other->m_metaObject->indexOfClassInfo("plugin_version");
               other_version = lib_other->m_metaObject->classInfo(index).value().toInteger<int>();
            }

            int lib_version = version.toInteger<int>();

            if (lib_other == nullptr || (other_version > CS_VERSION && lib_version <= CS_VERSION)) {
               d->libraryMap[item] = library;
               ++keyUsageCount;
            }
         }

         if (keyUsageCount  > 0 || keyList.isEmpty()) {
            // once loaded, do not unload

            library->setLoadHints(QLibrary::PreventUnloadHint);
            d->libraryList.append(library);

         } else {
            library->release();
         }
      }
   }

#else
   Q_D(QFactoryLoader);

#if defined(CS_SHOW_DEBUG_CORE_PLUGIN)
   qDebug() << "QFactoryLoader::QFactoryLoader() ignoring" << d->iid << "since plugins are disabled in static builds";
#endif

#endif
}

QFactoryLoader::~QFactoryLoader()
{
   QRecursiveMutexLocker locker(qt_factoryloader_mutex());
   qt_factory_loaders()->removeAll(this);
}

QObject *QFactoryLoader::instance(QString key) const
{
   if (key.isEmpty()) {
      return nullptr;
   }

   QLibraryHandle *library = m_pluginMap.value(key);

   if (library != nullptr) {

      if (! library->pluginObj) {
         // invoke default constructor
         library->pluginObj = library->m_metaObject->newInstance();
      }

      QObject *obj = library->pluginObj.data();

      if (obj) {
         if (! obj->parent()) {
            obj->moveToThread(QCoreApplicationPrivate::mainThread());
         }

         return obj;
      }

      return nullptr;
   }

/*
   emerald - static plugins

   QVector<QMetaObject *> staticPluginMetaObjects = QPluginLoader::staticPlugins();

   for (auto item : staticPluginMetaObjects) {

      if (object.value("IID") == d->iid) {
         return item.instance();
      }
   }
*/

   return nullptr;
}

QObject *QFactoryLoader::instance(QLibraryHandle *library) const
{
// Q_D(const QFactoryLoader);

   if (library == nullptr) {
      return nullptr;
   }

   if (! library->pluginObj) {
      // invoke default constructor
      library->pluginObj = library->m_metaObject->newInstance();
   }

   QObject *obj = library->pluginObj.data();

   if (obj) {
      if (! obj->parent()) {
         obj->moveToThread(QCoreApplicationPrivate::mainThread());
      }

      return obj;
   }

   return nullptr;

/*
   emerald - static plugins

   QVector<QMetaObject *> staticPluginMetaObjects = QPluginLoader::staticPlugins();

   for (auto item : staticPluginMetaObjects) {

      if (object.value("IID") == d->iid) {
         return item.instance();
      }
   }
*/

   return nullptr;
}

#if defined(Q_OS_UNIX) && ! defined (Q_OS_DARWIN)
QLibraryHandle *QFactoryLoader::library(const QString &key) const
{
   Q_D(const QFactoryLoader);
   return d->libraryMap.value(d->cs ? key : key.toLower());
}
#endif

void QFactoryLoader::refreshAll()
{
   QRecursiveMutexLocker locker(qt_factoryloader_mutex());
   QList<QFactoryLoader *> *loaders = qt_factory_loaders();

   for (auto item : *loaders) {
      item->setup();
   }
}

QSet<QString> QFactoryLoader::keySet() const
{
   Q_D(const QFactoryLoader);

   m_pluginMap.clear();
   QSet<QString> retval;

   for (auto lib_handle : d->libraryList) {
      int index = lib_handle->m_metaObject->indexOfClassInfo("plugin_key");

      if (index != -1) {
         const QString keyString = lib_handle->m_metaObject->classInfo(index).value();

         // there may be multiple keys
         QStringList keyList = keyString.split(",");

         for (const QString &item : keyList) {
            const QString key = item.trimmed();

            m_pluginMap.insert(key, lib_handle);
            retval.insert(key);
         }
      }
   }

   return retval;
}

QSet<QLibraryHandle *> QFactoryLoader::librarySet(QString key) const
{
   if (m_pluginMap.isEmpty()) {
      keySet();
   }

   QSet<QLibraryHandle *> retval = m_pluginMap.values(key).toSet();
   return retval;
}
