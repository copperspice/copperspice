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

#include "q3listview_plugin.h"
#include "q3listview_extrainfo.h"

#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>

#include <QtCore/qplugin.h>
#include <QtGui/QIcon>
#include <Qt3Support/Q3ListView>

QT_BEGIN_NAMESPACE

Q3ListViewPlugin::Q3ListViewPlugin(const QIcon &icon, QObject *parent)
        : QObject(parent), m_initialized(false), m_icon(icon)
{}

QString Q3ListViewPlugin::name() const
{ return QLatin1String("Q3ListView"); }

QString Q3ListViewPlugin::group() const
{ return QLatin1String("Qt 3 Support"); }

QString Q3ListViewPlugin::toolTip() const
{ return QString(); }

QString Q3ListViewPlugin::whatsThis() const
{ return QString(); }

QString Q3ListViewPlugin::includeFile() const
{ return QLatin1String("q3listview.h"); }

QIcon Q3ListViewPlugin::icon() const
{ return m_icon; }

bool Q3ListViewPlugin::isContainer() const
{ return false; }

QWidget *Q3ListViewPlugin::createWidget(QWidget *parent)
{ return new Q3ListView(parent); }

bool Q3ListViewPlugin::isInitialized() const
{ return m_initialized; }

void Q3ListViewPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);

    if (m_initialized)
        return;

    QExtensionManager *mgr = core->extensionManager();
    Q_ASSERT(mgr != 0);

    mgr->registerExtensions(new Q3ListViewExtraInfoFactory(core, mgr), Q_TYPEID(QDesignerExtraInfoExtension));

    m_initialized = true;
}

QString Q3ListViewPlugin::codeTemplate() const
{ return QString(); }

QString Q3ListViewPlugin::domXml() const
{ return QLatin1String("\
<ui language=\"c++\">\
    <widget class=\"Q3ListView\" name=\"listView\">\
        <property name=\"geometry\">\
            <rect>\
                <x>0</x>\
                <y>0</y>\
                <width>100</width>\
                <height>80</height>\
            </rect>\
        </property>\
    </widget>\
</ui>");
}



QT_END_NAMESPACE
