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

#ifndef QTOOLBAR_P_H
#define QTOOLBAR_P_H

#include <qtoolbar.h>
#include <qaction.h>
#include <qwidget_p.h>
#include <qbasictimer.h>

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
        toolButtonStyle(Qt::ToolButtonIconOnly), layout(nullptr), state(nullptr)
#ifdef Q_OS_DARWIN
      , macWindowDragging(false)
#endif
   {
   }

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

#ifdef Q_OS_DARWIN
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

#endif // QDYNAMICTOOLBAR_P_H
