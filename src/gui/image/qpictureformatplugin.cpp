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

#include "qpictureformatplugin.h"
#if !defined(QT_NO_PICTURE)
#include "qpicture.h"

QT_BEGIN_NAMESPACE

/*!
    \obsolete

    \class QPictureFormatPlugin
    \brief The QPictureFormatPlugin class provides an abstract base
    for custom picture format plugins.

    \ingroup plugins

    The picture format plugin is a simple plugin interface that makes
    it easy to create custom picture formats that can be used
    transparently by applications.

    Writing an picture format plugin is achieved by subclassing this
    base class, reimplementing the pure virtual functions keys(),
    loadPicture(), savePicture(), and installIOHandler(), and
    exporting the class with the Q_EXPORT_PLUGIN2() macro.

    \sa {How to Create Qt Plugins}
*/

/*!
    \fn QStringList QPictureFormatPlugin::keys() const

    Returns the list of picture formats this plugin supports.

    \sa installIOHandler()
*/

/*!
    \fn bool QPictureFormatPlugin::installIOHandler(const QString &format)

    Installs a QPictureIO picture I/O handler for the picture format \a
    format.

    \sa keys()
*/


/*!
    Constructs an picture format plugin with the given \a parent.
    This is invoked automatically by the Q_EXPORT_PLUGIN2() macro.
*/
QPictureFormatPlugin::QPictureFormatPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the picture format plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QPictureFormatPlugin::~QPictureFormatPlugin()
{
}


/*!
    Loads the picture stored in the file called \a fileName, with the
    given \a format, into *\a picture. Returns true on success;
    otherwise returns false.

    \sa savePicture()
*/
bool QPictureFormatPlugin::loadPicture(const QString &format, const QString &fileName, QPicture *picture)
{
    Q_UNUSED(format)
    Q_UNUSED(fileName)
    Q_UNUSED(picture)
    return false;
}

/*!
    Saves the given \a picture into the file called \a fileName,
    using the specified \a format. Returns true on success; otherwise
    returns false.

    \sa loadPicture()
*/
bool QPictureFormatPlugin::savePicture(const QString &format, const QString &fileName, const QPicture &picture)
{
    Q_UNUSED(format)
    Q_UNUSED(fileName)
    Q_UNUSED(picture)
    return false;
}

#endif // QT_NO_PICTURE

QT_END_NAMESPACE
