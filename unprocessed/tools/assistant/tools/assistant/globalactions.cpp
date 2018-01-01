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

#include "globalactions.h"

#include "centralwidget.h"
#include "helpviewer.h"
#include "tracer.h"

#include <QtGui/QAction>
#include <QtGui/QMenu>

#if !defined(QT_NO_WEBKIT)
#include <QtWebKit/QWebHistory>
#endif

GlobalActions *GlobalActions::instance(QObject *parent)
{
    Q_ASSERT(!m_instance != !parent);
    if (!m_instance)
        m_instance = new GlobalActions(parent);
    return m_instance;
}

GlobalActions::GlobalActions(QObject *parent) : QObject(parent)
{
    TRACE_OBJ

    // TODO: Put resource path in misc class
    QString resourcePath = QLatin1String(":/trolltech/assistant/images/");
#ifdef Q_OS_MAC
    resourcePath.append(QLatin1String("mac"));
#else
    resourcePath.append(QLatin1String("win"));
#endif
    CentralWidget *centralWidget = CentralWidget::instance();

    m_backAction = new QAction(tr("&Back"), parent);
    m_backAction->setEnabled(false);
    m_backAction->setShortcuts(QKeySequence::Back);
    m_backAction->setIcon(QIcon(resourcePath + QLatin1String("/previous.png")));
    connect(m_backAction, SIGNAL(triggered()), centralWidget, SLOT(backward()));
    m_actionList << m_backAction;

    m_nextAction = new QAction(tr("&Forward"), parent);
    m_nextAction->setPriority(QAction::LowPriority);
    m_nextAction->setEnabled(false);
    m_nextAction->setShortcuts(QKeySequence::Forward);
    m_nextAction->setIcon(QIcon(resourcePath + QLatin1String("/next.png")));
    connect(m_nextAction, SIGNAL(triggered()), centralWidget, SLOT(forward()));
    m_actionList << m_nextAction;

    setupNavigationMenus(m_backAction, m_nextAction, centralWidget);

    m_homeAction = new QAction(tr("&Home"), parent);
    m_homeAction->setShortcut(tr("ALT+Home"));
    m_homeAction->setIcon(QIcon(resourcePath + QLatin1String("/home.png")));
    connect(m_homeAction, SIGNAL(triggered()), centralWidget, SLOT(home()));
    m_actionList << m_homeAction;

    QAction *separator = new QAction(parent);
    separator->setSeparator(true);
    m_actionList << separator;

    m_zoomInAction = new QAction(tr("Zoom &in"), parent);
    m_zoomInAction->setPriority(QAction::LowPriority);
    m_zoomInAction->setIcon(QIcon(resourcePath + QLatin1String("/zoomin.png")));
    m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(m_zoomInAction, SIGNAL(triggered()), centralWidget, SLOT(zoomIn()));
    m_actionList << m_zoomInAction;

    m_zoomOutAction = new QAction(tr("Zoom &out"), parent);
    m_zoomOutAction->setPriority(QAction::LowPriority);
    m_zoomOutAction->setIcon(QIcon(resourcePath + QLatin1String("/zoomout.png")));
    m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(m_zoomOutAction, SIGNAL(triggered()), centralWidget, SLOT(zoomOut()));
    m_actionList << m_zoomOutAction;

    separator = new QAction(parent);
    separator->setSeparator(true);
    m_actionList << separator;

    m_copyAction = new QAction(tr("&Copy selected Text"), parent);
    m_copyAction->setPriority(QAction::LowPriority);
    m_copyAction->setIconText("&Copy");
    m_copyAction->setIcon(QIcon(resourcePath + QLatin1String("/editcopy.png")));
    m_copyAction->setShortcuts(QKeySequence::Copy);
    m_copyAction->setEnabled(false);
    connect(m_copyAction, SIGNAL(triggered()), centralWidget, SLOT(copy()));
    m_actionList << m_copyAction;

    m_printAction = new QAction(tr("&Print..."), parent);
    m_printAction->setPriority(QAction::LowPriority);
    m_printAction->setIcon(QIcon(resourcePath + QLatin1String("/print.png")));
    m_printAction->setShortcut(QKeySequence::Print);
    connect(m_printAction, SIGNAL(triggered()), centralWidget, SLOT(print()));
    m_actionList << m_printAction;

    m_findAction = new QAction(tr("&Find in Text..."), parent);
    m_findAction->setIconText(tr("&Find"));
    m_findAction->setIcon(QIcon(resourcePath + QLatin1String("/find.png")));
    m_findAction->setShortcuts(QKeySequence::Find);
    connect(m_findAction, SIGNAL(triggered()), centralWidget, SLOT(showTextSearch()));
    m_actionList << m_findAction;

#ifdef Q_WS_X11
    m_backAction->setIcon(QIcon::fromTheme("go-previous" , m_backAction->icon()));
    m_nextAction->setIcon(QIcon::fromTheme("go-next" , m_nextAction->icon()));
    m_zoomInAction->setIcon(QIcon::fromTheme("zoom-in" , m_zoomInAction->icon()));
    m_zoomOutAction->setIcon(QIcon::fromTheme("zoom-out" , m_zoomOutAction->icon()));
    m_copyAction->setIcon(QIcon::fromTheme("edit-copy" , m_copyAction->icon()));
    m_findAction->setIcon(QIcon::fromTheme("edit-find" , m_findAction->icon()));
    m_homeAction->setIcon(QIcon::fromTheme("go-home" , m_homeAction->icon()));
    m_printAction->setIcon(QIcon::fromTheme("document-print" , m_printAction->icon()));
#endif
}

void GlobalActions::updateActions()
{
    TRACE_OBJ
    CentralWidget *centralWidget = CentralWidget::instance();
    m_copyAction->setEnabled(centralWidget->hasSelection());
    m_nextAction->setEnabled(centralWidget->isForwardAvailable());
    m_backAction->setEnabled(centralWidget->isBackwardAvailable());
}

void GlobalActions::setCopyAvailable(bool available)
{
    TRACE_OBJ
    m_copyAction->setEnabled(available);
}

#if !defined(QT_NO_WEBKIT)

void GlobalActions::slotAboutToShowBackMenu()
{
    TRACE_OBJ
    m_backMenu->clear();
    if (QWebHistory *history = CentralWidget::instance()->currentHelpViewer()->history()) {
        const int currentItemIndex = history->currentItemIndex();
        QList<QWebHistoryItem> items = history->backItems(history->count());
        for (int i = items.count() - 1; i >= 0; --i) {
            QAction *action = new QAction(this);
            action->setText(items.at(i).title());
            action->setData(-1 * (currentItemIndex - i));
            m_backMenu->addAction(action);
        }
    }
}

void GlobalActions::slotAboutToShowNextMenu()
{
    TRACE_OBJ
    m_nextMenu->clear();
    if (QWebHistory *history = CentralWidget::instance()->currentHelpViewer()->history()) {
        const int count = history->count();
        QList<QWebHistoryItem> items = history->forwardItems(count);
        for (int i = 0; i < items.count(); ++i) {
            QAction *action = new QAction(this);
            action->setData(count - i);
            action->setText(items.at(i).title());
            m_nextMenu->addAction(action);
        }
    }
}

void GlobalActions::slotOpenActionUrl(QAction *action)
{
    TRACE_OBJ
    if (HelpViewer* viewer = CentralWidget::instance()->currentHelpViewer()) {
        const int offset = action->data().toInt();
        QWebHistory *history = viewer->history();
        if (offset > 0) {
            history->goToItem(history->forwardItems(history->count()
                - offset + 1).back());  // forward
        } else if (offset < 0) {
            history->goToItem(history->backItems(-1 * offset).first()); // back
        }
    }
}

#endif

void GlobalActions::setupNavigationMenus(QAction *back, QAction *next,
    QWidget *parent)
{
#if !defined(QT_NO_WEBKIT)
    m_backMenu = new QMenu(parent);
    connect(m_backMenu, SIGNAL(aboutToShow()), this,
        SLOT(slotAboutToShowBackMenu()));
    connect(m_backMenu, SIGNAL(triggered(QAction*)), this,
        SLOT(slotOpenActionUrl(QAction*)));
    back->setMenu(m_backMenu);

    m_nextMenu = new QMenu(parent);
    connect(m_nextMenu, SIGNAL(aboutToShow()), this,
        SLOT(slotAboutToShowNextMenu()));
    connect(m_nextMenu, SIGNAL(triggered(QAction*)), this,
        SLOT(slotOpenActionUrl(QAction*)));
    next->setMenu(m_nextMenu);
#else
    Q_UNUSED(back)
    Q_UNUSED(next)
    Q_UNUSED(parent)
#endif
}

GlobalActions *GlobalActions::m_instance = 0;
