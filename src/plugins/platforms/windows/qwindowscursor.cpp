/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef  QT_NO_CURSOR

#include <qwindowscursor.h>
#include <qwindowscontext.h>
#include <qwindowswindow.h>
#include <qwindowsscreen.h>
#include <qdebug.h>

#include <QScopedArrayPointer>
#include <QBitmap>
#include <QImage>
#include <QBitmap>
#include <QApplication>
#include <QScreen>

#include <qapplication_p.h>
#include <qhighdpiscaling_p.h>

static bool initResources()
{
#if ! defined(QT_NO_IMAGEFORMAT_PNG)
   extern int qInitResources_cursors();
   qInitResources_cursors();
#endif

   return true;
}

Q_GUI_EXPORT HBITMAP qt_pixmapToWinHBITMAP(const QPixmap &p, int hbitmapFormat = 0);
Q_GUI_EXPORT HBITMAP qt_createIconMask(const QBitmap &bitmap);

QWindowsPixmapCursorCacheKey::QWindowsPixmapCursorCacheKey(const QCursor &c)
   : bitmapCacheKey(c.pixmap().cacheKey()), maskCacheKey(0)
{
   if (! bitmapCacheKey) {
      Q_ASSERT(c.bitmap());
      Q_ASSERT(c.mask());

      bitmapCacheKey = c.bitmap()->cacheKey();
      maskCacheKey = c.mask()->cacheKey();
   }
}

HCURSOR QWindowsCursor::createPixmapCursor(QPixmap pixmap, const QPoint &hotSpot, qreal scaleFactor)
{
   HCURSOR cur = 0;
   const qreal pixmapScaleFactor = scaleFactor / pixmap.devicePixelRatioF();
   if (!qFuzzyCompare(pixmapScaleFactor, 1)) {
      pixmap = pixmap.scaled((pixmapScaleFactor * QSizeF(pixmap.size())).toSize(),
            Qt::KeepAspectRatio, Qt::SmoothTransformation);
   }
   QBitmap mask = pixmap.mask();
   if (mask.isNull()) {
      mask = QBitmap(pixmap.size());
      mask.fill(Qt::color1);
   }

   HBITMAP ic = qt_pixmapToWinHBITMAP(pixmap, /* HBitmapAlpha */ 2);
   const HBITMAP im = qt_createIconMask(mask);

   ICONINFO ii;
   ii.fIcon     = 0;
   ii.xHotspot  = DWORD(qRound(hotSpot.x() * scaleFactor));
   ii.yHotspot  = DWORD(qRound(hotSpot.y() * scaleFactor));
   ii.hbmMask   = im;
   ii.hbmColor  = ic;

   cur = CreateIconIndirect(&ii);

   DeleteObject(ic);
   DeleteObject(im);
   return cur;
}

// Create a cursor from image and mask of the format QImage::Format_Mono.
static HCURSOR createBitmapCursor(const QImage &bbits, const QImage &mbits,
   QPoint hotSpot = QPoint(-1, -1),
   bool invb = false, bool invm = false)
{
   const int width = bbits.width();
   const int height = bbits.height();

   if (hotSpot.x() < 0) {
      hotSpot.setX(width / 2);
   }

   if (hotSpot.y() < 0) {
      hotSpot.setY(height / 2);
   }

   const int n = qMax(1, width / 8);

   QScopedArrayPointer<uchar> xBits(new uchar[height * n]);
   QScopedArrayPointer<uchar> xMask(new uchar[height * n]);
   int x = 0;
   for (int i = 0; i < height; ++i) {
      const uchar *bits = bbits.constScanLine(i);
      const uchar *mask = mbits.constScanLine(i);
      for (int j = 0; j < n; ++j) {
         uchar b = bits[j];
         uchar m = mask[j];
         if (invb) {
            b ^= 0xff;
         }
         if (invm) {
            m ^= 0xff;
         }
         xBits[x] = ~m;
         xMask[x] = b ^ m;
         ++x;
      }
   }

   return CreateCursor(GetModuleHandle(0), hotSpot.x(), hotSpot.y(), width, height, xBits.data(), xMask.data());
}

// Create a cursor from image and mask of the format QImage::Format_Mono.
static HCURSOR createBitmapCursor(const QCursor &cursor, qreal scaleFactor = 1)
{
   Q_ASSERT(cursor.shape() == Qt::BitmapCursor && cursor.bitmap());

   QImage bbits = cursor.bitmap()->toImage();
   QImage mbits = cursor.mask()->toImage();
   scaleFactor /= bbits.devicePixelRatioF();

   if (!qFuzzyCompare(scaleFactor, 1)) {
      const QSize scaledSize = (QSizeF(bbits.size()) * scaleFactor).toSize();
      bbits = bbits.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
      mbits = mbits.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
   }

   bbits = bbits.convertToFormat(QImage::Format_Mono);
   mbits = mbits.convertToFormat(QImage::Format_Mono);
   const bool invb = bbits.colorCount() > 1 && qGray(bbits.color(0)) < qGray(bbits.color(1));
   const bool invm = mbits.colorCount() > 1 && qGray(mbits.color(0)) < qGray(mbits.color(1));

   return createBitmapCursor(bbits, mbits, cursor.hotSpot(), invb, invm);
}

static QSize systemCursorSize(const QPlatformScreen *screen = nullptr)
{
   const QSize primaryScreenCursorSize(GetSystemMetrics(SM_CXCURSOR), GetSystemMetrics(SM_CYCURSOR));
   if (screen) {
      // Correct the size if the DPI value of the screen differs from
      // that of the primary screen.
      if (const QScreen *primaryQScreen = QApplication::primaryScreen()) {
         const QPlatformScreen *primaryScreen = primaryQScreen->handle();
         if (screen != primaryScreen) {
            const qreal logicalDpi = screen->logicalDpi().first;
            const qreal primaryScreenLogicalDpi = primaryScreen->logicalDpi().first;
            if (!qFuzzyCompare(logicalDpi, primaryScreenLogicalDpi)) {
               return (QSizeF(primaryScreenCursorSize) * logicalDpi / primaryScreenLogicalDpi).toSize();
            }
         }
      }
   }
   return primaryScreenCursorSize;
}

#if defined (QT_NO_IMAGEFORMAT_PNG)

static inline QSize standardCursorSize()
{
   return QSize(32, 32);
}

// Create pixmap cursors from data and scale the image if the cursor size is
// higher than the standard 32. Note that bitmap cursors as produced by
// createBitmapCursor() only work for standard sizes (32,48,64...), which does
// not work when scaling the 16x16 openhand cursor bitmaps to 150% (resulting
// in a non-standard 24x24 size)

static QWindowsCursor::PixmapCursor createPixmapCursorFromData(const QSize &systemCursorSize,
                  const QSize &bitmapTargetCursorSize, int bitmapSize, const uchar *bits, const uchar *maskBits)
{

   QPixmap rawImage = QPixmap::fromImage(QBitmap::fromData(QSize(bitmapSize, bitmapSize), bits).toImage());
   rawImage.setMask(QBitmap::fromData(QSize(bitmapSize, bitmapSize), maskBits));

   const qreal factor = qreal(systemCursorSize.width()) / qreal(bitmapTargetCursorSize.width());

   // Scale images if the cursor size is significantly different, starting with 150% where the
   // system cursor size is 48.

   if (qAbs(factor - 1.0) > 0.4) {
      const QTransform transform = QTransform::fromScale(factor, factor);
      rawImage = rawImage.transformed(transform, Qt::SmoothTransformation);
   }

   const QPoint hotSpot(rawImage.width() / 2, rawImage.height() / 2);

   return QWindowsCursor::PixmapCursor(rawImage, hotSpot);
}

QWindowsCursor::PixmapCursor QWindowsCursor::customCursor(Qt::CursorShape cursorShape,
   const QPlatformScreen *screen)
{
   // Non-standard Windows cursors are created from bitmaps
   static const uchar vsplit_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x80, 0x00, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x03, 0x00,
      0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
      0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x00,
      0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
      0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00,
      0x00, 0xc0, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   };

   static const uchar vsplitm_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
      0x00, 0xc0, 0x01, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0xf0, 0x07, 0x00,
      0x00, 0xf8, 0x0f, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00,
      0x00, 0xc0, 0x01, 0x00, 0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00,
      0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x00,
      0x80, 0xff, 0xff, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0xc0, 0x01, 0x00,
      0x00, 0xc0, 0x01, 0x00, 0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf0, 0x07, 0x00,
      0x00, 0xe0, 0x03, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0x80, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   };

   static const uchar hsplit_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00,
      0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00,
      0x00, 0x41, 0x82, 0x00, 0x80, 0x41, 0x82, 0x01, 0xc0, 0x7f, 0xfe, 0x03,
      0x80, 0x41, 0x82, 0x01, 0x00, 0x41, 0x82, 0x00, 0x00, 0x40, 0x02, 0x00,
      0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00,
      0x00, 0x40, 0x02, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   };

   static const uchar hsplitm_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
      0x00, 0xe0, 0x07, 0x00, 0x00, 0xe2, 0x47, 0x00, 0x00, 0xe3, 0xc7, 0x00,
      0x80, 0xe3, 0xc7, 0x01, 0xc0, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0x07,
      0xc0, 0xff, 0xff, 0x03, 0x80, 0xe3, 0xc7, 0x01, 0x00, 0xe3, 0xc7, 0x00,
      0x00, 0xe2, 0x47, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00,
      0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
   };

   static const uchar openhand_bits[] = {
      0x80, 0x01, 0x58, 0x0e, 0x64, 0x12, 0x64, 0x52, 0x48, 0xb2, 0x48, 0x92,
      0x16, 0x90, 0x19, 0x80, 0x11, 0x40, 0x02, 0x40, 0x04, 0x40, 0x04, 0x20,
      0x08, 0x20, 0x10, 0x10, 0x20, 0x10, 0x00, 0x00
   };

   static const uchar openhandm_bits[] = {
      0x80, 0x01, 0xd8, 0x0f, 0xfc, 0x1f, 0xfc, 0x5f, 0xf8, 0xff, 0xf8, 0xff,
      0xf6, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0x7f, 0xfc, 0x7f, 0xfc, 0x3f,
      0xf8, 0x3f, 0xf0, 0x1f, 0xe0, 0x1f, 0x00, 0x00
   };

   static const uchar closedhand_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x0d, 0x48, 0x32, 0x08, 0x50,
      0x10, 0x40, 0x18, 0x40, 0x04, 0x40, 0x04, 0x20, 0x08, 0x20, 0x10, 0x10,
      0x20, 0x10, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00
   };

   static const uchar closedhandm_bits[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x0d, 0xf8, 0x3f, 0xf8, 0x7f,
      0xf0, 0x7f, 0xf8, 0x7f, 0xfc, 0x7f, 0xfc, 0x3f, 0xf8, 0x3f, 0xf0, 0x1f,
      0xe0, 0x1f, 0xe0, 0x1f, 0x00, 0x00, 0x00, 0x00
   };

   static const char *const moveDragCursorXpmC[] = {
      "11 20 3 1",
      ".        c None",
      "a        c #FFFFFF",
      "X        c #000000", // X11 cursor is traditionally black
      "aa.........",
      "aXa........",
      "aXXa.......",
      "aXXXa......",
      "aXXXXa.....",
      "aXXXXXa....",
      "aXXXXXXa...",
      "aXXXXXXXa..",
      "aXXXXXXXXa.",
      "aXXXXXXXXXa",
      "aXXXXXXaaaa",
      "aXXXaXXa...",
      "aXXaaXXa...",
      "aXa..aXXa..",
      "aa...aXXa..",
      "a.....aXXa.",
      "......aXXa.",
      ".......aXXa",
      ".......aXXa",
      "........aa."
   };

   static const char *const copyDragCursorXpmC[] = {
      "24 30 3 1",
      ".        c None",
      "a        c #000000",
      "X        c #FFFFFF",
      "XX......................",
      "XaX.....................",
      "XaaX....................",
      "XaaaX...................",
      "XaaaaX..................",
      "XaaaaaX.................",
      "XaaaaaaX................",
      "XaaaaaaaX...............",
      "XaaaaaaaaX..............",
      "XaaaaaaaaaX.............",
      "XaaaaaaXXXX.............",
      "XaaaXaaX................",
      "XaaXXaaX................",
      "XaX..XaaX...............",
      "XX...XaaX...............",
      "X.....XaaX..............",
      "......XaaX..............",
      ".......XaaX.............",
      ".......XaaX.............",
      "........XX...aaaaaaaaaaa",
      ".............aXXXXXXXXXa",
      ".............aXXXXXXXXXa",
      ".............aXXXXaXXXXa",
      ".............aXXXXaXXXXa",
      ".............aXXaaaaaXXa",
      ".............aXXXXaXXXXa",
      ".............aXXXXaXXXXa",
      ".............aXXXXXXXXXa",
      ".............aXXXXXXXXXa",
      ".............aaaaaaaaaaa"
   };

   static const char *const linkDragCursorXpmC[] = {
      "24 30 3 1",
      ".        c None",
      "a        c #000000",
      "X        c #FFFFFF",
      "XX......................",
      "XaX.....................",
      "XaaX....................",
      "XaaaX...................",
      "XaaaaX..................",
      "XaaaaaX.................",
      "XaaaaaaX................",
      "XaaaaaaaX...............",
      "XaaaaaaaaX..............",
      "XaaaaaaaaaX.............",
      "XaaaaaaXXXX.............",
      "XaaaXaaX................",
      "XaaXXaaX................",
      "XaX..XaaX...............",
      "XX...XaaX...............",
      "X.....XaaX..............",
      "......XaaX..............",
      ".......XaaX.............",
      ".......XaaX.............",
      "........XX...aaaaaaaaaaa",
      ".............aXXXXXXXXXa",
      ".............aXXXaaaaXXa",
      ".............aXXXXaaaXXa",
      ".............aXXXaaaaXXa",
      ".............aXXaaaXaXXa",
      ".............aXXaaXXXXXa",
      ".............aXXaXXXXXXa",
      ".............aXXXaXXXXXa",
      ".............aXXXXXXXXXa",
      ".............aaaaaaaaaaa"
   };

   switch (cursorShape) {
      case Qt::SplitVCursor:
         return createPixmapCursorFromData(systemCursorSize(screen), standardCursorSize(), 32, vsplit_bits, vsplitm_bits);

      case Qt::SplitHCursor:
         return createPixmapCursorFromData(systemCursorSize(screen), standardCursorSize(), 32, hsplit_bits, hsplitm_bits);

      case Qt::OpenHandCursor:
         return createPixmapCursorFromData(systemCursorSize(screen), standardCursorSize(), 16, openhand_bits, openhandm_bits);

      case Qt::ClosedHandCursor:
         return createPixmapCursorFromData(systemCursorSize(screen), standardCursorSize(), 16, closedhand_bits, closedhandm_bits);

      case Qt::DragCopyCursor:
         return QWindowsCursor::PixmapCursor(QPixmap(copyDragCursorXpmC), QPoint(0, 0));

      case Qt::DragMoveCursor:
         return QWindowsCursor::PixmapCursor(QPixmap(moveDragCursorXpmC), QPoint(0, 0));

      case Qt::DragLinkCursor:
         return QWindowsCursor::PixmapCursor(QPixmap(linkDragCursorXpmC), QPoint(0, 0));
   }

   return QWindowsCursor::PixmapCursor();
}

#else //  QT_NO_IMAGEFORMAT_PNG

struct QWindowsCustomPngCursor {
   Qt::CursorShape shape;
   int size;
   const char *fileName;
   int hotSpotX;
   int hotSpotY;
};

QWindowsCursor::PixmapCursor QWindowsCursor::customCursor(Qt::CursorShape cursorShape, const QPlatformScreen *screen)
{
   static const QWindowsCustomPngCursor pngCursors[] = {
      { Qt::SplitVCursor,     32, "splitvcursor_32.png",     11, 11 },
      { Qt::SplitVCursor,     48, "splitvcursor_48.png",     16, 17 },
      { Qt::SplitVCursor,     64, "splitvcursor_64.png",     22, 22 },
      { Qt::SplitHCursor,     32, "splithcursor_32.png",     11, 11 },
      { Qt::SplitHCursor,     48, "splithcursor_48.png",     16, 17 },
      { Qt::SplitHCursor,     64, "splithcursor_64.png",     22, 22 },
      { Qt::OpenHandCursor,   32, "openhandcursor_32.png",   10, 12 },
      { Qt::OpenHandCursor,   48, "openhandcursor_48.png",   15, 16 },
      { Qt::OpenHandCursor,   64, "openhandcursor_64.png",   20, 24 },
      { Qt::ClosedHandCursor, 32, "closedhandcursor_32.png", 10, 12 },
      { Qt::ClosedHandCursor, 48, "closedhandcursor_48.png", 15, 16 },
      { Qt::ClosedHandCursor, 64, "closedhandcursor_64.png", 20, 24 },
      { Qt::DragCopyCursor,   32, "dragcopycursor_32.png",    0, 0 },
      { Qt::DragCopyCursor,   48, "dragcopycursor_48.png",    0, 0 },
      { Qt::DragCopyCursor,   64, "dragcopycursor_64.png",    0, 0 },
      { Qt::DragMoveCursor,   32, "dragmovecursor_32.png",    0, 0 },
      { Qt::DragMoveCursor,   48, "dragmovecursor_48.png",    0, 0 },
      { Qt::DragMoveCursor,   64, "dragmovecursor_64.png",    0, 0 },
      { Qt::DragLinkCursor,   32, "draglinkcursor_32.png",    0, 0 },
      { Qt::DragLinkCursor,   48, "draglinkcursor_48.png",    0, 0 },
      { Qt::DragLinkCursor,   64, "draglinkcursor_64.png",    0, 0 }
   };

   const QSize cursorSize = systemCursorSize(screen);
   const QWindowsCustomPngCursor *sEnd    = pngCursors + sizeof(pngCursors) / sizeof(pngCursors[0]);
   const QWindowsCustomPngCursor *bestFit = nullptr;

   int sizeDelta = INT_MAX;
   for (const QWindowsCustomPngCursor *s = pngCursors; s < sEnd; ++s) {
      if (s->shape != cursorShape) {
         continue;
      }

      const int currentSizeDelta = qMax(s->size, cursorSize.width()) - qMin(s->size, cursorSize.width());

      if (currentSizeDelta < sizeDelta) {
         bestFit = s;

         if (currentSizeDelta == 0) {
            break;   // Perfect match found
         }
         sizeDelta = currentSizeDelta;
      }
   }

   if (! bestFit) {
      return PixmapCursor();
   }

   const QPixmap rawImage(":/copperspice/windows/cursors/images/" + QString::fromLatin1(bestFit->fileName));

   return PixmapCursor(rawImage, QPoint(bestFit->hotSpotX, bestFit->hotSpotY));
}
#endif // QT_NO_IMAGEFORMAT_PNG

struct QWindowsStandardCursorMapping {
   Qt::CursorShape shape;
   LPCWSTR resource;
};

HCURSOR QWindowsCursor::createCursorFromShape(Qt::CursorShape cursorShape, const QPlatformScreen *screen)
{
   Q_ASSERT(cursorShape != Qt::BitmapCursor);

   static const QWindowsStandardCursorMapping standardCursors[] = {
      { Qt::ArrowCursor,        IDC_ARROW    },
      { Qt::UpArrowCursor,      IDC_UPARROW  },
      { Qt::CrossCursor,        IDC_CROSS    },
      { Qt::WaitCursor,         IDC_WAIT     },
      { Qt::IBeamCursor,        IDC_IBEAM    },
      { Qt::SizeVerCursor,      IDC_SIZENS   },
      { Qt::SizeHorCursor,      IDC_SIZEWE   },
      { Qt::SizeBDiagCursor,    IDC_SIZENESW },
      { Qt::SizeFDiagCursor,    IDC_SIZENWSE },
      { Qt::SizeAllCursor,      IDC_SIZEALL  },
      { Qt::ForbiddenCursor,    IDC_NO       },
      { Qt::WhatsThisCursor,    IDC_HELP     },
      { Qt::BusyCursor,         IDC_APPSTARTING },
      { Qt::PointingHandCursor, IDC_HAND    }
   };

   switch (cursorShape) {
      case Qt::BlankCursor: {
         // ignore color table

         QImage blank = QImage(systemCursorSize(screen), QImage::Format_Mono);
         blank.fill(0);

         return createBitmapCursor(blank, blank);
      }

      case Qt::SplitVCursor:
      case Qt::SplitHCursor:
      case Qt::OpenHandCursor:
      case Qt::ClosedHandCursor:
      case Qt::DragCopyCursor:
      case Qt::DragMoveCursor:
      case Qt::DragLinkCursor:
         return QWindowsCursor::createPixmapCursor(customCursor(cursorShape, screen));

      default:
         break;
   }

   // load available standard cursors from resources
   const QWindowsStandardCursorMapping *sEnd = standardCursors + sizeof(standardCursors) / sizeof(standardCursors[0]);

   for (const QWindowsStandardCursorMapping *s = standardCursors; s < sEnd; ++s) {
      if (s->shape == cursorShape) {
         return static_cast<HCURSOR>(LoadImage(0, s->resource, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
      }
   }

   qWarning("QWindowsCursor::createCursorFromShape(): Invalid cursor shape %d", cursorShape);

   return 0;
}

CursorHandlePtr QWindowsCursor::standardWindowCursor(Qt::CursorShape shape)
{
   StandardCursorCache::iterator iter = m_standardCursorCache.find(shape);

   if (iter == m_standardCursorCache.end()) {

      if (const HCURSOR hc = QWindowsCursor::createCursorFromShape(shape, m_screen)) {
         iter = m_standardCursorCache.insert(shape, CursorHandlePtr(new CursorHandle(hc)));
      }
   }

   return iter != m_standardCursorCache.end() ? iter.value() : CursorHandlePtr(new CursorHandle);
}

CursorHandlePtr QWindowsCursor::pixmapWindowCursor(const QCursor &c)
{
   const QWindowsPixmapCursorCacheKey cacheKey(c);
   PixmapCursorCache::iterator it = m_pixmapCursorCache.find(cacheKey);

   if (it == m_pixmapCursorCache.end()) {
      if (m_pixmapCursorCache.size() > 50) {
         // Prevent the cursor cache from growing indefinitely hitting GDI resource
         // limits if new pixmap cursors are created repetitively by purging out
         // all-noncurrent pixmap cursors (QTBUG-43515)
         const HCURSOR currentCursor = GetCursor();
         for (it = m_pixmapCursorCache.begin(); it != m_pixmapCursorCache.end() ; ) {
            if (it.value()->handle() != currentCursor) {
               it = m_pixmapCursorCache.erase(it);
            } else {
               ++it;
            }
         }
      }

      const qreal scaleFactor = QHighDpiScaling::factor(m_screen);
      const QPixmap pixmap = c.pixmap();

      const HCURSOR hc = pixmap.isNull()
         ? createBitmapCursor(c, scaleFactor)
         : QWindowsCursor::createPixmapCursor(pixmap, c.hotSpot(), scaleFactor);

      it = m_pixmapCursorCache.insert(cacheKey, CursorHandlePtr(new CursorHandle(hc)));
   }

   return it.value();
}

QWindowsCursor::QWindowsCursor(const QPlatformScreen *screen)
   : m_screen(screen)
{
   initResources();
}

void QWindowsCursor::changeCursor(QCursor *cursorIn, QWindow *window)
{
   if (! window) {
      return;
   }

   if (! cursorIn) {
      QWindowsWindow::baseWindowOf(window)->setCursor(CursorHandlePtr(new CursorHandle));
      return;
   }

   CursorHandlePtr wcursor;

   if (cursorIn->shape() == Qt::BitmapCursor)  {
      wcursor = pixmapWindowCursor(*cursorIn);

   } else {
      wcursor = standardWindowCursor(cursorIn->shape());

   }

   if (wcursor->handle()) {
      QWindowsWindow::baseWindowOf(window)->setCursor(wcursor);

   } else {
      qWarning("QWindowsCursor::changeCursor(): Unable to obtain system cursor for %d", cursorIn->shape());
   }
}

QPoint QWindowsCursor::mousePosition()
{
   POINT p;
   GetCursorPos(&p);
   return QPoint(p.x, p.y);
}

QWindowsCursor::CursorState QWindowsCursor::cursorState()
{
   enum { cursorShowing = 0x1, cursorSuppressed = 0x2 }; // Windows 8: CURSOR_SUPPRESSED
   CURSORINFO cursorInfo;
   cursorInfo.cbSize = sizeof(CURSORINFO);

   if (GetCursorInfo(&cursorInfo)) {
      if (cursorInfo.flags & CursorShowing) {
         return CursorShowing;
      }
      if (cursorInfo.flags & cursorSuppressed) {
         return CursorSuppressed;
      }
   }

   return CursorHidden;
}

QPoint QWindowsCursor::pos() const
{
   return mousePosition();
}

void QWindowsCursor::setPos(const QPoint &pos)
{
   SetCursorPos(pos.x(), pos.y());
}

QPixmap QWindowsCursor::dragDefaultCursor(Qt::DropAction action) const
{
   switch (action) {
      case Qt::CopyAction:
         if (m_copyDragCursor.isNull()) {
            m_copyDragCursor = QWindowsCursor::customCursor(Qt::DragCopyCursor, m_screen).pixmap;
         }
         return m_copyDragCursor;
      case Qt::TargetMoveAction:
      case Qt::MoveAction:
         if (m_moveDragCursor.isNull()) {
            m_moveDragCursor = QWindowsCursor::customCursor(Qt::DragMoveCursor, m_screen).pixmap;
         }
         return m_moveDragCursor;
      case Qt::LinkAction:
         if (m_linkDragCursor.isNull()) {
            m_linkDragCursor = QWindowsCursor::customCursor(Qt::DragLinkCursor, m_screen).pixmap;
         }
         return m_linkDragCursor;
      default:
         break;
   }

   static const char *const ignoreDragCursorXpmC[] = {
      "24 30 3 1",
      ".        c None",
      "a        c #000000",
      "X        c #FFFFFF",
      "aa......................",
      "aXa.....................",
      "aXXa....................",
      "aXXXa...................",
      "aXXXXa..................",
      "aXXXXXa.................",
      "aXXXXXXa................",
      "aXXXXXXXa...............",
      "aXXXXXXXXa..............",
      "aXXXXXXXXXa.............",
      "aXXXXXXaaaa.............",
      "aXXXaXXa................",
      "aXXaaXXa................",
      "aXa..aXXa...............",
      "aa...aXXa...............",
      "a.....aXXa..............",
      "......aXXa.....XXXX.....",
      ".......aXXa..XXaaaaXX...",
      ".......aXXa.XaaaaaaaaX..",
      "........aa.XaaaXXXXaaaX.",
      "...........XaaaaX..XaaX.",
      "..........XaaXaaaX..XaaX",
      "..........XaaXXaaaX.XaaX",
      "..........XaaX.XaaaXXaaX",
      "..........XaaX..XaaaXaaX",
      "...........XaaX..XaaaaX.",
      "...........XaaaXXXXaaaX.",
      "............XaaaaaaaaX..",
      ".............XXaaaaXX...",
      "...............XXXX....."
   };

   if (m_ignoreDragCursor.isNull()) {

      HCURSOR cursor = LoadCursor(NULL, IDC_NO);
      ICONINFO iconInfo = {0, 0, 0, 0, 0};
      GetIconInfo(cursor, &iconInfo);
      BITMAP bmColor = {0, 0, 0, 0, 0, 0, 0};

      if (iconInfo.hbmColor
         && GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmColor)
         && bmColor.bmWidth == bmColor.bmWidthBytes / 4) {
         const int colorBitsLength = bmColor.bmHeight * bmColor.bmWidthBytes;
         uchar *colorBits = new uchar[colorBitsLength];
         GetBitmapBits(iconInfo.hbmColor, colorBitsLength, colorBits);
         const QImage colorImage(colorBits, bmColor.bmWidth, bmColor.bmHeight,
            bmColor.bmWidthBytes, QImage::Format_ARGB32);

         m_ignoreDragCursor = QPixmap::fromImage(colorImage);
         delete [] colorBits;
      } else {
         m_ignoreDragCursor = QPixmap(ignoreDragCursorXpmC);
      }

      DeleteObject(iconInfo.hbmMask);
      DeleteObject(iconInfo.hbmColor);
      DestroyCursor(cursor);

   }
   return m_ignoreDragCursor;
}

#endif // !QT_NO_CURSOR
