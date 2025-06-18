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

#include <qwayland_cursor_p.h>

#include <qdebug.h>
#include <qimagereader.h>

#include <wayland-cursor.h>

namespace QtWaylandClient {

QWaylandCursor::QWaylandCursor(QWaylandScreen *screen)
{
   // TODO: Make wl_cursor_theme_load arguments configurable here
   QByteArray cursorTheme = qgetenv("XCURSOR_THEME");

   if (cursorTheme.isEmpty()) {
      cursorTheme = QByteArray("default");
   }

   QByteArray cursorSizeFromEnv = qgetenv("XCURSOR_SIZE");
   bool hasCursorSize = false;
   int cursorSize = cursorSizeFromEnv.toInt(&hasCursorSize);

   if (! hasCursorSize || cursorSize <= 0) {
      cursorSize = 32;
   }

   mCursorTheme = nullptr;

#if defined(CS_SHOW_DEBUG_PLATFORM)
   if (mCursorTheme == nullptr) {
      qDebug() << "Unable to load theme" << cursorTheme;
   }
#endif
   initCursorMap();
}

QWaylandCursor::~QWaylandCursor()
{
   if (mCursorTheme != nullptr) {
      wl_cursor_theme_destroy(mCursorTheme);
   }
}

struct wl_cursor_image *QWaylandCursor::cursorImage(Qt::CursorShape newShape)
{
   struct wl_cursor *waylandCursor = nullptr;

   if (newShape < Qt::BitmapCursor) {
      waylandCursor = requestCursor((WaylandCursor)newShape);

   } else if (newShape == Qt::BitmapCursor) {
      // can not create a wl_cursor_image for a CursorShape
      return nullptr;

   } else {
      // TODO: Custom cursor logic (for resize arrows)
   }

   if (waylandCursor == nullptr) {
#if defined(CS_SHOW_DEBUG_PLATFORM)
      qDebug("Unable to not find cursor for shape %d", newShape);
#endif

      return nullptr;
   }

   struct wl_cursor_image *image = waylandCursor->images[0];

   struct wl_buffer *buffer = wl_cursor_image_get_buffer(image);

   if (buffer == nullptr) {
#if defined(CS_SHOW_DEBUG_PLATFORM)
      qDebug("Unable to find buffer for cursor");
#endif

      return nullptr;
   }

   return image;
}

QSharedPointer<QWaylandBuffer> QWaylandCursor::cursorBitmapImage(const QCursor *cursor)
{
   if (cursor->shape() != Qt::BitmapCursor) {
      return QSharedPointer<QWaylandBuffer>();
   }

   const QImage &img = cursor->pixmap().toImage();

   QSharedPointer<QWaylandBuffer> buffer;

   return buffer;
}

void QWaylandCursor::changeCursor(QCursor *cursor, QWindow *window)
{
   // pending implementation
}

void QWaylandCursor::pointerEvent(const QMouseEvent &event)
{
   mLastPos = event.globalPos();
}

QPoint QWaylandCursor::pos() const
{
   return mLastPos;
}

void QWaylandCursor::setPos(const QPoint &pos)
{
   (void) pos;
   qWarning("QWaylandCursor::setPos() Wayland does not support setting the cursor position");
}

wl_cursor *QWaylandCursor::requestCursor(WaylandCursor shape)
{
   struct wl_cursor *cursor = mCursors.value(shape, nullptr);

   // If the cursor has not been loaded already, load it
   if (! cursor) {
      if (! mCursorTheme) {
         return nullptr;
      }

      QList < QByteArray > cursorNames = mCursorNamesMap.values(shape);

      for (const QByteArray &name : cursorNames) {
         cursor = wl_cursor_theme_get_cursor(mCursorTheme, name.constData());

         if (cursor) {
            mCursors.insert(shape, cursor);
            break;
         }
      }
   }

   // If there still no cursor for a shape, use the default cursor
   if (! cursor && shape != ArrowCursor) {
      cursor = requestCursor(ArrowCursor);
   }

   return cursor;
}

void QWaylandCursor::initCursorMap()
{
   // Fill the cursor name map will the table of xcursor names
   mCursorNamesMap.insert(ArrowCursor, "left_ptr");
   mCursorNamesMap.insert(ArrowCursor, "default");
   mCursorNamesMap.insert(ArrowCursor, "top_left_arrow");
   mCursorNamesMap.insert(ArrowCursor, "left_arrow");

   mCursorNamesMap.insert(UpArrowCursor, "up_arrow");

   mCursorNamesMap.insert(CrossCursor, "cross");

   mCursorNamesMap.insert(WaitCursor, "wait");
   mCursorNamesMap.insert(WaitCursor, "watch");
   mCursorNamesMap.insert(WaitCursor, "0426c94ea35c87780ff01dc239897213");

   mCursorNamesMap.insert(IBeamCursor, "ibeam");
   mCursorNamesMap.insert(IBeamCursor, "text");
   mCursorNamesMap.insert(IBeamCursor, "xterm");

   mCursorNamesMap.insert(SizeVerCursor, "size_ver");
   mCursorNamesMap.insert(SizeVerCursor, "ns-resize");
   mCursorNamesMap.insert(SizeVerCursor, "v_double_arrow");
   mCursorNamesMap.insert(SizeVerCursor, "00008160000006810000408080010102");

   mCursorNamesMap.insert(SizeHorCursor, "size_hor");
   mCursorNamesMap.insert(SizeHorCursor, "ew-resize");
   mCursorNamesMap.insert(SizeHorCursor, "h_double_arrow");
   mCursorNamesMap.insert(SizeHorCursor, "028006030e0e7ebffc7f7070c0600140");

   mCursorNamesMap.insert(SizeBDiagCursor, "size_bdiag");
   mCursorNamesMap.insert(SizeBDiagCursor, "nesw-resize");
   mCursorNamesMap.insert(SizeBDiagCursor, "50585d75b494802d0151028115016902");
   mCursorNamesMap.insert(SizeBDiagCursor, "fcf1c3c7cd4491d801f1e1c78f100000");

   mCursorNamesMap.insert(SizeFDiagCursor, "size_fdiag");
   mCursorNamesMap.insert(SizeFDiagCursor, "nwse-resize");
   mCursorNamesMap.insert(SizeFDiagCursor, "38c5dff7c7b8962045400281044508d2");
   mCursorNamesMap.insert(SizeFDiagCursor, "c7088f0f3e6c8088236ef8e1e3e70000");

   mCursorNamesMap.insert(SizeAllCursor, "size_all");

   mCursorNamesMap.insert(SplitVCursor, "split_v");
   mCursorNamesMap.insert(SplitVCursor, "row-resize");
   mCursorNamesMap.insert(SplitVCursor, "sb_v_double_arrow");
   mCursorNamesMap.insert(SplitVCursor, "2870a09082c103050810ffdffffe0204");
   mCursorNamesMap.insert(SplitVCursor, "c07385c7190e701020ff7ffffd08103c");

   mCursorNamesMap.insert(SplitHCursor, "split_h");
   mCursorNamesMap.insert(SplitHCursor, "col-resize");
   mCursorNamesMap.insert(SplitHCursor, "sb_h_double_arrow");
   mCursorNamesMap.insert(SplitHCursor, "043a9f68147c53184671403ffa811cc5");
   mCursorNamesMap.insert(SplitHCursor, "14fef782d02440884392942c11205230");

   mCursorNamesMap.insert(PointingHandCursor, "pointing_hand");
   mCursorNamesMap.insert(PointingHandCursor, "pointer");
   mCursorNamesMap.insert(PointingHandCursor, "hand1");
   mCursorNamesMap.insert(PointingHandCursor, "e29285e634086352946a0e7090d73106");

   mCursorNamesMap.insert(ForbiddenCursor, "forbidden");
   mCursorNamesMap.insert(ForbiddenCursor, "not-allowed");
   mCursorNamesMap.insert(ForbiddenCursor, "crossed_circle");
   mCursorNamesMap.insert(ForbiddenCursor, "circle");
   mCursorNamesMap.insert(ForbiddenCursor, "03b6e0fcb3499374a867c041f52298f0");

   mCursorNamesMap.insert(WhatsThisCursor, "whats_this");
   mCursorNamesMap.insert(WhatsThisCursor, "help");
   mCursorNamesMap.insert(WhatsThisCursor, "question_arrow");
   mCursorNamesMap.insert(WhatsThisCursor, "5c6cd98b3f3ebcb1f9c7f1c204630408");
   mCursorNamesMap.insert(WhatsThisCursor, "d9ce0ab605698f320427677b458ad60b");

   mCursorNamesMap.insert(BusyCursor, "left_ptr_watch");
   mCursorNamesMap.insert(BusyCursor, "half-busy");
   mCursorNamesMap.insert(BusyCursor, "progress");
   mCursorNamesMap.insert(BusyCursor, "00000000000000020006000e7e9ffc3f");
   mCursorNamesMap.insert(BusyCursor, "08e8e1c95fe2fc01f976f1e063a24ccd");

   mCursorNamesMap.insert(OpenHandCursor, "openhand");
   mCursorNamesMap.insert(OpenHandCursor, "fleur");
   mCursorNamesMap.insert(OpenHandCursor, "5aca4d189052212118709018842178c0");
   mCursorNamesMap.insert(OpenHandCursor, "9d800788f1b08800ae810202380a0822");

   mCursorNamesMap.insert(ClosedHandCursor, "closedhand");
   mCursorNamesMap.insert(ClosedHandCursor, "grabbing");
   mCursorNamesMap.insert(ClosedHandCursor, "208530c400c041818281048008011002");

   mCursorNamesMap.insert(DragCopyCursor, "dnd-copy");
   mCursorNamesMap.insert(DragCopyCursor, "copy");

   mCursorNamesMap.insert(DragMoveCursor, "dnd-move");
   mCursorNamesMap.insert(DragMoveCursor, "move");

   mCursorNamesMap.insert(DragLinkCursor, "dnd-link");
   mCursorNamesMap.insert(DragLinkCursor, "link");

   mCursorNamesMap.insert(ResizeNorthCursor, "n-resize");
   mCursorNamesMap.insert(ResizeNorthCursor, "top_side");

   mCursorNamesMap.insert(ResizeSouthCursor, "s-resize");
   mCursorNamesMap.insert(ResizeSouthCursor, "bottom_side");

   mCursorNamesMap.insert(ResizeEastCursor, "e-resize");
   mCursorNamesMap.insert(ResizeEastCursor, "right_side");

   mCursorNamesMap.insert(ResizeWestCursor, "w-resize");
   mCursorNamesMap.insert(ResizeWestCursor, "left_side");

   mCursorNamesMap.insert(ResizeNorthWestCursor, "nw-resize");
   mCursorNamesMap.insert(ResizeNorthWestCursor, "top_left_corner");

   mCursorNamesMap.insert(ResizeSouthEastCursor, "se-resize");
   mCursorNamesMap.insert(ResizeSouthEastCursor, "bottom_right_corner");

   mCursorNamesMap.insert(ResizeNorthEastCursor, "ne-resize");
   mCursorNamesMap.insert(ResizeNorthEastCursor, "top_right_corner");

   mCursorNamesMap.insert(ResizeSouthWestCursor, "sw-resize");
   mCursorNamesMap.insert(ResizeSouthWestCursor, "bottom_left_corner");
}

}
