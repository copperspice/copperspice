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

#include "menutaskmenu.h"

#include <promotiontaskmenu_p.h>

#include <QtGui/QAction>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {
    // ------------ MenuTaskMenu
    MenuTaskMenu::MenuTaskMenu(QDesignerMenu *menu, QObject *parent) :
       QObject(parent),
       m_menu(menu),
       m_removeAction(new QAction(tr("Remove"), this)),
       m_promotionTaskMenu(new PromotionTaskMenu(menu, PromotionTaskMenu::ModeSingleWidget, this))
    {
        connect(m_removeAction, SIGNAL(triggered()), this, SLOT(removeMenu()));
    }

    QAction *MenuTaskMenu::preferredEditAction() const
    {
        return 0;
    }

    QList<QAction*> MenuTaskMenu::taskActions() const
    {
        QList<QAction*> rc;
        rc.push_back(m_removeAction);
        m_promotionTaskMenu->addActions(PromotionTaskMenu::LeadingSeparator, rc);
        return rc;
    }

    void MenuTaskMenu::removeMenu()
    {
        // Are we on a menu bar or on a menu?
        QWidget *pw = m_menu->parentWidget();
        if (QDesignerMenuBar *mb = qobject_cast<QDesignerMenuBar *>(pw)) {
            mb->deleteMenuAction(m_menu->menuAction());
            return;
        }
        if (QDesignerMenu *m = qobject_cast<QDesignerMenu *>(pw)) {
            m->deleteAction(m_menu->menuAction());
        }
    }

    // ------------- MenuBarTaskMenu
    MenuBarTaskMenu::MenuBarTaskMenu(QDesignerMenuBar *bar, QObject *parent) :
        QObject(parent),
        m_bar(bar)
    {
    }

    QAction *MenuBarTaskMenu::preferredEditAction() const
    {
        return 0;
    }

    QList<QAction*> MenuBarTaskMenu::taskActions() const
    {
        return m_bar->contextMenuActions();
    }
}

QT_END_NAMESPACE

