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

#include "abstractactioneditor.h"

QT_BEGIN_NAMESPACE

/*!
    \class QDesignerActionEditorInterface

    \brief The QDesignerActionEditorInterface class allows you to
    change the focus of Qt Designer's action editor.

    \inmodule QtDesigner

    The QDesignerActionEditorInterface class is not intended to be
    instantiated directly. You can retrieve an interface to \QD's
    action editor using the
    QDesignerFormEditorInterface::actionEditor() function.

    You can control which actions that are available in the action
    editor's window using the manageAction() and unmanageAction()
    functions. An action that is managed by \QD is available in the
    action editor while an unmanaged action is ignored.

    QDesignerActionEditorInterface also provides the core() function
    that you can use to retrieve a pointer to \QD's current
    QDesignerFormEditorInterface object, and the setFormWindow()
    function that enables you to change the currently selected form
    window.

    \sa QDesignerFormEditorInterface, QDesignerFormWindowInterface
*/

/*!
    Constructs an action editor interface with the given \a parent and
    the specified window \a flags.
*/
QDesignerActionEditorInterface::QDesignerActionEditorInterface(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
}

/*!
    Destroys the action editor interface.
*/
QDesignerActionEditorInterface::~QDesignerActionEditorInterface()
{
}

/*!
    Returns a pointer to \QD's current QDesignerFormEditorInterface
    object.
*/
QDesignerFormEditorInterface *QDesignerActionEditorInterface::core() const
{
    return 0;
}

/*!
    \fn void QDesignerActionEditorInterface::setFormWindow(QDesignerFormWindowInterface *formWindow)

    Sets the currently selected form window to \a formWindow.

*/

/*!
    \fn void QDesignerActionEditorInterface::manageAction(QAction *action)

    Instructs \QD to manage the specified \a action. An action that is
    managed by \QD is available in the action editor.

    \sa unmanageAction()
*/

/*!
    \fn void QDesignerActionEditorInterface::unmanageAction(QAction *action)

    Instructs \QD to ignore the specified \a action. An unmanaged
    action is not available in the action editor.

    \sa manageAction()
*/

QT_END_NAMESPACE
