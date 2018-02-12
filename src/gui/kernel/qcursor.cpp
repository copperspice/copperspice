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

#ifndef QT_NO_CURSOR

#include <qapplication.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qdatastream.h>
#include <qvariant.h>
#include <qcursor_p.h>

QDataStream &operator<<(QDataStream &s, const QCursor &c)
{
   s << (qint16)c.shape();                        // write shape id to stream

   if (c.shape() == Qt::BitmapCursor) {           // bitmap cursor
      bool isPixmap = false;

      isPixmap = !c.pixmap().isNull();
      s << isPixmap;

      if (isPixmap) {
         s << c.pixmap();
      } else {
         s << *c.bitmap() << *c.mask();
      }
      s << c.hotSpot();
   }

   return s;
}

QDataStream &operator>>(QDataStream &s, QCursor &c)
{
   qint16 shape;
   s >> shape;                                     // read shape id from stream

   if (shape == Qt::BitmapCursor) {                // read bitmap cursor
      bool isPixmap = false;

      s >> isPixmap;

      if (isPixmap) {
         QPixmap pm;
         QPoint hot;
         s >> pm >> hot;
         c = QCursor(pm, hot.x(), hot.y());
      } else {
         QBitmap bm, bmm;
         QPoint hot;
         s >> bm >> bmm >> hot;
         c = QCursor(bm, bmm, hot.x(), hot.y());
      }

   } else {
      c.setShape((Qt::CursorShape)shape);                // create cursor with shape
   }

   return s;
}

QCursor::QCursor(const QPixmap &pixmap, int hotX, int hotY)
   : d(0)
{
   QImage img = pixmap.toImage().convertToFormat(QImage::Format_Indexed8, Qt::ThresholdDither | Qt::AvoidDither);
   QBitmap bm = QBitmap::fromImage(img, Qt::ThresholdDither | Qt::AvoidDither);
   QBitmap bmm = pixmap.mask();
   if (!bmm.isNull()) {
      QBitmap nullBm;
      bm.setMask(nullBm);
   } else if (!pixmap.mask().isNull()) {
      QImage mimg = pixmap.mask().toImage().convertToFormat(QImage::Format_Indexed8, Qt::ThresholdDither | Qt::AvoidDither);
      bmm = QBitmap::fromImage(mimg, Qt::ThresholdDither | Qt::AvoidDither);
   } else {
      bmm = QBitmap(bm.size());
      bmm.fill(Qt::color1);
   }

   d = QCursorData::setBitmap(bm, bmm, hotX, hotY);
   d->pixmap = pixmap;
}

QCursor::QCursor(const QBitmap &bitmap, const QBitmap &mask, int hotX, int hotY)
   : d(0)
{
   d = QCursorData::setBitmap(bitmap, mask, hotX, hotY);
}

QCursorData *qt_cursorTable[Qt::LastCursor + 1];
bool QCursorData::initialized = false;

/*! \internal */
void QCursorData::cleanup()
{
   if (!QCursorData::initialized) {
      return;
   }

   for (int shape = 0; shape <= Qt::LastCursor; ++shape) {
      // In case someone has a static QCursor defined with this shape
      if (!qt_cursorTable[shape]->ref.deref()) {
         delete qt_cursorTable[shape];
      }
      qt_cursorTable[shape] = 0;
   }
   QCursorData::initialized = false;
}

/*! \internal */
void QCursorData::initialize()
{
   if (QCursorData::initialized) {
      return;
   }

   for (int shape = 0; shape <= Qt::LastCursor; ++shape) {
      qt_cursorTable[shape] = new QCursorData((Qt::CursorShape)shape);
   }

   QCursorData::initialized = true;
}

/*!
    Constructs a cursor with the default arrow shape.
*/
QCursor::QCursor()
{
   if (!QCursorData::initialized) {
      if (QApplication::startingUp()) {
         d = 0;
         return;
      }
      QCursorData::initialize();
   }
   QCursorData *c = qt_cursorTable[0];
   c->ref.ref();
   d = c;
}

/*!
    Constructs a cursor with the specified \a shape.

    See \l Qt::CursorShape for a list of shapes.

    \sa setShape()
*/
QCursor::QCursor(Qt::CursorShape shape)
   : d(0)
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }
   setShape(shape);
}


/*!
    Returns the cursor shape identifier. The return value is one of
    the \l Qt::CursorShape enum values (cast to an int).

    \sa setShape()
*/
Qt::CursorShape QCursor::shape() const
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }
   return d->cshape;
}

/*!
    Sets the cursor to the shape identified by \a shape.

    See \l Qt::CursorShape for the list of cursor shapes.

    \sa shape()
*/
void QCursor::setShape(Qt::CursorShape shape)
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }
   QCursorData *c = uint(shape) <= Qt::LastCursor ? qt_cursorTable[shape] : 0;
   if (!c) {
      c = qt_cursorTable[0];
   }
   c->ref.ref();
   if (!d) {
      d = c;
   } else {
      if (!d->ref.deref()) {
         delete d;
      }
      d = c;
   }
}

/*!
    Returns the cursor bitmap, or 0 if it is one of the standard
    cursors.
*/
const QBitmap *QCursor::bitmap() const
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }
   return d->bm;
}

/*!
    Returns the cursor bitmap mask, or 0 if it is one of the standard
    cursors.
*/

const QBitmap *QCursor::mask() const
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }
   return d->bmm;
}

/*!
    Returns the cursor pixmap. This is only valid if the cursor is a
    pixmap cursor.
*/

QPixmap QCursor::pixmap() const
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }
   return d->pixmap;
}

/*!
    Returns the cursor hot spot, or (0, 0) if it is one of the
    standard cursors.
*/

QPoint QCursor::hotSpot() const
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }
   return QPoint(d->hx, d->hy);
}

/*!
    Constructs a copy of the cursor \a c.
*/

QCursor::QCursor(const QCursor &c)
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }
   d = c.d;
   d->ref.ref();
}

/*!
    Destroys the cursor.
*/

QCursor::~QCursor()
{
   if (d && !d->ref.deref()) {
      delete d;
   }
}


/*!
    Assigns \a c to this cursor and returns a reference to this
    cursor.
*/

QCursor &QCursor::operator=(const QCursor &c)
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }
   if (c.d) {
      c.d->ref.ref();
   }
   if (d && !d->ref.deref()) {
      delete d;
   }
   d = c.d;
   return *this;
}

/*!
   Returns the cursor as a QVariant.
*/
QCursor::operator QVariant() const
{
   return QVariant(QVariant::Cursor, this);
}
QT_END_NAMESPACE
#endif // QT_NO_CURSOR

