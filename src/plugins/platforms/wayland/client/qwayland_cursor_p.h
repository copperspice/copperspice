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

#ifndef QWAYLAND_CURSOR_H
#define QWAYLAND_CURSOR_H

#include <qcursor.h>
#include <qmap.h>
#include <qpoint.h>
#include <qplatform_cursor.h>
#include <qwindow.h>

struct wl_cursor;
struct wl_cursor_image;
struct wl_cursor_theme;

namespace QtWaylandClient {

class QWaylandBuffer;
class QWaylandDisplay;
class QWaylandScreen;

class Q_WAYLAND_CLIENT_EXPORT QWaylandCursor : public QPlatformCursor
{
 public:
   QWaylandCursor(QWaylandScreen *screen);
   ~QWaylandCursor();

   void changeCursor(QCursor *cursor, QWindow *window) override;
   void pointerEvent(const QMouseEvent &event) override;
   QPoint pos() const override;
   void setPos(const QPoint &pos) override;

   struct wl_cursor_image *cursorImage(Qt::CursorShape shape);
   QSharedPointer<QWaylandBuffer> cursorBitmapImage(const QCursor *cursor);

 private:
   enum WaylandCursor {
      ArrowCursor = Qt::ArrowCursor,
      UpArrowCursor,
      CrossCursor,
      WaitCursor,
      IBeamCursor,
      SizeVerCursor,
      SizeHorCursor,
      SizeBDiagCursor,
      SizeFDiagCursor,
      SizeAllCursor,
      BlankCursor,
      SplitVCursor,
      SplitHCursor,
      PointingHandCursor,
      ForbiddenCursor,
      WhatsThisCursor,
      BusyCursor,
      OpenHandCursor,
      ClosedHandCursor,
      DragCopyCursor,
      DragMoveCursor,
      DragLinkCursor,
      ResizeNorthCursor = Qt::CustomCursor + 1,
      ResizeSouthCursor,
      ResizeEastCursor,
      ResizeWestCursor,
      ResizeNorthWestCursor,
      ResizeSouthEastCursor,
      ResizeNorthEastCursor,
      ResizeSouthWestCursor
   };

   void initCursorMap();
   wl_cursor *requestCursor(WaylandCursor shape);

   QWaylandDisplay *mDisplay;
   struct wl_cursor_theme *mCursorTheme;
   QPoint mLastPos;

   QMap<WaylandCursor, wl_cursor *> mCursors;
   QMultiMap<WaylandCursor, QByteArray> mCursorNamesMap;
};

}

#endif
