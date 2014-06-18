/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qdecorationplugin_qws.h"
#include "qdecoration_qws.h"

QT_BEGIN_NAMESPACE

/*!
    \class QDecorationPlugin
    \ingroup qws
    \ingroup plugins

    \brief The QDecorationPlugin class is an abstract base class for
    window decoration plugins in Qt for Embedded Linux.

    Note that this class is only available in \l{Qt for Embedded Linux}.

    \l{Qt for Embedded Linux} provides three ready-made decoration styles: \c
    Default, \c Styled and \c Windows. Custom decorations can be
    implemented by subclassing the QDecoration class and creating a
    decoration plugin.

    A decoration plugin can be created by subclassing
    QDecorationPlugin and implementing the pure virtual keys() and
    create() functions. By exporting the derived class using the
    Q_EXPORT_PLUGIN2() macro, the default implementation of the
    QDecorationFactory class will automatically detect the plugin and
    load the driver into the application at run-time. See \l{How to
    Create Qt Plugins} for details.

    To actually apply a decoration, use the
    QApplication::qwsSetDecoration() function.

    \sa QDecoration, QDecorationFactory
*/

/*!
    \fn QStringList QDecorationPlugin::keys() const

    Returns the list of valid keys, i.e., the decorations supported by
    this plugin.

    \sa create()
*/

/*!
    \fn QDecoration *QDecorationPlugin::create(const QString &key)

    Creates a decoration matching the given \a key. Note that keys are
    case-insensitive.

    \sa keys()
*/

/*!
    Constructs a decoration plugin with the given \a parent.

    Note that this constructor is invoked automatically by the
    Q_EXPORT_PLUGIN2() macro, so there is no need for calling it
    explicitly.
*/
QDecorationPlugin::QDecorationPlugin(QObject *parent)
   : QObject(parent)
{
}

/*!
    Destroys the decoration plugin.

    Note that Qt destroys a plugin automatically when it is no longer
    used, so there is no need for calling the destructor explicitly.
*/
QDecorationPlugin::~QDecorationPlugin()
{
}

QT_END_NAMESPACE
