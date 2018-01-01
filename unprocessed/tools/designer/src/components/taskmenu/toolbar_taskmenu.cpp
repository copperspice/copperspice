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

#include "toolbar_taskmenu.h"
#include "qdesigner_toolbar_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>

#include <promotiontaskmenu_p.h>
#include <qdesigner_command_p.h>

#include <QtGui/QAction>
#include <QtGui/QUndoStack>

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {
    // ------------ ToolBarTaskMenu
    ToolBarTaskMenu::ToolBarTaskMenu(QToolBar *tb, QObject *parent) :
        QObject(parent),
        m_toolBar(tb)
    {
    }

    QAction *ToolBarTaskMenu::preferredEditAction() const
    {
        return 0;
    }

    QList<QAction*> ToolBarTaskMenu::taskActions() const
    {
        if (ToolBarEventFilter *ef = ToolBarEventFilter::eventFilterOf(m_toolBar))
            return ef->contextMenuActions();
        return QList<QAction*>();
    }

    // ------------ StatusBarTaskMenu
    StatusBarTaskMenu::StatusBarTaskMenu(QStatusBar *sb, QObject *parent) :
        QObject(parent),
        m_statusBar(sb),
        m_removeAction(new QAction(tr("Remove"), this)),
        m_promotionTaskMenu(new PromotionTaskMenu(sb, PromotionTaskMenu::ModeSingleWidget, this))
    {
        connect(m_removeAction, SIGNAL(triggered()), this, SLOT(removeStatusBar()));
    }

    QAction *StatusBarTaskMenu::preferredEditAction() const
    {
        return 0;
    }

    QList<QAction*> StatusBarTaskMenu::taskActions() const
    {
        QList<QAction*> rc;
        rc.push_back(m_removeAction);
        m_promotionTaskMenu->addActions(PromotionTaskMenu::LeadingSeparator, rc);
        return rc;
    }

    void StatusBarTaskMenu::removeStatusBar()
    {
        if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(m_statusBar)) {
            DeleteStatusBarCommand *cmd = new DeleteStatusBarCommand(fw);
            cmd->init(m_statusBar);
            fw->commandHistory()->push(cmd);
        }
    }
}

QT_END_NAMESPACE

