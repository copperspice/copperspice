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

#include "q3mainwindow_plugin.h"
#include "q3mainwindow_container.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QExtensionManager>

#include <QtCore/qplugin.h>
#include <Qt3Support/Q3MainWindow>

QT_BEGIN_NAMESPACE

Q3MainWindowPlugin::Q3MainWindowPlugin(const QIcon &icon, QObject *parent)
    : QObject(parent), m_initialized(false), m_icon(icon)
{}

QString Q3MainWindowPlugin::name() const
{ return QLatin1String("Q3MainWindow"); }

QString Q3MainWindowPlugin::group() const
{ return QLatin1String("[invisible]"); }

QString Q3MainWindowPlugin::toolTip() const
{ return QString(); }

QString Q3MainWindowPlugin::whatsThis() const
{ return QString(); }

QString Q3MainWindowPlugin::includeFile() const
{ return QLatin1String("q3mainwindow.h"); }

QIcon Q3MainWindowPlugin::icon() const
{ return m_icon; }

bool Q3MainWindowPlugin::isContainer() const
{ return true; }

QWidget *Q3MainWindowPlugin::createWidget(QWidget *parent)
{ return new Q3MainWindow(parent); }

bool Q3MainWindowPlugin::isInitialized() const
{ return m_initialized; }

void Q3MainWindowPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);

    if (m_initialized)
        return;

    m_initialized = true;
    QExtensionManager *mgr = core->extensionManager();
    mgr->registerExtensions(new Q3MainWindowContainerFactory(mgr), Q_TYPEID(QDesignerContainerExtension));
}

QString Q3MainWindowPlugin::codeTemplate() const
{ return QString(); }

QString Q3MainWindowPlugin::domXml() const
{
    return QLatin1String("\
<ui language=\"c++\">\
    <widget class=\"Q3MainWindow\" name=\"mainWindow\">\
        <property name=\"geometry\">\
            <rect>\
                <x>0</x>\
                <y>0</y>\
                <width>100</width>\
                <height>80</height>\
            </rect>\
        </property>\
        <widget class=\"QWidget\" name=\"centralWidget\" />\
    </widget>\
</ui>");
}


QT_END_NAMESPACE
