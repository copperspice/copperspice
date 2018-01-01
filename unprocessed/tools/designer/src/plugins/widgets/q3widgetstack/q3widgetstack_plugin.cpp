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

#include "q3widgetstack_plugin.h"
#include "q3widgetstack_container.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QExtensionManager>

#include <QtCore/qplugin.h>
#include "qdesigner_q3widgetstack_p.h"

QT_BEGIN_NAMESPACE

Q3WidgetStackPlugin::Q3WidgetStackPlugin(const QIcon &icon, QObject *parent)
    : QObject(parent), m_initialized(false), m_icon(icon)
{}

QString Q3WidgetStackPlugin::name() const
{ return QLatin1String("Q3WidgetStack"); }

QString Q3WidgetStackPlugin::group() const
{ return QLatin1String("Qt 3 Support"); }

QString Q3WidgetStackPlugin::toolTip() const
{ return QString(); }

QString Q3WidgetStackPlugin::whatsThis() const
{ return QString(); }

QString Q3WidgetStackPlugin::includeFile() const
{ return QLatin1String("q3widgetstack.h"); }

QIcon Q3WidgetStackPlugin::icon() const
{ return m_icon; }

bool Q3WidgetStackPlugin::isContainer() const
{ return true; }

QWidget *Q3WidgetStackPlugin::createWidget(QWidget *parent)
{ return new QDesignerQ3WidgetStack(parent); }

bool Q3WidgetStackPlugin::isInitialized() const
{ return m_initialized; }

void Q3WidgetStackPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);

    if (m_initialized)
        return;

    m_initialized = true;
    QExtensionManager *mgr = core->extensionManager();
    mgr->registerExtensions(new Q3WidgetStackContainerFactory(mgr), Q_TYPEID(QDesignerContainerExtension));
}

QString Q3WidgetStackPlugin::codeTemplate() const
{ return QString(); }

QString Q3WidgetStackPlugin::domXml() const
{
    return QLatin1String("\
<ui language=\"c++\">\
    <widget class=\"Q3WidgetStack\" name=\"widgetStack\">\
        <property name=\"geometry\">\
            <rect>\
                <x>0</x>\
                <y>0</y>\
                <width>100</width>\
                <height>80</height>\
            </rect>\
        </property>\
        <widget class=\"QWidget\" name=\"page\"/>\
        <widget class=\"QWidget\" name=\"page_2\"/>\
    </widget>\
</ui>");
}

QT_END_NAMESPACE
