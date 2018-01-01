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

#include "qscriptextensionplugin.h"

#include "qscriptvalue.h"
#include "qscriptengine.h"

QT_BEGIN_NAMESPACE

/*!
    \since 4.3
    \class QScriptExtensionPlugin
    \brief The QScriptExtensionPlugin class provides an abstract base for custom QScript extension plugins.

    \ingroup plugins

    QScriptExtensionPlugin is a plugin interface that makes it
    possible to offer extensions that can be loaded dynamically into
    applications using the QScriptEngine class.

    Writing a script extension plugin is achieved by subclassing this
    base class, reimplementing the pure virtual keys() and initialize()
    functions, and exporting the class using the Q_EXPORT_PLUGIN2()
    macro. See \l {How to Create Qt Plugins} for details.

    \sa QScriptEngine::importExtension(), {Creating QtScript Extensions}
*/

/*!
    \fn QStringList QScriptExtensionPlugin::keys() const

    Returns the list of keys this plugin supports.

    These keys are usually the names of the "modules" or "packages"
    that are implemented in the plugin (e.g. \c{com.mycompany.MyProduct}).

    \sa initialize()
*/

/*!
    \fn void QScriptExtensionPlugin::initialize(const QString& key, QScriptEngine *engine)

    Initializes the extension specified by \a key in the given \a engine.
    The key must come from the set of keys().

    \sa keys()
*/

/*!
    Constructs a script extension plugin with the given \a parent.

    Note that this constructor is invoked automatically by the
    Q_EXPORT_PLUGIN2() macro, so there is no need for calling it
    explicitly.
*/
QScriptExtensionPlugin::QScriptExtensionPlugin(QObject *parent)
   : QObject(parent)
{
}

/*!
    Destroys the script extension plugin.

    Note that Qt destroys a plugin automatically when it is no longer
    used, so there is no need for calling the destructor explicitly.
*/
QScriptExtensionPlugin::~QScriptExtensionPlugin()
{
}


QScriptValue QScriptExtensionPlugin::setupPackage(
   const QString &key, QScriptEngine *engine) const
{
   QStringList components = key.split(QLatin1Char('.'));
   QScriptValue o = engine->globalObject();
   for (int i = 0; i < components.count(); ++i) {
      QScriptValue oo = o.property(components.at(i));
      if (!oo.isValid()) {
         oo = engine->newObject();
         o.setProperty(components.at(i), oo);
      }
      o = oo;
   }
   return o;
}

QT_END_NAMESPACE
