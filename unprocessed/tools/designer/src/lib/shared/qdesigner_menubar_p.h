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

#ifndef QDESIGNER_MENUBAR_H
#define QDESIGNER_MENUBAR_H

#include "shared_global_p.h"

#include <QtGui/QAction>
#include <QtGui/QMenuBar>

#include <QtCore/QPointer>
#include <QtCore/QMimeData>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QDesignerActionProviderExtension;

class QLineEdit;
class QMimeData;

namespace qdesigner_internal {
class PromotionTaskMenu;

class SpecialMenuAction: public QAction
{
    Q_OBJECT
public:
    SpecialMenuAction(QObject *parent = 0);
    virtual ~SpecialMenuAction();
};

} // namespace qdesigner_internal

class QDESIGNER_SHARED_EXPORT QDesignerMenuBar: public QMenuBar
{
    Q_OBJECT
public:
    QDesignerMenuBar(QWidget *parent = 0);
    virtual ~QDesignerMenuBar();

    bool eventFilter(QObject *object, QEvent *event);

    QDesignerFormWindowInterface *formWindow() const;
    QDesignerActionProviderExtension *actionProvider();

    void adjustSpecialActions();
    bool interactive(bool i);
    bool dragging() const;

    void moveLeft(bool ctrl = false);
    void moveRight(bool ctrl = false);
    void moveUp();
    void moveDown();

    // Helpers for MenuTaskMenu/MenuBarTaskMenu extensions
    QList<QAction *> contextMenuActions();
    void deleteMenuAction(QAction *action);

private slots:
    void deleteMenu();
    void slotRemoveMenuBar();

protected:
    virtual void actionEvent(QActionEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void paintEvent(QPaintEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

    bool handleEvent(QWidget *widget, QEvent *event);
    bool handleMouseDoubleClickEvent(QWidget *widget, QMouseEvent *event);
    bool handleMousePressEvent(QWidget *widget, QMouseEvent *event);
    bool handleMouseReleaseEvent(QWidget *widget, QMouseEvent *event);
    bool handleMouseMoveEvent(QWidget *widget, QMouseEvent *event);
    bool handleContextMenuEvent(QWidget *widget, QContextMenuEvent *event);
    bool handleKeyPressEvent(QWidget *widget, QKeyEvent *event);

    void startDrag(const QPoint &pos);

    enum ActionDragCheck { NoActionDrag, ActionDragOnSubMenu, AcceptActionDrag };
    ActionDragCheck checkAction(QAction *action) const;

    void adjustIndicator(const QPoint &pos);
    int findAction(const QPoint &pos) const;

    QAction *currentAction() const;
    int realActionCount() const;

    enum LeaveEditMode {
        Default = 0,
        ForceAccept
    };

    void enterEditMode();
    void leaveEditMode(LeaveEditMode mode);
    void showLineEdit();

    void showMenu(int index = -1);
    void hideMenu(int index = -1);

    QAction *safeActionAt(int index) const;

    bool swapActions(int a, int b);

private:
    void updateCurrentAction(bool selectAction);
    void movePrevious(bool ctrl);
    void moveNext(bool ctrl);

    QAction *m_addMenu;
    QPointer<QMenu> m_activeMenu;
    QPoint m_startPosition;
    int m_currentIndex;
    bool m_interactive;
    QLineEdit *m_editor;
    bool m_dragging;
    int m_lastMenuActionIndex;
    QPointer<QWidget> m_lastFocusWidget;
    qdesigner_internal::PromotionTaskMenu* m_promotionTaskMenu;
};

QT_END_NAMESPACE

#endif // QDESIGNER_MENUBAR_H
