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

#include <qcursor.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qdatastream.h>
#include <qvariant.h>
#include <qdebug.h>
#include <qplatform_cursor.h>

#include <qcursor_p.h>
#include <qguiapplication_p.h>
#include <qhighdpiscaling_p.h>

QPoint QCursor::pos(const QScreen *screen)
{
   if (screen) {
      if (const QPlatformCursor *cursor = screen->handle()->cursor()) {
         const QPlatformScreen *ps = screen->handle();
         QPoint nativePos = cursor->pos();
         ps = ps->screenForPosition(nativePos);
         return QHighDpi::fromNativePixels(nativePos, ps->screen());
      }
   }

   return QGuiApplicationPrivate::lastCursorPosition.toPoint();
}

QPoint QCursor::pos()
{
   return QCursor::pos(QGuiApplication::primaryScreen());
}

void QCursor::setPos(QScreen *screen, int x, int y)
{
   if (screen) {
      if (QPlatformCursor *cursor = screen->handle()->cursor()) {
         const QPoint devicePos = QHighDpi::toNativePixels(QPoint(x, y), screen);
         if (devicePos != cursor->pos()) {
            cursor->setPos(devicePos);
         }
      }
   }
}

void QCursor::setPos(int x, int y)
{
   QCursor::setPos(QGuiApplication::primaryScreen(), x, y);
}

#ifndef QT_NO_CURSOR

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
      c.setShape((Qt::CursorShape)shape);          // create cursor with shape
   }

   return s;
}

QCursor::QCursor(const QPixmap &pixmap, int hotX, int hotY)
   : d(nullptr)
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

   d = QCursorData::setBitmap(bm, bmm, hotX, hotY, pixmap.devicePixelRatio());
   d->pixmap = pixmap;
}

QCursor::QCursor(const QBitmap &bitmap, const QBitmap &mask, int hotX, int hotY)
   : d(nullptr)
{
   d = QCursorData::setBitmap(bitmap, mask, hotX, hotY, 1.0);
}

QCursor::QCursor()
{
   if (!QCursorData::initialized) {
      if (QCoreApplication::startingUp()) {
         d = nullptr;
         return;
      }
      QCursorData::initialize();
   }
   QCursorData *c = qt_cursorTable[0];
   c->ref.ref();
   d = c;
}

QCursor::QCursor(Qt::CursorShape shape)
   : d(nullptr)
{
   if (! QCursorData::initialized) {
      QCursorData::initialize();
   }

   setShape(shape);
}

Qt::CursorShape QCursor::shape() const
{
   if (! QCursorData::initialized) {
      QCursorData::initialize();
   }
   return d->cshape;
}

void QCursor::setShape(Qt::CursorShape shape)
{
   if (! QCursorData::initialized) {
      QCursorData::initialize();
   }

   QCursorData *c = uint(shape) <= Qt::LastCursor ? qt_cursorTable[shape] : nullptr;

   if (! c) {
      c = qt_cursorTable[0];
   }

   c->ref.ref();

   if (! d) {
      d = c;

   } else {
      if (! d->ref.deref()) {
         delete d;
      }
      d = c;
   }
}

const QBitmap *QCursor::bitmap() const
{
   if (! QCursorData::initialized) {
      QCursorData::initialize();
   }
   return d->bm;
}

const QBitmap *QCursor::mask() const
{
   if (!QCursorData::initialized) {
      QCursorData::initialize();
   }

   return d->bmm;
}


QPixmap QCursor::pixmap() const
{
   if (! QCursorData::initialized) {
      QCursorData::initialize();
   }

   return d->pixmap;
}

QPoint QCursor::hotSpot() const
{
   if (! QCursorData::initialized) {
      QCursorData::initialize();
   }
   return QPoint(d->hx, d->hy);
}

QCursor::QCursor(const QCursor &c)
{
   if (! QCursorData::initialized) {
      QCursorData::initialize();
   }

   d = c.d;
   d->ref.ref();
}

QCursor::~QCursor()
{
   if (d && ! d->ref.deref()) {
      delete d;
   }
}

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

QCursor::operator QVariant() const
{
   return QVariant(QVariant::Cursor, this);
}

QDebug operator<<(QDebug debug, const QCursor &c)
{
   QDebugStateSaver saver(debug);
   debug.nospace();

   debug << "QCursor(Qt::CursorShape(" << c.shape() << "))";

   return debug;
}

QCursorData *qt_cursorTable[Qt::LastCursor + 1];
bool QCursorData::initialized = false;

QCursorData::QCursorData(Qt::CursorShape s)
   : ref(1), cshape(s), bm(nullptr), bmm(nullptr), hx(0), hy(0)
{
}

QCursorData::~QCursorData()
{
   delete bm;
   delete bmm;
}

void QCursorData::cleanup()
{
   if (!QCursorData::initialized) {
      return;
   }

   for (int shape = 0; shape <= Qt::LastCursor; ++shape) {
      if (!qt_cursorTable[shape]->ref.deref()) {
         delete qt_cursorTable[shape];
      }
      qt_cursorTable[shape] = nullptr;
   }

   QCursorData::initialized = false;
}

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

QCursorData *QCursorData::setBitmap(const QBitmap &bitmap, const QBitmap &mask, int hotX, int hotY, qreal devicePixelRatio)
{
   if (! QCursorData::initialized) {
      QCursorData::initialize();
   }

   if (bitmap.depth() != 1 || mask.depth() != 1 || bitmap.size() != mask.size()) {
      qWarning("QCursor::setBitmap() Unable to create bitmap cursor, invalid bitmap(s)");
      QCursorData *c = qt_cursorTable[0];
      c->ref.ref();
      return c;
   }

   QCursorData *d = new QCursorData;
   d->bm  = new QBitmap(bitmap);
   d->bmm = new QBitmap(mask);
   d->cshape = Qt::BitmapCursor;
   d->hx = hotX >= 0 ? hotX : bitmap.width() / 2 / devicePixelRatio;
   d->hy = hotY >= 0 ? hotY : bitmap.height() / 2 / devicePixelRatio;

   return d;
}

void QCursorData::update()
{
}

#endif // QT_NO_CURSOR

