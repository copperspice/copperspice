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

#ifndef QWINDOWSCURSOR_H
#define QWINDOWSCURSOR_H

#include <qwin_additional.h>
#include <qplatform_cursor.h>
#include <qsharedpointer.h>
#include <qhash.h>

struct QWindowsPixmapCursorCacheKey {
   explicit QWindowsPixmapCursorCacheKey(const QCursor &c);

   qint64 bitmapCacheKey;
   qint64 maskCacheKey;
};

inline bool operator==(const QWindowsPixmapCursorCacheKey &k1, const QWindowsPixmapCursorCacheKey &k2)
{
   return k1.bitmapCacheKey == k2.bitmapCacheKey && k1.maskCacheKey == k2.maskCacheKey;
}

inline uint qHash(const QWindowsPixmapCursorCacheKey &k, uint seed)
{
   return (uint(k.bitmapCacheKey) + uint(k.maskCacheKey)) ^ seed;
}

class CursorHandle
{
 public:
   explicit CursorHandle(HCURSOR hcursor = nullptr)
      : m_hcursor(hcursor)
   {
   }

   CursorHandle(const CursorHandle &) = delete;
   CursorHandle &operator=(const CursorHandle &) = delete;

   ~CursorHandle() {
      if (m_hcursor) {
         DestroyCursor(m_hcursor);
      }
   }

   bool isNull() const {
      return !m_hcursor;
   }

   HCURSOR handle() const {
      return m_hcursor;
   }

 private:
   const HCURSOR m_hcursor;
};

using CursorHandlePtr = QSharedPointer<CursorHandle>;

class QWindowsCursor : public QPlatformCursor
{
 public:
   enum CursorState {
      CursorShowing,
      CursorHidden,
      CursorSuppressed       // Cursor suppressed by touch interaction (Windows 8).
   };

   struct PixmapCursor {
      explicit PixmapCursor(const QPixmap &pix = QPixmap(), const QPoint &h = QPoint()) : pixmap(pix), hotSpot(h) {}

      QPixmap pixmap;
      QPoint hotSpot;
   };

   explicit QWindowsCursor(const QPlatformScreen *screen);

   void changeCursor(QCursor *widgetCursor, QWindow *widget) override;
   QPoint pos() const override;
   void setPos(const QPoint &pos) override;

   static HCURSOR createPixmapCursor(QPixmap pixmap, const QPoint &hotSpot, qreal scaleFactor = 1);
   static HCURSOR createPixmapCursor(const PixmapCursor &pc, qreal scaleFactor = 1) {
      return createPixmapCursor(pc.pixmap, pc.hotSpot, scaleFactor);
   }
   static PixmapCursor customCursor(Qt::CursorShape cursorShape, const QPlatformScreen *screen = nullptr);

   static HCURSOR createCursorFromShape(Qt::CursorShape cursorShape, const QPlatformScreen *screen = nullptr);
   static QPoint mousePosition();
   static CursorState cursorState();

   CursorHandlePtr standardWindowCursor(Qt::CursorShape s = Qt::ArrowCursor);
   CursorHandlePtr pixmapWindowCursor(const QCursor &c);

   QPixmap dragDefaultCursor(Qt::DropAction action) const;

 private:
   typedef QHash<Qt::CursorShape, CursorHandlePtr> StandardCursorCache;
   typedef QHash<QWindowsPixmapCursorCacheKey, CursorHandlePtr> PixmapCursorCache;

   const QPlatformScreen *const m_screen;
   StandardCursorCache m_standardCursorCache;
   PixmapCursorCache m_pixmapCursorCache;

   mutable QPixmap m_copyDragCursor;
   mutable QPixmap m_moveDragCursor;
   mutable QPixmap m_linkDragCursor;
   mutable QPixmap m_ignoreDragCursor;
};

#endif
