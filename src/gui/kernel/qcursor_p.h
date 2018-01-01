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

#ifndef QCURSOR_P_H
#define QCURSOR_P_H

#include <QtCore/qatomic.h>
#include <QtCore/qglobal.h>
#include <QtCore/qnamespace.h>
#include <QtGui/qpixmap.h>

# if defined (Q_OS_MAC)
#  include <qt_mac_p.h>

# elif defined(Q_WS_X11)
#  include <qt_x11_p.h>

# elif defined(Q_OS_WIN)
#  include <QtCore/qt_windows.h>

#endif

QT_BEGIN_NAMESPACE

#if defined (Q_OS_MAC)
void *qt_mac_nsCursorForQCursor(const QCursor &c);
#endif

class QBitmap;

class QCursorData
{
 public:
   QCursorData(Qt::CursorShape s = Qt::ArrowCursor);
   ~QCursorData();

   static void initialize();
   static void cleanup();

   QAtomicInt ref;
   Qt::CursorShape cshape;
   QBitmap *bm, *bmm;
   QPixmap pixmap;
   short hx, hy;

#if defined (Q_OS_MAC)
   int mId;

#elif defined(Q_WS_QWS) || defined(Q_WS_QPA)
   int id;

#endif

#if defined (Q_OS_WIN)
   HCURSOR hcurs;

#elif defined (Q_WS_X11)
   XColor fg, bg;
   Cursor hcurs;
   Pixmap pm, pmm;

#elif defined (Q_OS_MAC)
   enum { TYPE_None, TYPE_ImageCursor, TYPE_ThemeCursor } type;

   union {
      struct {
         uint my_cursor: 1;
         void *nscursor;
      } cp;

   } curs;

   void initCursorFromBitmap();
   void initCursorFromPixmap();
#endif

   static bool initialized;
   void update();
   static QCursorData *setBitmap(const QBitmap &bitmap, const QBitmap &mask, int hotX, int hotY);
};

extern QCursorData *qt_cursorTable[Qt::LastCursor + 1]; // qcursor.cpp

QT_END_NAMESPACE

#endif // QCURSOR_P_H
