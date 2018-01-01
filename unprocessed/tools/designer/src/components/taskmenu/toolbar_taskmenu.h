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

#ifndef TOOLBAR_TASKMENU_H
#define TOOLBAR_TASKMENU_H

#include <QtDesigner/QDesignerTaskMenuExtension>

#include <extensionfactory_p.h>

#include <QtGui/QToolBar>
#include <QtGui/QStatusBar>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {
    class PromotionTaskMenu;

// ToolBarTaskMenu forwards the actions of ToolBarEventFilter
class ToolBarTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    explicit ToolBarTaskMenu(QToolBar *tb, QObject *parent = 0);

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private:
    QToolBar *m_toolBar;
};

// StatusBarTaskMenu provides promotion and deletion
class StatusBarTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    explicit StatusBarTaskMenu(QStatusBar *tb, QObject *parent = 0);

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private slots:
    void removeStatusBar();

private:
    QStatusBar  *m_statusBar;
    QAction *m_removeAction;
    PromotionTaskMenu *m_promotionTaskMenu;
};

typedef ExtensionFactory<QDesignerTaskMenuExtension, QToolBar, ToolBarTaskMenu> ToolBarTaskMenuFactory;
typedef ExtensionFactory<QDesignerTaskMenuExtension, QStatusBar, StatusBarTaskMenu> StatusBarTaskMenuFactory;

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // TOOLBAR_TASKMENU_H
