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

#include <qwin_nativeimage.h>

#include <qwin_context.h>

#include <qpaintengine_p.h>
#include <qpaintengine_raster_p.h>

typedef struct {
   BITMAPINFOHEADER bmiHeader;
   DWORD redMask;
   DWORD greenMask;
   DWORD blueMask;
} BITMAPINFO_MASK;

static inline HDC createDC()
{
   HDC display_dc = GetDC(nullptr);
   HDC hdc = CreateCompatibleDC(display_dc);
   ReleaseDC(nullptr, display_dc);
   Q_ASSERT(hdc);

   return hdc;
}

static inline HBITMAP createDIB(HDC hdc, int width, int height, QImage::Format format, uchar **bitsIn)
{
   BITMAPINFO_MASK bmi;
   memset(&bmi, 0, sizeof(bmi));
   bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth       = width;
   bmi.bmiHeader.biHeight      = -height; // top-down.
   bmi.bmiHeader.biPlanes      = 1;
   bmi.bmiHeader.biSizeImage   = 0;

   if (format == QImage::Format_RGB16) {
      bmi.bmiHeader.biBitCount = 16;
      bmi.bmiHeader.biCompression = BI_BITFIELDS;
      bmi.redMask = 0xF800;
      bmi.greenMask = 0x07E0;
      bmi.blueMask = 0x001F;
   } else {
      bmi.bmiHeader.biBitCount    = 32;
      bmi.bmiHeader.biCompression = BI_RGB;
      bmi.redMask   = 0;
      bmi.greenMask = 0;
      bmi.blueMask  = 0;
   }

   uchar *bits = nullptr;
   HBITMAP bitmap = CreateDIBSection(hdc, reinterpret_cast<BITMAPINFO *>(&bmi),
         DIB_RGB_COLORS, reinterpret_cast<void **>(&bits), nullptr, 0);

   if (! bitmap || !bits) {
      qFatal("%s: CreateDIBSection failed.", __FUNCTION__);
   }

   *bitsIn = bits;

   return bitmap;
}

QWindowsNativeImage::QWindowsNativeImage(int width, int height, QImage::Format format)
   : m_hdc(createDC()), m_bitmap(nullptr), m_null_bitmap(nullptr)
{
   if (width != 0 && height != 0) {
      uchar *bits;
      m_bitmap = createDIB(m_hdc, width, height, format, &bits);
      m_null_bitmap = static_cast<HBITMAP>(SelectObject(m_hdc, m_bitmap));
      m_image = QImage(bits, width, height, format);

      Q_ASSERT(m_image.paintEngine()->type() == QPaintEngine::Raster);
      static_cast<QRasterPaintEngine *>(m_image.paintEngine())->setDC(m_hdc);

   } else {
      m_image = QImage(width, height, format);
   }

   GdiFlush();
}

QWindowsNativeImage::~QWindowsNativeImage()
{
   if (m_hdc) {
      if (m_bitmap) {
         if (m_null_bitmap) {
            SelectObject(m_hdc, m_null_bitmap);
         }
         DeleteObject(m_bitmap);
      }

      DeleteDC(m_hdc);
   }
}

QImage::Format QWindowsNativeImage::systemFormat()
{
   static const int depth = QWindowsContext::instance()->screenDepth();
   return depth == 16 ? QImage::Format_RGB16 : QImage::Format_RGB32;
}

