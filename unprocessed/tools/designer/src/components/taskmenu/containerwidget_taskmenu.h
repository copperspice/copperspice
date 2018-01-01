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

#ifndef CONTAINERWIDGER_TASKMENU_H
#define CONTAINERWIDGER_TASKMENU_H

#include <qdesigner_taskmenu_p.h>
#include <shared_enums_p.h>

#include <extensionfactory_p.h>

#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QDesignerFormEditorInterface;
class QDesignerContainerExtension;
class QAction;
class QMdiArea;
class QWorkspace;
class QMenu;
class QWizard;

namespace qdesigner_internal {

class PromotionTaskMenu;

// ContainerWidgetTaskMenu: Task menu for containers with extension

class ContainerWidgetTaskMenu: public QDesignerTaskMenu
{
    Q_OBJECT
public:
    explicit ContainerWidgetTaskMenu(QWidget *widget, ContainerType type, QObject *parent = 0);
    virtual ~ContainerWidgetTaskMenu();

    virtual QAction *preferredEditAction() const;
    virtual QList<QAction*> taskActions() const;

private slots:
    void removeCurrentPage();
    void addPage();
    void addPageAfter();

protected:
    QDesignerContainerExtension *containerExtension() const;
    QList<QAction*> &containerActions() { return m_taskActions; }
    int pageCount() const;

private:
    QDesignerFormWindowInterface *formWindow() const;

private:
    static QString pageMenuText(ContainerType ct, int index, int count);
    bool canDeletePage() const;

    const ContainerType m_type;
    QWidget *m_containerWidget;
    QDesignerFormEditorInterface *m_core;
    PromotionTaskMenu *m_pagePromotionTaskMenu;
    QAction *m_pageMenuAction;
    QMenu *m_pageMenu;
    QList<QAction*> m_taskActions;
    QAction *m_actionDeletePage;
};

// WizardContainerWidgetTaskMenu: Provide next/back since QWizard
// has modes in which the "Back" button is not visible.

class WizardContainerWidgetTaskMenu : public ContainerWidgetTaskMenu {
    Q_OBJECT
public:
    explicit WizardContainerWidgetTaskMenu(QWizard *w, QObject *parent = 0);

    virtual QList<QAction*> taskActions() const;

private:
    QAction *m_nextAction;
    QAction *m_previousAction;
};


// MdiContainerWidgetTaskMenu: Provide tile/cascade for MDI containers in addition

class MdiContainerWidgetTaskMenu : public ContainerWidgetTaskMenu {
    Q_OBJECT
public:
    explicit MdiContainerWidgetTaskMenu(QMdiArea *m, QObject *parent = 0);
    explicit MdiContainerWidgetTaskMenu(QWorkspace *m, QObject *parent = 0);

    virtual QList<QAction*> taskActions() const;
private:
    void initializeActions();

    QAction *m_nextAction;
    QAction *m_previousAction;
    QAction *m_tileAction;
    QAction *m_cascadeAction;
};

class ContainerWidgetTaskMenuFactory: public QExtensionFactory
{
    Q_OBJECT
public:
    explicit ContainerWidgetTaskMenuFactory(QDesignerFormEditorInterface *core, QExtensionManager *extensionManager = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;

private:
    QDesignerFormEditorInterface *m_core;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // CONTAINERWIDGER_TASKMENU_H
