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
#include <qsettings.h>
#include <qdebug.h>
#include <qmutex.h>
#include <qplugin.h>
#include <qpluginloader.h>
#include <qlibraryinfo.h>
#include <qcoreapplication_p.h>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QList<QFactoryLoader *>, qt_factory_loaders)
Q_GLOBAL_STATIC_WITH_ARGS(QMutex, qt_factoryloader_mutex, (QMutex::Recursive))

class QFactoryLoaderPrivate
{
   Q_DECLARE_PUBLIC(QFactoryLoader)

 public:
   QFactoryLoaderPrivate() {}
   virtual ~QFactoryLoaderPrivate();

   mutable QMutex mutex;

   QString iid;
   QList<QLibraryPrivate *> libraryList;

   QStringList keyList;
   QString suffix;

   QStringList loadedPaths;

   Qt::CaseSensitivity cs;
   QMap<QString, QLibraryPrivate *> keyMap;

   void unloadPath(const QString &path);

 protected:
   QFactoryLoader *q_ptr;

};

QFactoryLoaderPrivate::~QFactoryLoaderPrivate()
{
   for (int i = 0; i < libraryList.count(); ++i) {
      libraryList.at(i)->release();
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

void QFactoryLoader::updateDir(const QString &pluginDir, QSettings &settings)
{
   Q_D(QFactoryLoader);

   QString path = pluginDir + d->suffix;
   if (! QDir(path).exists(QLatin1String("."))) {
      return;
   }

   QStringList plugins = QDir(path).entryList(QDir::Files);
   QLibraryPrivate *library = 0;

   for (int j = 0; j < plugins.count(); ++j) {
      QString fileName = QDir::cleanPath(path + QLatin1Char('/') + plugins.at(j));

      if (qt_debug_component()) {
         qDebug() << "QFactoryLoader::QFactoryLoader() Reviewing file:" << fileName;
      }

      library = QLibraryPrivate::findOrCreate(QFileInfo(fileName).canonicalFilePath());

      if (! library->isPlugin(&settings)) {

         if (qt_debug_component()) {
            qDebug() << library->errorString;
            qDebug() << "         not a plugin";
         }

         library->release();
         continue;
      }

      QString regkey = QString::fromLatin1("CopperSpice Factory Cache %1.%2/%3:/%4")
                       .arg((CS_VERSION & 0xff0000) >> 16)
                       .arg((CS_VERSION & 0xff00) >> 8)
                       .arg(d->iid)
                       .arg(fileName);

      QStringList reg, keys;
      reg = settings.value(regkey).toStringList();

      if (reg.count() && library->lastModified == reg[0]) {

         keys = reg;
         keys.removeFirst();

      } else {

         if (! library->loadPlugin()) {
            if (qt_debug_component()) {
               qDebug() << library->errorString;
               qDebug() << "           could not load";
            }

            library->release();
            continue;
         }

         QObject *instance = library->instance();

         if (! instance) {
            library->release();

            // ignore plugins that have a valid signature but can not be loaded
            continue;
         }

         QFactoryInterface *factory = qobject_cast<QFactoryInterface *>(instance);

         if (instance && factory)  {
            if (instance->cs_InstanceOf(d->iid))  {
               keys = factory->keys();
            }
         }

         if (keys.isEmpty())  {
            library->unload();
         }

         reg.clear();
         reg << library->lastModified;
         reg += keys;

         settings.setValue(regkey, reg);
      }

      if (qt_debug_component()) {
         qDebug() << "Keys: " << keys;
      }

      if (keys.isEmpty()) {
         library->release();
         continue;
      }

      int keysUsed = 0;
      for (int k = 0; k < keys.count(); ++k) {
         QString key = keys.at(k);

         if (!d->cs) {
            key = key.toLower();
         }

         QLibraryPrivate *previous = d->keyMap.value(key);
         if (!previous || (previous->cs_version > CS_VERSION && library->cs_version <= CS_VERSION)) {
            d->keyMap[key] = library;
            d->keyList += keys.at(k);
            keysUsed++;
         }
      }

      if (keysUsed) {
         d->libraryList += library;
      } else {
         library->release();
      }
   }
}

void QFactoryLoader::update()
{
#if ! defined(QT_STATIC)
   Q_D(QFactoryLoader);

   QStringList paths = QCoreApplication::libraryPaths();
   QSettings settings(QSettings::UserScope, QLatin1String("CopperSpice"));

   for (int i = 0; i < paths.count(); ++i) {
      const QString &pluginDir = paths.at(i);

      // already loaded, skip it...
      if (d->loadedPaths.contains(pluginDir)) {
         continue;
      }

      d->loadedPaths << pluginDir;
      updateDir(pluginDir, settings);
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

QStringList QFactoryLoader::keys() const
{
   Q_D(const QFactoryLoader);

   QMutexLocker locker(&d->mutex);

   QStringList keys = d->keyList;
   QObjectList instances = QPluginLoader::staticInstances();

   for (int i = 0; i < instances.count(); ++i) {
      if (QFactoryInterface *factory = qobject_cast<QFactoryInterface *>(instances.at(i))) {

         if (instances.at(i)->cs_InstanceOf(d->iid))  {
            keys += factory->keys();
         }

      }
   }

   return keys;
}

QObject *QFactoryLoader::instance(const QString &key) const
{
   Q_D(const QFactoryLoader);

   QMutexLocker locker(&d->mutex);
   QObjectList instances = QPluginLoader::staticInstances();

   for (int i = 0; i < instances.count(); ++i)  {

      QFactoryInterface *factory = qobject_cast<QFactoryInterface *>(instances.at(i));

      if (factory) {
         // this object is a static plugin

         if ( instances.at(i)->cs_InstanceOf(d->iid) )  {
            // does this factory match the iid for this QFactorLoader instance

            if (factory->keys().contains(key, Qt::CaseInsensitive))  {
               return instances.at(i);
            }
         }

      }

   }

   QString lowercaseKey = key;

   if (! d->cs) {
      lowercaseKey = key.toLower();
   }

   if (QLibraryPrivate *library = d->keyMap.value(lowercaseKey)) {

      if (library->instance || library->loadPlugin()) {

         if (QObject *obj = library->instance()) {

            if (obj && ! obj->parent())  {
               obj->moveToThread(QCoreApplicationPrivate::mainThread());
            }

            return obj;
         }
      }
   }

   return 0;
}

#ifdef Q_WS_X11
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

QT_END_NAMESPACE
