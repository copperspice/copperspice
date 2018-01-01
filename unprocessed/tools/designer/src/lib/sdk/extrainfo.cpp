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

#include "extrainfo.h"

QT_BEGIN_NAMESPACE

/*!
    \class QDesignerExtraInfoExtension
    \brief The QDesignerExtraInfoExtension class provides extra information about a widget in
    Qt Designer.
    \inmodule QtDesigner
    \internal
*/

/*!
    Returns the path to the working directory used by this extension.*/
QString QDesignerExtraInfoExtension::workingDirectory() const
{
    return m_workingDirectory;
}

/*!
    Sets the path to the working directory used by the extension to \a workingDirectory.*/
void QDesignerExtraInfoExtension::setWorkingDirectory(const QString &workingDirectory)
{
    m_workingDirectory = workingDirectory;
}

/*!
    \fn virtual QDesignerExtraInfoExtension::~QDesignerExtraInfoExtension()

    Destroys the extension.
*/

/*!
    \fn virtual QDesignerFormEditorInterface *QDesignerExtraInfoExtension::core() const = 0

    \omit
    ### Description required
    \endomit
*/

/*!
    \fn virtual QWidget *QDesignerExtraInfoExtension::widget() const = 0

    Returns the widget described by this extension.
*/

/*!
    \fn virtual bool QDesignerExtraInfoExtension::saveUiExtraInfo(DomUI *ui) = 0

    Saves the information about the user interface specified by \a ui, and returns true if
    successful; otherwise returns false.
*/

/*!
    \fn virtual bool QDesignerExtraInfoExtension::loadUiExtraInfo(DomUI *ui) = 0

    Loads extra information about the user interface specified by \a ui, and returns true if
    successful; otherwise returns false.
*/

/*!
    \fn virtual bool QDesignerExtraInfoExtension::saveWidgetExtraInfo(DomWidget *widget) = 0

    Saves the information about the specified \a widget, and returns true if successful;
    otherwise returns false.
*/

/*!
    \fn virtual bool QDesignerExtraInfoExtension::loadWidgetExtraInfo(DomWidget *widget) = 0

    Loads extra information about the specified \a widget, and returns true if successful;
    otherwise returns false.
*/

QT_END_NAMESPACE
