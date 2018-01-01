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

#include "abstractobjectinspector.h"

QT_BEGIN_NAMESPACE

/*!
    \class QDesignerObjectInspectorInterface

    \brief The QDesignerObjectInspectorInterface class allows you to
    change the focus of Qt Designer's object inspector.

    \inmodule QtDesigner

    You can use the QDesignerObjectInspectorInterface to change the
    current form window selection. For example, when implementing a
    custom widget plugin:

    \snippet doc/src/snippets/code/tools_designer_src_lib_sdk_abstractobjectinspector.cpp 0

    The QDesignerObjectInspectorInterface class is not intended to be
    instantiated directly. You can retrieve an interface to \QD's
    object inspector using the
    QDesignerFormEditorInterface::objectInspector() function.  A
    pointer to \QD's current QDesignerFormEditorInterface object (\c
    formEditor in the example above) is provided by the
    QDesignerCustomWidgetInterface::initialize() function's
    parameter. When implementing a custom widget plugin, you must
    subclass the QDesignerCustomWidgetInterface to expose your plugin
    to \QD.

    The interface provides the core() function that you can use to
    retrieve a pointer to \QD's current QDesignerFormEditorInterface
    object, and the setFormWindow() function that enables you to
    change the current form window selection.

    \sa QDesignerFormEditorInterface, QDesignerFormWindowInterface
*/

/*!
    Constructs an object inspector interface with the given \a parent
    and the specified window \a flags.
*/
QDesignerObjectInspectorInterface::QDesignerObjectInspectorInterface(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
}

/*!
    Destroys the object inspector interface.
*/
QDesignerObjectInspectorInterface::~QDesignerObjectInspectorInterface()
{
}

/*!
    Returns a pointer to \QD's current QDesignerFormEditorInterface
    object.
*/
QDesignerFormEditorInterface *QDesignerObjectInspectorInterface::core() const
{
    return 0;
}

/*!
    \fn void QDesignerObjectInspectorInterface::setFormWindow(QDesignerFormWindowInterface *formWindow)

    Sets the currently selected form window to \a formWindow.
*/

QT_END_NAMESPACE
