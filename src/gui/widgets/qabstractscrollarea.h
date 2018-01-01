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
   GUI_CS_OBJECT(QAbstractScrollArea)

   GUI_CS_PROPERTY_READ(verticalScrollBarPolicy, verticalScrollBarPolicy)
   GUI_CS_PROPERTY_WRITE(verticalScrollBarPolicy, setVerticalScrollBarPolicy)
   GUI_CS_PROPERTY_READ(horizontalScrollBarPolicy, horizontalScrollBarPolicy)
   GUI_CS_PROPERTY_WRITE(horizontalScrollBarPolicy, setHorizontalScrollBarPolicy)

 public:
   explicit QAbstractScrollArea(QWidget *parent = nullptr);
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

   QSize minimumSizeHint() const override;
   QSize sizeHint() const override;

 protected :
   GUI_CS_SLOT_1(Protected, void setupViewport(QWidget *viewport))
   GUI_CS_SLOT_2(setupViewport)

   QAbstractScrollArea(QAbstractScrollAreaPrivate &dd, QWidget *parent = nullptr);
   void setViewportMargins(int left, int top, int right, int bottom);
   void setViewportMargins(const QMargins &margins);

   bool event(QEvent *) override;
   virtual bool viewportEvent(QEvent *);

   void resizeEvent(QResizeEvent *) override;
   void paintEvent(QPaintEvent *) override;
   void mousePressEvent(QMouseEvent *) override;
   void mouseReleaseEvent(QMouseEvent *) override;
   void mouseDoubleClickEvent(QMouseEvent *) override;
   void mouseMoveEvent(QMouseEvent *) override;

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QWheelEvent *) override;
#endif

#ifndef QT_NO_CONTEXTMENU
   void contextMenuEvent(QContextMenuEvent *) override;
#endif

#ifndef QT_NO_DRAGANDDROP
   void dragEnterEvent(QDragEnterEvent *) override;
   void dragMoveEvent(QDragMoveEvent *) override;
   void dragLeaveEvent(QDragLeaveEvent *) override;
   void dropEvent(QDropEvent *) override;
#endif

   void keyPressEvent(QKeyEvent *) override;

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
