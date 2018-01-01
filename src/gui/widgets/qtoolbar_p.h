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

#ifndef QTOOLBAR_P_H
#define QTOOLBAR_P_H

#include <qtoolbar.h>
#include <QtGui/qaction.h>
#include <qwidget_p.h>
#include <QtCore/qbasictimer.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_TOOLBAR

class QToolBarLayout;
class QTimer;

class QToolBarPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QToolBar)

 public:
   inline QToolBarPrivate()
      : explicitIconSize(false), explicitToolButtonStyle(false), movable(true), floatable(true),
        allowedAreas(Qt::AllToolBarAreas), orientation(Qt::Horizontal),
        toolButtonStyle(Qt::ToolButtonIconOnly),
        layout(0), state(0)
#ifdef Q_OS_MAC
        , macWindowDragging(false)
#endif
   { }

   void init();
   void actionTriggered();
   void _q_toggleView(bool b);
   void _q_updateIconSize(const QSize &sz);
   void _q_updateToolButtonStyle(Qt::ToolButtonStyle style);

   bool explicitIconSize;
   bool explicitToolButtonStyle;
   bool movable;
   bool floatable;
   Qt::ToolBarAreas allowedAreas;
   Qt::Orientation orientation;
   Qt::ToolButtonStyle toolButtonStyle;
   QSize iconSize;

   QAction *toggleViewAction;

   QToolBarLayout *layout;

   struct DragState {
      QPoint pressPos;
      bool dragging;
      bool moving;
      QLayoutItem *widgetItem;
   };
   DragState *state;

#ifdef Q_OS_MAC
   bool macWindowDragging;
   QPoint macWindowDragPressPosition;
#endif

   bool mousePressEvent(QMouseEvent *e);
   bool mouseReleaseEvent(QMouseEvent *e);
   bool mouseMoveEvent(QMouseEvent *e);

   void updateWindowFlags(bool floating, bool unplug = false);
   void setWindowState(bool floating, bool unplug = false, const QRect &rect = QRect());
   void initDrag(const QPoint &pos);
   void startDrag(bool moving = false);
   void endDrag();

   void unplug(const QRect &r);
   void plug(const QRect &r);

   QBasicTimer waitForPopupTimer;
};

#endif // QT_NO_TOOLBAR

QT_END_NAMESPACE

#endif // QDYNAMICTOOLBAR_P_H
