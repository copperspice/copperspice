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
#include <qjsondocument.h>
#include <qjsonvalue.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qmutex.h>
#include <qplugin.h>
#include <qpluginloader.h>

#include <qcoreapplication_p.h>

Q_GLOBAL_STATIC(QList<QFactoryLoader *>, qt_factory_loaders)
Q_GLOBAL_STATIC_WITH_ARGS(QMutex, qt_factoryloader_mutex, (QMutex::Recursive))

namespace {

// avoid duplicate QStringLiteral data:
inline QString iidKeyLiteral()
{
   return QString("IID");
}

#ifdef QT_SHARED
inline QString versionKeyLiteral()
{
   return QString("version");
}
#endif

inline QString metaDataKeyLiteral()
{
   return QString("MetaData");
}
inline QString keysKeyLiteral()
{
   return QString("Keys");
}

}
class QFactoryLoaderPrivate
{
   Q_DECLARE_PUBLIC(QFactoryLoader)

 public:
   QFactoryLoaderPrivate() {}
   virtual ~QFactoryLoaderPrivate();

   mutable QMutex mutex;

   QString iid;
   QList<QLibraryPrivate *> libraryList;
   QMap<QString, QLibraryPrivate *> keyMap;

   QString suffix;
   QStringList loadedPaths;

   Qt::CaseSensitivity cs;

 protected:
   QFactoryLoader *q_ptr;
};

QFactoryLoaderPrivate::~QFactoryLoaderPrivate()
{
   for (int i = 0; i < libraryList.count(); ++i) {
      QLibraryPrivate *library = libraryList.at(i);

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

      // Already loaded, skip it...
      if (d->loadedPaths.contains(pluginDir)) {
         continue;
      }

      d->loadedPaths << pluginDir;

      QString path = pluginDir + d->suffix;

      if (! QDir(path).exists(".")) {
         continue;
      }

      QStringList plugins = QDir(path).entryList(QDir::Files);
      QLibraryPrivate *library = nullptr;

#ifdef Q_OS_MAC
      // Loading both the debug and release version of the cocoa plugins causes the objective-c runtime
      // to print "duplicate class definitions" warnings. Detect if QFactoryLoader is about to load both,
      // skip one of them (below). Find a better solution

      const bool isLoadingDebugAndReleaseCocoa = plugins.contains("libqcocoa_debug.dylib") && plugins.contains("libqcocoa.dylib");
#endif

      for (int j = 0; j < plugins.count(); ++j) {
         QString fileName = QDir::cleanPath(path + '/' + plugins.at(j));

#ifdef Q_OS_MAC
         if (isLoadingDebugAndReleaseCocoa) {
            if (fileName.contains("libqcocoa_debug.dylib")) {
               continue;    // Skip debug plugin in release mode
            }
         }
#endif

         library = QLibraryPrivate::findOrCreate(QFileInfo(fileName).canonicalFilePath());

         if (! library->isPlugin()) {

            library->release();
            continue;
         }

         QStringList keys;
         bool metaDataOk = false;

         QString iid = library->metaData.value(iidKeyLiteral()).toString();

         if (iid == d->iid) {

            QJsonObject object = library->metaData.value(metaDataKeyLiteral()).toObject();
            metaDataOk = true;

            QJsonArray k = object.value(keysKeyLiteral()).toArray();
            for (int i = 0; i < k.size(); ++i) {
               keys += d->cs ? k.at(i).toString() : k.at(i).toString().toLower();
            }
         }

         if (! metaDataOk) {
            library->release();
            continue;
         }

         int keyUsageCount = 0;
         for (int k = 0; k < keys.count(); ++k) {
            const QString &key = keys.at(k);
            QLibraryPrivate *previous = d->keyMap.value(key);

            int prev_cs_version = 0;

            if (previous) {
               prev_cs_version = (int)previous->metaData.value(versionKeyLiteral()).toDouble();

            }

            int cs_version = (int)library->metaData.value(versionKeyLiteral()).toDouble();





            if (! previous || (prev_cs_version > CS_VERSION && cs_version <= CS_VERSION)) {
               d->keyMap[key] = library;
               ++keyUsageCount;
            }
         }

         if (keyUsageCount || keys.isEmpty()) {
            library->setLoadHints(QLibrary::PreventUnloadHint); // once loaded, don't unload
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

QList<QJsonObject> QFactoryLoader::metaData() const
{
   Q_D(const QFactoryLoader);
   QMutexLocker locker(&d->mutex);
   QList<QJsonObject> metaData;

   for (int i = 0; i < d->libraryList.size(); ++i) {
      metaData.append(d->libraryList.at(i)->metaData);
   }

   for (const QStaticPlugin &plugin : QPluginLoader::staticPlugins()) {
      const QJsonObject object = plugin.metaData();

      if (object.value(iidKeyLiteral()) != d->iid) {
         continue;
      }

      metaData.append(object);
   }

   return metaData;
}

QObject *QFactoryLoader::instance(int index) const
{
   Q_D(const QFactoryLoader);
   if (index < 0) {
      return 0;
   }
   if (index < d->libraryList.size()) {
      QLibraryPrivate *library = d->libraryList.at(index);
      if (library->instance || library->loadPlugin()) {
         if (!library->inst) {
            library->inst = library->instance();
         }
         QObject *obj = library->inst.data();
         if (obj) {
            if (!obj->parent()) {
               obj->moveToThread(QCoreApplicationPrivate::mainThread());
            }
            return obj;
         }
      }
      return 0;
   }
   index -= d->libraryList.size();
   QVector<QStaticPlugin> staticPlugins = QPluginLoader::staticPlugins();
   for (int i = 0; i < staticPlugins.count(); ++i) {
      const QJsonObject object = staticPlugins.at(i).metaData();

      if (object.value(iidKeyLiteral()) != d->iid) {
         continue;
      }
      if (index == 0) {
         return staticPlugins.at(i).instance();
      }
      --index;
   }
   return 0;
}

#if defined(Q_OS_UNIX) && !defined (Q_OS_MAC)
QLibraryPrivate *QFactoryLoader::library(const QString &key) const
{
   Q_D(const QFactoryLoader);
   return d->keyMap.value(d->cs ? key : key.toLower());
}
#endif

void QFactoryLoader::refreshAll()
{
   QMutexLocker locker(qt_factoryloader_mutex());
   QList<QFactoryLoader *> *loaders = qt_factory_loaders();

   for (QList<QFactoryLoader *>::const_iterator it = loaders->constBegin(); it != loaders->constEnd(); ++it) {
      (*it)->update();
   }
}

QMultiMap<int, QString> QFactoryLoader::keyMap() const
{
   QMultiMap<int, QString> result;
   const QString metaDataKey = metaDataKeyLiteral();
   const QString keysKey = keysKeyLiteral();
   const QList<QJsonObject> metaDataList = metaData();

   for (int i = 0; i < metaDataList.size(); ++i) {
      const QJsonObject metaData = metaDataList.at(i).value(metaDataKey).toObject();
      const QJsonArray keys = metaData.value(keysKey).toArray();
      const int keyCount = keys.size();

      for (int k = 0; k < keyCount; ++k) {
         result.insert(i, keys.at(k).toString());
      }
   }
   return result;
}
int QFactoryLoader::indexOf(const QString &needle) const
{
   const QString metaDataKey = metaDataKeyLiteral();
   const QString keysKey = keysKeyLiteral();
   const QList<QJsonObject> metaDataList = metaData();
   for (int i = 0; i < metaDataList.size(); ++i) {
      const QJsonObject metaData = metaDataList.at(i).value(metaDataKey).toObject();
      const QJsonArray keys = metaData.value(keysKey).toArray();
      const int keyCount = keys.size();
      for (int k = 0; k < keyCount; ++k) {
         if (!keys.at(k).toString().compare(needle, Qt::CaseInsensitive)) {
            return i;
         }
      }
   }
   return -1;
}
