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

#include "q3textedit_plugin.h"
#include "q3textedit_extrainfo.h"

#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>

#include <QtCore/qplugin.h>
#include <QtGui/QIcon>
#include <Qt3Support/Q3TextEdit>

QT_BEGIN_NAMESPACE

Q3TextEditPlugin::Q3TextEditPlugin(const QIcon &icon, QObject *parent)
        : QObject(parent), m_initialized(false), m_icon(icon)
{}

QString Q3TextEditPlugin::name() const
{ return QLatin1String("Q3TextEdit"); }

QString Q3TextEditPlugin::group() const
{ return QLatin1String("Qt 3 Support"); }

QString Q3TextEditPlugin::toolTip() const
{ return QString(); }

QString Q3TextEditPlugin::whatsThis() const
{ return QString(); }

QString Q3TextEditPlugin::includeFile() const
{ return QLatin1String("q3textedit.h"); }

QIcon Q3TextEditPlugin::icon() const
{ return m_icon; }


bool Q3TextEditPlugin::isContainer() const
{ return false; }

QWidget *Q3TextEditPlugin::createWidget(QWidget *parent)
{ return new Q3TextEdit(parent); }

bool Q3TextEditPlugin::isInitialized() const
{ return m_initialized; }

void Q3TextEditPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);

    if (m_initialized)
        return;

    QExtensionManager *mgr = core->extensionManager();
    Q_ASSERT(mgr != 0);

    mgr->registerExtensions(new Q3TextEditExtraInfoFactory(core, mgr), Q_TYPEID(QDesignerExtraInfoExtension));

    m_initialized = true;
}

QString Q3TextEditPlugin::codeTemplate() const
{ return QString(); }

QString Q3TextEditPlugin::domXml() const
{ return QLatin1String("\
<ui language=\"c++\">\
    <widget class=\"Q3TextEdit\" name=\"textEdit\">\
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
