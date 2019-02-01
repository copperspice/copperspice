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

#include <qfactoryloader_p.h>
#include <qfactoryinterface.h>
#include <qmap.h>
#include <qdir.h>
#include <qdebug.h>
#include <qmutex.h>
#include <qplugin.h>
#include <qpluginloader.h>

#include <qcoreapplication_p.h>

Q_GLOBAL_STATIC(QList<QFactoryLoader *>, qt_factory_loaders)
Q_GLOBAL_STATIC_WITH_ARGS(QMutex, qt_factoryloader_mutex, (QMutex::Recursive))

class QFactoryLoaderPrivate
{
   Q_DECLARE_PUBLIC(QFactoryLoader)

 public:
   QFactoryLoaderPrivate() {}
   virtual ~QFactoryLoaderPrivate();

   QString iid;
   Qt::CaseSensitivity cs;
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

QFactoryLoader::QFactoryLoader(const QString &iid, const QString &suffix, Qt::CaseSensitivity cs)
   : QObject(nullptr), d_ptr(new QFactoryLoaderPrivate)
{
   d_ptr->q_ptr = this;

   moveToThread(QCoreApplicationPrivate::mainThread());
   Q_D(QFactoryLoader);

   d->iid    = iid;
   d->cs     = cs;
   d->suffix = suffix;

   QMutexLocker locker(qt_factoryloader_mutex());
   update();
   qt_factory_loaders()->append(this);
}

void QFactoryLoader::update()
{
#ifdef QT_SHARED
   Q_D(QFactoryLoader);
   QStringList paths = QCoreApplication::libraryPaths();

   for (int i = 0; i < paths.count(); ++i) {
      const QString &pluginDir = paths.at(i);

      // already loaded
      if (d->loadedPaths.contains(pluginDir)) {
         continue;
      }

      d->loadedPaths << pluginDir;

      QString path = pluginDir + d->suffix;

      if (! QDir(path).exists(".")) {
         continue;
      }

      QStringList plugins = QDir(path).entryList(QDir::Files);
      QLibraryHandle *library = nullptr;

      for (int j = 0; j < plugins.count(); ++j) {
         QString fileName = QDir::cleanPath(path + '/' + plugins.at(j));

         library = QLibraryHandle::findOrLoad(QFileInfo(fileName).canonicalFilePath());

         if (! library->isPlugin()) {
            library->release();
            continue;
         }

         if (library->m_metaObject == nullptr) {
            library->release();
            continue;
         }

         QString iid;
         QString key;
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
            key = library->m_metaObject->classInfo(index).value();
         }

         index = library->m_metaObject->indexOfClassInfo("plugin_version");
         if (index == -1) {
            library->release();
            continue;
         }
         version = library->m_metaObject->classInfo(index).value();

         //
         QStringList keyList;

         if (iid == d->iid && ! key.isEmpty()) {
            // we found an iid for the plugin and there are keys
            keyList += d->cs ? key : key.toLower();
         }

         //
         int keyUsageCount = 0;

         for (const auto &item : keyList) {;
            QLibraryHandle *lib_other = d->libraryMap.value(item);

            int other_version = 0;

            if (lib_other) {
               int index     = lib_other->m_metaObject->indexOfClassInfo("plugin_version");
               other_version = lib_other->m_metaObject->classInfo(index).value().toInteger<int>();
            }

            int lib_version = version.toInteger<int>();

            if (lib_other != nullptr || (other_version > CS_VERSION && lib_version <= CS_VERSION)) {
               d->libraryMap[item] = library;
               ++keyUsageCount;
            }
         }

         if (keyUsageCount  > 0|| keyList.isEmpty()) {
            // once loaded, do not unload
            library->setLoadHints(QLibrary::PreventUnloadHint);
            d->libraryList += library;

         } else {
            library->release();
         }
      }
   }

#else
   Q_D(QFactoryLoader);

   if (qt_debug_component()) {
      qDebug() << "QFactoryLoader::QFactoryLoader() ignoring" << d->iid << "since plugins are disabled in static builds";
   }
#endif
}

QFactoryLoader::~QFactoryLoader()
{
   QMutexLocker locker(qt_factoryloader_mutex());
   qt_factory_loaders()->removeAll(this);
}

QObject *QFactoryLoader::instance(QString key) const
{
   Q_D(const QFactoryLoader);

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

QObject *QFactoryLoader::instance(QLibraryHandle * library) const
{
   Q_D(const QFactoryLoader);

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


#if defined(Q_OS_UNIX) && ! defined (Q_OS_MAC)
QLibraryHandle *QFactoryLoader::library(const QString &key) const
{
   Q_D(const QFactoryLoader);
   return d->libraryMap.value(d->cs ? key : key.toLower());
}
#endif

void QFactoryLoader::refreshAll()
{
   QMutexLocker locker(qt_factoryloader_mutex());
   QList<QFactoryLoader *> *loaders = qt_factory_loaders();

   for (auto item : *loaders) {
      item->update();
   }
}

QSet<QString> QFactoryLoader::keySet() const
{
   Q_D(const QFactoryLoader);

   m_pluginMap.clear();
   QSet<QString> retval;

   for (auto lib_handle : d->libraryList) {
      // only works for one key

      int index = lib_handle->m_metaObject->indexOfClassInfo("plugin_key");

      if (index != -1) {
         const QString &key = lib_handle->m_metaObject->classInfo(index).value();

         m_pluginMap.insert(key, lib_handle);
         retval.insert(key);
      }
   }

   return retval;
}

QSet<QLibraryHandle *> QFactoryLoader::librarySet(QString key) const
{
   QSet<QLibraryHandle *> retval = m_pluginMap.values(key).toSet();
   return retval;
}

