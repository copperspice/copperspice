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

#ifndef CUSTOMWIDGET_H
#define CUSTOMWIDGET_H

#include <QtDesigner/extension.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QIcon>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QWidget;
class QDesignerFormEditorInterface;

class QDesignerCustomWidgetInterface
{
public:
    virtual ~QDesignerCustomWidgetInterface() {}

    virtual QString name() const = 0;
    virtual QString group() const = 0;
    virtual QString toolTip() const = 0;
    virtual QString whatsThis() const = 0;
    virtual QString includeFile() const = 0;
    virtual QIcon icon() const = 0;

    virtual bool isContainer() const = 0;

    virtual QWidget *createWidget(QWidget *parent) = 0;

    virtual bool isInitialized() const { return false; }
    virtual void initialize(QDesignerFormEditorInterface *core) { Q_UNUSED(core); }

    virtual QString domXml() const
    {
        return QString::fromUtf8("<widget class=\"%1\" name=\"%2\"/>")
            .arg(name()).arg(name().toLower());
    }

    virtual QString codeTemplate() const { return QString(); }
};
Q_DECLARE_INTERFACE(QDesignerCustomWidgetInterface, "com.trolltech.Qt.Designer.CustomWidget")


class QDesignerCustomWidgetCollectionInterface
{
public:
    virtual ~QDesignerCustomWidgetCollectionInterface() {}

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const = 0;
};
Q_DECLARE_INTERFACE(QDesignerCustomWidgetCollectionInterface,
    "com.trolltech.Qt.Designer.CustomWidgetCollection")

QT_END_NAMESPACE

QT_END_HEADER

#endif // CUSTOMWIDGET_H
