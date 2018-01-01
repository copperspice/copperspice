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

#include <qcursor.h>
#include <qcursor_p.h>
#include <qbitmap.h>
#include <qwsdisplay_qws.h>

QT_BEGIN_NAMESPACE

QT_USE_NAMESPACE

#ifndef QT_NO_CURSOR

static int nextCursorId = Qt::BitmapCursor;

/*****************************************************************************
  Internal QCursorData class
 *****************************************************************************/

QCursorData::QCursorData(Qt::CursorShape s)
   : cshape(s), bm(0), bmm(0), hx(0), hy(0), id(s)
{
   ref = 1;
}

QCursorData::~QCursorData()
{
   delete bm;
   delete bmm;
   QT_TRY {
      QPaintDevice::qwsDisplay()->destroyCursor(id);
   } QT_CATCH(const std::bad_alloc &) {
      // do nothing.
   }
}


/*****************************************************************************
  Global cursors
 *****************************************************************************/

int QCursor::handle() const
{
   return d->id;
}


QCursorData *QCursorData::setBitmap(const QBitmap &bitmap, const QBitmap &mask, int hotX, int hotY)
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }
   if (bitmap.depth() != 1 || mask.depth() != 1 || bitmap.size() != mask.size()) {
      qWarning("QCursor: Cannot create bitmap cursor; invalid bitmap(s)");
      QCursorData *c = qt_cursorTable[0];
      c->ref.ref();
      return c;
   }
   QCursorData *d = new QCursorData;
   d->bm  = new QBitmap(bitmap);
   d->bmm = new QBitmap(mask);
   d->cshape = Qt::BitmapCursor;
   d->id = ++nextCursorId;
   d->hx = hotX >= 0 ? hotX : bitmap.width() / 2;
   d->hy = hotY >= 0 ? hotY : bitmap.height() / 2;

   QPaintDevice::qwsDisplay()->defineCursor(d->id, *d->bm, *d->bmm, d->hx, d->hy);
   return d;
}

void QCursorData::update()
{
}

#endif //QT_NO_CURSOR

extern int *qt_last_x, *qt_last_y;

QPoint QCursor::pos()
{
   // This doesn't know about hotspots yet so we disable it
   //qt_accel_update_cursor();
   if (qt_last_x) {
      return QPoint(*qt_last_x, *qt_last_y);
   } else {
      return QPoint();
   }
}

void QCursor::setPos(int x, int y)
{
   // Need to check, since some X servers generate null mouse move
   // events, causing looping in applications which call setPos() on
   // every mouse move event.
   //
   if (pos() == QPoint(x, y)) {
      return;
   }
   QPaintDevice::qwsDisplay()->setCursorPosition(x, y);
}

QT_END_NAMESPACE
