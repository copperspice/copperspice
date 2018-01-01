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

#ifndef QABSTRACTSCROLLAREA_P_H
#define QABSTRACTSCROLLAREA_P_H

#include <qframe_p.h>
#include <qabstractscrollarea.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SCROLLAREA

class QScrollBar;
class QAbstractScrollAreaScrollBarContainer;
class QBoxLayout;

class Q_GUI_EXPORT QAbstractScrollAreaPrivate: public QFramePrivate
{
   Q_DECLARE_PUBLIC(QAbstractScrollArea)

 public:
   QAbstractScrollAreaPrivate();

   void replaceScrollBar(QScrollBar *scrollBar, Qt::Orientation orientation);

   QAbstractScrollAreaScrollBarContainer *scrollBarContainers[Qt::Vertical + 1];
   QScrollBar *hbar, *vbar;
   Qt::ScrollBarPolicy vbarpolicy, hbarpolicy;

   QWidget *viewport;
   QWidget *cornerWidget;
   QRect cornerPaintingRect;

#ifdef Q_OS_MAC
   QRect reverseCornerPaintingRect;
#endif
   int left, top, right, bottom; // viewport margin

   int xoffset, yoffset;

   void init();
   void layoutChildren();

   // ### Fix for 4.4, talk to Bjoern E or Girish.
   virtual void scrollBarPolicyChanged(Qt::Orientation, Qt::ScrollBarPolicy) {}

   void _q_hslide(int);
   void _q_vslide(int);
   void _q_showOrHideScrollBars();

   virtual QPoint contentsOffset() const;

   inline bool viewportEvent(QEvent *event) {
      return q_func()->viewportEvent(event);
   }
   QScopedPointer<QObject> viewportFilter;

#ifdef Q_OS_WIN
   bool singleFingerPanEnabled;
   void setSingleFingerPanEnabled(bool on = true);
#endif
};

class QAbstractScrollAreaFilter : public QObject
{
   GUI_CS_OBJECT(QAbstractScrollAreaFilter)

 public:
   QAbstractScrollAreaFilter(QAbstractScrollAreaPrivate *p) : d(p) {
      setObjectName(QLatin1String("qt_abstractscrollarea_filter"));
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
   enum LogicalPosition { LogicalLeft = 1, LogicalRight = 2 };

   QAbstractScrollAreaScrollBarContainer(Qt::Orientation orientation, QWidget *parent);
   void addWidget(QWidget *widget, LogicalPosition position);
   QWidgetList widgets(LogicalPosition position);
   void removeWidget(QWidget *widget);

   QScrollBar *scrollBar;
   QBoxLayout *layout;

 private:
   int scrollBarLayoutIndex() const;

   Qt::Orientation orientation;
};

#endif // QT_NO_SCROLLAREA

QT_END_NAMESPACE

#endif // QABSTRACTSCROLLAREA_P_H
