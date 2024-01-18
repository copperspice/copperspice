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

#ifndef QWINDOW_P_H
#define QWINDOW_P_H

#include <qscreen.h>
#include <qwindow.h>
#include <qplatform_window.h>

#include <qelapsedtimer.h>
#include <qicon.h>

#define QWINDOWSIZE_MAX ((1<<24)-1)

class Q_GUI_EXPORT QWindowPrivate
{
   Q_DECLARE_PUBLIC(QWindow)

 public:
   enum PositionPolicy {
      WindowFrameInclusive,
      WindowFrameExclusive
   };

   QWindowPrivate()
      : surfaceType(QWindow::RasterSurface), m_flags(Qt::Window), parentWindow(nullptr), platformWindow(nullptr)
      , visible(false), visibilityOnDestroy(false), exposed(false), windowState(Qt::WindowNoState)
      , visibility(QWindow::Hidden), resizeEventPending(true), receivedExpose(false), positionPolicy(WindowFrameExclusive)
      , positionAutomatic(true), contentOrientation(Qt::PrimaryOrientation), opacity(qreal(1.0))
      , minimumSize(0, 0), maximumSize(QWINDOWSIZE_MAX, QWINDOWSIZE_MAX), modality(Qt::NonModal)
      , blockedByModalWindow(false), updateRequestPending(false), updateTimer(0), transientParent(nullptr), topLevelScreen(nullptr)

#ifndef QT_NO_CURSOR
      , cursor(Qt::ArrowCursor)
      , hasCursor(false)
#endif

      , compositing(false) {
   }

   virtual ~QWindowPrivate() {
   }

   void init();

   void maybeQuitOnLastWindowClosed();

#ifndef QT_NO_CURSOR
   void setCursor(const QCursor *c = nullptr);
   bool applyCursor();
#endif

   void deliverUpdateRequest();

   QPoint globalPosition() const {
      Q_Q(const QWindow);

      QPoint offset = q->position();
      for (const QWindow *p = q->parent(); p; p = p->parent()) {
         offset += p->position();
      }
      return offset;
   }

   QWindow *topLevelWindow() const;

   virtual QWindow *eventReceiver() {
      Q_Q(QWindow);
      return q;
   }

   void updateVisibility();
   void _q_clearAlert();

   bool windowRecreationRequired(QScreen *newScreen) const;
   void create(bool recursive);
   void setTopLevelScreen(QScreen *newScreen, bool recreate);
   void connectToScreen(QScreen *topLevelScreen);
   void disconnectFromScreen();
   void emitScreenChangedRecursion(QScreen *newScreen);
   QScreen *screenForGeometry(const QRect &rect);

   virtual void clearFocusObject();
   virtual QRectF closestAcceptableGeometry(const QRectF &rect) const;

   bool isPopup() const {
      return (m_flags & Qt::WindowType_Mask) == Qt::Popup;
   }

   static QWindowPrivate *get(QWindow *window) {
      return window->d_func();
   }

   QWindow::SurfaceType surfaceType;
   Qt::WindowFlags m_flags;
   QWindow *parentWindow;
   QPlatformWindow *platformWindow;
   bool visible;
   bool visibilityOnDestroy;
   bool exposed;
   QSurfaceFormat requestedFormat;
   QString windowTitle;
   QString windowFilePath;
   QIcon windowIcon;
   QRect geometry;
   Qt::WindowState windowState;
   QWindow::Visibility visibility;
   bool resizeEventPending;
   bool receivedExpose;
   PositionPolicy positionPolicy;
   bool positionAutomatic;
   Qt::ScreenOrientation contentOrientation;
   qreal opacity;
   QRegion mask;

   QSize minimumSize;
   QSize maximumSize;
   QSize baseSize;
   QSize sizeIncrement;

   Qt::WindowModality modality;
   bool blockedByModalWindow;

   bool updateRequestPending;
   int updateTimer;

   QPointer<QWindow> transientParent;
   QPointer<QScreen> topLevelScreen;

#ifndef QT_NO_CURSOR
   QCursor cursor;
   bool hasCursor;
#endif

   bool compositing;
   QElapsedTimer lastComposeTime;

 protected:
   QWindow *q_ptr;
};

#endif
