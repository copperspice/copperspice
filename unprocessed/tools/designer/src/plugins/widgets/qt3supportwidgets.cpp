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

#include "q3toolbar/q3toolbar_plugin.h"
#include "q3iconview/q3iconview_plugin.h"
#include "q3wizard/q3wizard_plugin.h"
#include "q3mainwindow/q3mainwindow_plugin.h"
#include "q3widgetstack/q3widgetstack_plugin.h"
#include "q3listview/q3listview_plugin.h"
#include "q3table/q3table_plugin.h"
#include "q3listbox/q3listbox_plugin.h"
#include "q3listview/q3listview_plugin.h"
#include "q3textedit/q3textedit_plugin.h"
#include "q3widgets/q3widget_plugins.h"

#include <QtDesigner/QDesignerCustomWidgetCollectionInterface>
#include <QtCore/qplugin.h>
#include <QtCore/qdebug.h>
#include <QtGui/QIcon>

QT_BEGIN_NAMESPACE

class Qt3SupportWidgets: public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
public:
    Qt3SupportWidgets(QObject *parent = 0);

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

private:
    QList<QDesignerCustomWidgetInterface*> m_plugins;
};

Qt3SupportWidgets::Qt3SupportWidgets(QObject *parent)
    : QObject(parent)
{
    const QIcon qt3Icon(QLatin1String(":/trolltech/formeditor/images/qt3logo.png"));
    m_plugins.append(new Q3ToolBarPlugin(qt3Icon, this));
    m_plugins.append(new Q3IconViewPlugin(qt3Icon, this));
    m_plugins.append(new Q3GroupBoxPlugin(qt3Icon, this));
    m_plugins.append(new Q3FramePlugin(qt3Icon, this));
    m_plugins.append(new Q3WizardPlugin(qt3Icon, this));
    m_plugins.append(new Q3MainWindowPlugin(qt3Icon, this));
    m_plugins.append(new Q3WidgetStackPlugin(qt3Icon, this));
    m_plugins.append(new Q3ButtonGroupPlugin(qt3Icon, this));
    m_plugins.append(new Q3TablePlugin(qt3Icon, this));
    m_plugins.append(new Q3ListBoxPlugin(qt3Icon, this));
    m_plugins.append(new Q3ListViewPlugin(qt3Icon, this));
    m_plugins.append(new Q3ComboBoxPlugin(qt3Icon, this));
    m_plugins.append(new Q3TextEditPlugin(qt3Icon, this));
    m_plugins.append(new Q3DateEditPlugin(qt3Icon, this));
    m_plugins.append(new Q3TimeEditPlugin(qt3Icon, this));
    m_plugins.append(new Q3DateTimeEditPlugin(qt3Icon, this));
    m_plugins.append(new Q3ProgressBarPlugin(qt3Icon, this));
    m_plugins.append(new Q3TextBrowserPlugin(qt3Icon, this));
}

QList<QDesignerCustomWidgetInterface*> Qt3SupportWidgets::customWidgets() const
{
    return m_plugins;
}

Q_EXPORT_PLUGIN(Qt3SupportWidgets)

QT_END_NAMESPACE

#include "qt3supportwidgets.moc"
