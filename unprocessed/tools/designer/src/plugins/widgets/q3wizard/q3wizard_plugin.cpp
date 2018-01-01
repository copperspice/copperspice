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

#include "q3wizard_plugin.h"
#include "q3wizard_container.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QExtensionManager>

#include <QtCore/qplugin.h>
#include <QtGui/QPushButton>
#include <Qt3Support/Q3Wizard>

QT_BEGIN_NAMESPACE

Q3WizardPlugin::Q3WizardPlugin(const QIcon &icon, QObject *parent)
    : QObject(parent), m_initialized(false), m_icon(icon)
{}

QString Q3WizardPlugin::name() const
{ return QLatin1String("Q3Wizard"); }

QString Q3WizardPlugin::group() const
{ return QLatin1String("[invisible]"); }

QString Q3WizardPlugin::toolTip() const
{ return QString(); }

QString Q3WizardPlugin::whatsThis() const
{ return QString(); }

QString Q3WizardPlugin::includeFile() const
{ return QLatin1String("q3wizard.h"); }

QIcon Q3WizardPlugin::icon() const
{ return m_icon; }

bool Q3WizardPlugin::isContainer() const
{ return true; }

QWidget *Q3WizardPlugin::createWidget(QWidget *parent)
{
    Q3Wizard *wizard = new Q3Wizard(parent);
    new Q3WizardHelper(wizard);
    wizard->backButton()->setObjectName(QLatin1String("__qt__passive_") + wizard->backButton()->objectName());
    wizard->nextButton()->setObjectName(QLatin1String("__qt__passive_") + wizard->nextButton()->objectName());
    return wizard;
}

bool Q3WizardPlugin::isInitialized() const
{ return m_initialized; }

void Q3WizardPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);

    if (m_initialized)
        return;

    m_initialized = true;
    QExtensionManager *mgr = core->extensionManager();
    Q3WizardPropertySheetFactory::registerExtension(mgr);
    mgr->registerExtensions(new Q3WizardContainerFactory(mgr), Q_TYPEID(QDesignerContainerExtension));
    mgr->registerExtensions(new Q3WizardExtraInfoFactory(core, mgr), Q_TYPEID(QDesignerExtraInfoExtension));
}

QString Q3WizardPlugin::codeTemplate() const
{ return QString(); }

QString Q3WizardPlugin::domXml() const
{
    return QLatin1String("\
<ui language=\"c++\">\
    <widget class=\"Q3Wizard\" name=\"wizard\">\
        <property name=\"geometry\">\
            <rect>\
                <x>0</x>\
                <y>0</y>\
                <width>100</width>\
                <height>80</height>\
            </rect>\
        </property>\
        <widget class=\"QWidget\" />\
        <widget class=\"QWidget\" />\
    </widget>\
</ui>");
}


QT_END_NAMESPACE
