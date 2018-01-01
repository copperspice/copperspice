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

#ifndef EXTRAINFO_H
#define EXTRAINFO_H

#include <QtDesigner/sdk_global.h>
#include <QtDesigner/extension.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class DomWidget;
class DomUI;
class QWidget;

class QDesignerFormEditorInterface;

class QDESIGNER_SDK_EXPORT QDesignerExtraInfoExtension
{
public:
    virtual ~QDesignerExtraInfoExtension() {}

    virtual QDesignerFormEditorInterface *core() const = 0;
    virtual QWidget *widget() const = 0;

    virtual bool saveUiExtraInfo(DomUI *ui) = 0;
    virtual bool loadUiExtraInfo(DomUI *ui) = 0;

    virtual bool saveWidgetExtraInfo(DomWidget *ui_widget) = 0;
    virtual bool loadWidgetExtraInfo(DomWidget *ui_widget) = 0;

    QString workingDirectory() const;
    void setWorkingDirectory(const QString &workingDirectory);

private:
    QString m_workingDirectory;
};
Q_DECLARE_EXTENSION_INTERFACE(QDesignerExtraInfoExtension, "com.trolltech.Qt.Designer.ExtraInfo.2")

QT_END_NAMESPACE

QT_END_HEADER

#endif // EXTRAINFO_H
