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

#include <QtCore/qdebug.h>
#include <QtCore/qplugin.h>
#include <QtGui/QAction>
#include "view3d_plugin.h"
#include "view3d_tool.h"

QView3DPlugin::QView3DPlugin()
{
    m_core = 0;
    m_action = 0;
}

bool QView3DPlugin::isInitialized() const
{
    return m_core != 0;
}

void QView3DPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_ASSERT(!isInitialized());

    m_action = new QAction(tr("3D View"), this);
    m_core = core;
    setParent(core);

    connect(core->formWindowManager(), SIGNAL(formWindowAdded(QDesignerFormWindowInterface*)),
            this, SLOT(addFormWindow(QDesignerFormWindowInterface*)));

    connect(core->formWindowManager(), SIGNAL(formWindowRemoved(QDesignerFormWindowInterface*)),
            this, SLOT(removeFormWindow(QDesignerFormWindowInterface*)));

    connect(core->formWindowManager(), SIGNAL(activeFormWindowChanged(QDesignerFormWindowInterface*)),
                this, SLOT(activeFormWindowChanged(QDesignerFormWindowInterface*)));
}

QAction *QView3DPlugin::action() const
{
    return m_action;
}

QDesignerFormEditorInterface *QView3DPlugin::core() const
{
    return m_core;
}

void QView3DPlugin::activeFormWindowChanged(QDesignerFormWindowInterface *formWindow)
{
    m_action->setEnabled(formWindow != 0);
}

void QView3DPlugin::addFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_ASSERT(formWindow != 0);
    Q_ASSERT(m_tool_list.contains(formWindow) == false);

    QView3DTool *tool = new QView3DTool(formWindow, this);
    m_tool_list[formWindow] = tool;
    connect(m_action, SIGNAL(triggered()), tool->action(), SLOT(trigger()));
    formWindow->registerTool(tool);
}

void QView3DPlugin::removeFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_ASSERT(formWindow != 0);
    Q_ASSERT(m_tool_list.contains(formWindow));

    QView3DTool *tool = m_tool_list.value(formWindow);
    m_tool_list.remove(formWindow);
    disconnect(m_action, SIGNAL(triggered()), tool->action(), SLOT(trigger()));

    delete tool;
}

Q_EXPORT_PLUGIN2(view3d, QView3DPlugin)
