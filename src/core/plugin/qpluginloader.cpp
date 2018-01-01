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
#include <qplugin.h>
#include <qpluginloader.h>
#include <qfileinfo.h>
#include <qlibrary_p.h>
#include <qdebug.h>
#include <qdir.h>

QT_BEGIN_NAMESPACE

QPluginLoader::QPluginLoader(QObject *parent)
   : QObject(parent), d(0), did_load(false)
{
}

QPluginLoader::QPluginLoader(const QString &fileName, QObject *parent)
   : QObject(parent), d(0), did_load(false)
{
   setFileName(fileName);
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

/*!
    Returns the root component object of the plugin. The plugin is
    loaded if necessary. The function returns 0 if the plugin could
    not be loaded or if the root component object could not be
    instantiated.

    If the root component object was destroyed, calling this function
    creates a new instance.

    The root component, returned by this function, is not deleted when
    the QPluginLoader is destroyed. If you want to ensure that the root
    component is deleted, you should call unload() as soon you don't
    need to access the core component anymore.  When the library is
    finally unloaded, the root component will automatically be deleted.

    The component object is a QObject. Use qobject_cast() to access
    interfaces you are interested in.

    \sa load()
*/
QObject *QPluginLoader::instance()
{
   if (!load()) {
      return 0;
   }
   if (!d->inst && d->instance) {
      d->inst = d->instance();
   }
   return d->inst.data();
}

/*!
    Loads the plugin and returns true if the plugin was loaded
    successfully; otherwise returns false. Since instance() always
    calls this function before resolving any symbols it is not
    necessary to call it explicitly. In some situations you might want
    the plugin loaded in advance, in which case you would use this
    function.

    \sa unload()
*/
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

   bool retval = d->loadPlugin();
   did_load = true;

   return retval;
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

void QPluginLoader::setFileName(const QString &fileName)
{
#if ! defined(QT_STATIC)
   QLibrary::LoadHints lh;

   if (d) {
      lh = d->loadHints;
      d->release();
      d = 0;
      did_load = false;
   }

   QString fn = QFileInfo(fileName).canonicalFilePath();
   d = QLibraryPrivate::findOrCreate(fn);
   d->loadHints = lh;

   if (fn.isEmpty()) {
      d->errorString = QLibrary::tr("The shared library was not found.");
   }

#else
   if (qt_debug_component()) {
      qWarning("Can not load %s into a statically linked CopperSpice library.",
               QFile::encodeName(fileName).constData() );
   }

   Q_UNUSED(fileName);
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

typedef QList<QtPluginInstanceFunction> StaticInstanceFunctionList;
Q_GLOBAL_STATIC(StaticInstanceFunctionList, staticInstanceFunctionList)

void QPluginLoader::setLoadHints(QLibrary::LoadHints loadHints)
{
   if (!d) {
      d = QLibraryPrivate::findOrCreate(QString());   // ugly, but we need a d-ptr
      d->errorString.clear();
   }

   d->loadHints = loadHints;
}

QLibrary::LoadHints QPluginLoader::loadHints() const
{
   if (!d) {
      QPluginLoader *that = const_cast<QPluginLoader *>(this);
      that->d = QLibraryPrivate::findOrCreate(QString());   // ugly, but we need a d-ptr
      that->d->errorString.clear();
   }
   return d->loadHints;
}

void Q_CORE_EXPORT qRegisterStaticPluginInstanceFunction(QtPluginInstanceFunction function)
{
   staticInstanceFunctionList()->append(function);
}

QObjectList QPluginLoader::staticInstances()
{
   QObjectList instances;
   StaticInstanceFunctionList *functions = staticInstanceFunctionList();

   if (functions) {
      for (int i = 0; i < functions->count(); ++i) {
         instances.append((*functions)[i]());
      }
   }

   return instances;
}

QT_END_NAMESPACE
