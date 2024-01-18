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

#ifndef QWIDGETWINDOW_P_H
#define QWIDGETWINDOW_P_H

#include <qwindow.h>
#include <qevent_p.h>

class QCloseEvent;
class QMoveEvent;

class QWidgetWindow : public QWindow
{
   GUI_CS_OBJECT(QWidgetWindow)

 public:
   QWidgetWindow(QWidget *widget);
   ~QWidgetWindow();

   QWidget *widget() const {
      return m_widget;
   }

#ifndef QT_NO_ACCESSIBILITY
   QAccessibleInterface *accessibleRoot() const override;
#endif

   QObject *focusObject() const override;

 protected:
   bool event(QEvent *) override;

   void handleCloseEvent(QCloseEvent *);
   void handleEnterLeaveEvent(QEvent *);
   void handleFocusInEvent(QFocusEvent *);
   void handleKeyEvent(QKeyEvent *);
   void handleMouseEvent(QMouseEvent *);
   void handleNonClientAreaMouseEvent(QMouseEvent *);
   void handleTouchEvent(QTouchEvent *);
   void handleMoveEvent(QMoveEvent *);
   void handleResizeEvent(QResizeEvent *);

#ifndef QT_NO_WHEELEVENT
   void handleWheelEvent(QWheelEvent *);
#endif

#ifndef QT_NO_DRAGANDDROP
   void handleDragEnterMoveEvent(QDragMoveEvent *);
   void handleDragLeaveEvent(QDragLeaveEvent *);
   void handleDropEvent(QDropEvent *);
#endif

   void handleExposeEvent(QExposeEvent *);
   void handleWindowStateChangedEvent(QWindowStateChangeEvent *event);
   bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

#ifndef QT_NO_TABLETEVENT
   void handleTabletEvent(QTabletEvent *);
#endif

#ifndef QT_NO_GESTURES
   void handleGestureEvent(QNativeGestureEvent *);
#endif

#ifndef QT_NO_CONTEXTMENU
   void handleContextMenuEvent(QContextMenuEvent *);
#endif

 private:
   void repaintWindow();
   bool updateSize();
   bool updatePos();
   void updateMargins();
   void updateNormalGeometry();

   enum FocusWidgets {
      FirstFocusWidget,
      LastFocusWidget
   };
   QWidget *getFocusWidget(FocusWidgets fw);

   QPointer<QWidget> m_widget;
   QPointer<QWidget> m_implicit_mouse_grabber;

#ifndef QT_NO_DRAGANDDROP
   QPointer<QWidget> m_dragTarget;
#endif

   GUI_CS_SLOT_1(Private, void updateObjectName())
   GUI_CS_SLOT_2(updateObjectName)

   GUI_CS_SLOT_1(Private, void handleScreenChange())
   GUI_CS_SLOT_2(handleScreenChange)
};

#endif