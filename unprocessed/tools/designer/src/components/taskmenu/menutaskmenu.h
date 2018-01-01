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

#ifndef MENUTASKMENU_H
#define MENUTASKMENU_H

#include <QtDesigner/QDesignerTaskMenuExtension>

#include <qdesigner_menu_p.h>
#include <qdesigner_menubar_p.h>
#include <extensionfactory_p.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

    class PromotionTaskMenu;

// The QMenu task menu provides promotion and a remove option. The actual
// menu context options are not forwarded since they make only sense 
// when a menu is being edited/visible.

class MenuTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    explicit MenuTaskMenu(QDesignerMenu *menu, QObject *parent = 0);

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private slots:
    void removeMenu();

private:
    QDesignerMenu *m_menu;
    QAction *m_removeAction;
    PromotionTaskMenu *m_promotionTaskMenu;
};

// The QMenuBar task menu forwards the actions of QDesignerMenuBar,
// making them available in the object inspector.

class MenuBarTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    explicit MenuBarTaskMenu(QDesignerMenuBar *bar, QObject *parent = 0);

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private:
    QDesignerMenuBar *m_bar;
};

typedef ExtensionFactory<QDesignerTaskMenuExtension, QDesignerMenu, MenuTaskMenu> MenuTaskMenuFactory;
typedef ExtensionFactory<QDesignerTaskMenuExtension, QDesignerMenuBar, MenuBarTaskMenu> MenuBarTaskMenuFactory;

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // MENUTASKMENU_H
