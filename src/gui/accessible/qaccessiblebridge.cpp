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

#include <qaccessiblebridge.h>

#ifndef QT_NO_ACCESSIBILITY

QT_BEGIN_NAMESPACE

/*!
    \class QAccessibleBridge
    \brief The QAccessibleBridge class is the base class for
    accessibility back-ends.

    \ingroup accessibility

    Qt supports Microsoft Active Accessibility (MSAA), Mac OS X
    Accessibility, and the Unix/X11 AT-SPI standard. By subclassing
    QAccessibleBridge, you can support other backends than the
    predefined ones.

    Currently, custom bridges are only supported on Unix. We might
    add support for them on other platforms as well if there is
    enough demand.

    \sa QAccessible, QAccessibleBridgePlugin
*/

/*!
    \fn QAccessibleBridge::~QAccessibleBridge()

    Destroys the accessibility bridge object.
*/

/*!
    \fn void QAccessibleBridge::setRootObject(QAccessibleInterface *object)

    This function is called by Qt at application startup to set the
    root accessible object of the application to \a object. All other
    accessible objects in the application can be reached by the
    client using object navigation.
*/

/*!
    \fn void QAccessibleBridge::notifyAccessibilityUpdate(int reason, QAccessibleInterface *interface, int child)

    This function is called by Qt to notify the bridge about a change
    in the accessibility information for object wrapped by the given
    \a interface.

    \a reason specifies the cause of the change. It can take values
    of type QAccessible::Event.

    \a child is the (1-based) index of the child element that has
    changed. When \a child is 0, the object itself has changed.

    \sa QAccessible::updateAccessibility()
*/

/*!
    \class QAccessibleBridgePlugin
    \brief The QAccessibleBridgePlugin class provides an abstract
    base for accessibility bridge plugins.

    \ingroup plugins
    \ingroup accessibility

    Writing an accessibility bridge plugin is achieved by subclassing
    this base class, reimplementing the pure virtual functions keys()
    and create(), and exporting the class with the
    Q_EXPORT_PLUGIN2() macro.

    \sa QAccessibleBridge, QAccessiblePlugin, {How to Create Qt Plugins}
*/

/*!
    Constructs an accessibility bridge plugin with the given \a
    parent. This is invoked automatically by the Q_EXPORT_PLUGIN2()
    macro.
*/
QAccessibleBridgePlugin::QAccessibleBridgePlugin(QObject *parent)
   : QObject(parent)
{

}

/*!
    Destroys the accessibility bridge plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QAccessibleBridgePlugin::~QAccessibleBridgePlugin()
{

}

/*!
    \fn QStringList QAccessibleBridgePlugin::keys() const

    Returns the list of keys this plugins supports.

    These keys must be the names of the bridges that this
    plugin provides.

    \sa create()
*/

/*!
    \fn QAccessibleBridge *QAccessibleBridgePlugin::create(const QString &key)

    Creates and returns the QAccessibleBridge object corresponding to
    the given \a key. Keys are case sensitive.

    \sa keys()
*/

QT_END_NAMESPACE

#endif // QT_NO_ACCESSIBILITY
