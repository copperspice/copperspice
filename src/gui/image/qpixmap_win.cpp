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

#include <qpixmap.h>
#include <qpixmap_raster_p.h>

#include <qbitmap.h>
#include <qimage.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qdatastream.h>
#include <qbuffer.h>
#include <qapplication.h>
#include <qevent.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdatetime.h>
#include <qpixmapcache.h>
#include <qimagereader.h>
#include <qimagewriter.h>
#include <qdebug.h>
#include <qt_windows.h>

#ifndef CAPTUREBLT
#define CAPTUREBLT ((DWORD)0x40000000)
#endif

QT_BEGIN_NAMESPACE

QPixmap QPixmap::grabWindow(WId winId, int x, int y, int w, int h )
{
   RECT r;
   GetClientRect(winId, &r);

   if (w < 0) {
      w = r.right - r.left;
   }
   if (h < 0) {
      h = r.bottom - r.top;
   }

   // Create and setup bitmap
   HDC display_dc = GetDC(0);
   HDC bitmap_dc = CreateCompatibleDC(display_dc);
   HBITMAP bitmap = CreateCompatibleBitmap(display_dc, w, h);
   HGDIOBJ null_bitmap = SelectObject(bitmap_dc, bitmap);

   // copy data
   HDC window_dc = GetDC(winId);
   BitBlt(bitmap_dc, 0, 0, w, h, window_dc, x, y, SRCCOPY
         );

   // clean up all but bitmap
   ReleaseDC(winId, window_dc);
   SelectObject(bitmap_dc, null_bitmap);
   DeleteDC(bitmap_dc);

   QPixmap pixmap = QPixmap::fromWinHBITMAP(bitmap);

   DeleteObject(bitmap);
   ReleaseDC(0, display_dc);

   return pixmap;
}

HBITMAP QPixmap::toWinHBITMAP(HBitmapFormat format) const
{
   if (isNull()) {
      return 0;
   }

   HBITMAP bitmap = 0;
   if (data->classId() == QPixmapData::RasterClass) {
      QRasterPixmapData *d = static_cast<QRasterPixmapData *>(data.data());
      int w = d->image.width();
      int h = d->image.height();

      HDC display_dc = GetDC(0);

      // Define the header
      BITMAPINFO bmi;
      memset(&bmi, 0, sizeof(bmi));
      bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
      bmi.bmiHeader.biWidth       = w;
      bmi.bmiHeader.biHeight      = -h;
      bmi.bmiHeader.biPlanes      = 1;
      bmi.bmiHeader.biBitCount    = 32;
      bmi.bmiHeader.biCompression = BI_RGB;
      bmi.bmiHeader.biSizeImage   = w * h * 4;

      // Create the pixmap
      uchar *pixels = 0;
      bitmap = CreateDIBSection(display_dc, &bmi, DIB_RGB_COLORS, (void **) &pixels, 0, 0);
      ReleaseDC(0, display_dc);
      if (!bitmap) {
         qErrnoWarning("QPixmap::toWinHBITMAP(), failed to create dibsection");
         return 0;
      }
      if (!pixels) {
         qErrnoWarning("QPixmap::toWinHBITMAP(), did not allocate pixel data");
         return 0;
      }

      // Copy over the data
      QImage::Format imageFormat = QImage::Format_ARGB32;
      if (format == NoAlpha) {
         imageFormat = QImage::Format_RGB32;
      } else if (format == PremultipliedAlpha) {
         imageFormat = QImage::Format_ARGB32_Premultiplied;
      }
      const QImage image = d->image.convertToFormat(imageFormat);
      int bytes_per_line = w * 4;
      for (int y = 0; y < h; ++y) {
         memcpy(pixels + y * bytes_per_line, image.scanLine(y), bytes_per_line);
      }

   } else {
      QPixmapData *data = new QRasterPixmapData(depth() == 1 ?
            QPixmapData::BitmapType : QPixmapData::PixmapType);
      data->fromImage(toImage(), Qt::AutoColor);
      return QPixmap(data).toWinHBITMAP(format);
   }
   return bitmap;
}

QPixmap QPixmap::fromWinHBITMAP(HBITMAP bitmap, HBitmapFormat format)
{
   // Verify size
   BITMAP bitmap_info;
   memset(&bitmap_info, 0, sizeof(BITMAP));

   int res = GetObject(bitmap, sizeof(BITMAP), &bitmap_info);
   if (!res) {
      qErrnoWarning("QPixmap::fromWinHBITMAP(), failed to get bitmap info");
      return QPixmap();
   }
   int w = bitmap_info.bmWidth;
   int h = bitmap_info.bmHeight;

   BITMAPINFO bmi;
   memset(&bmi, 0, sizeof(bmi));
   bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth       = w;
   bmi.bmiHeader.biHeight      = -h;
   bmi.bmiHeader.biPlanes      = 1;
   bmi.bmiHeader.biBitCount    = 32;
   bmi.bmiHeader.biCompression = BI_RGB;
   bmi.bmiHeader.biSizeImage   = w * h * 4;

   QImage result;
   // Get bitmap bits
   uchar *data = (uchar *) qMalloc(bmi.bmiHeader.biSizeImage);

   HDC display_dc = GetDC(0);
   if (GetDIBits(display_dc, bitmap, 0, h, data, &bmi, DIB_RGB_COLORS)) {

      QImage::Format imageFormat = QImage::Format_ARGB32_Premultiplied;
      uint mask = 0;
      if (format == NoAlpha) {
         imageFormat = QImage::Format_RGB32;
         mask = 0xff000000;
      }

      // Create image and copy data into image.
      QImage image(w, h, imageFormat);
      if (!image.isNull()) { // failed to alloc?
         int bytes_per_line = w * sizeof(QRgb);
         for (int y = 0; y < h; ++y) {
            QRgb *dest = (QRgb *) image.scanLine(y);
            const QRgb *src = (const QRgb *) (data + y * bytes_per_line);
            for (int x = 0; x < w; ++x) {
               const uint pixel = src[x];
               if ((pixel & 0xff000000) == 0 && (pixel & 0x00ffffff) != 0) {
                  dest[x] = pixel | 0xff000000;
               } else {
                  dest[x] = pixel | mask;
               }
            }
         }
      }
      result = image;
   } else {
      qWarning("QPixmap::fromWinHBITMAP(), failed to get bitmap bits");
   }
   ReleaseDC(0, display_dc);
   qFree(data);
   return fromImage(result);
}

HBITMAP qt_createIconMask(const QBitmap &bitmap)
{
   QImage bm = bitmap.toImage().convertToFormat(QImage::Format_Mono);
   int w = bm.width();
   int h = bm.height();
   int bpl = ((w + 15) / 16) * 2;                  // bpl, 16 bit alignment
   uchar *bits = new uchar[bpl * h];
   bm.invertPixels();
   for (int y = 0; y < h; y++) {
      memcpy(bits + y * bpl, bm.scanLine(y), bpl);
   }
   HBITMAP hbm = CreateBitmap(w, h, 1, 1, bits);
   delete [] bits;
   return hbm;
}

HICON QPixmap::toWinHICON() const
{
   QBitmap maskBitmap = mask();
   if (maskBitmap.isNull()) {
      maskBitmap = QBitmap(size());
      maskBitmap.fill(Qt::color1);
   }

   ICONINFO ii;
   ii.fIcon    = true;
   ii.hbmMask  = qt_createIconMask(maskBitmap);
   ii.hbmColor = toWinHBITMAP(QPixmap::Alpha);
   ii.xHotspot = 0;
   ii.yHotspot = 0;

   HICON hIcon = CreateIconIndirect(&ii);

   DeleteObject(ii.hbmColor);
   DeleteObject(ii.hbmMask);

   return hIcon;
}

#ifdef Q_OS_WIN

static QImage qt_fromWinHBITMAP(HDC hdc, HBITMAP bitmap, int w, int h)
{
   BITMAPINFO bmi;
   memset(&bmi, 0, sizeof(bmi));
   bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth       = w;
   bmi.bmiHeader.biHeight      = -h;
   bmi.bmiHeader.biPlanes      = 1;
   bmi.bmiHeader.biBitCount    = 32;
   bmi.bmiHeader.biCompression = BI_RGB;
   bmi.bmiHeader.biSizeImage   = w * h * 4;

   QImage image(w, h, QImage::Format_ARGB32_Premultiplied);
   if (image.isNull()) {
      return image;
   }

   // Get bitmap bits
   uchar *data = (uchar *) qMalloc(bmi.bmiHeader.biSizeImage);

   if (GetDIBits(hdc, bitmap, 0, h, data, &bmi, DIB_RGB_COLORS)) {
      // Create image and copy data into image.
      for (int y = 0; y < h; ++y) {
         void *dest = (void *) image.scanLine(y);
         void *src = data + y * image.bytesPerLine();
         memcpy(dest, src, image.bytesPerLine());
      }
   } else {
      qWarning("qt_fromWinHBITMAP(), failed to get bitmap bits");
   }
   qFree(data);

   return image;
}

QPixmap QPixmap::fromWinHICON(HICON icon)
{
   bool foundAlpha = false;
   HDC screenDevice = GetDC(0);
   HDC hdc = CreateCompatibleDC(screenDevice);
   ReleaseDC(0, screenDevice);

   ICONINFO iconinfo;
   bool result = GetIconInfo(icon, &iconinfo); //x and y Hotspot describes the icon center
   if (!result) {
      qWarning("QPixmap::fromWinHICON(), failed to GetIconInfo()");
   }

   int w = iconinfo.xHotspot * 2;
   int h = iconinfo.yHotspot * 2;

   BITMAPINFOHEADER bitmapInfo;
   bitmapInfo.biSize        = sizeof(BITMAPINFOHEADER);
   bitmapInfo.biWidth       = w;
   bitmapInfo.biHeight      = h;
   bitmapInfo.biPlanes      = 1;
   bitmapInfo.biBitCount    = 32;
   bitmapInfo.biCompression = BI_RGB;
   bitmapInfo.biSizeImage   = 0;
   bitmapInfo.biXPelsPerMeter = 0;
   bitmapInfo.biYPelsPerMeter = 0;
   bitmapInfo.biClrUsed       = 0;
   bitmapInfo.biClrImportant  = 0;
   DWORD *bits;

   HBITMAP winBitmap = CreateDIBSection(hdc, (BITMAPINFO *)&bitmapInfo, DIB_RGB_COLORS, (VOID **)&bits, NULL, 0);
   HGDIOBJ oldhdc = (HBITMAP)SelectObject(hdc, winBitmap);
   DrawIconEx( hdc, 0, 0, icon, iconinfo.xHotspot * 2, iconinfo.yHotspot * 2, 0, 0, DI_NORMAL);
   QImage image = qt_fromWinHBITMAP(hdc, winBitmap, w, h);

   for (int y = 0 ; y < h && !foundAlpha ; y++) {
      QRgb *scanLine = reinterpret_cast<QRgb *>(image.scanLine(y));
      for (int x = 0; x < w ; x++) {
         if (qAlpha(scanLine[x]) != 0) {
            foundAlpha = true;
            break;
         }
      }
   }
   if (!foundAlpha) {
      //If no alpha was found, we use the mask to set alpha values
      DrawIconEx( hdc, 0, 0, icon, w, h, 0, 0, DI_MASK);
      QImage mask = qt_fromWinHBITMAP(hdc, winBitmap, w, h);

      for (int y = 0 ; y < h ; y++) {
         QRgb *scanlineImage = reinterpret_cast<QRgb *>(image.scanLine(y));
         QRgb *scanlineMask = mask.isNull() ? 0 : reinterpret_cast<QRgb *>(mask.scanLine(y));
         for (int x = 0; x < w ; x++) {
            if (scanlineMask && qRed(scanlineMask[x]) != 0) {
               scanlineImage[x] = 0;   //mask out this pixel
            } else {
               scanlineImage[x] |= 0xff000000;   // set the alpha channel to 255
            }
         }
      }
   }
   //dispose resources created by iconinfo call
   DeleteObject(iconinfo.hbmMask);
   DeleteObject(iconinfo.hbmColor);

   SelectObject(hdc, oldhdc); //restore state
   DeleteObject(winBitmap);
   DeleteDC(hdc);
   return QPixmap::fromImage(image);
}

#endif

QT_END_NAMESPACE
