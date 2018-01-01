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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef ACTIONEDITOR_H
#define ACTIONEDITOR_H

#include "shared_global_p.h"
#include <QtDesigner/QDesignerActionEditorInterface>

#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QDesignerPropertyEditorInterface;
class QDesignerSettingsInterface;
class QMenu;
class QActionGroup;
class QSignalMapper;
class QItemSelection;
class QListWidget;
class QPushButton;
class QLineEdit;
class QToolButton;

namespace qdesigner_internal {

class ActionView;
class ResourceMimeData;

class QDESIGNER_SHARED_EXPORT ActionEditor: public QDesignerActionEditorInterface
{
    Q_OBJECT
public:
    explicit ActionEditor(QDesignerFormEditorInterface *core, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~ActionEditor();

    QDesignerFormWindowInterface *formWindow() const;
    virtual void setFormWindow(QDesignerFormWindowInterface *formWindow);

    virtual QDesignerFormEditorInterface *core() const;

    QAction *actionNew() const;
    QAction *actionDelete() const;

    QString filter() const;

    virtual void manageAction(QAction *action);
    virtual void unmanageAction(QAction *action);

    static QString actionTextToName(const QString &text, const QString &prefix = QLatin1String("action"));

    // Utility to create a configure button with menu for usage on toolbars
    static QToolButton *createConfigureMenuButton(const QString &t, QMenu **ptrToMenu);

public slots:
    void setFilter(const QString &filter);
    void mainContainerChanged();

private slots:
    void slotCurrentItemChanged(QAction *item);
    void slotSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void editAction(QAction *item);
    void editCurrentAction();
    void navigateToSlotCurrentAction();
    void slotActionChanged();
    void slotNewAction();
    void slotDelete();
    void resourceImageDropped(const QString &path, QAction *action);
    void slotContextMenuRequested(QContextMenuEvent *, QAction *);
    void slotViewMode(QAction *a);
    void slotSelectAssociatedWidget(QWidget *w);
    void slotCopy();
    void slotCut();
    void slotPaste();

signals:
    void itemActivated(QAction *item);
    // Context menu for item or global menu if item == 0.
    void contextMenuRequested(QMenu *menu, QAction *item);

private:
    typedef QList<QAction *> ActionList;
    void deleteActions(QDesignerFormWindowInterface *formWindow, const ActionList &);
    void copyActions(QDesignerFormWindowInterface *formWindow, const ActionList &);

    void restoreSettings();
    void saveSettings();

    void updateViewModeActions();

    QDesignerFormEditorInterface *m_core;
    QPointer<QDesignerFormWindowInterface> m_formWindow;
    QListWidget *m_actionGroups;

    ActionView *m_actionView;

    QAction *m_actionNew;
    QAction *m_actionEdit;
    QAction *m_actionNavigateToSlot;
    QAction *m_actionCopy;
    QAction *m_actionCut;
    QAction *m_actionPaste;
    QAction *m_actionSelectAll;
    QAction *m_actionDelete;

    QActionGroup *m_viewModeGroup;
    QAction *m_iconViewAction;
    QAction *m_listViewAction;

    QString m_filter;
    QWidget *m_filterWidget;
    QSignalMapper *m_selectAssociatedWidgetsMapper;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // ACTIONEDITOR_H
