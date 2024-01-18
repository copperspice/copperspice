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

#ifndef QXCB_CURSOR_H
#define QXCB_CURSOR_H

#include <qplatform_cursor.h>
#include <qxcb_screen.h>

#ifndef QT_NO_CURSOR

struct QXcbCursorCacheKey {
   explicit QXcbCursorCacheKey(const QCursor &c);
   explicit QXcbCursorCacheKey(Qt::CursorShape s) : shape(s), bitmapCacheKey(0), maskCacheKey(0) {}
   QXcbCursorCacheKey() : shape(Qt::CustomCursor), bitmapCacheKey(0), maskCacheKey(0) {}

   Qt::CursorShape shape;
   qint64 bitmapCacheKey;
   qint64 maskCacheKey;
};

inline bool operator==(const QXcbCursorCacheKey &k1, const QXcbCursorCacheKey &k2)
{
   return k1.shape == k2.shape && k1.bitmapCacheKey == k2.bitmapCacheKey && k1.maskCacheKey == k2.maskCacheKey;
}

inline uint qHash(const QXcbCursorCacheKey &k, uint seed)
{
   return (uint(k.shape) + uint(k.bitmapCacheKey) + uint(k.maskCacheKey)) ^ seed;
}

#endif // !QT_NO_CURSOR

class QXcbCursor : public QXcbObject, public QPlatformCursor
{
 public:
   QXcbCursor(QXcbConnection *conn, QXcbScreen *screen);
   ~QXcbCursor();

#ifndef QT_NO_CURSOR
   void changeCursor(QCursor *cursor, QWindow *widget) override;
#endif

   QPoint pos() const override;
   void setPos(const QPoint &pos) override;

   static void queryPointer(QXcbConnection *c, QXcbVirtualDesktop **virtualDesktop, QPoint *pos, int *keybMask = nullptr);

 private:

#ifndef QT_NO_CURSOR
   typedef QHash<QXcbCursorCacheKey, xcb_cursor_t> CursorHash;

   xcb_cursor_t createFontCursor(int cshape);
   xcb_cursor_t createBitmapCursor(QCursor *cursor);
   xcb_cursor_t createNonStandardCursor(int cshape);
#endif

   QXcbScreen *m_screen;

#ifndef QT_NO_CURSOR
   CursorHash m_cursorHash;
#endif

#if defined(XCB_USE_XLIB)
   static void cursorThemePropertyChanged(QXcbVirtualDesktop *screen, const QByteArray &name,
      const QVariant &property, void *handle);
#endif

   bool m_gtkCursorThemeInitialized;
};


#endif
