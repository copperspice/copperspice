/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QABSTRACTSCROLLAREA_P_H
#define QABSTRACTSCROLLAREA_P_H

#include <qabstractscrollarea.h>

#include <qframe_p.h>

#ifndef QT_NO_SCROLLAREA

class QAbstractScrollAreaScrollBarContainer;
class QBoxLayout;
class QScrollBar;

class Q_GUI_EXPORT QAbstractScrollAreaPrivate: public QFramePrivate
{
   Q_DECLARE_PUBLIC(QAbstractScrollArea)

 public:
   QAbstractScrollAreaPrivate();
   ~QAbstractScrollAreaPrivate();

   bool canStartScrollingAt(const QPoint &startPos);

   virtual QPoint contentsOffset() const;

   void flashScrollBars();

   void init();
   void layoutChildren();

   void _q_hslide(int);
   void _q_vslide(int);
   void _q_showOrHideScrollBars();

   void replaceScrollBar(QScrollBar *scrollBar, Qt::Orientation orientation);

   virtual void scrollBarPolicyChanged(Qt::Orientation, Qt::ScrollBarPolicy)
   { }

   void setScrollBarTransient(QScrollBar *scrollBar, bool transient);

   bool viewportEvent(QEvent *event) {
      return q_func()->viewportEvent(event);
   }

   QAbstractScrollAreaScrollBarContainer *scrollBarContainers[Qt::Vertical + 1];

   QScrollBar *hbar;
   QScrollBar *vbar;

   Qt::ScrollBarPolicy vbarpolicy;
   Qt::ScrollBarPolicy hbarpolicy;

   bool shownOnce;
   bool inResize;
   mutable QSize sizeHint;
   QAbstractScrollArea::SizeAdjustPolicy sizeAdjustPolicy;

   QWidget *viewport;
   QWidget *cornerWidget;
   QRect cornerPaintingRect;

   // viewport margin
   int m_viewPort_left;
   int m_viewPort_top;
   int m_viewPort_right;
   int m_viewPort_bottom;

   int xoffset;
   int yoffset;
   QPoint overshoot;

   QScopedPointer<QObject> viewportFilter;
};

class QAbstractScrollAreaFilter : public QObject
{
   GUI_CS_OBJECT(QAbstractScrollAreaFilter)

 public:
   QAbstractScrollAreaFilter(QAbstractScrollAreaPrivate *p)
      : d(p)
   {
      setObjectName("qt_abstractscrollarea_filter");
   }

   bool eventFilter(QObject *o, QEvent *e) override {
      return (o == d->viewport ? d->viewportEvent(e) : false);
   }

 private:
   QAbstractScrollAreaPrivate *d;
};

class QAbstractScrollAreaScrollBarContainer : public QWidget
{
 public:
   enum LogicalPosition {
      LogicalLeft  = 1,
      LogicalRight = 2
   };

   QAbstractScrollAreaScrollBarContainer(Qt::Orientation orientation, QWidget *parent);
   void addWidget(QWidget *widget, LogicalPosition position);
   QWidgetList widgets(LogicalPosition position);
   void removeWidget(QWidget *widget);

   QScrollBar *scrollBar;
   QBoxLayout *layout;

 private:
   int scrollBarLayoutIndex() const;

   Qt::Orientation m_orientation;
};

#endif // QT_NO_SCROLLAREA

#endif
