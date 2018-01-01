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

#include "qdeclarativeview_plugin.h"

#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>

#include <QtCore/qplugin.h>
#include <QtDeclarative/QDeclarativeView>

static const char toolTipC[] = "QtDeclarative view widget";

QT_BEGIN_NAMESPACE

QDeclarativeViewPlugin::QDeclarativeViewPlugin(QObject *parent) :
    QObject(parent),
    m_initialized(false)
{
}

QString QDeclarativeViewPlugin::name() const
{
    return QLatin1String("QDeclarativeView");
}

QString QDeclarativeViewPlugin::group() const
{
    return QLatin1String("Display Widgets");
}

QString QDeclarativeViewPlugin::toolTip() const
{
    return tr(toolTipC);
}

QString QDeclarativeViewPlugin::whatsThis() const
{
    return tr(toolTipC);
}

QString QDeclarativeViewPlugin::includeFile() const
{
    return QLatin1String("QtDeclarative/QDeclarativeView");
}

QIcon QDeclarativeViewPlugin::icon() const
{
    return QIcon();
}

bool QDeclarativeViewPlugin::isContainer() const
{
    return false;
}

QWidget *QDeclarativeViewPlugin::createWidget(QWidget *parent)
{
    return new QDeclarativeView(parent);
}

bool QDeclarativeViewPlugin::isInitialized() const
{
    return m_initialized;
}

void QDeclarativeViewPlugin::initialize(QDesignerFormEditorInterface * /*core*/)
{
    if (m_initialized)
        return;

    m_initialized = true;
}

QString QDeclarativeViewPlugin::domXml() const
{
    return QLatin1String("\
    <ui language=\"c++\">\
        <widget class=\"QDeclarativeView\" name=\"declarativeView\">\
            <property name=\"geometry\">\
                <rect>\
                    <x>0</x>\
                    <y>0</y>\
                    <width>300</width>\
                    <height>200</height>\
                </rect>\
            </property>\
        </widget>\
    </ui>");
}

Q_EXPORT_PLUGIN2(customwidgetplugin, QDeclarativeViewPlugin)

QT_END_NAMESPACE
