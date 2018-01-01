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

// sdk
#include "abstractmetadatabase.h"

QT_BEGIN_NAMESPACE

/*!
    \class QDesignerMetaDataBaseInterface
    \brief The QDesignerMetaDataBaseInterface class provides an interface to Qt Designer's
    object meta database.
    \inmodule QtDesigner
    \internal
*/

/*!
    Constructs an interface to the meta database with the given \a parent.
*/
QDesignerMetaDataBaseInterface::QDesignerMetaDataBaseInterface(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the interface to the meta database.
*/
QDesignerMetaDataBaseInterface::~QDesignerMetaDataBaseInterface()
{
}

/*!
    \fn QDesignerMetaDataBaseItemInterface *QDesignerMetaDataBaseInterface::item(QObject *object) const

    Returns the item in the meta database associated with the given \a object.
*/

/*!
    \fn void QDesignerMetaDataBaseInterface::add(QObject *object)

    Adds the specified \a object to the meta database.
*/

/*!
    \fn void QDesignerMetaDataBaseInterface::remove(QObject *object)

    Removes the specified \a object from the meta database.
*/

/*!
    \fn QList<QObject*> QDesignerMetaDataBaseInterface::objects() const

    Returns the list of objects that have corresponding items in the meta database.
*/

/*!
    \fn QDesignerFormEditorInterface *QDesignerMetaDataBaseInterface::core() const

    Returns the core interface that is associated with the meta database.
*/


// Doc: Interface only

/*!
    \class QDesignerMetaDataBaseItemInterface
    \brief The QDesignerMetaDataBaseItemInterface class provides an interface to individual
    items in Qt Designer's meta database.
    \inmodule QtDesigner
    \internal

    This class allows individual items in \QD's meta-data database to be accessed and modified.
    Use the QDesignerMetaDataBaseInterface class to change the properties of the database itself.
*/

/*!
    \fn QDesignerMetaDataBaseItemInterface::~QDesignerMetaDataBaseItemInterface()

    Destroys the item interface to the meta-data database.
*/

/*!
    \fn QString QDesignerMetaDataBaseItemInterface::name() const

    Returns the name of the item in the database.

    \sa setName()
*/

/*!
    \fn void QDesignerMetaDataBaseItemInterface::setName(const QString &name)

    Sets the name of the item to the given \a name.

    \sa name()
*/

/*!
    \fn QList<QWidget*> QDesignerMetaDataBaseItemInterface::tabOrder() const

    Returns a list of widgets in the order defined by the form's tab order.

    \sa setTabOrder()
*/


/*!
    \fn void QDesignerMetaDataBaseItemInterface::setTabOrder(const QList<QWidget*> &tabOrder)

    Sets the tab order in the form using the list of widgets defined by \a tabOrder.

    \sa tabOrder()
*/

    
/*!
    \fn bool QDesignerMetaDataBaseItemInterface::enabled() const

    Returns whether the item is enabled.

    \sa setEnabled()
*/

/*!
    \fn void QDesignerMetaDataBaseItemInterface::setEnabled(bool enabled)

    If \a enabled is true, the item is enabled; otherwise it is disabled.

    \sa enabled()
*/

QT_END_NAMESPACE
