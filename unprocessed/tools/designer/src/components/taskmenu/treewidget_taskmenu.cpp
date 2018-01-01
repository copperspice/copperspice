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

#include "treewidget_taskmenu.h"
#include "treewidgeteditor.h"

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QLineEdit>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

using namespace qdesigner_internal;

TreeWidgetTaskMenu::TreeWidgetTaskMenu(QTreeWidget *button, QObject *parent)
    : QDesignerTaskMenu(button, parent),
      m_treeWidget(button),
      m_editItemsAction(new QAction(tr("Edit Items..."), this))
{
    connect(m_editItemsAction, SIGNAL(triggered()), this, SLOT(editItems()));
    m_taskActions.append(m_editItemsAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}


TreeWidgetTaskMenu::~TreeWidgetTaskMenu()
{
}

QAction *TreeWidgetTaskMenu::preferredEditAction() const
{
    return m_editItemsAction;
}

QList<QAction*> TreeWidgetTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void TreeWidgetTaskMenu::editItems()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_treeWidget);
    if (m_formWindow.isNull())
        return;

    Q_ASSERT(m_treeWidget != 0);

    TreeWidgetEditorDialog dlg(m_formWindow, m_treeWidget->window());
    TreeWidgetContents oldCont = dlg.fillContentsFromTreeWidget(m_treeWidget);
    if (dlg.exec() == QDialog::Accepted) {
        TreeWidgetContents newCont = dlg.contents();
        if (newCont != oldCont) {
            ChangeTreeContentsCommand *cmd = new ChangeTreeContentsCommand(m_formWindow);
            cmd->init(m_treeWidget, oldCont, newCont);
            m_formWindow->commandHistory()->push(cmd);
        }
    }
}

void TreeWidgetTaskMenu::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

QT_END_NAMESPACE
