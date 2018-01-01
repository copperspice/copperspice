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

#include "tablewidget_taskmenu.h"
#include "tablewidgeteditor.h"

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtGui/QTableWidget>
#include <QtGui/QAction>
#include <QtGui/QLineEdit>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

using namespace qdesigner_internal;

TableWidgetTaskMenu::TableWidgetTaskMenu(QTableWidget *button, QObject *parent)
    : QDesignerTaskMenu(button, parent),
      m_tableWidget(button),
      m_editItemsAction(new QAction(tr("Edit Items..."), this))
{
    connect(m_editItemsAction, SIGNAL(triggered()), this, SLOT(editItems()));
    m_taskActions.append(m_editItemsAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}


TableWidgetTaskMenu::~TableWidgetTaskMenu()
{
}

QAction *TableWidgetTaskMenu::preferredEditAction() const
{
    return m_editItemsAction;
}

QList<QAction*> TableWidgetTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void TableWidgetTaskMenu::editItems()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_tableWidget);
    if (m_formWindow.isNull())
        return;

    Q_ASSERT(m_tableWidget != 0);

    TableWidgetEditorDialog dlg(m_formWindow, m_tableWidget->window());
    TableWidgetContents oldCont = dlg.fillContentsFromTableWidget(m_tableWidget);
    if (dlg.exec() == QDialog::Accepted) {
        TableWidgetContents newCont = dlg.contents();
        if (newCont != oldCont) {
            ChangeTableContentsCommand *cmd = new ChangeTableContentsCommand(m_formWindow);
            cmd->init(m_tableWidget, oldCont, newCont);
            m_formWindow->commandHistory()->push(cmd);
        }
    }
}

void TableWidgetTaskMenu::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

QT_END_NAMESPACE
