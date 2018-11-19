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

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qplugin.h>
#include <qpluginloader.h>
#include <qstringview.h>

#include <qlibrary_p.h>

QPluginLoader::QPluginLoader(QObject *parent)
   : QObject(parent), d(0), did_load(false)
{
}

QPluginLoader::QPluginLoader(const QString &fileName, QObject *parent)
   : QObject(parent), d(0), did_load(false)
{
   setFileName(fileName);
   setLoadHints(QLibrary::PreventUnloadHint);
}

/*!
    Destroys the QPluginLoader object.

    Unless unload() was called explicitly, the plugin stays in memory
    until the application terminates.

    \sa isLoaded(), unload()
*/
QPluginLoader::~QPluginLoader()
{
   if (d) {
      d->release();
   }
}

QObject *QPluginLoader::instance()
{
   if (!isLoaded() && !load()) {
      return 0;
   }

   if (!d->inst && d->instance) {
      d->inst = d->instance();
   }
   return d->inst.data();

}
QJsonObject QPluginLoader::metaData() const
{
   if (!d) {
      return QJsonObject();
   }
   return d->metaData;
}


bool QPluginLoader::load()
{
   if (! d || d->fileName.isEmpty()) {
      return false;
   }

   if (did_load) {
      return d->pHnd && d->instance;
   }

   if (! d->isPlugin()) {
      return false;
   }


   did_load = true;
   return d->loadPlugin();
}

bool QPluginLoader::unload()
{
   if (did_load) {
      did_load = false;
      return d->unload();
   }
   if (d) { // Ouch
      d->errorString = tr("The plugin was not loaded.");
   }
   return false;
}

bool QPluginLoader::isLoaded() const
{
   return d && d->pHnd && d->instance;
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

   QStringList prefixes = QLibraryPrivate::prefixes_sys();
   prefixes.prepend(QString());

   QStringList suffixes = QLibraryPrivate::suffixes_sys(QString());
   suffixes.prepend(QString());

   // Split up "subdir/filename"
   const int slash = fileName.lastIndexOf('/');

   const QStringView baseName = fileName.midView(slash + 1);
   const QStringView basePath  = isAbsolute ? QStringView() : fileName.leftView(slash + 1);    // keep the '/'

   const bool debug = qt_debug_component();

   QStringList paths;
   if (isAbsolute) {
      paths.append(fileName.left(slash));         // don't include the '/'
   } else {
      paths = QCoreApplication::libraryPaths();
      paths.prepend(".");                         // search in current dir first
   }

   for (const QString &path : paths) {
      for (const QString &prefix : prefixes) {
         for (const QString &suffix : suffixes) {
            const QString fn = path + '/' + basePath + prefix + baseName + suffix;

            if (debug) {
               qDebug() << "Trying..." << fn;
            }
            if (QFileInfo(fn).isFile()) {
               return fn;
            }
         }
      }
   }

   if (debug) {
      qDebug() << fileName << "not found";
   }
   return QString();
}
#endif

void QPluginLoader::setFileName(const QString &fileName)
{
#if ! defined(QT_STATIC)
   QLibrary::LoadHints lh = QLibrary::PreventUnloadHint;

   if (d) {
      lh = d->loadHints();
      d->release();
      d = 0;
      did_load = false;
   }

   const QString fn = locatePlugin(fileName);

   d = QLibraryPrivate::findOrCreate(fn, QString(), lh);
   if (! fn.isEmpty()) {
      d->updatePluginState();
   }

#else
   if (qt_debug_component()) {
      qWarning("Can not load %s into a statically linked CopperSpice library.",
         QFile::encodeName(fileName).constData() );
   }

#endif
}

QString QPluginLoader::fileName() const
{
   if (d) {
      return d->fileName;
   }
   return QString();
}

QString QPluginLoader::errorString() const
{
   return (!d || d->errorString.isEmpty()) ? tr("Unknown error") : d->errorString;
}

typedef QVector<QStaticPlugin> StaticPluginList;
Q_GLOBAL_STATIC(StaticPluginList, staticPluginList)

void QPluginLoader::setLoadHints(QLibrary::LoadHints loadHints)
{
   if (!d) {
      d = QLibraryPrivate::findOrCreate(QString());   // ugly, but we need a d-ptr
      d->errorString.clear();
   }

   d->setLoadHints(loadHints);
}

QLibrary::LoadHints QPluginLoader::loadHints() const
{
   return d ? d->loadHints() : QLibrary::LoadHints();
}



void Q_CORE_EXPORT qRegisterStaticPluginFunction(QStaticPlugin plugin)
{
   staticPluginList()->append(plugin);
}

QObjectList QPluginLoader::staticInstances()
{
   QObjectList instances;
   const StaticPluginList *plugins = staticPluginList();

   if (plugins) {
      const int numPlugins = plugins->size();

      for (int i = 0; i < numPlugins; ++i) {
         instances += plugins->at(i).instance();
      }
   }

   return instances;

}


QVector<QStaticPlugin> QPluginLoader::staticPlugins()
{
   StaticPluginList *plugins = staticPluginList();
   if (plugins) {
      return *plugins;
   }
   return QVector<QStaticPlugin>();
}

QJsonObject QStaticPlugin::metaData() const
{
   return QLibraryPrivate::fromRawMetaData(rawMetaData()).object();
}

