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

#ifndef QCURSOR_H
#define QCURSOR_H

#include <QtCore/qpoint.h>
#include <QtGui/qwindowdefs.h>

QT_BEGIN_NAMESPACE

class QVariant;

#ifdef QT_NO_CURSOR
// fake class, used on touchscreen devices

class Q_GUI_EXPORT QCursor
{
 public:
   static QPoint pos();
   static void setPos(int x, int y);
   inline static void setPos(const QPoint &p) {
      setPos(p.x(), p.y());
   }

 private:
   QCursor();
};

#endif


#ifndef QT_NO_CURSOR

class QCursorData;
class QBitmap;
class QPixmap;

#if defined(Q_OS_MAC)
void qt_mac_set_cursor(const QCursor *c);
#endif

class Q_GUI_EXPORT QCursor
{
 public:
   QCursor();
   QCursor(Qt::CursorShape shape);
   QCursor(const QBitmap &bitmap, const QBitmap &mask, int hotX = -1, int hotY = -1);
   QCursor(const QPixmap &pixmap, int hotX = -1, int hotY = -1);
   QCursor(const QCursor &cursor);
   ~QCursor();

   QCursor &operator=(const QCursor &cursor);

   inline QCursor &operator=(QCursor && other) {
      qSwap(d, other.d);
      return *this;
   }

   operator QVariant() const;

   Qt::CursorShape shape() const;
   void setShape(Qt::CursorShape newShape);

   const QBitmap *bitmap() const;
   const QBitmap *mask() const;
   QPixmap pixmap() const;
   QPoint hotSpot() const;

   static QPoint pos();
   static void setPos(int x, int y);
   inline static void setPos(const QPoint &p) {
      setPos(p.x(), p.y());
   }

#if defined(Q_OS_WIN)
   HCURSOR handle() const;
   QCursor(HCURSOR cursor);

#elif defined(Q_WS_X11)
   Qt::HANDLE handle() const;
   QCursor(Qt::HANDLE cursor);
   static int x11Screen();

#elif defined(Q_OS_MAC)
   Qt::HANDLE handle() const;

#elif defined(Q_WS_QWS) || defined(Q_WS_QPA)
   int handle() const;

#endif


 private:
   QCursorData *d;
#if defined(Q_OS_MAC)
   friend void *qt_mac_nsCursorForQCursor(const QCursor &c);
   friend void qt_mac_set_cursor(const QCursor *c);
   friend void qt_mac_updateCursorWithWidgetUnderMouse(QWidget *widgetUnderMouse);
#endif
};

#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &outS, const QCursor &cursor);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &inS, QCursor &cursor);
#endif
#endif // QT_NO_CURSOR

QT_END_NAMESPACE

#endif // QCURSOR_H
