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

#ifndef GLOBALACTION_H
#define GLOBALACTION_H

#include <QtCore/QList>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QAction;
class QMenu;

class GlobalActions : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(GlobalActions)
public:
    static GlobalActions *instance(QObject *parent = 0);

    QList<QAction *> actionList() const { return m_actionList; }
    QAction *backAction() const { return m_backAction; }
    QAction *nextAction() const { return m_nextAction; }
    QAction *homeAction() const { return m_homeAction; }
    QAction *zoomInAction() const { return m_zoomInAction; }
    QAction *zoomOutAction() const { return m_zoomOutAction; }
    QAction *copyAction() const { return m_copyAction; }
    QAction *printAction() const { return m_printAction; }
    QAction *findAction() const { return m_findAction; }

    Q_SLOT void updateActions();
    Q_SLOT void setCopyAvailable(bool available);

#if !defined(QT_NO_WEBKIT)
private slots:
    void slotAboutToShowBackMenu();
    void slotAboutToShowNextMenu();
    void slotOpenActionUrl(QAction *action);
#endif

private:
    void setupNavigationMenus(QAction *back, QAction *next, QWidget *parent);

private:
    GlobalActions(QObject *parent);

    static GlobalActions *m_instance;

    QAction *m_backAction;
    QAction *m_nextAction;
    QAction *m_homeAction;
    QAction *m_zoomInAction;
    QAction *m_zoomOutAction;
    QAction *m_copyAction;
    QAction *m_printAction;
    QAction *m_findAction;

    QList<QAction *> m_actionList;

    QMenu *m_backMenu;
    QMenu *m_nextMenu;
};

QT_END_NAMESPACE

#endif // GLOBALACTION_H
