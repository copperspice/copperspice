/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QABSTRACTSCROLLAREA_H
#define QABSTRACTSCROLLAREA_H

#include <QtGui/qframe.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SCROLLAREA

class QMargins;
class QScrollBar;
class QAbstractScrollAreaPrivate;

class Q_GUI_EXPORT QAbstractScrollArea : public QFrame
{
    CS_OBJECT(QAbstractScrollArea)

    GUI_CS_PROPERTY_READ(verticalScrollBarPolicy, verticalScrollBarPolicy)
    GUI_CS_PROPERTY_WRITE(verticalScrollBarPolicy, setVerticalScrollBarPolicy)
    GUI_CS_PROPERTY_READ(horizontalScrollBarPolicy, horizontalScrollBarPolicy)
    GUI_CS_PROPERTY_WRITE(horizontalScrollBarPolicy, setHorizontalScrollBarPolicy)

public:
    explicit QAbstractScrollArea(QWidget* parent=0);
    ~QAbstractScrollArea();

    Qt::ScrollBarPolicy verticalScrollBarPolicy() const;
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy);
    QScrollBar *verticalScrollBar() const;
    void setVerticalScrollBar(QScrollBar *scrollbar);

    Qt::ScrollBarPolicy horizontalScrollBarPolicy() const;
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy);
    QScrollBar *horizontalScrollBar() const;
    void setHorizontalScrollBar(QScrollBar *scrollbar);

    QWidget *cornerWidget() const;
    void setCornerWidget(QWidget *widget);

    void addScrollBarWidget(QWidget *widget, Qt::Alignment alignment);
    QWidgetList scrollBarWidgets(Qt::Alignment alignment);

    QWidget *viewport() const;
    void setViewport(QWidget *widget);
    QSize maximumViewportSize() const;

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected :
    GUI_CS_SLOT_1(Protected, void setupViewport(QWidget * viewport))
    GUI_CS_SLOT_2(setupViewport) 

    QAbstractScrollArea(QAbstractScrollAreaPrivate &dd, QWidget *parent = 0);
    void setViewportMargins(int left, int top, int right, int bottom);
    void setViewportMargins(const QMargins &margins);

    bool event(QEvent *);
    virtual bool viewportEvent(QEvent *);

    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *);
#endif

#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent *);
#endif

#ifndef QT_NO_DRAGANDDROP
    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void dragLeaveEvent(QDragLeaveEvent *);
    void dropEvent(QDropEvent *);
#endif

    void keyPressEvent(QKeyEvent *);

    virtual void scrollContentsBy(int dx, int dy);

private:
    Q_DECLARE_PRIVATE(QAbstractScrollArea)
    Q_DISABLE_COPY(QAbstractScrollArea)

    GUI_CS_SLOT_1(Private, void _q_hslide(int un_named_arg1))
    GUI_CS_SLOT_2(_q_hslide)

    GUI_CS_SLOT_1(Private, void _q_vslide(int un_named_arg1))
    GUI_CS_SLOT_2(_q_vslide)

    GUI_CS_SLOT_1(Private, void _q_showOrHideScrollBars())
    GUI_CS_SLOT_2(_q_showOrHideScrollBars)

    friend class QStyleSheetStyle;
    friend class QWidgetPrivate;
};

#endif // QT_NO_SCROLLAREA

QT_END_NAMESPACE

#endif // QABSTRACTSCROLLAREA_H
