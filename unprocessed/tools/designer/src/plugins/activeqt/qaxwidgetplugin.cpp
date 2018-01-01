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

#include "qaxwidgetplugin.h"
#include "qaxwidgetextrainfo.h"
#include "qdesigneraxwidget.h"
#include "qaxwidgetpropertysheet.h"
#include "qaxwidgettaskmenu.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtCore/qplugin.h>
#include <QtGui/QIcon>
#include <ActiveQt/QAxWidget>

QT_BEGIN_NAMESPACE

QAxWidgetPlugin::QAxWidgetPlugin(QObject *parent) :
    QObject(parent),
    m_core(0)
{
}

QString QAxWidgetPlugin::name() const
{
    return QLatin1String("QAxWidget");
}

QString QAxWidgetPlugin::group() const
{
    return QLatin1String("Containers");
}

QString QAxWidgetPlugin::toolTip() const
{
    return tr("ActiveX control");
}

QString QAxWidgetPlugin::whatsThis() const
{
    return tr("ActiveX control widget");
}

QString QAxWidgetPlugin::includeFile() const
{
    return QLatin1String("qaxwidget.h");
}

QIcon QAxWidgetPlugin::icon() const
{
    return QIcon(QDesignerAxWidget::widgetIcon());
}

bool QAxWidgetPlugin::isContainer() const
{
    return false;
}

QWidget *QAxWidgetPlugin::createWidget(QWidget *parent)
{
    // Construction from Widget box or on a form?
    const bool isFormEditor = parent != 0 && QDesignerFormWindowInterface::findFormWindow(parent) != 0;
    QDesignerAxWidget *rc = new QDesignerAxPluginWidget(parent);
    if (!isFormEditor)
        rc->setDrawFlags(QDesignerAxWidget::DrawFrame|QDesignerAxWidget::DrawControl);
    return rc;
}

bool QAxWidgetPlugin::isInitialized() const
{
    return m_core != 0;
}

void QAxWidgetPlugin::initialize(QDesignerFormEditorInterface *core)
{
    if (m_core != 0)
        return;

    m_core = core;

    QExtensionManager *mgr = core->extensionManager();
    ActiveXPropertySheetFactory::registerExtension(mgr);
    ActiveXTaskMenuFactory::registerExtension(mgr, Q_TYPEID(QDesignerTaskMenuExtension));
    QAxWidgetExtraInfoFactory *extraInfoFactory = new QAxWidgetExtraInfoFactory(core, mgr);
    mgr->registerExtensions(extraInfoFactory, Q_TYPEID(QDesignerExtraInfoExtension));
}

QString QAxWidgetPlugin::domXml() const
{
    return QLatin1String("\
<ui language=\"c++\">\
    <widget class=\"QAxWidget\" name=\"axWidget\">\
        <property name=\"geometry\">\
            <rect>\
                <x>0</x>\
                <y>0</y>\
                <width>80</width>\
                <height>70</height>\
            </rect>\
        </property>\
    </widget>\
</ui>");
}

Q_EXPORT_PLUGIN(QAxWidgetPlugin)

QT_END_NAMESPACE
