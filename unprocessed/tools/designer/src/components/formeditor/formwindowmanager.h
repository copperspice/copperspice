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

#ifndef FORMWINDOWMANAGER_H
#define FORMWINDOWMANAGER_H

#include "formeditor_global.h"

#include <QtDesigner/private/qdesigner_formwindowmanager_p.h>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QPointer>
#include <QtCore/QMap>

QT_BEGIN_NAMESPACE

class QAction;
class QActionGroup;
class QUndoGroup;
class QDesignerFormEditorInterface;
class QDesignerWidgetBoxInterface;

namespace qdesigner_internal {

class FormWindow;
class PreviewManager;
class PreviewActionGroup;

class QT_FORMEDITOR_EXPORT FormWindowManager
    : public QDesignerFormWindowManager
{
    Q_OBJECT
public:
    explicit FormWindowManager(QDesignerFormEditorInterface *core, QObject *parent = 0);
    virtual ~FormWindowManager();

    virtual QDesignerFormEditorInterface *core() const;

    inline QAction *actionCut() const { return m_actionCut; }
    inline QAction *actionCopy() const { return m_actionCopy; }
    inline QAction *actionPaste() const { return m_actionPaste; }
    inline QAction *actionDelete() const { return m_actionDelete; }
    inline QAction *actionSelectAll() const { return m_actionSelectAll; }
    inline QAction *actionLower() const { return m_actionLower; }
    inline QAction *actionRaise() const { return m_actionRaise; }
    QAction *actionUndo() const;
    QAction *actionRedo() const;

    inline QAction *actionHorizontalLayout() const { return m_actionHorizontalLayout; }
    inline QAction *actionVerticalLayout() const { return m_actionVerticalLayout; }
    inline QAction *actionSplitHorizontal() const { return m_actionSplitHorizontal; }
    inline QAction *actionSplitVertical() const { return m_actionSplitVertical; }
    inline QAction *actionGridLayout() const { return m_actionGridLayout; }
    inline QAction *actionBreakLayout() const { return m_actionBreakLayout; }
    inline QAction *actionAdjustSize() const { return m_actionAdjustSize; }

    inline QAction *actionDefaultPreview() const { return m_actionDefaultPreview; }
    QActionGroup *actionGroupPreviewInStyle() const;
    virtual QAction *actionShowFormWindowSettingsDialog() const;

    QDesignerFormWindowInterface *activeFormWindow() const;

    int formWindowCount() const;
    QDesignerFormWindowInterface *formWindow(int index) const;

    QDesignerFormWindowInterface *createFormWindow(QWidget *parentWidget = 0, Qt::WindowFlags flags = 0);

    QPixmap createPreviewPixmap(QString *errorMessage);

    bool eventFilter(QObject *o, QEvent *e);

    void dragItems(const QList<QDesignerDnDItemInterface*> &item_list);

    QUndoGroup *undoGroup() const;

    virtual PreviewManager *previewManager() const { return m_previewManager; }

public slots:
    void addFormWindow(QDesignerFormWindowInterface *formWindow);
    void removeFormWindow(QDesignerFormWindowInterface *formWindow);
    void setActiveFormWindow(QDesignerFormWindowInterface *formWindow);
    void closeAllPreviews();
    void deviceProfilesChanged();

private slots:
    void slotActionCutActivated();
    void slotActionCopyActivated();
    void slotActionPasteActivated();
    void slotActionDeleteActivated();
    void slotActionSelectAllActivated();
    void slotActionLowerActivated();
    void slotActionRaiseActivated();
    void createLayout();
    void slotActionBreakLayoutActivated();
    void slotActionAdjustSizeActivated();
    void slotActionSimplifyLayoutActivated();
    void slotActionDefaultPreviewActivated();
    void slotActionGroupPreviewInStyle(const QString &style, int deviceProfileIndex);
    void slotActionShowFormWindowSettingsDialog();

    void slotUpdateActions();

private:
    void setupActions();
    FormWindow *findFormWindow(QWidget *w);
    QWidget *findManagedWidget(FormWindow *fw, QWidget *w);

    void setCurrentUndoStack(QUndoStack *stack);

private:
    enum CreateLayoutContext { LayoutContainer, LayoutSelection, MorphLayout };

    QDesignerFormEditorInterface *m_core;
    FormWindow *m_activeFormWindow;
    QList<FormWindow*> m_formWindows;

    PreviewManager *m_previewManager;

    /* Context of the layout actions and base for morphing layouts. Determined
     * in slotUpdateActions() and used later on in the action slots. */
    CreateLayoutContext m_createLayoutContext;
    QWidget *m_morphLayoutContainer;

    // edit actions
    QAction *m_actionCut;
    QAction *m_actionCopy;
    QAction *m_actionPaste;
    QAction *m_actionSelectAll;
    QAction *m_actionDelete;
    QAction *m_actionLower;
    QAction *m_actionRaise;
    // layout actions
    QAction *m_actionHorizontalLayout;
    QAction *m_actionVerticalLayout;
    QAction *m_actionSplitHorizontal;
    QAction *m_actionSplitVertical;
    QAction *m_actionGridLayout;
    QAction *m_actionBreakLayout;
    QAction *m_actionAdjustSize;
    // preview actions
    QAction *m_actionDefaultPreview;
    mutable PreviewActionGroup *m_actionGroupPreviewInStyle;
    QAction *m_actionShowFormWindowSettingsDialog;

    QAction *m_actionUndo;
    QAction *m_actionRedo;

    QMap<QWidget *,bool> getUnsortedLayoutsToBeBroken(bool firstOnly) const;
    bool hasLayoutsToBeBroken() const;
    QWidgetList layoutsToBeBroken(QWidget *w) const;
    QWidgetList layoutsToBeBroken() const;

    QUndoGroup *m_undoGroup;

};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // FORMWINDOWMANAGER_H
