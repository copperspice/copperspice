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

#include "qkbddriverplugin_qws.h"
#include "qkbd_qws.h"

QT_BEGIN_NAMESPACE

/*!
    \class QKbdDriverPlugin
    \ingroup plugins
    \ingroup qws

    \brief The QKbdDriverPlugin class is an abstract base class for
    keyboard driver plugins in Qt for Embedded Linux.

    Note that this class is only available in \l{Qt for Embedded Linux}.

    \l{Qt for Embedded Linux} provides ready-made drivers for several keyboard
    protocols, see the \l{Qt for Embedded Linux Character Input}{character
    input} documentation for details. Custom keyboard drivers can be
    implemented by subclassing the QWSKeyboardHandler class and
    creating a keyboard driver plugin.

    A keyboard driver plugin can be created by subclassing
    QKbdDriverPlugin and reimplementing the pure virtual keys() and
    create() functions. By exporting the derived class using the
    Q_EXPORT_PLUGIN2() macro, the default implementation of the
    QKbdDriverFactory class will automatically detect the plugin and
    load the driver into the server application at run-time. See
    \l{How to Create Qt Plugins} for details.

    \sa QKbdDriverFactory, QWSKeyboardHandler
*/

/*!
    \fn QStringList QKbdDriverPlugin::keys() const

    Implement this function to return the list of valid keys, i.e. the
    keyboard drivers supported by this plugin.

    \l{Qt for Embedded Linux} provides ready-made drivers for several keyboard
    protocols, see the \l{Qt for Embedded Linux Character Input}{character
    input} documentation for details.

    \sa create()
*/

/*!
    Constructs a keyboard driver plugin with the given \a parent.

    Note that this constructor is invoked automatically by the
    Q_EXPORT_PLUGIN2() macro, so there is no need for calling it
    explicitly.
*/
QKbdDriverPlugin::QKbdDriverPlugin(QObject *parent)
   : QObject(parent)
{
}

/*!
    Destroys the keyboard driver plugin.

    Note that Qt destroys a plugin automatically when it is no longer
    used, so there is no need for calling the destructor explicitly.
*/
QKbdDriverPlugin::~QKbdDriverPlugin()
{
}

/*!
    \fn QScreen *QKbdDriverPlugin::create(const QString &key, const QString &device)

    Implement this function to create a driver matching the type
    specified by the given \a key and \a device parameters. Note that
    keys are case-insensitive.

    \sa keys()
*/

QT_END_NAMESPACE

