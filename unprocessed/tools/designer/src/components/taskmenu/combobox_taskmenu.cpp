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

#include "combobox_taskmenu.h"
#include "listwidgeteditor.h"
#include "qdesigner_utils_p.h"
#include <qdesigner_command_p.h>

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QLineEdit>
#include <QtGui/QFontComboBox>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

using namespace qdesigner_internal;

ComboBoxTaskMenu::ComboBoxTaskMenu(QComboBox *button, QObject *parent)
    : QDesignerTaskMenu(button, parent),
      m_comboBox(button)
{
    m_editItemsAction = new QAction(this);
    m_editItemsAction->setText(tr("Edit Items..."));
    connect(m_editItemsAction, SIGNAL(triggered()), this, SLOT(editItems()));
    m_taskActions.append(m_editItemsAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}

ComboBoxTaskMenu::~ComboBoxTaskMenu()
{
}

QAction *ComboBoxTaskMenu::preferredEditAction() const
{
    return m_editItemsAction;
}

QList<QAction*> ComboBoxTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void ComboBoxTaskMenu::editItems()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_comboBox);
    if (m_formWindow.isNull())
        return;

    Q_ASSERT(m_comboBox != 0);

    ListWidgetEditor dlg(m_formWindow, m_comboBox->window());
    ListContents oldItems = dlg.fillContentsFromComboBox(m_comboBox);
    if (dlg.exec() == QDialog::Accepted) {
        ListContents items = dlg.contents();
        if (items != oldItems) {
            ChangeListContentsCommand *cmd = new ChangeListContentsCommand(m_formWindow);
            cmd->init(m_comboBox, oldItems, items);
            cmd->setText(tr("Change Combobox Contents"));
            m_formWindow->commandHistory()->push(cmd);
        }
    }
}

ComboBoxTaskMenuFactory::ComboBoxTaskMenuFactory(const QString &iid, QExtensionManager *extensionManager) :
    ExtensionFactory<QDesignerTaskMenuExtension, QComboBox, ComboBoxTaskMenu>(iid, extensionManager)
{
}

QComboBox *ComboBoxTaskMenuFactory::checkObject(QObject *qObject) const
{
    QComboBox *combo = qobject_cast<QComboBox*>(qObject);
    if (!combo)
        return 0;
    if (qobject_cast<QFontComboBox*>(combo))
        return 0;
    return combo;
}

void ComboBoxTaskMenu::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

QT_END_NAMESPACE
