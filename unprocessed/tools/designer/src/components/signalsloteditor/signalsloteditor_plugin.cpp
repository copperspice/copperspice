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

#include "signalsloteditor_plugin.h"
#include "signalsloteditor_tool.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowManagerInterface>

#include <QtGui/QAction>

QT_BEGIN_NAMESPACE

using namespace qdesigner_internal;

SignalSlotEditorPlugin::SignalSlotEditorPlugin()
    : m_initialized(false), m_action(0)
{
}

SignalSlotEditorPlugin::~SignalSlotEditorPlugin()
{
}

bool SignalSlotEditorPlugin::isInitialized() const
{
    return m_initialized;
}

void SignalSlotEditorPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_ASSERT(!isInitialized());

    m_action = new QAction(tr("Edit Signals/Slots"), this);
    m_action->setObjectName(QLatin1String("__qt_edit_signals_slots_action"));
    m_action->setShortcut(tr("F4"));
    QIcon icon = QIcon::fromTheme("designer-edit-signals",
                                  QIcon(core->resourceLocation() + QLatin1String("/signalslottool.png")));
    m_action->setIcon(icon);
    m_action->setEnabled(false);

    setParent(core);
    m_core = core;
    m_initialized = true;

    connect(core->formWindowManager(), SIGNAL(formWindowAdded(QDesignerFormWindowInterface*)),
            this, SLOT(addFormWindow(QDesignerFormWindowInterface*)));

    connect(core->formWindowManager(), SIGNAL(formWindowRemoved(QDesignerFormWindowInterface*)),
            this, SLOT(removeFormWindow(QDesignerFormWindowInterface*)));

    connect(core->formWindowManager(), SIGNAL(activeFormWindowChanged(QDesignerFormWindowInterface*)),
                this, SLOT(activeFormWindowChanged(QDesignerFormWindowInterface*)));
}

QDesignerFormEditorInterface *SignalSlotEditorPlugin::core() const
{
    return m_core;
}

void SignalSlotEditorPlugin::addFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_ASSERT(formWindow != 0);
    Q_ASSERT(m_tools.contains(formWindow) == false);

    SignalSlotEditorTool *tool = new SignalSlotEditorTool(formWindow, this);
    connect(m_action, SIGNAL(triggered()), tool->action(), SLOT(trigger()));
    m_tools[formWindow] = tool;
    formWindow->registerTool(tool);
}

void SignalSlotEditorPlugin::removeFormWindow(QDesignerFormWindowInterface *formWindow)
{
    Q_ASSERT(formWindow != 0);
    Q_ASSERT(m_tools.contains(formWindow) == true);

    SignalSlotEditorTool *tool = m_tools.value(formWindow);
    m_tools.remove(formWindow);
    disconnect(m_action, SIGNAL(triggered()), tool->action(), SLOT(trigger()));
    // ### FIXME disable the tool

    delete tool;
}

QAction *SignalSlotEditorPlugin::action() const
{
    return m_action;
}

void SignalSlotEditorPlugin::activeFormWindowChanged(QDesignerFormWindowInterface *formWindow)
{
    m_action->setEnabled(formWindow != 0);
}

QT_END_NAMESPACE
