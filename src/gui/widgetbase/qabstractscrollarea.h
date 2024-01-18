/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QABSTRACTSCROLLAREA_H
#define QABSTRACTSCROLLAREA_H

#include <qframe.h>

#ifndef QT_NO_SCROLLAREA

class QMargins;
class QScrollBar;
class QAbstractScrollAreaPrivate;

class Q_GUI_EXPORT QAbstractScrollArea : public QFrame
{
   GUI_CS_OBJECT(QAbstractScrollArea)

   GUI_CS_ENUM(SizeAdjustPolicy)

   GUI_CS_PROPERTY_READ(verticalScrollBarPolicy,  verticalScrollBarPolicy)
   GUI_CS_PROPERTY_WRITE(verticalScrollBarPolicy, setVerticalScrollBarPolicy)

   GUI_CS_PROPERTY_READ(horizontalScrollBarPolicy,  horizontalScrollBarPolicy)
   GUI_CS_PROPERTY_WRITE(horizontalScrollBarPolicy, setHorizontalScrollBarPolicy)

   GUI_CS_PROPERTY_READ(sizeAdjustPolicy,  sizeAdjustPolicy)
   GUI_CS_PROPERTY_WRITE(sizeAdjustPolicy, setSizeAdjustPolicy)

 public:
   GUI_CS_REGISTER_ENUM(
      enum SizeAdjustPolicy {
         AdjustIgnored,
         AdjustToContentsOnFirstShow,
         AdjustToContents
      };
   )

   explicit QAbstractScrollArea(QWidget *parent = nullptr);

   QAbstractScrollArea(const QAbstractScrollArea &) = delete;
   QAbstractScrollArea &operator=(const QAbstractScrollArea &) = delete;

   ~QAbstractScrollArea();

   Qt::ScrollBarPolicy verticalScrollBarPolicy() const;
   void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy);
   QScrollBar *verticalScrollBar() const;
   void setVerticalScrollBar(QScrollBar *scrollBar);

   Qt::ScrollBarPolicy horizontalScrollBarPolicy() const;
   void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy);
   QScrollBar *horizontalScrollBar() const;
   void setHorizontalScrollBar(QScrollBar *scrollBar);

   QWidget *cornerWidget() const;
   void setCornerWidget(QWidget *widget);

   void addScrollBarWidget(QWidget *widget, Qt::Alignment alignment);
   QWidgetList scrollBarWidgets(Qt::Alignment alignment);

   QWidget *viewport() const;
   void setViewport(QWidget *widget);
   QSize maximumViewportSize() const;

   QSize minimumSizeHint() const override;
   QSize sizeHint() const override;

   virtual void setupViewport(QWidget *viewport);
   SizeAdjustPolicy sizeAdjustPolicy() const;
   void setSizeAdjustPolicy(SizeAdjustPolicy policy);

 protected:
   QAbstractScrollArea(QAbstractScrollAreaPrivate &dd, QWidget *parent = nullptr);
   void setViewportMargins(int left, int top, int right, int bottom);
   void setViewportMargins(const QMargins &margins);
   QMargins viewportMargins() const;

   bool eventFilter(QObject *object, QEvent *event) override;
   bool event(QEvent *event) override;
   virtual bool viewportEvent(QEvent *event);

   void resizeEvent(QResizeEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void mouseDoubleClickEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QWheelEvent *event) override;
#endif

#ifndef QT_NO_CONTEXTMENU
   void contextMenuEvent(QContextMenuEvent *event) override;
#endif

#ifndef QT_NO_DRAGANDDROP
   void dragEnterEvent(QDragEnterEvent *event) override;
   void dragMoveEvent(QDragMoveEvent *event) override;
   void dragLeaveEvent(QDragLeaveEvent *event) override;
   void dropEvent(QDropEvent *event) override;
#endif

   void keyPressEvent(QKeyEvent *event) override;

   virtual void scrollContentsBy(int dx, int dy);
   virtual QSize viewportSizeHint() const;

 private:
   Q_DECLARE_PRIVATE(QAbstractScrollArea)

   GUI_CS_SLOT_1(Private, void _q_hslide(int x))
   GUI_CS_SLOT_2(_q_hslide)

   GUI_CS_SLOT_1(Private, void _q_vslide(int y))
   GUI_CS_SLOT_2(_q_vslide)

   GUI_CS_SLOT_1(Private, void _q_showOrHideScrollBars())
   GUI_CS_SLOT_2(_q_showOrHideScrollBars)

   friend class QStyleSheetStyle;
   friend class QWidgetPrivate;
};

#endif // QT_NO_SCROLLAREA

#endif