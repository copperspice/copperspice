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

#ifndef WIDGETSELECTION_H
#define WIDGETSELECTION_H

#include "formeditor_global.h"
#include <invisible_widget_p.h>

#include <QtCore/QHash>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QMouseEvent;
class QPaintEvent;

namespace qdesigner_internal {

class FormWindow;
class WidgetSelection;

class QT_FORMEDITOR_EXPORT WidgetHandle: public InvisibleWidget
{
    Q_OBJECT
public:
    enum Type
    {
        LeftTop,
        Top,
        RightTop,
        Right,
        RightBottom,
        Bottom,
        LeftBottom,
        Left,

        TypeCount
    };

    WidgetHandle(FormWindow *parent, Type t, WidgetSelection *s);
    void setWidget(QWidget *w);
    void setActive(bool a);
    void updateCursor();

    void setEnabled(bool) {}

    QDesignerFormEditorInterface *core() const;

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

private:
    void changeGridLayoutItemSpan();
    void changeFormLayoutItemSpan();
    void trySetGeometry(QWidget *w, int x, int y, int width, int height);
    void tryResize(QWidget *w, int width, int height);

private:
    QWidget *m_widget;
    const Type m_type;
    QPoint m_origPressPos;
    FormWindow *m_formWindow;
    WidgetSelection *m_sel;
    QRect m_geom, m_origGeom;
    bool m_active;
};

class QT_FORMEDITOR_EXPORT WidgetSelection: public QObject
{
    Q_OBJECT
public:
    WidgetSelection(FormWindow *parent);

    void setWidget(QWidget *w);
    bool isUsed() const;

    void updateActive();
    void updateGeometry();
    void hide();
    void show();
    void update();

    QWidget *widget() const;

    QDesignerFormEditorInterface *core() const;

    virtual bool eventFilter(QObject *object, QEvent *event);

    enum  WidgetState { UnlaidOut, LaidOut, ManagedGridLayout, ManagedFormLayout };
    static WidgetState widgetState(const QDesignerFormEditorInterface *core, QWidget *w);

private:
    WidgetHandle *m_handles[WidgetHandle::TypeCount];
    QPointer<QWidget> m_widget;
    FormWindow *m_formWindow;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // WIDGETSELECTION_H
