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

#ifndef TABORDEREDITOR_H
#define TABORDEREDITOR_H

#include "tabordereditor_global.h"

#include <QtCore/QPointer>
#include <QtGui/QWidget>
#include <QtGui/QRegion>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>

QT_BEGIN_NAMESPACE

class QUndoStack;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class QT_TABORDEREDITOR_EXPORT TabOrderEditor : public QWidget
{
    Q_OBJECT

public:
    TabOrderEditor(QDesignerFormWindowInterface *form, QWidget *parent);

    QDesignerFormWindowInterface *formWindow() const;

public slots:
    void setBackground(QWidget *background);
    void updateBackground();
    void widgetRemoved(QWidget*);
    void initTabOrder();

private slots:
    void showTabOrderDialog();

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void contextMenuEvent(QContextMenuEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void showEvent(QShowEvent *e);

private:
    QRect indicatorRect(int index) const;
    int widgetIndexAt(const QPoint &pos) const;
    bool skipWidget(QWidget *w) const;

    QPointer<QDesignerFormWindowInterface> m_form_window;

    QWidgetList m_tab_order_list;

    QWidget *m_bg_widget;
    QUndoStack *m_undo_stack;
    QRegion m_indicator_region;

    QFontMetrics m_font_metrics;
    int m_current_index;
    bool m_beginning;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif
