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

#ifndef QDESIGNER_TOOLBAR_H
#define QDESIGNER_TOOLBAR_H

#include "shared_global_p.h"

#include <QtGui/QAction>
#include <QtGui/QToolButton>

#include <QtCore/QList>
#include <QtCore/QPoint>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QToolBar;
class QRect;
class QAction;

namespace qdesigner_internal {

class PromotionTaskMenu;

// Special event filter for  tool bars in designer.
// Handles drag and drop to and from. Ensures that each
// child widget is  WA_TransparentForMouseEvents to enable  drag and drop.

class QDESIGNER_SHARED_EXPORT ToolBarEventFilter : public QObject {
    Q_OBJECT

public:
    static void install(QToolBar *tb);

    // Find action by position. Note that QToolBar::actionAt() will
    // not work as designer sets WA_TransparentForMouseEvents on its tool bar buttons
    // to be able to drag them. This function will return the dummy
    // sentinel action when applied to tool bars created by designer if the position matches.
    static QAction *actionAt(const QToolBar *tb, const QPoint &pos);

    static bool withinHandleArea(const QToolBar *tb, const QPoint &pos);
    static QRect handleArea(const QToolBar *tb);
    static QRect freeArea(const QToolBar *tb);

    // Utility to create an action
    static QAction *createAction(QDesignerFormWindowInterface *fw, const QString &objectName, bool separator);

    virtual bool eventFilter (QObject *watched, QEvent *event);

    // Helper for task menu extension
    QList<QAction *> contextMenuActions(const QPoint &globalPos = QPoint(-1, -1));

    static ToolBarEventFilter *eventFilterOf(const QToolBar *tb);

private slots:
    void slotRemoveSelectedAction();
    void slotRemoveToolBar();
    void slotInsertSeparator();

private:
    explicit ToolBarEventFilter(QToolBar *tb);

    bool handleContextMenuEvent(QContextMenuEvent * event);
    bool handleDragEnterMoveEvent(QDragMoveEvent *event);
    bool handleDragLeaveEvent(QDragLeaveEvent *);
    bool handleDropEvent(QDropEvent *event);
    bool handleMousePressEvent(QMouseEvent *event);
    bool handleMouseReleaseEvent(QMouseEvent *event);
    bool handleMouseMoveEvent(QMouseEvent *event);

    QDesignerFormWindowInterface *formWindow() const;
    void adjustDragIndicator(const QPoint &pos);
    void hideDragIndicator();
    void startDrag(const QPoint &pos, Qt::KeyboardModifiers modifiers);
    bool withinHandleArea(const QPoint &pos) const;

    QToolBar *m_toolBar;
    PromotionTaskMenu *m_promotionTaskMenu;
    QPoint m_startPosition;
};
} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QDESIGNER_TOOLBAR_H
