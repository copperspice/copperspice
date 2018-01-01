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

#ifndef QWINDOWSURFACE_RASTER_P_H
#define QWINDOWSURFACE_RASTER_P_H

#include <qglobal.h>
#include <qwindowsurface_p.h>

#ifdef Q_OS_MAC
#include <qt_cocoa_helpers_mac_p.h>
#endif

QT_BEGIN_NAMESPACE

#ifdef Q_OS_WIN
#define Q_WS_EX_LAYERED           0x00080000 // copied from WS_EX_LAYERED in winuser.h
#define Q_LWA_ALPHA               0x00000002 // copied from LWA_ALPHA in winuser.h
#define Q_ULW_ALPHA               0x00000002 // copied from ULW_ALPHA in winuser.h
#define Q_AC_SRC_ALPHA            0x00000001 // copied from AC_SRC_ALPHA in winuser.h

struct Q_UPDATELAYEREDWINDOWINFO {
   DWORD cbSize;
   HDC hdcDst;
   const POINT *pptDst;
   const SIZE *psize;
   HDC hdcSrc;
   const POINT *pptSrc;
   COLORREF crKey;
   const BLENDFUNCTION *pblend;
   DWORD dwFlags;
   const RECT *prcDirty;
};

typedef BOOL (WINAPI *PtrUpdateLayeredWindow)(HWND hwnd, HDC hdcDst, const POINT *pptDst,
      const SIZE *psize, HDC hdcSrc, const POINT *pptSrc, COLORREF crKey,
      const BLENDFUNCTION *pblend, DWORD dwflags);

typedef BOOL (WINAPI *PtrUpdateLayeredWindowIndirect)(HWND hwnd, const Q_UPDATELAYEREDWINDOWINFO *pULWInfo);
extern PtrUpdateLayeredWindow ptrUpdateLayeredWindow;
extern PtrUpdateLayeredWindowIndirect ptrUpdateLayeredWindowIndirect;
#endif

class QPaintDevice;
class QPoint;
class QRegion;
class QRegion;
class QSize;
class QWidget;
class QRasterWindowSurfacePrivate;
class QNativeImage;

class Q_GUI_EXPORT QRasterWindowSurface : public QWindowSurface
{
 public:
   QRasterWindowSurface(QWidget *widget, bool setDefaultSurface = true);
   ~QRasterWindowSurface();

   QPaintDevice *paintDevice() override;
   void flush(QWidget *widget, const QRegion &region, const QPoint &offset) override;
   void beginPaint(const QRegion &rgn) override; 
   void setGeometry(const QRect &rect) override;
   bool scroll(const QRegion &area, int dx, int dy) override;
   WindowSurfaceFeatures features() const override;

#ifdef Q_OS_MAC
   CGContextRef imageContext();

   bool needsFlush;
   QRegion regionToFlush;
#endif

 private:
#if defined(Q_WS_X11) && !defined(QT_NO_MITSHM)
   void syncX();
#endif
   void prepareBuffer(QImage::Format format, QWidget *widget);
   Q_DECLARE_PRIVATE(QRasterWindowSurface)

   QScopedPointer<QRasterWindowSurfacePrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QWINDOWSURFACE_RASTER_P_H
