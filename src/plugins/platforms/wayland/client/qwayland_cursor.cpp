/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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
#include <qscreen.h>

#include <wayland-cursor.h>

#include <qwayland_buffer_p.h>
#include <qwayland_display_p.h>
#include <qwayland_screen_p.h>
#include <qwayland_shm_backingstore_p.h>

namespace QtWaylandClient {

QWaylandCursor::QWaylandCursor(QWaylandScreen *screen)
   : m_display(screen->display())
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

   m_cursorTheme = wl_cursor_theme_load(cursorTheme.constData(), cursorSize, m_display->shm());

#if defined(CS_SHOW_DEBUG_PLATFORM)
   if (m_cursorTheme == nullptr) {
      qDebug() << "Unable to load theme" << cursorTheme;
   }
#endif

   initCursorMap();
}

QWaylandCursor::~QWaylandCursor()
{
   if (m_cursorTheme != nullptr) {
      wl_cursor_theme_destroy(m_cursorTheme);
   }
}

struct wl_cursor_image *QWaylandCursor::cursorImage(Qt::CursorShape newShape)
{
   struct wl_cursor *waylandCursor = nullptr;

   // hide cursor
   if (newShape == Qt::BlankCursor) {
      m_display->setCursor(nullptr, nullptr, 1);
      return nullptr;
   }

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
      return QSharedPointer<QWaylandShmBuffer>();
   }

   const QImage &img = cursor->pixmap().toImage();

   QSharedPointer<QWaylandShmBuffer> buffer(new QWaylandShmBuffer(m_display, img.size(), img.format()));
   memcpy(buffer->image()->bits(), img.bits(), img.byteCount());

   return buffer;
}

void QWaylandCursor::changeCursor(QCursor *cursor, QWindow *window)
{
   const Qt::CursorShape newShape = cursor ? cursor->shape() : Qt::ArrowCursor;

   if (newShape == Qt::BlankCursor) {
      m_display->setCursor(nullptr, nullptr, 1);
      return;
   }

   if (newShape == Qt::BitmapCursor) {
      m_display->setCursor(cursorBitmapImage(cursor), cursor->hotSpot(), window->screen()->devicePixelRatio());
      return;
   }

   struct wl_cursor_image *image = cursorImage(newShape);

   if (image == nullptr) {
      return;
   }

   struct wl_buffer *buffer = wl_cursor_image_get_buffer(image);

   m_display->setCursor(buffer, image, window->screen()->devicePixelRatio());
}

void QWaylandCursor::pointerEvent(const QMouseEvent &event)
{
   m_lastPos = event.globalPos();
}

QPoint QWaylandCursor::pos() const
{
   return m_lastPos;
}

void QWaylandCursor::setPos(const QPoint &pos)
{
   (void) pos;
   qWarning("QWaylandCursor::setPos() Wayland does not support setting the cursor position");
}

wl_cursor *QWaylandCursor::requestCursor(WaylandCursor shape)
{
   struct wl_cursor *cursor = m_cursors.value(shape, nullptr);

   // If the cursor has not been loaded already, load it
   if (cursor == nullptr) {
      if (m_cursorTheme == nullptr) {
         return nullptr;
      }

      QList<QByteArray> cursorNames = m_cursorNamesMap.values(shape);

      for (const QByteArray &item : cursorNames) {
         cursor = wl_cursor_theme_get_cursor(m_cursorTheme, item.constData());

         if (cursor != nullptr) {
            m_cursors.insert(shape, cursor);
            break;
         }
      }
   }

   // If there still no cursor for a shape, use the default cursor
   if (cursor == nullptr && shape != ArrowCursor) {
      cursor = requestCursor(ArrowCursor);
   }

   return cursor;
}

void QWaylandCursor::initCursorMap()
{
   // Fill the cursor name map will the table of xcursor names
   m_cursorNamesMap.insert(ArrowCursor, "left_ptr");
   m_cursorNamesMap.insert(ArrowCursor, "default");
   m_cursorNamesMap.insert(ArrowCursor, "top_left_arrow");
   m_cursorNamesMap.insert(ArrowCursor, "left_arrow");

   m_cursorNamesMap.insert(UpArrowCursor, "up_arrow");

   m_cursorNamesMap.insert(CrossCursor, "cross");

   m_cursorNamesMap.insert(WaitCursor, "wait");
   m_cursorNamesMap.insert(WaitCursor, "watch");
   m_cursorNamesMap.insert(WaitCursor, "0426c94ea35c87780ff01dc239897213");

   m_cursorNamesMap.insert(IBeamCursor, "ibeam");
   m_cursorNamesMap.insert(IBeamCursor, "text");
   m_cursorNamesMap.insert(IBeamCursor, "xterm");

   m_cursorNamesMap.insert(SizeVerCursor, "size_ver");
   m_cursorNamesMap.insert(SizeVerCursor, "ns-resize");
   m_cursorNamesMap.insert(SizeVerCursor, "v_double_arrow");
   m_cursorNamesMap.insert(SizeVerCursor, "00008160000006810000408080010102");

   m_cursorNamesMap.insert(SizeHorCursor, "size_hor");
   m_cursorNamesMap.insert(SizeHorCursor, "ew-resize");
   m_cursorNamesMap.insert(SizeHorCursor, "h_double_arrow");
   m_cursorNamesMap.insert(SizeHorCursor, "028006030e0e7ebffc7f7070c0600140");

   m_cursorNamesMap.insert(SizeBDiagCursor, "size_bdiag");
   m_cursorNamesMap.insert(SizeBDiagCursor, "nesw-resize");
   m_cursorNamesMap.insert(SizeBDiagCursor, "50585d75b494802d0151028115016902");
   m_cursorNamesMap.insert(SizeBDiagCursor, "fcf1c3c7cd4491d801f1e1c78f100000");

   m_cursorNamesMap.insert(SizeFDiagCursor, "size_fdiag");
   m_cursorNamesMap.insert(SizeFDiagCursor, "nwse-resize");
   m_cursorNamesMap.insert(SizeFDiagCursor, "38c5dff7c7b8962045400281044508d2");
   m_cursorNamesMap.insert(SizeFDiagCursor, "c7088f0f3e6c8088236ef8e1e3e70000");

   m_cursorNamesMap.insert(SizeAllCursor, "size_all");

   m_cursorNamesMap.insert(SplitVCursor, "split_v");
   m_cursorNamesMap.insert(SplitVCursor, "row-resize");
   m_cursorNamesMap.insert(SplitVCursor, "sb_v_double_arrow");
   m_cursorNamesMap.insert(SplitVCursor, "2870a09082c103050810ffdffffe0204");
   m_cursorNamesMap.insert(SplitVCursor, "c07385c7190e701020ff7ffffd08103c");

   m_cursorNamesMap.insert(SplitHCursor, "split_h");
   m_cursorNamesMap.insert(SplitHCursor, "col-resize");
   m_cursorNamesMap.insert(SplitHCursor, "sb_h_double_arrow");
   m_cursorNamesMap.insert(SplitHCursor, "043a9f68147c53184671403ffa811cc5");
   m_cursorNamesMap.insert(SplitHCursor, "14fef782d02440884392942c11205230");

   m_cursorNamesMap.insert(PointingHandCursor, "pointing_hand");
   m_cursorNamesMap.insert(PointingHandCursor, "pointer");
   m_cursorNamesMap.insert(PointingHandCursor, "hand1");
   m_cursorNamesMap.insert(PointingHandCursor, "e29285e634086352946a0e7090d73106");

   m_cursorNamesMap.insert(ForbiddenCursor, "forbidden");
   m_cursorNamesMap.insert(ForbiddenCursor, "not-allowed");
   m_cursorNamesMap.insert(ForbiddenCursor, "crossed_circle");
   m_cursorNamesMap.insert(ForbiddenCursor, "circle");
   m_cursorNamesMap.insert(ForbiddenCursor, "03b6e0fcb3499374a867c041f52298f0");

   m_cursorNamesMap.insert(WhatsThisCursor, "whats_this");
   m_cursorNamesMap.insert(WhatsThisCursor, "help");
   m_cursorNamesMap.insert(WhatsThisCursor, "question_arrow");
   m_cursorNamesMap.insert(WhatsThisCursor, "5c6cd98b3f3ebcb1f9c7f1c204630408");
   m_cursorNamesMap.insert(WhatsThisCursor, "d9ce0ab605698f320427677b458ad60b");

   m_cursorNamesMap.insert(BusyCursor, "left_ptr_watch");
   m_cursorNamesMap.insert(BusyCursor, "half-busy");
   m_cursorNamesMap.insert(BusyCursor, "progress");
   m_cursorNamesMap.insert(BusyCursor, "00000000000000020006000e7e9ffc3f");
   m_cursorNamesMap.insert(BusyCursor, "08e8e1c95fe2fc01f976f1e063a24ccd");

   m_cursorNamesMap.insert(OpenHandCursor, "openhand");
   m_cursorNamesMap.insert(OpenHandCursor, "fleur");
   m_cursorNamesMap.insert(OpenHandCursor, "5aca4d189052212118709018842178c0");
   m_cursorNamesMap.insert(OpenHandCursor, "9d800788f1b08800ae810202380a0822");

   m_cursorNamesMap.insert(ClosedHandCursor, "closedhand");
   m_cursorNamesMap.insert(ClosedHandCursor, "grabbing");
   m_cursorNamesMap.insert(ClosedHandCursor, "208530c400c041818281048008011002");

   m_cursorNamesMap.insert(DragCopyCursor, "dnd-copy");
   m_cursorNamesMap.insert(DragCopyCursor, "copy");

   m_cursorNamesMap.insert(DragMoveCursor, "dnd-move");
   m_cursorNamesMap.insert(DragMoveCursor, "move");

   m_cursorNamesMap.insert(DragLinkCursor, "dnd-link");
   m_cursorNamesMap.insert(DragLinkCursor, "link");

   m_cursorNamesMap.insert(ResizeNorthCursor, "n-resize");
   m_cursorNamesMap.insert(ResizeNorthCursor, "top_side");

   m_cursorNamesMap.insert(ResizeSouthCursor, "s-resize");
   m_cursorNamesMap.insert(ResizeSouthCursor, "bottom_side");

   m_cursorNamesMap.insert(ResizeEastCursor, "e-resize");
   m_cursorNamesMap.insert(ResizeEastCursor, "right_side");

   m_cursorNamesMap.insert(ResizeWestCursor, "w-resize");
   m_cursorNamesMap.insert(ResizeWestCursor, "left_side");

   m_cursorNamesMap.insert(ResizeNorthWestCursor, "nw-resize");
   m_cursorNamesMap.insert(ResizeNorthWestCursor, "top_left_corner");

   m_cursorNamesMap.insert(ResizeSouthEastCursor, "se-resize");
   m_cursorNamesMap.insert(ResizeSouthEastCursor, "bottom_right_corner");

   m_cursorNamesMap.insert(ResizeNorthEastCursor, "ne-resize");
   m_cursorNamesMap.insert(ResizeNorthEastCursor, "top_right_corner");

   m_cursorNamesMap.insert(ResizeSouthWestCursor, "sw-resize");
   m_cursorNamesMap.insert(ResizeSouthWestCursor, "bottom_left_corner");
}

}
