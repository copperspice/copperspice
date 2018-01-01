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
#include <qplatformcursor_qpa.h>
#include <qbitmap.h>

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
}


/*****************************************************************************
  Global cursors
 *****************************************************************************/

extern QCursorData *qt_cursorTable[Qt::LastCursor + 1]; // qcursor.cpp

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

   return d;
}

void QCursorData::update()
{
}

#endif //QT_NO_CURSOR

extern int qt_last_x, qt_last_y;

QPoint QCursor::pos()
{
   QList<QWeakPointer<QPlatformCursor> > cursors = QPlatformCursorPrivate::getInstances();
   int cursorCount = cursors.count();
   for (int i = 0; i < cursorCount; ++i) {
      const QWeakPointer<QPlatformCursor> &cursor(cursors.at(i));
      if (cursor) {
         return cursor.data()->pos();
      }
   }
   return QPoint(qt_last_x, qt_last_y);
}

void QCursor::setPos(int x, int y)
{
   QPoint target(x, y);

   // Need to check, since some X servers generate null mouse move
   // events, causing looping in applications which call setPos() on
   // every mouse move event.
   //
   if (pos() == target) {
      return;
   }

   QList<QWeakPointer<QPlatformCursor> > cursors = QPlatformCursorPrivate::getInstances();
   int cursorCount = cursors.count();
   for (int i = 0; i < cursorCount; ++i) {
      const QWeakPointer<QPlatformCursor> &cursor(cursors.at(i));
      if (cursor) {
         cursor.data()->setPos(target);
      }
   }
}

QT_END_NAMESPACE
